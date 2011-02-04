/*
 * HD audio interface patch for AD1989A,B with models for the
 * custom hardware implementation on Eurotech v.3
 *
 * Copyright (c) 2010-2011 Aleksandar Mihaylov (amihaylo@uwo.ca)
 *
 *  This driver is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This driver is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
 */

#include <linux/init.h>
#include <linux/delay.h>
#include <linux/slab.h>
#include <linux/pci.h>
#include <sound/core.h>

#include "hda_codec.h"
#include "hda_local.h"
#include "hda_beep.h"  // make sure to setup the beep amp to route out of port A

MODULE_ALIAS("snd-hda-codec-id:11d4989a");//change as conflict with patch_analog
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Aleksandar Mihaylov sashkosnail@gmail.com");
MODULE_DESCRIPTION("NCA dev. AD1989:EUROTECH");
MODULE_SUPPORTED_DEVICE("AD1989");

#define set_beep_amp(spec, nid, idx, dir) \
       ((spec)->beep_amp = HDA_COMPOSE_AMP_VAL(nid, 1, idx, dir)) /* mono */

#define AD1989_VENDORID 0x11D4989A
#define AD1989_REV3 0x00100300
#define AD1989_SUBID 0xBFD80000

#define HP_DAC_NID 0x03
#define MIC1_ADC_NID 0x09
#define MIC2_ADC_NID 0x08
#define MIC3_ADC_NID 0x0f
#define BEEP_AMP_NID 0x10
//comment to disable debug output
#define LOCAL_DEBUG
//comment to disbable beeper
//#define WITH_BEEP
/*
 * Codec spec will hold the rquired configuration
 * data until it is patch into the caller
 */
struct ad1989_spec {
	struct snd_kcontrol_new *mixers[3];
	int num_mixers;

	unsigned int beep_amp;

	const struct hda_verb *init_verbs[5];
	unsigned int num_init_verbs;
	//capture
	unsigned int num_adc;
	hda_nid_t *adc_nids;
	//const struct hda_input_mux *input_mux; //label and id in hda_input_mux_item
 	const struct hda_channel_mode *channel_mode;
	//PCM information
	struct hda_pcm pcm_rec[3]; //name, stream[2]
	struct snd_array kctls;
};
/*
 * clear controls form memory once setup or if nolonger needed
 */
static void ad1989_free_kctls(struct hda_codec *codec)
{
	struct ad1989_spec *spec = codec->spec;
	
	if(spec->kctls.list){
		struct snd_kcontrol_new *kctl = spec->kctls.list;
		int i;
		for(i=0; i< spec->kctls.used; i++)
			kfree(kctl[i].name);
	}
	snd_array_free(&spec->kctls);
}
/*
 * PCM stream callbakcs and initialization: Playback
 */
static int ad1989_hp_open(struct hda_pcm_stream *hinfo, struct hda_codec *codec,
				struct snd_pcm_substream *substream)
{
	struct snd_pcm_runtime *runtime = substream->runtime;
	runtime->hw.channels_max = hinfo->channels_max;
	runtime->hw.channels_min = hinfo->channels_min;
	runtime->hw.rates = hinfo->rates;
	runtime->hw.formats = hinfo->formats;
#ifdef LOCAL_DEBUG
	printk( KERN_INFO "OPENING playback stream\n");
#endif
	return snd_pcm_hw_constraint_step(substream->runtime, 0,
		SNDRV_PCM_HW_PARAM_CHANNELS, 2);
}

static int ad1989_hp_close(struct hda_pcm_stream *info, struct hda_codec *codec,
				struct snd_pcm_substream *substream)
{
#ifdef LOCAL_DEBUG
	printk( KERN_INFO "CLOSING playback stream\n");
#endif
	return 0;//dummy
}

static int ad1989_hp_prep(struct hda_pcm_stream *hinfo, struct hda_codec *codec,
				unsigned int stream_tag, unsigned int format,
				struct snd_pcm_substream *substream)
{
#ifdef LOCAL_DEBUG
	printk( KERN_INFO "PREP pcm playback stream_tag:%d, NID:%d format:%d\n"\
			, stream_tag, HP_DAC_NID, format);
#endif
	snd_hda_codec_setup_stream(codec, HP_DAC_NID, stream_tag, 0, format);
	return 0;
}

