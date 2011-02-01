#!/bin/bash
if [ -z $1 ]; then
	flags=27
else
	flags=$1
fi
echo $flags >> /proc/asound/card0/pcm0p/xrun_debug
echo $flags >> /proc/asound/card0/pcm0c/xrun_debug
