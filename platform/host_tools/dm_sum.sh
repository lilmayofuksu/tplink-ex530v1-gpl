#!/bin/sh

if [ ! -f "str.txt" ]
then 
	echo "Please write data to str.txt. To get the data, enable _DM_OBJ_DEBUG_INFO in dm_init.c then get the data from console."
	exit -1
fi

cat str.txt | tac > out.txt
for ((i=1;i<10;i++))
do
	cat out.txt | awk '{if(substr($0,'$i',1)=="-") {sum+=$7;print $0} else if (sum!=0) {sum+=$7;print $0" "sum;sum=0} else {print $0}}' > .out.txt
	mv -f .out.txt out.txt
done
cat out.txt | tac
rm -f out.txt