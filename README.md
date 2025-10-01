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


from node1 (mounting fuse):
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

on nfs if foo doesn't exist: Wrote -1, then -1 bytes
but on fusefs if foo doesn't exist: Wrote -1, then -1 bytes
  (but then if you try again, foo is cached and it does Wrote 100, then 100 bytes)


check ip on node1 (first ip is for private netowrk, second ip is for public network)
hostname -I

on node0 add to /etc/exports:
sudo ufw allow from (client_ip) to any port nfs (update firewall rules)
/tmp/nfs_share  *(sync,wdelay,hide,no_subtree_check,sec=sys,rw,secure,no_root_squash no_all_squash)

sudo exportfs -v
showmount -e localhost

node0
nfs_mount- nfs mountpoint
mount_point- fusefs mountpoint
sudo umount nfs_mount

to mount nfs:
sudo mount -t nfs ...111307...:/tmp/nfs_share ~/nfs_mount




system tools:

ifconfig / ip addr show
> eno1


> sudo tcpdump -i eno1 -w dhcp_capture.pcap 'port 67 or port 68'
tcpdump: listening on eno1, link-type EN10MB (Ethernet), snapshot length 262144 bytes
^C
8 packets captured
10 packets received by filter
0 packets dropped by kernel

> sudo dhclient -r eno1
> sudo dhclient eno1
RTNETLINK answers: File exists

> tcpdump -r dhcp_capture.pcap -nev
protocol listed when lits say proto UDP
mac address on DHCP-Message Discover packet listed in
  ethernet header (ethertyp IPv4, [source mac] > [destination mac])
  00:5d:73:ef:XX:XX
dhcp server ip on DHCP-Message Offer packet, source ip 128.104.222.9

```