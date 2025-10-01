#!/bin/bash

SERVER="krisub@c220g5-111307.wisc.cloudlab.us"

echo "creating 256MB test file on server..."

# create 256MB file with random data
ssh $SERVER "dd if=/dev/urandom of=/tmp/fuse_server/testfile_256mb.dat bs=1M count=256 2>/dev/null"
ssh $SERVER "dd if=/dev/urandom of=/tmp/nfs_share/testfile_256mb.dat bs=1M count=256 2>/dev/null"

echo "verifying file sizes..."
ssh $SERVER "ls -lh /tmp/fuse_server/testfile_256mb.dat"
ssh $SERVER "ls -lh /tmp/nfs_share/testfile_256mb.dat"

echo "test files created successfully"