static int ad1989_hp_clean(struct hda_pcm_stream *hinfo, struct hda_codec *codec,
				struct snd_pcm_substream *substream)
{
#ifdef LOCAL_DEBUG
	printk( KERN_INFO "CLEANUP playback stream\n");
#endif
	snd_hda_codec_cleanup_stream(codec, HP_DAC_NID);
	return 0;
}

static struct hda_pcm_stream ad1989_pcm_playback = {
	.substreams = 1,
	.channels_min = 1,
	.channels_max = 2,
	.nid = HP_DAC_NID,
	.ops = {
		.open = ad1989_hp_open,
		.close = ad1989_hp_close,
		.prepare = ad1989_hp_prep,
		.cleanup = ad1989_hp_clean,
	}
};
/*
 * PCM stream callbacks and initialization: Capture
 */
static hda_nid_t ad1989_adc_nids[3] =      {
       MIC1_ADC_NID, MIC2_ADC_NID//, MIC3_ADC_NID
       };

static int ad1989_mic_open(struct hda_pcm_stream *hinfo, struct hda_codec *codec,
				struct snd_pcm_substream *substream)
{
	struct snd_pcm_runtime *runtime = substream->runtime;
	runtime->hw.channels_max = hinfo->channels_max;
	runtime->hw.channels_min = hinfo->channels_min;
	runtime->hw.rates = hinfo->rates;
	runtime->hw.formats = hinfo->formats;
	
#ifdef LOCAL_DEBUG
	printk( KERN_INFO "OPENING capture stream\n");
#endif
	return snd_pcm_hw_constraint_step(substream->runtime, 0,
		SNDRV_PCM_HW_PARAM_CHANNELS, 2);
}

static int ad1989_mic_close(struct hda_pcm_stream *hinfo, struct hda_codec *codec,
				struct snd_pcm_substream *substream)
{
#ifdef LOCAL_DEBUG
	printk( KERN_INFO "CLOSING capture stream\n");
#endif
	return 0;//dummy
}

static int ad1989_mic_prep(struct hda_pcm_stream *hinfo, struct hda_codec *codec,
				unsigned int stream_tag, unsigned int format,
				struct snd_pcm_substream *substream)
{
	struct ad1989_spec *spec = codec->spec;
#ifdef LOCAL_DEBUG
	printk( KERN_INFO "PREP mic pcm, stream_tag:%d, NID:%d, format:%d\n",\
			stream_tag, spec->adc_nids[substream->number],format);
#endif
	snd_hda_codec_setup_stream(codec, spec->adc_nids[substream->number],
					stream_tag, 0, format); 
	return 0;
}

static int ad1989_mic_clean(struct hda_pcm_stream *hinfo, struct hda_codec *codec,
				struct snd_pcm_substream *substream)
{
	struct ad1989_spec *spec = codec->spec;
#ifdef LOCAL_DEBUG
	printk( KERN_INFO "CLEANUP mic stream");	
#endif
	snd_hda_codec_cleanup_stream(codec, spec->adc_nids[substream->number]);
	return 0;
}

static struct hda_pcm_stream ad1989_pcm_capture = {
	.substreams = 3,
	.channels_min = 1,
	.channels_max = 6,
	.nid = 0,	//blank for now
	.ops = {
		.open = ad1989_mic_open,
		.close = ad1989_mic_close,
		.prepare = ad1989_mic_prep,
		.cleanup = ad1989_mic_clean,
	}
};
/*
 * Init verb table
 */
