#!/bin/bash
ind=`ls codec#1* 2>/dev/null |tail -1|cut -d_ -f2|cut -d. -f1`
loc_file=./codec#1_"$((ind+1))".svg
scp user@129.100.33.18:~/alsa_mod/utils/codecgraph/codec#1.svg $loc_file 2>/dev/null
if [ -e $loc_file ]; then
	eog $loc_file
else
	echo "No file"
fi
