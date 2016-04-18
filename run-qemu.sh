#!/bin/bash

cat /proc/mounts | grep huge >/dev/null
if [ $? -ne 0 ]; then
	[ -e ~/huge/ ] || mkdir ~/huge/
	sudo mount -t hugetlbfs none ~/huge/
fi

# assuming 2MB page
num_huge_page=`cat /sys/devices/system/node/node0/hugepages/hugepages-2048kB/nr_hugepages`
if [ $num_huge_page -eq 0 ]; then
	sudo bash -c 'echo 300 > /sys/devices/system/node/node0/hugepages/hugepages-2048kB/nr_hugepages'
fi

num_huge_page=`cat /sys/devices/system/node/node0/hugepages/hugepages-2048kB/nr_hugepages`
#if [ $num_huge_page -ne 300 ]; then
#	echo "cannot run: only $num_huge_page hugepages reserved"
#	exit 1
#fi

sudo /home/cys/bin/qemu-2.5/bin/qemu-system-x86_64 -enable-kvm -m 256 \
		-object memory-backend-file,id=hugemem,size=256M,mem-path=/home/cys/huge,share=on \
		-numa node,memdev=hugemem \
		-chardev socket,id=uds0,path=/tmp/user.uds -netdev vhost-user,id=user0,chardev=uds0 -device virtio-net-pci,id=net0,netdev=user0 \
		-drive file=~/data/vm-images/qemu/tinycore.raw,format=raw \
		-cdrom ~/data/iso/tiny-core-linux/Core-current.iso
