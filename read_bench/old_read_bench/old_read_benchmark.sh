#!/bin/bash

# run read benchmark on both filesystems

NUM_READS=100
TESTFILE="testfile_256mb.dat"

echo "================================================"
echo "read benchmark: reading 256MB file $NUM_READS times"
echo "================================================"
echo ""

# compile benchmark program
cd ~/fuse_lab
gcc -o read_benchmark read_benchmark.c
echo "Benchmark program compiled."
echo ""

# clear caches on both client and server
echo "clearing caches..."
sync
sudo rm -rf /tmp/fuse_cache/*
sudo sh -c 'echo 3 > /proc/sys/vm/drop_caches'
ssh krisub@c220g5-111307.wisc.cloudlab.us "sync && sudo sh -c 'echo 3 > /proc/sys/vm/drop_caches'"
sleep 2
echo ""

# test FUSE filesystem
echo "=== testing FUSE Filesystem ==="
cd ~/mount_point
sleep 5
if [ ! -f "$TESTFILE" ]; then
    echo "Error: $TESTFILE not found in FUSE mount"
    exit 1
fi

~/fuse_lab/read_benchmark "$TESTFILE" $NUM_READS > ~/fuse_results.csv
echo "FUSE results saved to ~/fuse_results.csv"
echo ""

# clear caches again
echo "clearing caches..."
sync
sudo sh -c 'echo 3 > /proc/sys/vm/drop_caches'
ssh krisub@c220g5-111307.wisc.cloudlab.us "sync && sudo sh -c 'echo 3 > /proc/sys/vm/drop_caches'"
sleep 2
echo ""

# test NFS filesystem
echo "=== testing NFS Filesystem ==="
cd ~/nfs_mount
if [ ! -f "$TESTFILE" ]; then
    echo "Error: $TESTFILE not found in NFS mount"
    exit 1
fi

~/fuse_lab/read_benchmark "$TESTFILE" $NUM_READS > ~/nfs_results.csv
echo "NFS results saved to ~/nfs_results.csv"
echo ""

echo "================================================"
echo "benchmark complete"
echo "================================================"
echo ""
echo "results summary:"
echo ""
echo "FUSE:"
tail -n 1 ~/fuse_results.csv | grep -E "Total|Average" || tail -n 3 ~/fuse_results.csv
echo ""
echo "NFS:"
tail -n 1 ~/nfs_results.csv | grep -E "Total|Average" || tail -n 3 ~/nfs_results.csv
echo ""
echo "Raw data files:"
echo "  FUSE: ~/fuse_results.csv"
echo "  NFS:  ~/nfs_results.csv"