static struct hda_verb ad1989_init_verbs[] = {
/* Mute and disable all pin widgets*/
	{0x11, AC_VERB_SET_AMP_GAIN_MUTE,	0xF080},
	{0x11, AC_VERB_SET_PIN_WIDGET_CONTROL,	0x00},
	{0x12, AC_VERB_SET_AMP_GAIN_MUTE,	0xF080},
	{0x12, AC_VERB_SET_PIN_WIDGET_CONTROL,	0x00},
	{0x13, AC_VERB_SET_AMP_GAIN_MUTE,	0xF080},
	{0x13, AC_VERB_SET_PIN_WIDGET_CONTROL,	0x00},
	{0x14, AC_VERB_SET_AMP_GAIN_MUTE,	0xF080},
	{0x14, AC_VERB_SET_PIN_WIDGET_CONTROL,	0x00},
	{0x15, AC_VERB_SET_AMP_GAIN_MUTE,	0xF080},
	{0x15, AC_VERB_SET_PIN_WIDGET_CONTROL,	0x00},
	{0x16, AC_VERB_SET_AMP_GAIN_MUTE,	0xF080},
	{0x16, AC_VERB_SET_PIN_WIDGET_CONTROL,	0x00},
	{0x17, AC_VERB_SET_AMP_GAIN_MUTE,	0xF080},
	{0x17, AC_VERB_SET_PIN_WIDGET_CONTROL,	0x00},
	{0x24, AC_VERB_SET_AMP_GAIN_MUTE,	0xF080},
	{0x24, AC_VERB_SET_PIN_WIDGET_CONTROL,	0x00},
	{0x25, AC_VERB_SET_AMP_GAIN_MUTE,	0xF080},
	{0x25, AC_VERB_SET_PIN_WIDGET_CONTROL,	0x00},
/* Power down all ADC and DAC pairs */
	{0x03, AC_VERB_SET_POWER_STATE, AC_PWRST_D3},
	{0x04, AC_VERB_SET_POWER_STATE, AC_PWRST_D3},
	{0x05, AC_VERB_SET_POWER_STATE, AC_PWRST_D3},
	{0x06, AC_VERB_SET_POWER_STATE, AC_PWRST_D3},
	{0x0a, AC_VERB_SET_POWER_STATE, AC_PWRST_D3},
	{0x08, AC_VERB_SET_POWER_STATE, AC_PWRST_D3},
	{0x09, AC_VERB_SET_POWER_STATE, AC_PWRST_D3},
	{0x0f, AC_VERB_SET_POWER_STATE, AC_PWRST_D3},
/* mute analog mix */
	{0x20, AC_VERB_SET_AMP_GAIN_MUTE, AMP_IN_MUTE(0)},
	{0x20, AC_VERB_SET_AMP_GAIN_MUTE, AMP_IN_MUTE(1)},
	{0x20, AC_VERB_SET_AMP_GAIN_MUTE, AMP_IN_MUTE(2)},
	{0x20, AC_VERB_SET_AMP_GAIN_MUTE, AMP_IN_MUTE(3)},
	{0x20, AC_VERB_SET_AMP_GAIN_MUTE, AMP_IN_MUTE(4)},
	{0x20, AC_VERB_SET_AMP_GAIN_MUTE, AMP_IN_MUTE(5)},
	{0x20, AC_VERB_SET_AMP_GAIN_MUTE, AMP_IN_MUTE(6)},
	{0x20, AC_VERB_SET_AMP_GAIN_MUTE, AMP_IN_MUTE(7)},
/* Analog Mix mute output */
	{0x21, AC_VERB_SET_AMP_GAIN_MUTE, AMP_OUT_MUTE},
/* Mute Mic Selectors, preload 0.0dB */
	{0x0C, AC_VERB_SET_AMP_GAIN_MUTE, 	0xb0a7},
	{0x0D, AC_VERB_SET_AMP_GAIN_MUTE,	0xb0a7},
	{0x0E, AC_VERB_SET_AMP_GAIN_MUTE,	0xb0a7},
/*	
 * Port-A headphone path
 * Path: 0x03->0x37->0x22->0x11
 * Pin Complex: 0x02214110
 * Pin Control: 0xC0 HP,OUT,Hi-Z
 */
	{0x11, AC_VERB_SET_CONFIG_DEFAULT_BYTES_0,	0x10},//Association 1, seq 0
	{0x11, AC_VERB_SET_CONFIG_DEFAULT_BYTES_1,	0x41},//HP green, no jack detect
	{0x11, AC_VERB_SET_CONFIG_DEFAULT_BYTES_2,	0x21},//HP jack, 3.5mm
	{0x11, AC_VERB_SET_CONFIG_DEFAULT_BYTES_3,	0x02},//External jack on front
	{0x11, AC_VERB_SET_PIN_WIDGET_CONTROL,		0xC0},//HP, OUT enable, HI-z
	{0x22, AC_VERB_SET_AMP_GAIN_MUTE,		0x7000},//unmute idx0
	{0x22, AC_VERB_SET_AMP_GAIN_MUTE,		0x7180},//mute idx1
	{0x37, AC_VERB_SET_CONNECT_SEL, 		0x00},//select dac3
	{0x03, AC_VERB_SET_POWER_STATE,			AC_PWRST_D0},//powerup DAC0
	{0x03, AC_VERB_SET_AMP_GAIN_MUTE,		0xb027},//set gain 0.0dB
	{0x11, AC_VERB_SET_AMP_GAIN_MUTE,		AMP_OUT_UNMUTE},//complete HP path
/*
 * Port-B microphone 1 path
 * Path: 0x14->0x39->0x0c->0x08
 * Pin Complex: 0x02a19120
 * Pin Control: 0x24
 */
	{0x08, AC_VERB_SET_POWER_STATE, 		AC_PWRST_D0},//power up ADC0
	{0x0C, AC_VERB_SET_CONNECT_SEL,			0x01},//select PortB boost, NID 0x38
	{0x0C, AC_VERB_SET_AMP_GAIN_MUTE,		AMP_OUT_UNMUTE},//unmute amp
	{0x39, AC_VERB_SET_AMP_GAIN_MUTE,		AMP_OUT_ZERO},//pre-amp @0db
	{0x14, AC_VERB_SET_CONFIG_DEFAULT_BYTES_0,	0x20},//Association 2, seq 0
	{0x14, AC_VERB_SET_CONFIG_DEFAULT_BYTES_1,	0x91},//pink, no jack detect
	{0x14, AC_VERB_SET_CONFIG_DEFAULT_BYTES_2,	0xA1},//mic, 3.5mm
	{0x14, AC_VERB_SET_CONFIG_DEFAULT_BYTES_3,	0x02},//external, front
	{0x14, AC_VERB_SET_PIN_WIDGET_CONTROL, 		PIN_VREF80},//mic bias 80%5V
	{0x14, AC_VERB_SET_AMP_GAIN_MUTE,		AMP_OUT_UNMUTE},//complete Mic1 path
/*
 * Port-C microphone 2 path
 * Path: 0x15->0x3A->0x0D->0x09
 * Pin Complex: 0x02a19121
 * Pin Control: 0x24
 */
	{0x09, AC_VERB_SET_POWER_STATE, 		AC_PWRST_D0},//power up ADC1
	{0x0D, AC_VERB_SET_CONNECT_SEL,			0x02},//select PortC boost, NID 0x39
	{0x0D, AC_VERB_SET_AMP_GAIN_MUTE,		AMP_OUT_UNMUTE},//unmute amp
	{0x3A, AC_VERB_SET_AMP_GAIN_MUTE,		AMP_OUT_ZERO},//pre-amp @0db
	{0x15, AC_VERB_SET_CONFIG_DEFAULT_BYTES_0,	0x21},//Association 2, seq 1
	{0x15, AC_VERB_SET_CONFIG_DEFAULT_BYTES_1,	0x91},//pink, no jack detect
	{0x15, AC_VERB_SET_CONFIG_DEFAULT_BYTES_2,	0xA1},//mic, 3.5mm
	{0x15, AC_VERB_SET_CONFIG_DEFAULT_BYTES_3,	0x02},//external, front
	{0x15, AC_VERB_SET_PIN_WIDGET_CONTROL, 		PIN_VREF80},//mic bias 80%5V
	{0x15, AC_VERB_SET_AMP_GAIN_MUTE,		AMP_OUT_UNMUTE},//complete Mic2 path
/*
 * Port-E microphone 3 path
 * Path: 0x17->0x3C->0x0E->0x0F
 * Pin Complex: 0x02a19122
 * Pin Control: 0x24
 */
	{0x0f, AC_VERB_SET_POWER_STATE, 		AC_PWRST_D0},//power up ADC2
	{0x0E, AC_VERB_SET_CONNECT_SEL,			0x04},//select PortE boost, NID 0x3c
	{0x0E, AC_VERB_SET_AMP_GAIN_MUTE,		AMP_OUT_UNMUTE},//unmute amp
	{0x3C, AC_VERB_SET_AMP_GAIN_MUTE,		AMP_OUT_ZERO},//pre-amp @0db
	{0x17, AC_VERB_SET_CONFIG_DEFAULT_BYTES_0,	0x22},//Association 2, seq 1
	{0x17, AC_VERB_SET_CONFIG_DEFAULT_BYTES_1,	0x91},//pink, no jack detect
	{0x17, AC_VERB_SET_CONFIG_DEFAULT_BYTES_2,	0xA1},//mic, 3.5mm
	{0x17, AC_VERB_SET_CONFIG_DEFAULT_BYTES_3,	0x02},//external, front
	{0x17, AC_VERB_SET_PIN_WIDGET_CONTROL, 		PIN_VREF80},//mic bias 80%5V
	{0x17, AC_VERB_SET_AMP_GAIN_MUTE,		AMP_OUT_UNMUTE},//complete Mic3 path

