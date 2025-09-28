```
test program:
open- getattr, open
write- write
close- release
read- read

extra:
readdir
  for ls/ls -la
create
  create new files (touch, nano)
truncate
  add/overwrite files
access
  check perms (test)


opening file- server -> cache if not cached
writing to file- write to cache copy
closing file- cached copy -> server



node0: server
  ssh ...111307...
  will host the actual files (/tmp/fuse_server/)
  run ssh daemon for file transfer
node1: client
  ssh ...111304...
  run fuse fs
  cache files (/tmp/fuse_cache/)
  transfer files via scp/sftp


from node1:
  ./fusefs ...111307... ~/mount_point -f 
    f is to see debug output

then in other terminal (still node1):
  cd ~/mount_point
  ls # should see files on  server
  ~/fuse_lab/test # run the test program from the mounted fs
  cat foo
  nano x
  mkdir mydir


prints "Wrote 100, then 100 bytes"

Network filesystem starting...
Server: ...111307...
Server path: /tmp/fuse_server
Cache path: /tmp/fuse_cache
getattr called for: /
getattr called for: /
access called for: /
getattr called for: /~
getattr called for: /foo
open called for: /foo
open called for: /foo
write called for: /foo, size: 100, offset: 0, result: 100
release called for: /foo
Pushing file: /tmp/fuse_cache/foo -> ...111307...:/tmp/fuse_server/foo
read called for: /foo, size: 4096, offset: 0, result: 100
release called for: /foo
Pushing file: /tmp/fuse_cache/foo -> ...111307...:/tmp/fuse_server/foo
```