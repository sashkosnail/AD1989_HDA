#!/bin/bash
work_dir="/home/user/alsa_mod/test"
if [ -z $1 ]; then 
	dev_num=0
else
	dev_num=$1
fi
if [ -z $2 ]; then
	delay=10
else
	delay=$2
fi
if [ -z $3 ]; then
	play_delay=1
else
	play_delay=$3
fi

if [ -e "$work_dir/test.wav" ]; then
	rm -f $work_dir/test.wav
fi
file_size=0;
while [ $file_size -le 44 ]; do
	strace -o $work_dir/../DEBUG/strace_so_mic$dev_num.txt arecord -Dhw:0,0,$dev_num -fdat $work_dir/test.wav -d 10 &
	echo "Recording Begin..."
	sleep $play_delay
	echo "Playback Begin..."
	aplay -Dhw:0,0,0 $work_dir/test.wav
	sleep $(($delay-$play_delay));
	file_size=`stat -c%s $work_dir/test.wav`
done
