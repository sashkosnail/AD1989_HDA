#!/bin/bash
#run with root priv. to get more details`
filename=alsa_info.txt
workdir=/home/user/alsa_mod/DEBUG
workfile=$workdir/$filename
echo -e "ALSA-INFO dump:\n" > $workfile
alsa-info --stdout >> $workfile
echo -e ".asoundrc:\n" >> $workfile
cat $workdir/../../.asoundrc >> $workfile
echo -e "Detailed LSPCI:\n" >> $workfile
lspci -vvvnnxxx |grep Audio -A58 >> $workfile
echo -e "END" >> $workfile
echo "Output Stored in $workfile"