	{}//end
};
/*
 * Mixer setup 
 */

static struct snd_kcontrol_new ad1989_mixers[] = {
	HDA_CODEC_VOLUME("Headphone Playback Volume", 0x03, 0, HDA_OUTPUT),
	HDA_CODEC_MUTE("Headphone Playback Switch", 0x11, 0, HDA_OUTPUT),
	HDA_CODEC_VOLUME("Microphone 1 Capture Volume", 0x0C, 0, HDA_OUTPUT),
	HDA_BIND_MUTE("Microphone 1 Capture Switch", 0x14, 0, HDA_OUTPUT),
	HDA_CODEC_VOLUME("Microphone 2 Capture Volume", 0x0D, 0, HDA_OUTPUT),
	HDA_BIND_MUTE("Microphone 2 Capture Switch", 0x15, 0, HDA_OUTPUT),
	HDA_CODEC_VOLUME("Microphone 3 Capture Volume", 0x0E, 0, HDA_OUTPUT),
	HDA_BIND_MUTE("Microphone 3 Capture Switch", 0x17, 0, HDA_OUTPUT),
	HDA_CODEC_VOLUME("Mic 1 Capture Boost", 0x39, 0 ,HDA_OUTPUT),
	HDA_CODEC_VOLUME("Mic 2 Capture Boost", 0x3A, 0 ,HDA_OUTPUT),
	HDA_CODEC_VOLUME("Mic 3 Capture Boost", 0x3C, 0 ,HDA_OUTPUT),
	{}
};
#ifdef WITH_BEEP
static struct snd_kcontrol_new ad1989_beep_mix[] = {
	HDA_CODEC_VOLUME("Beep Playback Volume", BEEP_AMP_NID, 0, HDA_OUTPUT),
	HDA_CODEC_MUTE_BEEP("Beep Playback Switch", BEEP_AMP_NID, 0, HDA_OUTPUT),
	{}
};
#endif
/*
 * Patch ops callbacks:
 */
