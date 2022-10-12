/*-
 * Copyright (c) 2011-2022 Hans Petter Selasky. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include "midipp.h"

#include "midipp_chansel.h"
#include "midipp_devsel.h"
#include "midipp_buttonmap.h"
#include "midipp_mode.h"
#include "midipp_spinbox.h"
#include "midipp_scores.h"
#include "midipp_mainwindow.h"
#include "midipp_checkbox.h"
#include "midipp_groupbox.h"
#include "midipp_volume.h"

MppMode :: MppMode(MppScoreMain *_parent, uint8_t _vi) :
    MppDialog(_parent->mainWindow, QObject::tr("View %1 mode").arg(QChar('A' + _vi)))
{
	MppDialog *d = this;
	MppModeBase *b = this;

	sm = _parent;
	view_index = _vi;

	gl = new QGridLayout(this);

	gb_iconfig = new MppGroupBox(QObject::tr("MIDI input config"));
	gb_oconfig = new MppGroupBox(QObject::tr("MIDI output config"));
	gb_contrast = new MppGroupBox(QString());
	gb_delay = new MppGroupBox(QString());

	cbx_norm = new MppCheckBox();
	cbx_norm->setChecked(true);
	b->connect(cbx_norm, SIGNAL(toggled(bool)), b, SLOT(handle_changed()));

	sli_contrast = new QSlider();
	sli_contrast->setRange(0, 255);
	sli_contrast->setOrientation(Qt::Horizontal);
	sli_contrast->setValue(128);
	b->connect(sli_contrast, SIGNAL(valueChanged(int)), b, SLOT(handle_contrast_label(int)));
	b->connect(sli_contrast, SIGNAL(valueChanged(int)), b, SLOT(handle_changed()));
	handle_contrast_label(sli_contrast->value());

	but_song_events = new MppButtonMap("Send MIDI song events\0OFF\0ON\0", 2, 2);
	b->connect(but_song_events, SIGNAL(selectionChanged(int)), b, SLOT(handle_changed()));

	but_mode = new MppKeyModeButtonMap("Input key mode");
	b->connect(but_mode, SIGNAL(selectionChanged(int)), b, SLOT(handle_changed()));

	but_reset = new QPushButton(QObject::tr("Reset"));
	b->connect(but_reset, SIGNAL(released()), b, SLOT(handle_reset()));

	but_done = new QPushButton(QObject::tr("Done"));
	d->connect(but_done, SIGNAL(released()), d, SLOT(accept()));

	spn_base = new MppSpinBox(0,0);
	spn_base->setValue(0);
	b->connect(spn_base, SIGNAL(valueChanged(int)), b, SLOT(handle_changed()));

	sli_delay = new QSlider();
	sli_delay->setRange(0, 256);
	sli_delay->setValue(0);
	sli_delay->setOrientation(Qt::Horizontal);
	b->connect(sli_delay, SIGNAL(valueChanged(int)), b, SLOT(handle_delay_label(int)));
	b->connect(sli_delay, SIGNAL(valueChanged(int)), b, SLOT(handle_changed()));
	handle_delay_label(sli_delay->value());

	spn_input_chan = new MppChanSel(sm->mainWindow, -1, MPP_CHAN_ANY_MASK | MPP_CHAN_MPE_MASK);
	b->connect(spn_input_chan, SIGNAL(valueChanged(int)), b, SLOT(handle_changed()));

	spn_pri_chan = new MppChanSel(sm->mainWindow, 0, 0);
	b->connect(spn_pri_chan, SIGNAL(valueChanged(int)), b, SLOT(handle_changed()));

	spn_pri_dev = new MppDevSel(_parent->mainWindow, -1, MPP_DEV_ALL);
	b->connect(spn_pri_dev, SIGNAL(valueChanged(int)), b, SLOT(handle_changed()));

	spn_sec_base_chan = new MppChanSel(sm->mainWindow, -1, MPP_CHAN_NONE_MASK);
	b->connect(spn_sec_base_chan, SIGNAL(valueChanged(int)), b, SLOT(handle_changed()));

	spn_sec_base_dev = new MppDevSel(_parent->mainWindow, -1, MPP_DEV_ALL);
	b->connect(spn_sec_base_dev, SIGNAL(valueChanged(int)), b, SLOT(handle_changed()));

	spn_sec_treb_chan = new MppChanSel(sm->mainWindow, -1, MPP_CHAN_NONE_MASK);
	b->connect(spn_sec_treb_chan, SIGNAL(valueChanged(int)), b, SLOT(handle_changed()));

	spn_sec_treb_dev = new MppDevSel(_parent->mainWindow, -1, MPP_DEV_ALL);
	b->connect(spn_sec_treb_dev, SIGNAL(valueChanged(int)), b, SLOT(handle_changed()));

	spn_pri_volume = new MppVolume();
	spn_pri_volume->setRange(0, MPP_VOLUME_MAX, MPP_VOLUME_UNIT);
	b->connect(spn_pri_volume, SIGNAL(valueChanged(int)), b, SLOT(handle_changed()));

	spn_sec_base_volume = new MppVolume();
	spn_sec_base_volume->setRange(0, MPP_VOLUME_MAX, MPP_VOLUME_UNIT);
	b->connect(spn_sec_base_volume, SIGNAL(valueChanged(int)), b, SLOT(handle_changed()));

	spn_sec_treb_volume = new MppVolume();
	spn_sec_treb_volume->setRange(0, MPP_VOLUME_MAX, MPP_VOLUME_UNIT);
	b->connect(spn_sec_treb_volume, SIGNAL(valueChanged(int)), b, SLOT(handle_changed()));

	spn_aux_chan = new MppChanSel(sm->mainWindow, -1, MPP_CHAN_NONE_MASK);
	b->connect(spn_aux_chan, SIGNAL(valueChanged(int)), b, SLOT(handle_changed()));

	spn_aux_base_chan = new MppChanSel(sm->mainWindow, -1, MPP_CHAN_NONE_MASK);
	b->connect(spn_aux_base_chan, SIGNAL(valueChanged(int)), b, SLOT(handle_changed()));

	spn_aux_treb_chan = new MppChanSel(sm->mainWindow, -1, MPP_CHAN_NONE_MASK);
	b->connect(spn_aux_treb_chan, SIGNAL(valueChanged(int)), b, SLOT(handle_changed()));
	
	but_note_mode = new MppButtonMap("Output note mode\0" "Normal\0" "SysEx\0", 2, 2);
	b->connect(but_note_mode, SIGNAL(selectionChanged(int)), b, SLOT(handle_changed()));

	gl->addWidget(gb_iconfig, 0, 0, 2, 2);
	gl->addWidget(gb_oconfig, 2, 2, 3, 2);

	gl->addWidget(gb_delay, 0, 2, 1, 2);
	gl->addWidget(gb_contrast, 1, 2, 1, 2);

	gl->addWidget(but_song_events, 2, 0, 1, 2);
	gl->addWidget(but_note_mode, 3, 0, 1, 2);
	gl->addWidget(but_mode, 4, 0, 1, 2);

	gl->addWidget(but_reset, 5, 0, 1, 2);
	gl->addWidget(but_done, 5, 2, 1, 2);

	gb_delay->addWidget(sli_delay, 0, 0, 1, 1);

	gb_contrast->addWidget(sli_contrast, 0, 0, 1, 2);
	gb_contrast->addWidget(new QLabel(QObject::tr("Normalize chord pressure")), 1, 0, 1, 1);
	gb_contrast->addWidget(cbx_norm, 1, 1, 1, 1);

	gb_iconfig->addWidget(new QLabel(QObject::tr("Play Key")), 0, 0, 1, 1);
	gb_iconfig->addWidget(spn_base, 0, 1, 1, 1);
	gb_iconfig->addWidget(new QLabel(QObject::tr("Channel")), 1, 0, 1, 1);
	gb_iconfig->addWidget(spn_input_chan, 1, 1, 1, 1);

	gb_oconfig->addWidget(new QLabel(QObject::tr("Main\nchannel")), 0, 1, 1, 1, Qt::AlignCenter);
	gb_oconfig->addWidget(new QLabel(QObject::tr("Auxilary\nchannel")), 0, 2, 1, 1, Qt::AlignCenter);
	gb_oconfig->addWidget(new QLabel(QObject::tr("Device")), 0, 3, 1, 1, Qt::AlignCenter);
	gb_oconfig->addWidget(new QLabel(QObject::tr("Volume")), 0, 4, 1, 1, Qt::AlignCenter);

	gb_oconfig->addWidget(new QLabel(QObject::tr("Primary")), 1, 0, 1, 1);
	gb_oconfig->addWidget(spn_pri_chan, 1, 1, 1, 1);
	gb_oconfig->addWidget(spn_aux_chan, 1, 2, 1, 1);
	gb_oconfig->addWidget(spn_pri_dev, 1, 3, 1, 1);
	gb_oconfig->addWidget(spn_pri_volume, 1, 4, 1, 1);

	gb_oconfig->addWidget(new QLabel(QObject::tr("Secondary\nbass")), 2, 0, 1, 1);
	gb_oconfig->addWidget(spn_sec_base_chan, 2, 1, 1, 1);
	gb_oconfig->addWidget(spn_aux_base_chan, 2, 2, 1, 1);
	gb_oconfig->addWidget(spn_sec_base_dev, 2, 3, 1, 1);
	gb_oconfig->addWidget(spn_sec_base_volume, 2, 4, 1, 1);

	gb_oconfig->addWidget(new QLabel(QObject::tr("Secondary\ntreble")), 3, 0, 1, 1);
	gb_oconfig->addWidget(spn_sec_treb_chan, 3, 1, 1, 1);
	gb_oconfig->addWidget(spn_aux_treb_chan, 3, 2, 1, 1);
	gb_oconfig->addWidget(spn_sec_treb_dev, 3, 3, 1, 1);
	gb_oconfig->addWidget(spn_sec_treb_volume, 3, 4, 1, 1);
}

void
MppModeBase :: sanity_check(void)
{
  	int channel;
	int channelBase;
	int channelTreb;
	int auxChannel;
	int auxChannelBase;
	int auxChannelTreb;
	int device;
	int deviceBase;
	int deviceTreb;

	uint32_t usemask[MPP_MAX_DEVS] = {};
	uint32_t usemask_all = 0;

	sm->mainWindow->atomic_lock();
	channel = sm->synthChannel;
	channelBase = sm->synthChannelBase;
	channelTreb = sm->synthChannelTreb;
	auxChannel = sm->auxChannel;
	auxChannelBase = sm->auxChannelBase;
	auxChannelTreb = sm->auxChannelTreb;
	device = sm->synthDevice;
	deviceBase = sm->synthDeviceBase;
	deviceTreb = sm->synthDeviceTreb;
	sm->mainWindow->atomic_unlock();

	/* collect channel use mask */
	
	if (device == -1) {
		for (int x = 0; x != MPP_MAX_DEVS; x++)
			usemask[x] |= 1U << channel;
		if (auxChannel > -1) {
			for (int x = 0; x != MPP_MAX_DEVS; x++)
				usemask[x] |= 1U << auxChannel;
		}
	} else {
		usemask[device] |= 1U << channel;
		if (auxChannel > -1)
			usemask[device] |= 1U << auxChannel;
	}

	if (deviceBase == -1) {
		if (channelBase > -1) {
			for (int x = 0; x != MPP_MAX_DEVS; x++)
				usemask[x] |= 1U << channelBase;
		}
		if (auxChannelBase > -1) {
			for (int x = 0; x != MPP_MAX_DEVS; x++)
				usemask[x] |= 1U << auxChannelBase;
		}
	} else {
		if (channelBase > -1)
			usemask[deviceBase] |= 1U << channelBase;
		if (auxChannelBase > -1)
			usemask[deviceBase] |= 1U << auxChannelBase;
	}

	if (deviceTreb == -1) {
		if (channelTreb > -1) {
			for (int x = 0; x != MPP_MAX_DEVS; x++)
				usemask[x] |= 1U << channelTreb;
		}
		if (auxChannelTreb > -1) {
			for (int x = 0; x != MPP_MAX_DEVS; x++)
				usemask[x] |= 1U << auxChannelTreb;
		}
	} else {
		if (channelTreb > -1)
			usemask[deviceTreb] |= 1U << channelTreb;
		if (auxChannelTreb > -1)
			usemask[deviceTreb] |= 1U << auxChannelTreb;
	}

	for (int x = 0; x != MPP_MAX_DEVS; x++)
		usemask_all |= usemask[x];

	/* disable channels that are conflicting */

	if (device == -1) {
		spn_pri_chan->setChannelMask(~usemask_all | (1U << channel));
		if (auxChannel > -1)
			spn_aux_chan->setChannelMask(~usemask_all | (1U << auxChannel));
		else
			spn_aux_chan->setChannelMask(~usemask_all);
	} else {
		spn_pri_chan->setChannelMask(~usemask[device] | (1U << channel));
		if (auxChannel > -1)
			spn_aux_chan->setChannelMask(~usemask[device] | (1U << auxChannel));
		else
			spn_aux_chan->setChannelMask(~usemask[device]);
	}

	if (deviceBase == -1) {
		if (channelBase > -1)
			spn_sec_base_chan->setChannelMask(~usemask_all | (1U << channelBase));
		else
			spn_sec_base_chan->setChannelMask(~usemask_all);
		if (auxChannelBase > -1)
			spn_aux_base_chan->setChannelMask(~usemask_all | (1U << auxChannelBase));
		else
			spn_aux_base_chan->setChannelMask(~usemask_all);
	} else {
		if (channelBase > -1)
			spn_sec_base_chan->setChannelMask(~usemask[deviceBase] | (1U << channelBase));
		else
			spn_sec_base_chan->setChannelMask(~usemask[deviceBase]);
		if (auxChannelBase > -1)
			spn_aux_base_chan->setChannelMask(~usemask[deviceBase] | (1U << auxChannelBase));
		else
			spn_aux_base_chan->setChannelMask(~usemask[deviceBase]);
	}

	if (deviceTreb == -1) {
		if (channelTreb > -1)
			spn_sec_treb_chan->setChannelMask(~usemask_all | (1U << channelTreb));
		else
			spn_sec_treb_chan->setChannelMask(~usemask_all);
		if (auxChannelTreb > -1)
			spn_aux_treb_chan->setChannelMask(~usemask_all | (1U << auxChannelTreb));
		else
			spn_aux_treb_chan->setChannelMask(~usemask_all);
	} else {
		if (channelTreb > -1)
			spn_sec_treb_chan->setChannelMask(~usemask[deviceTreb] | (1U << channelTreb));
		else
			spn_sec_treb_chan->setChannelMask(~usemask[deviceTreb]);
		if (auxChannelTreb > -1)
			spn_aux_treb_chan->setChannelMask(~usemask[deviceTreb] | (1U << auxChannelTreb));
		else
			spn_aux_treb_chan->setChannelMask(~usemask[deviceTreb]);
	}
}

