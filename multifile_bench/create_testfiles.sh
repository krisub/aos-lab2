#!/bin/bash

# create many large files

SERVER_USER="krisub"
SERVER_HOST="c220g5-111307.wisc.cloudlab.us"
NUM_FILES=5
FILE_SIZE_MB=512
BASE_FILENAME="largefile"

echo "================================================"
echo "Creating ${NUM_FILES} test files of ${FILE_SIZE_MB}MB each on the server..."
echo "================================================"

for i in $(seq 1 $NUM_FILES)
do
    FILENAME="${BASE_FILENAME}_${i}.dat"
    echo "Creating ${FILENAME}..."
    ssh ${SERVER_USER}@${SERVER_HOST} "
        dd if=/dev/urandom of=/tmp/fuse_server/${FILENAME} bs=1M count=${FILE_SIZE_MB} 2>/dev/null;
        cp /tmp/fuse_server/${FILENAME} /tmp/nfs_share/${FILENAME};
    "
done

echo ""
echo "file creation complete."
echo "listing created files on server:"
ssh ${SERVER_USER}@${SERVER_HOST} "ls -lh /tmp/fuse_server/${BASE_FILENAME}* && ls -lh /tmp/nfs_share/${BASE_FILENAME}*"