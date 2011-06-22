
#define proc pcm status
play=`cat /proc/asound/card0/pcm?p/sub0/status |sed 's/state: //g'` 
mic1=`cat /proc/asound/card0/pcm?c/sub0/status |sed 's/state: //g'`
mic2=`cat /proc/asound/card0/pcm?c/sub1/status |sed 's/state: //g'`
mic3=`cat /proc/asound/card0/pcm?c/sub2/status |grep "owner_pid"|cut -d":" -f2`
echo "$mic3"
