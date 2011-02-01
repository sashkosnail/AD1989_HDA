#!/bin/bash
if [ -z $1 ];then 
	mod_dir="/home/user/alsa_mod/"
else
	mod_dir="$1"
fi
if [ -z $2 ]; then
	mod_list=`cat /proc/modules |cut -d" " -f1|grep snd_hda`
else
	mod_list="snd_hda_intel snd_hda_codec_analog snd_hda_codec_ad1989 snd_hda_codec_idt snd_hda_codec_realtek snd_hda_codec"
fi
echo -e "Removing from kernel:\n$mod_list"
rmmod -f $mod_list 2>/dev/null
echo -e "Wiping Objects:\n"
rm -vf $mod_dir/*.[kom]*
rm -vf $mod_dir/.*.[kom]*
rm -vf $mod_dir/./bin/*
