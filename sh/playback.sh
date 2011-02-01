#!/bin/bash
work_dir=/home/user/alsa_mod/wav_sample
if [ -z $1 ];then 
	filename=$work_dir/A_widex.wav
else
	filename=$1
fi
aplay -Dhw:0,0,0 $filename
