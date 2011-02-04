#!/bin/bash
work_dir="/home/user/alsa_mod/"
amix_run=`ps -e|grep alsamixer|cut -d" " -f1`
echo $amix
if [ ! -z $amix_run ]; then
	kill -s SIGQUIT $amix_run
fi
$work_dir/sh/clean.sh
make HLC=yes
if [ $? -eq 0 ]; then	
	mv $work_dir/*.ko $work_dir/bin/
	chmod 744 $work_dir/bin/*.ko
	cp -f $work_dir/bin/* /lib/modules/`uname -r`/kernel/sound/pci/hda/
	depmod -a
	modprobe -v snd_hda_intel
	$work_dir/sh/xrun.sh 3
fi
