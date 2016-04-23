# vhost-user
vhost-user demo for understanding its internal

**NOTE: The code is buggy!**

## How to build
```
$ make
# This produces a binary named 'app'
```

## How to run
```
# following steps may require super privilege

# setup linux bridge
$ brctl addbr br0
$ brctl addif br0 eth0
$ ifconfig br0 up
$ ifconfig eth0 promisc

# setup tap; app rx/tx packets via tap
$ ip tuntap add tap0 mode tap
$ ifconfig tap0 up
$ brctl addif br0 tap0

# start app; Modify the script to fit your environment!
$ ./run-app.sh

# start qemu in another terminal; Modify the script to fit your environment!
$ ./run-qemu.sh

# now check the network status inside virtual machine
```
