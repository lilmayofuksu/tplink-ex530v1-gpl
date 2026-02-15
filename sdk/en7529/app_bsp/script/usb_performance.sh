#!/bin/sh


if [ -z $1 ] || [ -z $2 ]; then
	echo "Usage: sh /userfs/script/usb_performance.sh partition loop_count"
	echo "       partition is [sda/sda1/sda2/sdb/sdb1/sdb2]"
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
		echo "Usage: sh /userfs/script/usb_performance.sh partition loop_count"
		echo "       partition is [sda/sda1/sda2/sdb/sdb1/sdb2]"
		exit 0
		;;
esac

partition=$1
loop=$2
dir=/tmp/test
w_time=0
r_time=0

mkdir $dir
mount -t vfat /dev/$partition $dir
if [ ! -e $dir/test_2000M ]; then
	dd if=/dev/urandom of=$dir/test_2000M bs=4096 count=512000 > /dev/null 2>&1
fi
umount $dir

i=0
while [ $i -lt $loop ]
do
echo 3 > /proc/sys/vm/drop_caches > /dev/null 2>&1
mount -t vfat /dev/$partition $dir
r_time_s=`date +%s`
dd if=$dir/test_2000M of=/dev/null bs=4096 count=512000 > /dev/null 2>&1
r_time_e=`date +%s`

echo 3 > /proc/sys/vm/drop_caches > /dev/null 2>&1
w_time_s=`date +%s`
if [ " $3" != " " ]; then
	dd if=/dev/zero of=$dir/test_1000M bs=4096 count=256000 > /dev/null 2>&1
else
	dd if=/dev/zero of=$dir/test_200M bs=4096 count=51200 > /dev/null 2>&1
fi
umount $dir
w_time_e=`date +%s`

w_time=`expr $w_time + $w_time_e - $w_time_s`
r_time=`expr $r_time + $r_time_e - $r_time_s`

i=`expr $i + 1`
done

if [ " $3" != " " ]; then
	a=`expr 1000 \* $loop \* 100 / $w_time`
else
	a=`expr 200 \* $loop \* 100 / $w_time`
fi
b=`expr $a / 100`
c=`expr $a % 100`
echo Write Rate: $b.$c MB/s

a=`expr 2000 \* $loop \* 100 / $r_time`
b=`expr $a / 100`
c=`expr $a % 100`
echo Read Rate: $b.$c MB/s

