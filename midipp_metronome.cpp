/*-
 * Copyright (c) 2015 Hans Petter Selasky. All rights reserved.
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

#include "midipp_metronome.h"
#include "midipp_volume.h"
#include "midipp_spinbox.h"
#include "midipp_chansel.h"
#include "midipp_buttonmap.h"
#include "midipp_mainwindow.h"
#include "midipp_groupbox.h"

static void
MppMetronomeCallback(void *arg)
{
        MppMetronome *mm = (MppMetronome *)arg;
        MppMainWindow *mw = mm->mainWindow;
	struct mid_data *d = &mw->mid_data;

        pthread_mutex_lock(&mw->mtx);
        if (mm->enabled != 0 && mw->midiTriggered != 0) {
		if (mw->check_play(mm->chan, 0)) {
			switch (mm->mode) {
			case 1:
				mid_key_press(d, mm->key, mm->volume,
				    60000 / (2 * mm->bpm));
				break;
			default:
				mid_key_press(d, mm->key, mm->volume,
				    60000 / (8 * mm->bpm));
				mid_delay(d, 60000 / (4 * mm->bpm));
				mid_key_press(d, mm->key, mm->volume,
				    60000 / (8 * mm->bpm));
				break;
			}
		}
	}
        pthread_mutex_unlock(&mw->mtx);
}

MppMetronome ::  MppMetronome(MppMainWindow *parent)
{
	mainWindow = parent;

	gb = new MppGroupBox(tr("Metronome"));

	volume = 64;
	bpm = 120;
	enabled = 0;
	chan = 9;
	key = C4;
	mode = 1;

	spn_volume = new MppVolume();
	spn_volume->setRange(0, 127, 64);
	spn_volume->setValue(volume);
	connect(spn_volume, SIGNAL(valueChanged(int)), this, SLOT(handleVolumeChanged(int)));

	spn_bpm = new QSpinBox();
	spn_bpm->setRange(1, 6000);
	spn_bpm->setValue(bpm);
	spn_bpm->setSuffix(tr(" bpm"));
	connect(spn_bpm, SIGNAL(valueChanged(int)), this, SLOT(handleBPMChanged(int)));

	but_onoff = new MppButtonMap("Enable\0" "OFF\0" "ON\0", 2, 2);
	but_onoff->setSelection(enabled);
	connect(but_onoff, SIGNAL(selectionChanged(int)), this, SLOT(handleEnableChanged(int)));

	but_mode = new MppButtonMap("Time signature\0" "3 / 4\0" "4 / 4\0", 2, 2);
	but_mode->setSelection(mode);
	connect(but_mode, SIGNAL(selectionChanged(int)), this, SLOT(handleModeChanged(int)));

	spn_chan = new MppChanSel(chan, 0);
	connect(spn_chan, SIGNAL(valueChanged(int)), this, SLOT(handleChanChanged(int)));

	spn_key = new MppSpinBox();
	spn_key->setValue(key);
	connect(spn_key, SIGNAL(valueChanged(int)), this, SLOT(handleKeyChanged(int)));

	gb->addWidget(new QLabel(tr("Volume")),0,0,1,1,Qt::AlignVCenter|Qt::AlignLeft);
	gb->addWidget(spn_volume, 0,1,1,1,Qt::AlignCenter);
	
	gb->addWidget(new QLabel(tr("Key")),1,0,1,1,Qt::AlignVCenter|Qt::AlignLeft);
	gb->addWidget(spn_key,  1,1,1,1,Qt::AlignCenter);

	gb->addWidget(new QLabel(tr("Rate")),2,0,1,1,Qt::AlignVCenter|Qt::AlignLeft);
	gb->addWidget(spn_bpm,  2,1,1,1,Qt::AlignCenter);

	gb->addWidget(new QLabel(tr("Channel")),3,0,1,1,Qt::AlignVCenter|Qt::AlignLeft);
	gb->addWidget(spn_chan, 3,1,1,1,Qt::AlignCenter);
	gb->addWidget(but_mode, 4,0,1,2,Qt::AlignCenter);
	gb->addWidget(but_onoff, 5,0,1,2,Qt::AlignCenter);

	gb->setRowStretch(6,1);
	gb->setColumnStretch(2,1);

	handleBPMChanged(bpm);
}

MppMetronome :: ~MppMetronome()
{
	umidi20_unset_timer(&MppMetronomeCallback, this);
}

void
MppMetronome :: handleVolumeChanged(int val)
{
	pthread_mutex_lock(&mainWindow->mtx);
	volume = val;
	pthread_mutex_unlock(&mainWindow->mtx);
}

void
MppMetronome :: handleBPMChanged(int val)
{
	uint32_t time_ms;

	pthread_mutex_lock(&mainWindow->mtx);
	bpm = val;
	pthread_mutex_unlock(&mainWindow->mtx);

	time_ms = 60000 / bpm;
	
        umidi20_update_timer(&MppMetronomeCallback, this, time_ms, 1);
}

void
MppMetronome :: handleEnableChanged(int val)
{
  	pthread_mutex_lock(&mainWindow->mtx);
	enabled = val;
	pthread_mutex_unlock(&mainWindow->mtx);
}

void
MppMetronome :: handleChanChanged(int val)
{
	pthread_mutex_lock(&mainWindow->mtx);
	chan = val;
	pthread_mutex_unlock(&mainWindow->mtx);
}

void
MppMetronome :: handleKeyChanged(int val)
{
  	pthread_mutex_lock(&mainWindow->mtx);
	key = val;
	pthread_mutex_unlock(&mainWindow->mtx);
}

void
MppMetronome :: handleModeChanged(int val)
{
  	pthread_mutex_lock(&mainWindow->mtx);
	mode = val;
	pthread_mutex_unlock(&mainWindow->mtx);
}