void
MppModeBase :: update_all(void)
{
	int base_key;
	int key_delay;
	int channelInput;
	int channel;
	int channelBase;
	int channelTreb;
	int auxChannel;
	int auxChannelBase;
	int auxChannelTreb;
	int device;
	int deviceBase;
	int deviceTreb;
	int volume;
	int volumeBase;
	int volumeTreb;
	int key_mode;
	int note_mode;
	int chord_contrast;
	int chord_norm;
	int song_events;

	sm->mainWindow->atomic_lock();
	base_key = sm->baseKey;
	key_delay = sm->delayNoise;
	channelInput = sm->inputChannel;
	channel = sm->synthChannel;
	channelBase = sm->synthChannelBase;
	channelTreb = sm->synthChannelTreb;
	auxChannel = sm->auxChannel;
	auxChannelBase = sm->auxChannelBase;
	auxChannelTreb = sm->auxChannelTreb;
	device = sm->synthDevice;
	deviceBase = sm->synthDeviceBase;
	deviceTreb = sm->synthDeviceTreb;
	volume = sm->mainWindow->trackVolume[MPP_DEFAULT_TRACK(sm->unit)];
	volumeBase = sm->mainWindow->trackVolume[MPP_BASS_TRACK(sm->unit)];
	volumeTreb = sm->mainWindow->trackVolume[MPP_TREBLE_TRACK(sm->unit)];
	key_mode = sm->keyMode;
	chord_contrast = sm->chordContrast;
	chord_norm = sm->chordNormalize;
	song_events = sm->songEventsOn;
	note_mode = sm->noteMode;
	sm->mainWindow->atomic_unlock();

	MPP_BLOCKED(spn_base,setValue(base_key));
	MPP_BLOCKED(sli_delay,setValue(key_delay));
	handle_delay_label(key_delay);
	MPP_BLOCKED(sli_contrast,setValue(chord_contrast));
	handle_contrast_label(chord_contrast);
	MPP_BLOCKED(spn_input_chan,setValue(channelInput));
	MPP_BLOCKED(spn_pri_chan,setValue(channel));
	MPP_BLOCKED(spn_sec_base_chan,setValue(channelBase));
	MPP_BLOCKED(spn_sec_treb_chan,setValue(channelTreb));
	MPP_BLOCKED(spn_aux_chan,setValue(auxChannel));
	MPP_BLOCKED(spn_aux_base_chan,setValue(auxChannelBase));
	MPP_BLOCKED(spn_aux_treb_chan,setValue(auxChannelTreb));
	MPP_BLOCKED(spn_pri_dev,setValue(device));
	MPP_BLOCKED(spn_sec_base_dev,setValue(deviceBase));
	MPP_BLOCKED(spn_sec_treb_dev,setValue(deviceTreb));
	MPP_BLOCKED(spn_pri_volume,setValue(volume));
	MPP_BLOCKED(spn_sec_base_volume,setValue(volumeBase));
	MPP_BLOCKED(spn_sec_treb_volume,setValue(volumeTreb));
	MPP_BLOCKED(but_mode,setSelection(key_mode));
	MPP_BLOCKED(cbx_norm,setChecked(chord_norm));
	MPP_BLOCKED(but_song_events,setSelection(song_events));
	MPP_BLOCKED(but_note_mode,setSelection(note_mode));

	sanity_check();
}

