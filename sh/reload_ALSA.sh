#!/bin/bash
#c_mod=`lsmod|grep "^snd_hda"|sed -e "s/\(^[a-z_0-9]*\).*/\1/"`
#c_mod=`sed -e "s/\(^[a-z_0-9]*\)
c_mod="snd_hda_intel snd_hda_codec_ad1989 snd_hda_codec"
if [ -n c_mod ];then
	lsmod|grep snd
	modprobe -vr $c_mod
	if [ $? -eq 0 ]; then
		lsmod |grep snd
		modprobe -v snd_hda_intel
		if [ $? -eq 0 ]; then
		#	aplay -vDhw:0,0 /home/user/alsa_mod/wav_sample/A_widex.wav
		alsamixer
		echo done
		else
			echo "Cannot Load Module"
		fi
	else
		echo "Cannot remove module"
	fi
else
	echo "No HDA driver loaded"
fi
