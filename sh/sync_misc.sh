#!/bin/bash
dir_list="DEBUG archive wav_sample test graphs"
for i in $dir_list; do
	if [ -d $i ]; then
		scp user@129.100.33.18:~/alsa_mod/$i/* ./$i/		
	fi
done