void
MppModeBase :: handle_contrast_label(int v)
{
	v = (v * 100) / 255;

	if (v > 100)
		v = 100;
	else if (v < 0)
		v = 0;

	gb_contrast->setTitle(tr("FIXED and TRANSP mode chord (%1%) vs melody (%2%)").arg(100 - v).arg(v));
}

void
MppModeBase :: handle_delay_label(int v)
{
	gb_delay->setTitle(tr("FIXED and TRANSP mode random key delay (%1ms)").arg(v));
}

void
MppModeBase :: handle_reset()
{
	sli_contrast->setValue(128);
	sli_delay->setValue(25);
	spn_input_chan->setValue(MPP_CHAN_ANY);
	spn_base->setValue(MPP_DEFAULT_BASE_KEY);
	spn_pri_chan->setValue(0);
	spn_sec_base_chan->setValue(MPP_CHAN_NONE);
	spn_sec_treb_chan->setValue(MPP_CHAN_NONE);
	spn_aux_chan->setValue(-MPP_CHAN_NONE);
	spn_aux_base_chan->setValue(MPP_CHAN_NONE);
	spn_aux_treb_chan->setValue(MPP_CHAN_NONE);
	spn_pri_dev->setValue(-1);
	spn_sec_base_dev->setValue(-1);
	spn_sec_treb_dev->setValue(-1);
	spn_pri_volume->setValue(MPP_VOLUME_UNIT);
	spn_sec_base_volume->setValue(MPP_VOLUME_UNIT);
	spn_sec_treb_volume->setValue(MPP_VOLUME_UNIT);
	cbx_norm->setChecked(1);
	but_mode->setSelection(0);
	but_song_events->setSelection(0);
}

