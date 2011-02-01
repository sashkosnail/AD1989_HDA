snd-hda-intel-objs := hda_intel.o

snd-hda-codec-y := hda_codec.o
snd-hda-codec-$(CONFIG_SND_HDA_GENERIC) += hda_generic.o
snd-hda-codec-$(CONFIG_PROC_FS) += hda_proc.o
snd-hda-codec-$(CONFIG_SND_HDA_HWDEP) += hda_hwdep.o
snd-hda-codec-$(CONFIG_SND_HDA_INPUT_BEEP) += hda_beep.o

snd-hda-codec-analog-objs :=	patch_analog.o
snd-hda-codec-idt-objs :=	patch_sigmatel.o
snd-hda-codec-ad1989-objs :=	my_ad1989.o
snd-hda-codec-realtek-objs :=	patch_realtek.o

# common driver
obj-$(CONFIG_SND_HDA_INTEL) := snd-hda-codec.o

# codec drivers (note: CONFIG_SND_HDA_CODEC_XXX are booleans)
ifdef CONFIG_SND_HDA_CODEC_ANALOG
obj-$(CONFIG_SND_HDA_INTEL) += snd-hda-codec-analog.o
obj-$(CONFIG_SND_HDA_INTEL) += snd-hda-codec-ad1989.o
endif
ifdef CONFIG_SND_HDA_CODEC_SIGMATEL
obj-$(CONFIG_SND_HDA_INTEL) += snd-hda-codec-idt.o
endif
ifdef CONFIG_SND_HDA_CODEC_REALTEK
obj-$(CONFIG_SND_HDA_INTEL) += snd-hda-codec-realtek.o
endif
# this must be the last entry after codec drivers;
# otherwise the codec patches won't be hooked before the PCI probe
# when built in kernel
obj-$(CONFIG_SND_HDA_INTEL) += snd-hda-intel.o
ifdef HLC
all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules
clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
endif
