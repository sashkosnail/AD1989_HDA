#!/bin/bash
c_mod=`lsmod|grep snd_hda_intel`
if [ -n c_mod ];then
	modprobe -vr snd_hda_intel
	if [ $? -eq 0 ]; then
		modprobe -v snd_hda_intel
		if [ $? -eq 0 ]; then
		#	aplay -vDhw:0,0 /home/user/alsa_mod/wav_sample/A_widex.wav
			alsamixer
		else
			echo "Cannot Load Module"
		fi
	else
		echo "Cannot remove module"
	fi
else
	echo "No HDA driver loaded"
fi