void
MppModeBase :: handle_changed()
{
	int base_key;
	int key_delay;
	int channelInput;
	int channel;
	int channelBase;
	int channelTreb;
	int auxChannel;
	int auxChannelBase;
	int auxChannelTreb;
	int device;
	int deviceBase;
	int deviceTreb;
	int volume;
	int volumeBase;
	int volumeTreb;
	int key_mode;
	int note_mode;
	int chord_contrast;
	int chord_norm;
	int song_events;

	base_key = spn_base->value();
	key_delay = sli_delay->value();
	channelInput = spn_input_chan->value();
	channel = spn_pri_chan->value();
	channelBase = spn_sec_base_chan->value();
	channelTreb = spn_sec_treb_chan->value();
	auxChannel = spn_aux_chan->value();
	auxChannelBase = spn_aux_base_chan->value();
	auxChannelTreb = spn_aux_treb_chan->value();
	device = spn_pri_dev->value();
	deviceBase = spn_sec_base_dev->value();
	deviceTreb = spn_sec_treb_dev->value();
	volume = spn_pri_volume->value();
	volumeBase = spn_sec_base_volume->value();
	volumeTreb = spn_sec_treb_volume->value();
	chord_contrast = sli_contrast->value();
	chord_norm = cbx_norm->isChecked();
	key_mode = but_mode->currSelection;
	song_events = but_song_events->currSelection;
	note_mode = but_note_mode->currSelection;

	if (key_mode < 0 || key_mode >= MM_PASS_MAX)
		key_mode = 0;

	sm->mainWindow->atomic_lock();
	sm->baseKey = base_key;
	sm->delayNoise = key_delay;
	sm->inputChannel = channelInput;
	sm->synthChannel = channel;
	sm->synthChannelBase = channelBase;
	sm->synthChannelTreb = channelTreb;
	sm->auxChannel = auxChannel;
	sm->auxChannelBase = auxChannelBase;
	sm->auxChannelTreb = auxChannelTreb;
	sm->synthDevice = device;
	sm->synthDeviceBase = deviceBase;
	sm->synthDeviceTreb = deviceTreb;
	sm->mainWindow->trackVolume[MPP_DEFAULT_TRACK(sm->unit)] = volume;
	sm->mainWindow->trackVolume[MPP_BASS_TRACK(sm->unit)] = volumeBase;
	sm->mainWindow->trackVolume[MPP_TREBLE_TRACK(sm->unit)] = volumeTreb;
	sm->keyMode = key_mode;
	sm->chordContrast = chord_contrast;
	sm->chordNormalize = chord_norm;
	sm->songEventsOn = song_events;
	sm->noteMode = note_mode;
	sm->mainWindow->atomic_unlock();

	sanity_check();
}
