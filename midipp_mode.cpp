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

#include "midipp_buttonmap.h"
#include "midipp_mode.h"
#include "midipp_spinbox.h"
#include "midipp_scores.h"
#include "midipp_mainwindow.h"
#include "midipp_checkbox.h"

MppMode :: MppMode(MppScoreMain *_parent, uint8_t _vi)
{
	uint32_t x;
	char buf[64];

	sm = _parent;
	view_index = _vi;

	gl = new QGridLayout(this);

	gb_contrast = new QGroupBox();
	gb_delay = new QGroupBox();

	gb_idev = new QGroupBox();
	gb_idev->setTitle(tr("Input devices"));

	gl_contrast = new QGridLayout(gb_contrast);
	gl_delay = new QGridLayout(gb_delay);
	gl_idev = new QGridLayout(gb_idev);

	setWindowTitle(tr("View ") + QChar('A' + _vi) + tr(" mode"));
	setWindowIcon(QIcon(QString(MPP_ICON_FILE)));

	lbl_norm = new QLabel(tr("Normalize chord pressure"));
	lbl_base = new QLabel(tr("Base play key"));
	lbl_cmd = new QLabel(tr("Base command key"));
	lbl_chan = new QLabel(tr("Synth channel"));

	cbx_norm = new MppCheckBox();
	cbx_norm->setChecked(1);

	for (x = 0; x != MPP_MAX_DEVS; x++) {

		snprintf(buf, sizeof(buf), "Dev%d", x);

		cbx_dev[x] = new MppCheckBox();
		cbx_dev[x]->setChecked(view_index == 0);

		lbl_dev[x] = new QLabel(tr(buf));
	}

	sli_contrast = new QSlider();
	sli_contrast->setRange(0, 255);
	sli_contrast->setOrientation(Qt::Horizontal);
	sli_contrast->setValue(128);
	connect(sli_contrast, SIGNAL(valueChanged(int)),
	    this, SLOT(handle_contrast_changed(int)));
	handle_contrast_changed(sli_contrast->value());
	gl_contrast->addWidget(sli_contrast,0,0,1,1);

	but_song_events = new MppButtonMap("Send MIDI song events\0OFF\0ON\0", 2, 2);

	but_mode = new MppButtonMap("Key mode\0" "ALL\0" "MIXED\0" "FIXED\0" "TRANSP\0" "CHORD\0", 5, 3);

	but_reset = new QPushButton(tr("Reset"));
	connect(but_reset, SIGNAL(released()), this, SLOT(handle_reset()));

	but_done = new QPushButton(tr("Close"));
	connect(but_done, SIGNAL(released()), this, SLOT(handle_done()));

	but_set_all = new QPushButton(tr("All devs"));
	connect(but_set_all, SIGNAL(released()), this, SLOT(handle_set_all_devs()));

	but_clear_all = new QPushButton(tr("No devs"));
	connect(but_clear_all, SIGNAL(released()), this, SLOT(handle_clear_all_devs()));

	spn_cmd = new MppSpinBox();
	spn_cmd->setRange(0, 127);
	spn_cmd->setValue(0);

	spn_base = new MppSpinBox();
	spn_base->setRange(0, 127);
	spn_base->setValue(0);

	sli_delay = new QSlider();
	sli_delay->setRange(0, 256);
	sli_delay->setValue(0);
	sli_delay->setOrientation(Qt::Horizontal);
	connect(sli_delay, SIGNAL(valueChanged(int)),
	    this, SLOT(handle_delay_changed(int)));
	handle_delay_changed(sli_delay->value());
	gl_delay->addWidget(sli_delay,0,0,1,1);

	spn_chan = new QSpinBox();
	spn_chan->setRange(0, 15);
	spn_chan->setValue(0);

	for (x = 0; x != MPP_MAX_DEVS; x++) {
		gl_idev->addWidget(lbl_dev[x], x, 0, 1, 1, Qt::AlignCenter);
		gl_idev->addWidget(cbx_dev[x], x, 1, 1, 1, Qt::AlignCenter);
	}

	gl->addWidget(gb_idev, 0, 0, 9, 2);

	gl->addWidget(lbl_cmd, 0, 2, 1, 1);
	gl->addWidget(spn_cmd, 0, 3, 1, 1);

	gl->addWidget(lbl_base, 1, 2, 1, 1);
	gl->addWidget(spn_base, 1, 3, 1, 1);

	gl->addWidget(lbl_chan, 2, 2, 1, 1);
	gl->addWidget(spn_chan, 2, 3, 1, 1);

	gl->addWidget(lbl_norm, 3, 2, 1, 1);
	gl->addWidget(cbx_norm, 3, 3, 1, 1);

	gl->addWidget(gb_delay, 4, 2, 1, 2);

	gl->addWidget(gb_contrast, 5, 2, 1, 2);

	gl->addWidget(but_song_events, 6, 2, 1, 2);

	gl->addWidget(but_mode, 7, 2, 2, 2);

	gl->setRowStretch(9, 1);
	gl->setColumnStretch(4, 1);

	gl->addWidget(but_set_all, 10, 0, 1, 1);
	gl->addWidget(but_clear_all, 10, 1, 1, 1);
	gl->addWidget(but_reset, 10, 2, 1, 1);
	gl->addWidget(but_done, 10, 3, 1, 1);
}

