#!/bin/bash

qemu="/home/cys/bin/qemu-2.5/bin/qemu-system-x86_64"
USER_SOCK="/tmp/user.uds"
MPATH="/home/cys/huge"
NPAGE=300
MEMSZ=256

cat /proc/mounts | grep huge >/dev/null
if [ $? -ne 0 ]; then
	[ -e ${MPATH} ] || mkdir ${MPATH}
	sudo mount -t hugetlbfs none ${MPATH}
fi

# assuming 2MB page
num_huge_page=`cat /sys/devices/system/node/node0/hugepages/hugepages-2048kB/nr_hugepages`
if [ ${num_huge_page} -eq 0 ]; then
	sudo bash -c "echo ${NPAGE} > /sys/devices/system/node/node0/hugepages/hugepages-2048kB/nr_hugepages"
fi

num_huge_page=`cat /sys/devices/system/node/node0/hugepages/hugepages-2048kB/nr_hugepages`
#if [ ${num_huge_page} -ne ${NPAGE} ]; then
#	echo "cannot run: only $num_huge_page hugepages reserved"
#	exit 1
#fi

sudo ${qemu} -enable-kvm -m ${MEMSZ} \
		-object memory-backend-file,id=hugemem,size=${MEMSZ}M,mem-path=${MPATH},share=on \
		-numa node,memdev=hugemem \
		-chardev socket,id=uds0,path=${USER_SOCK} -netdev vhost-user,id=user0,chardev=uds0 -device virtio-net-pci,id=net0,netdev=user0 \
		-drive file=~/data/vm-images/qemu/tinycore.raw,format=raw \
		-cdrom ~/data/iso/tiny-core-linux/Core-current.iso
