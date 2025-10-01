#!/bin/bash

# changing to this random read benchmark on a larger file
TESTFILE="testfile_10mb.dat"
SERVER_USER="krisub"
SERVER_HOST="c220g5-111307.wisc.cloudlab.us"

echo "================================================"
echo "read benchmark: 10000 random 4KB reads"
echo "on a 256MB file"
echo "================================================"
echo ""

# create the test file on the server if it doesn't exist but it should exist
echo "checking for 256MB test file on server..."
ssh ${SERVER_USER}@${SERVER_HOST} "
    if [ ! -f /tmp/fuse_server/${TESTFILE} ]; then
        echo 'creating 256MB test file on server...';
        dd if=/dev/urandom of=/tmp/fuse_server/${TESTFILE} bs=1M count=256 2>/dev/null;
        cp /tmp/fuse_server/${TESTFILE} /tmp/nfs_share/${TESTFILE};
        echo 'test file created.';
    else
        echo 'test file already exists.';
    fi
"
echo ""

# compile the new!! benchmark program
cd ~/fuse_lab
gcc -o read_benchmark read_benchmark.c
echo "Benchmark program compiled."
echo ""

# clear caches on both client and server
echo "clearing caches..."
sync
sudo rm -rf /tmp/fuse_cache/*
sudo sh -c 'echo 3 > /proc/sys/vm/drop_caches'
ssh ${SERVER_USER}@${SERVER_HOST} "sync && sudo sh -c 'echo 3 > /proc/sys/vm/drop_caches'"
sleep 2
echo ""

# test FUSE filesystem
echo "=== testing FUSE Filesystem ==="
cd ~/mount_point
# if [ ! -f "$TESTFILE" ]; then
#     echo "Error: $TESTFILE not found in FUSE mount. Waiting a bit..."
#     sleep 5 # cuz it doesn't load immediately
# fi

# if [ ! -f "$TESTFILE" ]; then
#     echo "Error: $TESTFILE still not found in FUSE mount. Exiting."
#     exit 1
# fi

~/fuse_lab/read_benchmark "$TESTFILE" > ~/fuse_read_results.csv
echo "FUSE results saved to ~/fuse_read_results.csv"
echo ""

# clear caches again
echo "clearing caches..."
sync
sudo sh -c 'echo 3 > /proc/sys/vm/drop_caches'
ssh ${SERVER_USER}@${SERVER_HOST} "sync && sudo sh -c 'echo 3 > /proc/sys/vm/drop_caches'"
sleep 2
echo ""

# test NFS filesystem
echo "=== testing NFS Filesystem ==="
cd ~/nfs_mount
if [ ! -f "$TESTFILE" ]; then
    echo "Error: $TESTFILE not found in NFS mount"
    exit 1
fi

~/fuse_lab/read_benchmark "$TESTFILE" > ~/nfs_read_results.csv
echo "NFS results saved to ~/nfs_read_results.csv"
echo ""

echo "================================================"
echo "benchmark complete"
echo "================================================"
echo ""
echo "results tail:"
echo ""
echo "FUSE (reads):"
tail -n 3 ~/fuse_read_results.csv
echo ""
echo "NFS (reads):"
tail -n 3 ~/nfs_read_results.csv
echo ""
echo "raw data files:"
echo "  FUSE: ~/fuse_read_results.csv"
echo "  NFS:  ~/nfs_read_results.csv"