MppMode :: ~MppMode()
{

}

void
MppMode :: update_all(void)
{
	int base_key;
	int cmd_key;
	int key_delay;
	int channel;
	int key_mode;
	int input_mask;
	int chord_contrast;
	int chord_norm;
	int song_events;
	int x;

	pthread_mutex_lock(&sm->mainWindow->mtx);
	base_key = sm->baseKey;
	cmd_key = sm->cmdKey;
	key_delay = sm->delayNoise;
	channel = sm->synthChannel;
	key_mode = sm->keyMode;
	input_mask = sm->devInputMask;
	chord_contrast = sm->chordContrast;
	chord_norm = sm->chordNormalize;
	song_events = sm->songEventsOn;
	pthread_mutex_unlock(&sm->mainWindow->mtx);

	for (x = 0; x != MPP_MAX_DEVS; x++)
		cbx_dev[x]->setChecked((input_mask >> x) & 1);

	spn_base->setValue(base_key);
	spn_cmd->setValue(cmd_key);
	sli_delay->setValue(key_delay);
	sli_contrast->setValue(chord_contrast);
	spn_chan->setValue(channel);
	but_mode->setSelection(key_mode);
	cbx_norm->setChecked(chord_norm);
	but_song_events->setSelection(song_events);
}

void
MppMode :: handle_set_all_devs()
{
	uint32_t x;

	for (x = 0; x != MPP_MAX_DEVS; x++)
		cbx_dev[x]->setChecked(1);
}

void
MppMode :: handle_clear_all_devs()
{
	uint32_t x;

	for (x = 0; x != MPP_MAX_DEVS; x++)
		cbx_dev[x]->setChecked(0);
}

void
MppMode :: handle_contrast_changed(int v)
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
MppMode :: handle_delay_changed(int v)
{
	char buf[32];
	snprintf(buf, sizeof(buf), "Random T/F/M key delay (%dms)", v);
	gb_delay->setTitle(tr(buf));
}

void
MppMode :: handle_reset()
{
	uint32_t x;

	sli_contrast->setValue(128);
	sli_delay->setValue(25);
	spn_cmd->setValue(MPP_DEFAULT_CMD_KEY);
	spn_base->setValue(MPP_DEFAULT_BASE_KEY);
	spn_chan->setValue(0);
	cbx_norm->setChecked(1);
	but_mode->setSelection(0);
	but_song_events->setSelection(0);

	for (x = 0; x != MPP_MAX_DEVS; x++)
		cbx_dev[x]->setChecked(1);
}

void
MppMode :: handle_done()
{
	int base_key;
	int cmd_key;
	int key_delay;
	int channel;
	int key_mode;
	int input_mask;
	int chord_contrast;
	int chord_norm;
	int song_events;
	int x;

	input_mask = 0;

	for (x = 0; x != MPP_MAX_DEVS; x++) {
		if (cbx_dev[x]->isChecked())
			input_mask |= (1U << x);
	}

	base_key = spn_base->value();
	cmd_key = spn_cmd->value();
	key_delay = sli_delay->value();
	channel = spn_chan->value();
	chord_contrast = sli_contrast->value();
	chord_norm = cbx_norm->isChecked();
	key_mode = but_mode->currSelection;
	song_events = but_song_events->currSelection;

	if (key_mode < 0 || key_mode >= MM_PASS_MAX)
		key_mode = 0;

	pthread_mutex_lock(&sm->mainWindow->mtx);
	sm->baseKey = base_key;
	sm->cmdKey = cmd_key;
	sm->delayNoise = key_delay;
	sm->synthChannel = channel;
	sm->keyMode = key_mode;
	sm->devInputMask = input_mask;
	sm->chordContrast = chord_contrast;
	sm->chordNormalize = chord_norm;
	sm->songEventsOn = song_events;
	pthread_mutex_unlock(&sm->mainWindow->mtx);

	accept();
}