static int ad1989_init(struct hda_codec *codec)
{
	struct ad1989_spec *spec = codec->spec;
	int i,j;
	const struct hda_verb *seq;
	for(i=0;i<spec->num_init_verbs;i++)
	{
		seq=spec->init_verbs[i];
#ifdef LOCAL_DEBUG
		printk( KERN_INFO "Sending initialization verb table #%d, first NID:%x\n", i, seq->nid);
#endif
		//snd_hda_sequence_write(codec, spec->init_verbs[i]);
		for (j=0; seq->nid; seq++)
                {
#ifdef LOCAL_DEBUG
			printk( KERN_INFO "%d:\t NID:%x\tVERB:%x\tPARAM:%x\n",j++,seq->nid,seq->verb,seq->param);
#endif
			snd_hda_codec_write(codec, seq->nid, 0, seq->verb, seq->param);
		}
	}
	return 0;
}

static void ad1989_free(struct hda_codec *codec)
{
	struct ad1989_spec *spec = codec->spec;
	if(!spec)
		return;
	snd_hda_shutup_pins(codec);
	
	ad1989_free_kctls(codec);

	kfree(spec);
#ifdef CONFIG_SND_HDA_INPUT_BEEP
#ifdef WITH_BEEP
	snd_hda_detach_beep_device(codec);
#endif
#endif
}

static int ad1989_build_pcm(struct hda_codec *codec)
{
	struct ad1989_spec *spec = codec->spec;
	struct hda_pcm *info = spec->pcm_rec;

	codec->num_pcms = 1;
	codec->pcm_info = info;

	info->name = "AD1989 EuroPCM";
	info->stream[SNDRV_PCM_STREAM_PLAYBACK] = ad1989_pcm_playback;
	info->stream[SNDRV_PCM_STREAM_CAPTURE] = ad1989_pcm_capture;
	info->stream[SNDRV_PCM_STREAM_PLAYBACK].nid = HP_DAC_NID;
	info->stream[SNDRV_PCM_STREAM_CAPTURE].nid = ad1989_adc_nids[0];
	
	return 0;
//TODO
}

