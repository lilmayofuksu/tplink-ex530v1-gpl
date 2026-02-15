#!/bin/bash

echo cp kernel dir!!

PWD_DIR=`pwd`
#KERNEL_DIR=$PWD_DIR/../linux-4.4.115
KERNEL_DIR=$1

#set -v
pwd

echo "$KERNEL_DIR"

#find ./ -name "*" | xargs -n 1

if [ -d $KERNEL_DIR/arch/mips/econet ]
then
	echo no need cp all!
else
	cp -rf ./* $KERNEL_DIR/
fi

function cp_file(){
	#echo "diff -s $1 ../linux-4.4.115/$1"
	if [ -f $KERNEL_DIR/$1 ]
	then
		diff -s $1 $KERNEL_DIR/$1 > /dev/null
		if [ ${?} != 0 ]; then
			echo changefile:$1!!!
			cp -f $1 $KERNEL_DIR/$1
		fi
	else
		echo cp new file $1
		cp -f $1 $KERNEL_DIR/$1 
	fi	
}

function read_dir(){
	for file in `ls $1`
	do
				
		if [ -d $1"/"$file ]
		then
			if [ -d $KERNEL_DIR/$1"/"$file ]
			then
				read_dir $1"/"$file
			else
				echo cp new fold $1"/"$file
				cp -rf $1"/"$file $KERNEL_DIR/$1"/"$file
			fi		
		else
			cp_file $1"/"$file
		fi
				
	done
}



read_dir .
