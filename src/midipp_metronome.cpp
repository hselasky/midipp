/*-
 * Copyright (c) 2015-2018 Hans Petter Selasky
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
#include "midipp_buttonmap.h"
#include "midipp_mainwindow.h"
#include "midipp_groupbox.h"
#include "midipp_scores.h"

static void
MppMetronomeOutput(MppMetronome *mm, struct mid_data *d)
{
	unsigned x;
	unsigned y = mm->mode + 1;

	for (x = 0; x != y; x++) {
		int key = ((x != 0) ? mm->key_beat : mm->key_bar) / MPP_BAND_STEP_12;
		if (key > -1 && key < 128)
			mid_key_press(d, key, mm->volume, 60000 / (2 * y * mm->bpm) + 1);
		mid_delay(d, 60000 / (y * mm->bpm) + 1);
	}
}

static void
MppMetronomeCallback(void *arg)
{
        MppMetronome *mm = (MppMetronome *)arg;
        MppMainWindow *mw = mm->mainWindow;

        mw->atomic_lock();
        if (mm->enabled != 0 && mw->midiTriggered != 0) {
		MppScoreMain *sm = mw->scores_main[mm->view];

		if (mw->check_play(MPP_DEFAULT_TRACK(sm->unit), sm->synthChannel, 0))
			MppMetronomeOutput(mm, &mw->mid_data);
		if (mm->enabled == 2 &&
		    mw->check_record(MPP_DEFAULT_TRACK(sm->unit), sm->synthChannel, 0))
			MppMetronomeOutput(mm, &mw->mid_data);
	}
        mw->atomic_unlock();
}

MppMetronome ::  MppMetronome(MppMainWindow *parent)
{
	mainWindow = parent;

	gb = new MppGroupBox(tr("Metronome"));

	volume = 64;
	bpm = 120;
	enabled = 0;
	key_bar = (MPP_C0 + (5 * 12)) * MPP_BAND_STEP_12;
	key_beat = (MPP_C0 + (4 * 12)) * MPP_BAND_STEP_12;
	mode = 0;
	view = 0;

	tim_config = new QTimer();
	tim_config->setSingleShot(1);
	connect(tim_config, SIGNAL(timeout()), this, SLOT(handleTimeout()));

	spn_volume = new MppVolume();
	spn_volume->setRange(1, 127, 64);
	spn_volume->setValue(volume);
	connect(spn_volume, SIGNAL(valueChanged(int)), this, SLOT(handleVolumeChanged(int)));

	spn_bpm = new QSpinBox();
	spn_bpm->setRange(1, 6000);
	spn_bpm->setValue(bpm);
	spn_bpm->setSuffix(tr(" bpm"));
	connect(spn_bpm, SIGNAL(valueChanged(int)), this, SLOT(handleBPMChanged(int)));

	but_onoff = new MppButtonMap("Mode\0" "OFF\0" "ON\0" "RECORD\0", 3, 3);
	but_onoff->setSelection(enabled);
	connect(but_onoff, SIGNAL(selectionChanged(int)), this, SLOT(handleEnableChanged(int)));

	but_mode = new MppButtonMap("Time signature\0"
				    "1 / N\0" "2 / N\0" "3 / N\0"
				    "4 / N\0" "5 / N\0" "6 / N\0"
				    "7 / N\0" "8 / N\0" "9 / N\0", 9, 3);
	but_mode->setSelection(mode);
	connect(but_mode, SIGNAL(selectionChanged(int)), this, SLOT(handleModeChanged(int)));

	but_view = new MppButtonMap("Output view\0" "View-A\nPrimary channel\0" "View-B\nPrimary channel\0", 2, 2);
	but_view->setSelection(enabled);
	connect(but_view, SIGNAL(selectionChanged(int)), this, SLOT(handleViewChanged(int)));

	spn_key_bar = new MppSpinBox(0,0);
	spn_key_bar->setValue(key_bar);
	connect(spn_key_bar, SIGNAL(valueChanged(int)), this, SLOT(handleKeyBarChanged(int)));

	spn_key_beat = new MppSpinBox(0,0);
	spn_key_beat->setValue(key_beat);
	connect(spn_key_beat, SIGNAL(valueChanged(int)), this, SLOT(handleKeyBeatChanged(int)));

	gb->addWidget(new QLabel(tr("Volume")),0,0,1,1,Qt::AlignVCenter|Qt::AlignLeft);
	gb->addWidget(spn_volume, 0,1,1,1,Qt::AlignCenter);
	
	gb->addWidget(new QLabel(tr("Bar Key")),1,0,1,1,Qt::AlignVCenter|Qt::AlignLeft);
	gb->addWidget(spn_key_bar,  1,1,1,1,Qt::AlignCenter);

	gb->addWidget(new QLabel(tr("Beat Key")),2,0,1,1,Qt::AlignVCenter|Qt::AlignLeft);
	gb->addWidget(spn_key_beat,  2,1,1,1,Qt::AlignCenter);

	gb->addWidget(new QLabel(tr("Rate")),3,0,1,1,Qt::AlignVCenter|Qt::AlignLeft);
	gb->addWidget(spn_bpm,  3,1,1,1,Qt::AlignCenter);

	gb->addWidget(but_view, 4,0,1,2);
	gb->addWidget(but_mode, 5,0,1,2);
	gb->addWidget(but_onoff, 6,0,1,2);

	gb->setRowStretch(7,1);
	gb->setColumnStretch(2,1);

	umidi20_set_timer(&MppMetronomeCallback, this, 60000 / bpm);
}

MppMetronome :: ~MppMetronome()
{
	tim_config->stop();

	umidi20_unset_timer(&MppMetronomeCallback, this);
}

void
MppMetronome :: handleVolumeChanged(int val)
{
	mainWindow->atomic_lock();
	volume = val;
	mainWindow->atomic_unlock();
}

void
MppMetronome :: handleTimeout()
{
	mainWindow->atomic_lock();
	handleUpdateLocked();
	mainWindow->atomic_unlock();
}

void
MppMetronome :: handleBPMChanged(int val)
{
	mainWindow->atomic_lock();
	bpm = val;
	mainWindow->atomic_unlock();

	tim_config->start(500);
}

void
MppMetronome :: handleUpdateLocked()
{
	umidi20_update_timer(&MppMetronomeCallback, this, 60000 / bpm, 1);
}

void
MppMetronome :: handleEnableChanged(int val)
{
  	mainWindow->atomic_lock();
	enabled = val;
	mainWindow->atomic_unlock();
}

void
MppMetronome :: handleKeyBarChanged(int val)
{
  	mainWindow->atomic_lock();
	key_bar = val;
	mainWindow->atomic_unlock();
}

void
MppMetronome :: handleKeyBeatChanged(int val)
{
	mainWindow->atomic_lock();
	key_beat = val;
	mainWindow->atomic_unlock();
}

void
MppMetronome :: handleModeChanged(int val)
{
	mainWindow->atomic_lock();
	mode = val;
	mainWindow->atomic_unlock();
}

void
MppMetronome :: handleViewChanged(int val)
{
	mainWindow->atomic_lock();
	view = val;
	mainWindow->atomic_unlock();
}
