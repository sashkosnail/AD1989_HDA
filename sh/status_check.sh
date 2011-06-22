clear
irq_n_old=0;
pcm_list="/proc/asound/card0/pcm0p/sub0/status
/proc/asound/card0/pcm0c/sub0/status 
/proc/asound/card0/pcm0c/sub1/status 
/proc/asound/card0/pcm0c/sub2/status
"
if [ -z "$1" ]; then
	irq="hda_intel"
else
	irq=$1
fi
if [ -z "$2" ]; then
	num_itt="-1"
else
	num_itt="$1"
fi
if [ -z "$3" ]; then
	update_s=2
else
	update_s=$2
fi
#echo -e "\033[3;0fiterattions: $num_itt, update time: $update_s"
for (( i=num_itt; i != 0; i--)); do 
#Grab IRQ status 
	IRQ_status=`echo $(cat /proc/interrupts |grep $irq|cut --characters=12-45)|tr [:blank:] '|'`
	irq_nCPU0=`expr "$IRQ_status" : '\([0-9][0-9]*\)'`
	irq_nCPU1=`expr "$IRQ_status" : '.*\([0-9][0-9]*\)'`
	irq_type=`expr "$IRQ_status" : '.*\(|[A-Z]*-[A-Z]*-[a-z]*\)'`
	irq_n=$((irq_nCPU0+irq_nCPU1))
#Show IRQ data
 	irq_text="\033[2;45f"	
	if `test $irq_n -ne $irq_n_old`; then
		irq_n_old=$irq_n
		irq_text+="\e[1;32m$irq_nCPU0 \007"
	else
		irq_text+="\e[1;31m$irq_nCPU0 "
	fi
#Finish outputting and reset txt
	irq_text+="\033[2;55f$irq_nCPU1"
	irq_text+="\033[2;65f$irq_type"
	irq_text+="\033[0m\033[0;0f"

#Grab PCM status
	pcm_text+="\033[2;0f\t"
	for d_idx in $pcm_list; do
		PCM_status=`head -1 $d_idx|sed 's/state: //g'`
#Show PCM data
		pcm_text+="$PCM_status\t"
	done
#Insert header
	echo -e "\033[2J\033[0;0fPCM\tHP\tM1\tM2\tM3\033[0;45fIRQ_CPU0\033[0;55fIRQ_CPU1\t"
#Show all data
	echo -ne "$pcm_text"
	echo -ne "$irq_text"
#Update every update_s in sec
	sleep $update_s
done
  16:        915          0   IO-APIC-fasteoi   hda_intel