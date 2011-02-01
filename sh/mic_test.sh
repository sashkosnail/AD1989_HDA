#!/bin/bash
workdir="~/alsa_mod/test"
if [ -z "$1" ]; then
	run_order="012"
else
	run_order="$1"
fi
if [ -z "$2" ]; then
	delay=0
else
	delay=$2
fi
watchdog=0
str_len=$((`expr length "$run_order"`))
filelist=""
echo str_len=$str_len
for (( i=0; i<str_len ; i++));do 
	mic_num=${run_order:i:1}
	echo "Open mic $mic_num"
	echo "-NDhw:0,0,$mic_num -fdat ~/alsa_mod/test/test$mic_num.wav &"
	filelist+="$workdir/test$mic_num.wav\n"
	strace -o ~/alsa_mod/DEBUG/strace_mic$mic_num.txt arecord -NDhw:0,0,$mic_num -fdat ~/alsa_mod/test/test$mic_num.wav &
	if [ $delay -ne 0 ];then
		sleep $(($delay/10))
	fi
done
filelist=`echo -e $filelist`
for i in $filelist; do
	stat -c%s $i
done