static int ad1989_build_ctl(struct hda_codec *codec)
{
	struct ad1989_spec *spec = codec->spec;
	int i,err;
	
	for (i = 0; i < spec->num_mixers; i++) {
		err=snd_hda_add_new_ctls(codec, spec->mixers[i]);
		if(err < 0)
			return err;
	}
#ifdef CONFIG_SND_HDA_INPUT_BEEP
#ifdef WITH_BEEP
	if(spec->beep_amp) {
		struct snd_kcontrol_new *kbeep;
		kbeep = ad1989_beep_mix;
		for(;kbeep->name;kbeep++) {
			struct snd_kcontrol *kctl;
			kctl = snd_ctl_new1(kbeep, codec);
			if(!kctl)
				return -ENOMEM;
			kctl->private_value = spec->beep_amp;
			err=snd_hda_ctl_add(codec, 0, kctl);
				if(err < 0)
					return err;
		}
	}
#endif
#endif
//Always ensure that we have a master playback, to avoid having to set it up here
	ad1989_free_kctls(codec);
//TODO: include Capture/Input source parsing if needed
	return 0;
}
/*
 * AD1989 , patch_ops for the eurotech setup
 */
struct hda_codec_ops ad1989_patch_ops = {
	.build_controls = ad1989_build_ctl,
	.build_pcms	= ad1989_build_pcm,
	.init		= ad1989_init,
	.free		= ad1989_free
};
/*
 * Main patch routine
 * setup codec-pec with the required patch_ops and 
 * init verbs
 */
static int patch_ad1989(struct hda_codec *codec)
{
	struct ad1989_spec *spec;
	//int err;
/*
 * Initialize a new special codec config
 * and assign it to the current codec
 */
	spec = kzalloc(sizeof(*spec),GFP_KERNEL);
	if(spec==NULL)
		return -ENOMEM;
	codec->spec=spec;
/*
 * Initialize codec patch_ops
 */
	codec->patch_ops=ad1989_patch_ops;

/*
 * Print some usefull info to the kernel log
 */
#ifdef LOCAL_DEBUG
	printk( KERN_INFO "Apply patch ad1989 to codec @addr %d\n",codec->addr);
	printk( KERN_INFO "INFO SECTION\n*******************\n");
	printk( KERN_INFO "Subsystem ID:%x Vendor ID:%x\n",\
			codec->subsystem_id, codec->vendor_id);
	printk( KERN_INFO "Vendor Name: %s Chip Name: %s Model Name: %s\n",\
			codec->vendor_name, codec->chip_name, codec->modelname);
	printk( KERN_INFO "*******************\nEND INFO SECTION\n");
#endif		

/*
 * Install the pc_beep control(NID=BEEP_AMP_NID)
 */
#ifdef WITH_BEEP
	err=snd_hda_attach_beep_device(codec,BEEP_AMP_NID);
	if( err < 0) {
		ad1989_free(codec);
		return err;
	}
	set_beep_amp(spec, BEEP_AMP_NID, 0, HDA_OUTPUT);
#endif
/*
 * Apply spec options
 */
	spec->num_adc =		3;
	spec->adc_nids =	ad1989_adc_nids;
	spec->num_init_verbs = 	1;
	spec->init_verbs[0] =	ad1989_init_verbs;
	spec->num_mixers =	1;
	spec->mixers[0] =	ad1989_mixers;
//	spec->pcm_rec =		;
	spec->beep_amp =	BEEP_AMP_NID;
	//spec->kctls coming from add_control and snd_array_init	
	return 0;
}

static struct hda_codec_preset snd_hda_preset_analog[] = {
	 { .id = 0x11d4989a, .name = "AD1989A", .patch = patch_ad1989 },
	 { .id = 0x11d4989b, .name = "AD1989B", .patch = patch_ad1989 },
	 {}	//list end
	//TODO add afg members to speed up find_preset()
};

static struct hda_codec_preset_list my_preset_list = {
	 .owner = THIS_MODULE,
	 .preset = snd_hda_preset_analog,
};
/*
 * Module initialization
 */
static int __init my_ad1989_init(void)
{
	//printk(KERN_INFO "Loading ad1989 custom module, preset table should be modified");
	return snd_hda_add_codec_preset(&my_preset_list);
}
/*
 * Module Removal
 */
static void __exit my_ad1989_exit(void)
{
	//printk(KERN_INFO "Unloading my_ad1989, preset table modified");
	snd_hda_delete_codec_preset(&my_preset_list);
};
/*
 * Kernel Hooks
 */
module_init(my_ad1989_init);
module_exit(my_ad1989_exit);
