#!/bin/bash

# read first 4kb of many large files to show NFS advantage
NUM_FILES=5
FILE_SIZE_MB=512
BASE_FILENAME="largefile"

SERVER_USER="krisub"
SERVER_HOST="c220g5-111307.wisc.cloudlab.us"

echo "================================================"
echo "Multifile Benchmark"
echo "reading first 4KB of ${NUM_FILES} x ${FILE_SIZE_MB}MB files"
echo "================================================"
echo ""

FUSE_FILES=""
NFS_FILES=""
for i in $(seq 1 $NUM_FILES)
do
    FUSE_FILES+=" ${HOME}/mount_point/${BASE_FILENAME}_${i}.dat"
    NFS_FILES+=" ${HOME}/nfs_mount/${BASE_FILENAME}_${i}.dat"
done

cd ~/fuse_lab
gcc -o multifile_benchmark multifile_bench.c
echo "Benchmark program compiled."
echo ""

echo "clearing caches..."
sync
sudo rm -rf /tmp/fuse_cache/*
sudo sh -c 'echo 3 > /proc/sys/vm/drop_caches'
ssh ${SERVER_USER}@${SERVER_HOST} "sync && sudo sh -c 'echo 3 > /proc/sys/vm/drop_caches'"
sleep 2
echo ""

echo "=== testing FUSE Filesystem ==="
~/fuse_lab/multifile_benchmark ${FUSE_FILES} > ~/fuse_multifile_results.csv
echo "FUSE results saved to ~/fuse_multifile_results.csv"
echo ""

echo "clearing caches..."
sync
sudo sh -c 'echo 3 > /proc/sys/vm/drop_caches'
ssh ${SERVER_USER}@${SERVER_HOST} "sync && sudo sh -c 'echo 3 > /proc/sys/vm/drop_caches'"
sleep 2
echo ""

echo "=== testing NFS Filesystem ==="
~/fuse_lab/multifile_benchmark ${NFS_FILES} > ~/nfs_multifile_results.csv
echo "NFS results saved to ~/nfs_multifile_results.csv"
echo ""

echo "================================================"
echo "benchmark complete"
echo "================================================"
echo ""
echo "results tail:"
echo ""
echo "FUSE (multifile):"
tail -n 3 ~/fuse_multifile_results.csv
echo ""
echo "NFS (multifile):"
tail -n 3 ~/nfs_multifile_results.csv
echo ""