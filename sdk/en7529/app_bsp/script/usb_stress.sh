#!/bin/sh


if [ -z $1 ] || [ -z $2 ] || [ -z $3 ]; then
	echo "Usage: sh /userfs/script/usb_stress.sh partition1 partition2 loop_count"
	echo "       partition1/partition2 is [sda/sda1/sda2/sdb/sdb1/sdb2]"
	exit 0
fi

case $1 in
	sda)
		;;
	sda1)
		;;
	sda2)
		;;
	sdb)
		;;
	sdb1)
		;;
	sdb2)
		;;
	*)
		echo "Usage: sh /userfs/script/usb_stress.sh partition1 partition2 loop_count"
		echo "       partition1/partition2 is [sda/sda1/sda2/sdb/sdb1/sdb2]"
		exit 0
		;;
esac

case $2 in
	sda)
		;;
	sda1)
		;;
	sda2)
		;;
	sdb)
		;;
	sdb1)
		;;
	sdb2)
		;;
	*)
		echo "Usage: sh /userfs/script/usb_stress.sh partition1 partition2 loop_count"
		echo "       partition1/partition2 is [sda/sda1/sda2/sdb/sdb1/sdb2]"
		exit 0
		;;
esac

partition1=$1
partition2=$2
loop=$3
dir1=/tmp/sda
dir2=/tmp/sdb

mkdir $dir1
mkdir $dir2

mount -t vfat -o sync /dev/$partition1 $dir1
mount -t vfat -o sync /dev/$partition2 $dir2
if [ ! -e $dir1/test_50M ]; then
	dd if=/dev/urandom of=$dir1/test_50M bs=4096 count=12800 > /dev/null 2>&1
fi
md5sum $dir1/test_50M

i=0
while [ $i -lt $loop ]
do
echo 3 > /proc/sys/vm/drop_caches > /dev/null 2>&1

dd if=$dir1/test_50M of=$dir2/test_stress bs=4096 count=12800 > /dev/null 2>&1
dd if=$dir2/test_stress of=$dir1/test_stress bs=4096 count=12800 > /dev/null 2>&1

sum1=`md5sum $dir1/test_stress | awk '{print $1}'`
sum2=`md5sum $dir2/test_stress | awk '{print $1}'`
if [ $sum1 != $sum2 ]; then
	umount $dir1
	umount $dir2
	echo Round $i: $sum1 and $sum2 is mismatch
	echo FAIL
	exit 127
else
	echo Round $i: $sum1 and $sum2 is identical
	rm $dir1/test_stress
	rm $dir2/test_stress
	i=`expr $i + 1`
fi
done

umount $dir1
umount $dir2
echo PASS
