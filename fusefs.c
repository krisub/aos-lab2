#define FUSE_USE_VERSION 26
#include <fuse.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <sys/types.h>
#include <dirent.h>

static char *server_host = NULL;
static char *server_path = "/tmp/fuse_server";
static char *cache_path = "/tmp/fuse_cache";

char *build_remote_path(const char *path) {
    char *remote_path = malloc(512);
    snprintf(remote_path, 512, "%s:%s%s", server_host, server_path, path);
    return remote_path;
}

char *build_cache_path(const char *path) {
    char *local_path = malloc(512);
    snprintf(local_path, 512, "%s%s", cache_path, path);
    return local_path;
}

void ensure_cache_dir(const char *path) {
    char *local_path = build_cache_path(path);
    char *dir = strdup(local_path);
    char *last_slash = strrchr(dir, '/');
    
    if (last_slash && last_slash != dir) {
        *last_slash = '\0';
        char cmd[1024];
        snprintf(cmd, sizeof(cmd), "mkdir -p %s", dir);
        system(cmd);
    }
    
    free(dir);
    free(local_path);
}

int fetch_file(const char *path) {
    char *remote_path = build_remote_path(path);
    char *local_path = build_cache_path(path);
    char cmd[1024];
    
    ensure_cache_dir(path);
    
    printf("Fetching file: %s -> %s\n", remote_path, local_path);
    snprintf(cmd, sizeof(cmd), "scp -q %s %s", remote_path, local_path);
    int result = system(cmd);
    
    free(remote_path);
    free(local_path);
    return result;
}

int push_file(const char *path) {
    char *remote_path = build_remote_path(path);
    char *local_path = build_cache_path(path);
    char cmd[1024];
    
    printf("Pushing file: %s -> %s\n", local_path, remote_path);
    snprintf(cmd, sizeof(cmd), "scp -q %s %s", local_path, remote_path);
    int result = system(cmd);
    
    free(remote_path);
    free(local_path);
    return result;
}

int file_exists_on_server(const char *path) {
    char cmd[1024];
    snprintf(cmd, sizeof(cmd), "ssh -q %s 'test -e %s%s'", 
             server_host, server_path, path);
    return system(cmd) == 0;
}

static int fusefs_getattr(const char *path, struct stat *stbuf) {
    char *local_path = build_cache_path(path);
    int res = 0;
    
    printf("getattr called for: %s\n", path);
    
    // try local cache first
    if (lstat(local_path, stbuf) == 0) {
        free(local_path);
        return 0;
    }
    
    // not in cache, check if exists on server and fetch
    if (file_exists_on_server(path)) {
        if (fetch_file(path) == 0) {
            res = lstat(local_path, stbuf);
        } else {
            res = -ENOENT;
        }
    } else {
        res = -ENOENT;
    }
    
    free(local_path);
    return res;
}

static int fusefs_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
                         off_t offset, struct fuse_file_info *fi) {
    char cmd[1024];
    FILE *fp;
    char filename[256];
    
    printf("readdir called for: %s\n", path);
    
    filler(buf, ".", NULL, 0);
    filler(buf, "..", NULL, 0);
    
    // ssh to list directory on server
    snprintf(cmd, sizeof(cmd), "ssh -q %s 'ls %s%s 2>/dev/null'", 
             server_host, server_path, path);
    
    fp = popen(cmd, "r");
    if (fp == NULL) {
        return -ENOENT;
    }
    
    while (fgets(filename, sizeof(filename), fp) != NULL) {
        // rm newline
        filename[strcspn(filename, "\n")] = 0;
        if (strlen(filename) > 0) {
            filler(buf, filename, NULL, 0);
        }
    }
    
    pclose(fp);
    return 0;
}

static int fusefs_open(const char *path, struct fuse_file_info *fi) {
    char *local_path = build_cache_path(path);
    int fd;
    
    printf("open called for: %s\n", path);

    struct stat st;
    if (lstat(local_path, &st) != 0) {
        if (fetch_file(path) != 0) {
            free(local_path);
            return -ENOENT;
        }
    }
    
    fd = open(local_path, fi->flags);
    if (fd == -1) {
        free(local_path);
        return -errno;
    }
    
    fi->fh = fd;
    free(local_path);
    return 0;
}

static int fusefs_read(const char *path, char *buf, size_t size, off_t offset,
                      struct fuse_file_info *fi) {
    int res = pread(fi->fh, buf, size, offset);
    printf("read called for: %s, size: %zu, offset: %ld, result: %d\n", 
           path, size, offset, res);
    if (res == -1)
        res = -errno;
    return res;
}

static int fusefs_write(const char *path, const char *buf, size_t size,
                       off_t offset, struct fuse_file_info *fi) {
    int res = pwrite(fi->fh, buf, size, offset);
    printf("write called for: %s, size: %zu, offset: %ld, result: %d\n", 
           path, size, offset, res);
    if (res == -1)
        res = -errno;
    return res;
}

static int fusefs_release(const char *path, struct fuse_file_info *fi) {
    printf("release called for: %s\n", path);
    
    close(fi->fh);
    
    push_file(path);
    
    return 0;
}

static int fusefs_create(const char *path, mode_t mode,
                        struct fuse_file_info *fi) {
    char *local_path = build_cache_path(path);
    int fd;
    
    printf("create called for: %s\n", path);
    
    ensure_cache_dir(path);
    
    fd = creat(local_path, mode);
    if (fd == -1) {
        free(local_path);
        return -errno;
    }
    
    fi->fh = fd;
    free(local_path);
    return 0;
}

static int fusefs_truncate(const char *path, off_t size) {
    char *local_path = build_cache_path(path);
    int res;
    
    printf("truncate called for: %s, size: %ld\n", path, size);

    struct stat st;
    if (lstat(local_path, &st) != 0) {
        fetch_file(path);
    }
    
    res = truncate(local_path, size);
    if (res == -1) {
        free(local_path);
        return -errno;
    }
    
    free(local_path);
    return 0;
}

static int fusefs_access(const char *path, int mask) {
    printf("access called for: %s\n", path);
    
    if (file_exists_on_server(path)) {
        return 0;
    }
    
    return -ENOENT;
}

static struct fuse_operations fusefs_oper = {
    .getattr    = fusefs_getattr,
    .readdir    = fusefs_readdir,
    .open       = fusefs_open,
    .read       = fusefs_read,
    .write      = fusefs_write,
    .release    = fusefs_release,
    .create     = fusefs_create,
    .truncate   = fusefs_truncate,
    .access     = fusefs_access,
};

int main(int argc, char *argv[]) {
    if (argc < 3) {
        fprintf(stderr, "usage: %s <server_host> <mount_point> [fuse_options]\n", argv[0]);
        return 1;
    }
    
    server_host = strdup(argv[1]);
    
    system("mkdir -p /tmp/fuse_cache"); // already made but just in case
    
    printf("Network filesystem starting...\n");
    printf("Server: %s\n", server_host);
    printf("Server path: %s\n", server_path);
    printf("Cache path: %s\n", cache_path);
    
    // server host does not count as a fuse argument
    for (int i = 1; i < argc - 1; i++) {
        argv[i] = argv[i + 1];
    }
    argc--;
    
    return fuse_main(argc, argv, &fusefs_oper, NULL);
}