/*-
 * Copyright (c) 2011,2013 Hans Petter Selasky. All rights reserved.
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
#include "midipp_buttonmap.h"
#include "midipp_mode.h"
#include "midipp_spinbox.h"
#include "midipp_scores.h"
#include "midipp_mainwindow.h"
#include "midipp_checkbox.h"
#include "midipp_groupbox.h"

MppMode :: MppMode(MppScoreMain *_parent, uint8_t _vi)
{
	sm = _parent;
	view_index = _vi;

	gl = new QGridLayout(this);

	gb_iconfig = new MppGroupBox(tr("MIDI input config"));
	gb_oconfig = new MppGroupBox(tr("MIDI output config"));
	gb_contrast = new MppGroupBox(QString());
	gb_delay = new MppGroupBox(QString());

	setWindowTitle(tr("View %1 mode").arg(QChar('A' + _vi)));
	setWindowIcon(QIcon(QString(MPP_ICON_FILE)));

	cbx_norm = new MppCheckBox();
	cbx_norm->setChecked(1);
	connect(cbx_norm, SIGNAL(stateChanged(int,int)), this, SLOT(handle_changed()));

	sli_contrast = new QSlider();
	sli_contrast->setRange(0, 255);
	sli_contrast->setOrientation(Qt::Horizontal);
	sli_contrast->setValue(128);
	connect(sli_contrast, SIGNAL(valueChanged(int)), this, SLOT(handle_contrast_label(int)));
	connect(sli_contrast, SIGNAL(valueChanged(int)), this, SLOT(handle_changed()));
	handle_contrast_label(sli_contrast->value());

	but_song_events = new MppButtonMap("Send MIDI song events\0OFF\0ON\0", 2, 2);

	but_mode = new MppButtonMap("Key mode\0" "ALL\0" "MIXED\0" "FIXED\0" "TRANSP\0" "CHORD-PIANO\0" "CHORD-GUITAR\0", 6, 3);
	connect(but_mode, SIGNAL(selectionChanged(int)), this, SLOT(handle_changed()));

	but_reset = new QPushButton(tr("Reset"));
	connect(but_reset, SIGNAL(released()), this, SLOT(handle_reset()));

	but_done = new QPushButton(tr("Close"));
	connect(but_done, SIGNAL(released()), this, SLOT(accept()));

	spn_base = new MppSpinBox();
	spn_base->setRange(0, 127);
	spn_base->setValue(0);
	connect(spn_base, SIGNAL(valueChanged(int)), this, SLOT(handle_changed()));

	sli_delay = new QSlider();
	sli_delay->setRange(0, 256);
	sli_delay->setValue(0);
	sli_delay->setOrientation(Qt::Horizontal);
	connect(sli_delay, SIGNAL(valueChanged(int)), this, SLOT(handle_delay_label(int)));
	connect(sli_delay, SIGNAL(valueChanged(int)), this, SLOT(handle_changed()));
	handle_delay_label(sli_delay->value());

	spn_input_chan = new MppChanSel(-1, MPP_CHAN_ANY);
	connect(spn_input_chan, SIGNAL(valueChanged(int)), this, SLOT(handle_changed()));

	spn_pri_chan = new MppChanSel(0, 0);
	connect(spn_pri_chan, SIGNAL(valueChanged(int)), this, SLOT(handle_changed()));

	spn_sec_base_chan = new MppChanSel(-1, MPP_CHAN_NONE);
	connect(spn_sec_base_chan, SIGNAL(valueChanged(int)), this, SLOT(handle_changed()));

	spn_sec_treb_chan = new MppChanSel(-1, MPP_CHAN_NONE);
	connect(spn_sec_treb_chan, SIGNAL(valueChanged(int)), this, SLOT(handle_changed()));

	gl->addWidget(gb_iconfig, 0, 0, 1, 2);
	gl->addWidget(gb_oconfig, 1, 0, 1, 2);
	gl->addWidget(gb_delay, 2, 0, 1, 2);
	gl->addWidget(gb_contrast, 3, 0, 1, 2);
	gl->addWidget(but_song_events, 4, 0, 1, 2);
	gl->addWidget(but_mode, 5, 0, 1, 2);
	gl->addWidget(but_reset, 7, 0, 1, 1);
	gl->addWidget(but_done, 7, 1, 1, 1);
	gl->setRowStretch(6, 1);
	gl->setColumnStretch(2, 1);

	gb_delay->addWidget(sli_delay, 0, 0, 1, 1);

	gb_contrast->addWidget(sli_contrast, 0, 0, 1, 2);
	gb_contrast->addWidget(new QLabel(tr("Normalize chord pressure")), 1, 0, 1, 1);
	gb_contrast->addWidget(cbx_norm, 1, 1, 1, 1);

	gb_iconfig->addWidget(new QLabel(tr("Play Key")), 0, 0, 1, 1);
	gb_iconfig->addWidget(spn_base, 0, 1, 1, 1);
	gb_iconfig->addWidget(new QLabel(tr("Channel")), 1, 0, 1, 1);
	gb_iconfig->addWidget(spn_input_chan, 1, 1, 1, 1);

	gb_oconfig->addWidget(new QLabel(tr("Primary channel")), 0, 0, 1, 1);
	gb_oconfig->addWidget(spn_pri_chan, 0, 1, 1, 1);
	gb_oconfig->addWidget(new QLabel(tr("Secondary base channel")), 1, 0, 1, 1);
	gb_oconfig->addWidget(spn_sec_base_chan, 1, 1, 1, 1);
	gb_oconfig->addWidget(new QLabel(tr("Secondary treble channel")), 2, 0, 1, 1);
	gb_oconfig->addWidget(spn_sec_treb_chan, 2, 1, 1, 1);
}

MppMode :: ~MppMode()
{

}

void
MppMode :: update_all(void)
{
	int base_key;
	int key_delay;
	int channelInput;
	int channel;
	int channelBase;
	int channelTreb;
	int key_mode;
	int chord_contrast;
	int chord_norm;
	int song_events;

	pthread_mutex_lock(&sm->mainWindow->mtx);
	base_key = sm->baseKey;
	key_delay = sm->delayNoise;
	channelInput = sm->inputChannel;
	channel = sm->synthChannel;
	channelBase = sm->synthChannelBase;
	channelTreb = sm->synthChannelTreb;
	key_mode = sm->keyMode;
	chord_contrast = sm->chordContrast;
	chord_norm = sm->chordNormalize;
	song_events = sm->songEventsOn;
	pthread_mutex_unlock(&sm->mainWindow->mtx);

	spn_base->setValue(base_key);
	sli_delay->setValue(key_delay);
	sli_contrast->setValue(chord_contrast);
	spn_input_chan->setValue(channelInput);
	spn_pri_chan->setValue(channel);
	spn_sec_base_chan->setValue(channelBase);
	spn_sec_treb_chan->setValue(channelTreb);
	but_mode->setSelection(key_mode);
	cbx_norm->setChecked(chord_norm);
	but_song_events->setSelection(song_events);
}

void
MppMode :: handle_contrast_label(int v)
{
	char buf[32];
	v = ((v - 128) * 100) / 127;

	if (v > 100)
		v = 100;
	else if (v < -100)
		v = -100;

	snprintf(buf, sizeof(buf),
	    "Chord(-) vs Melody(+) (%d%%)", v);
	gb_contrast->setTitle(tr(buf));
}

void
MppMode :: handle_delay_label(int v)
{
	char buf[32];
	snprintf(buf, sizeof(buf), "Random T/F/M key delay (%dms)", v);
	gb_delay->setTitle(tr(buf));
}

void
MppMode :: handle_reset()
{
	sli_contrast->setValue(128);
	sli_delay->setValue(25);
	spn_input_chan->setValue(-1);
	spn_base->setValue(MPP_DEFAULT_BASE_KEY);
	spn_pri_chan->setValue(0);
	spn_sec_base_chan->setValue(-1);
	spn_sec_treb_chan->setValue(-1);
	cbx_norm->setChecked(1);
	but_mode->setSelection(0);
	but_song_events->setSelection(0);
}

void
MppMode :: handle_changed()
{
	int base_key;
	int key_delay;
	int channelInput;
	int channel;
	int channelBase;
	int channelTreb;
	int key_mode;
	int chord_contrast;
	int chord_norm;
	int song_events;

	base_key = spn_base->value();
	key_delay = sli_delay->value();
	channelInput = spn_input_chan->value();
	channel = spn_pri_chan->value();
	channelBase = spn_sec_base_chan->value();
	channelTreb = spn_sec_treb_chan->value();
	chord_contrast = sli_contrast->value();
	chord_norm = cbx_norm->isChecked();
	key_mode = but_mode->currSelection;
	song_events = but_song_events->currSelection;

	if (key_mode < 0 || key_mode >= MM_PASS_MAX)
		key_mode = 0;

	pthread_mutex_lock(&sm->mainWindow->mtx);
	sm->baseKey = base_key;
	sm->delayNoise = key_delay;
	sm->inputChannel = channelInput;
	sm->synthChannel = channel;
	sm->synthChannelBase = channelBase;
	sm->synthChannelTreb = channelTreb;
	sm->keyMode = key_mode;
	sm->chordContrast = chord_contrast;
	sm->chordNormalize = chord_norm;
	sm->songEventsOn = song_events;
	pthread_mutex_unlock(&sm->mainWindow->mtx);
}
