#!/bin/bash
start=0
end=10
while [ $start -le $end ]; do
	echo "sleep 90s!"
	sleep 90
	curMemFree=$(cat /proc/meminfo | grep "MemFree"| awk '{print $2}')
	threshold=26624
	#never end
	#i=$((i+1))
	if [ $curMemFree -le $threshold ]; then
		echo "Free Mem Less then 26624 drop_caches 3!"
		echo 3 > /proc/sys/vm/drop_caches
	fi
done