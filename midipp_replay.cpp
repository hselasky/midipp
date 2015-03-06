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

#include "midipp_replay.h"
#include "midipp_groupbox.h"
#include "midipp_mainwindow.h"
#include "midipp_scores.h"
#include "midipp_buttonmap.h"
#include "midipp_mode.h"
#include "midipp_metronome.h"

MppReplayTab :: MppReplayTab(MppMainWindow *parent)
{
	mainWindow = parent;

	gl = new QGridLayout(this);

	gb_start = new MppGroupBox(tr("1: Select starting point"));
	gb_configure = new MppGroupBox(tr("2: Configure view A mode"));
	gb_rehearse = new MppGroupBox(tr("3: Rehearse"));
	gb_record = new MppGroupBox(tr("4: Record"));

	metronome = new MppMetronome(parent);

	saved_mode = -1;

	butEmptyFile = new QPushButton(tr("Empty\nFile"));
	connect(butEmptyFile, SIGNAL(released()), this, SLOT(handleEmptyFile()));

	butUseExistingMidiFile = new QPushButton(tr("Use existing\nMIDI file"));
	connect(butUseExistingMidiFile, SIGNAL(released()), this, SLOT(handleUseExistingMidiFile()));

	butStartRecordSomething = new QPushButton(tr("Start MIDI\nrecording"));
	connect(butStartRecordSomething, SIGNAL(released()), this, SLOT(handleStartRecordSomething()));

	butStopRecordSomething = new QPushButton(tr("Stop MIDI\nrecording"));
	connect(butStopRecordSomething, SIGNAL(released()), this, SLOT(handleStopRecordSomething()));

	butViewMode = new QPushButton(tr("View A mode"));
	connect(butViewMode, SIGNAL(released()), this, SLOT(handleViewMode()));

	butStartRehearse = new QPushButton(tr("Start"));
	connect(butStartRehearse, SIGNAL(released()), this, SLOT(handleStartRehearse()));

	butRecordNew = new QPushButton(tr("Start new"));
	connect(butRecordNew, SIGNAL(released()), this, SLOT(handleRecordNew()));

	butRecordTrigger = new QPushButton(tr("Trigger"));
	connect(butRecordTrigger, SIGNAL(released()), this, SLOT(handleRecordTrigger()));

	butRecordStop = new QPushButton(tr("Stop"));
	connect(butRecordStop, SIGNAL(released()), this, SLOT(handleRecordStop()));

	butRecordPlayback = new QPushButton(tr("Playback"));
	connect(butRecordPlayback, SIGNAL(released()), this, SLOT(handleRecordPlayback()));

	butRecordExport = new QPushButton(tr("Export"));
	connect(butRecordExport, SIGNAL(released()), this, SLOT(handleRecordExport()));

	gb_start->addWidget(butEmptyFile, 0,0,2,1);
	gb_start->addWidget(butUseExistingMidiFile, 0,1,2,1);
	gb_start->addWidget(butStartRecordSomething, 0,2,1,1);
	gb_start->addWidget(butStopRecordSomething, 1,2,1,1);

	gb_configure->addWidget(butViewMode, 0,0,1,1);

	gb_rehearse->addWidget(butStartRehearse, 0,0,1,1);

	gb_record->addWidget(butRecordNew, 0,0,1,1);
	gb_record->addWidget(butRecordTrigger, 0,1,1,1);
	gb_record->addWidget(butRecordStop, 0,2,1,1);
	gb_record->addWidget(butRecordPlayback, 1,0,1,1);
	gb_record->addWidget(butRecordExport, 1,1,1,1);

	gl->addWidget(gb_start, 0,0,1,1);
	gl->addWidget(gb_configure, 1,0,1,1);
	gl->addWidget(gb_rehearse, 2,0,1,1);
	gl->addWidget(gb_record, 3,0,1,1);
	gl->addWidget(metronome->gb, 0,1,4,1);
	gl->setRowStretch(4,1);
	gl->setColumnStretch(2,1);
}

void
MppReplayTab :: handleEmptyFile()
{
  	mainWindow->handle_rewind();
	handleRestoreMode();
	mainWindow->scores_main[0]->handleScoreFileNew(0);
	mainWindow->mbm_midi_record->setSelection(0);
	mainWindow->mbm_midi_play->setSelection(0);
	mainWindow->handle_midi_file_new();

	mainWindow->scores_main[0]->editWidget->setPlainText(
		QString("L0:\n"
			"\n"
			"/* write your scores here */\n"
			"\n"
			"J0\n"));
}

void
MppReplayTab :: handleUseExistingMidiFile()
{
  	mainWindow->handle_rewind();
	handleRestoreMode();
	mainWindow->scores_main[0]->handleScoreFileNew(1);
	mainWindow->mbm_midi_record->setSelection(0);
	mainWindow->mbm_midi_play->setSelection(0);
	mainWindow->handle_midi_file_open(2);
}

void
MppReplayTab :: handleStartRecordSomething()
{
	handleEraseMidiTracks();

	if (saved_mode < 0) {
		pthread_mutex_lock(&mainWindow->mtx);
		saved_mode = mainWindow->scores_main[0]->keyMode;
		mainWindow->scores_main[0]->keyMode = MM_PASS_ALL;
		pthread_mutex_unlock(&mainWindow->mtx);

		mainWindow->handle_mode(0, 0);
	}
	mainWindow->handle_rewind();
	mainWindow->mbm_midi_record->setSelection(1);
	mainWindow->mbm_midi_play->setSelection(0);
}

void
MppReplayTab :: handleRestoreMode()
{
	if (saved_mode < 0)
		return;

	pthread_mutex_lock(&mainWindow->mtx);
	mainWindow->scores_main[0]->keyMode = saved_mode;
	pthread_mutex_unlock(&mainWindow->mtx);

	mainWindow->handle_mode(0, 0);

	saved_mode = -1;
}

void
MppReplayTab :: handleStopRecordSomething()
{
	/* check if recording */
	if (saved_mode < 0)
		return;
  
	mainWindow->handle_rewind();
	mainWindow->mbm_midi_record->setSelection(0);
	handleRestoreMode();
	mainWindow->scores_main[0]->handleScoreFileNew(0);
	mainWindow->handle_midi_file_import(0);
}

void
MppReplayTab :: handleViewMode()
{
	mainWindow->handle_mode(0, 1);
	saved_mode = -1;
}

void
MppReplayTab :: handleStartRehearse()
{
	handleRestoreMode();
	handleCheckMode();
	mainWindow->handle_rewind();
	mainWindow->handle_compile();
	mainWindow->handle_jump(0);
	mainWindow->mbm_midi_record->setSelection(0);
	mainWindow->mbm_midi_play->setSelection(0);
}

void
MppReplayTab :: handleRecordNew()
{
	handleEraseMidiTracks();
	handleRestoreMode();
	handleCheckMode();

	mainWindow->handle_rewind();
	mainWindow->handle_compile();
	mainWindow->handle_jump(0);
	mainWindow->mbm_midi_record->setSelection(1);
	mainWindow->mbm_midi_play->setSelection(0);
}

void
MppReplayTab :: handleRecordTrigger()
{
  	handleRestoreMode();

	mainWindow->mbm_midi_record->setSelection(1);
	mainWindow->handle_midi_trigger();
}

void
MppReplayTab :: handleRecordStop()
{
	mainWindow->handle_rewind();
	mainWindow->mbm_midi_record->setSelection(0);
	mainWindow->mbm_midi_play->setSelection(0);
}

void
MppReplayTab :: handleRecordPlayback()
{
	mainWindow->handle_rewind();
	mainWindow->mbm_midi_record->setSelection(0);
	mainWindow->mbm_midi_play->setSelection(1);
	mainWindow->handle_midi_trigger();
}

void
MppReplayTab :: handleRecordExport()
{
	handleRecordStop();
	mainWindow->handle_midi_file_save_as();
}

void
MppReplayTab :: handleEraseMidiTracks()
{
	mainWindow->handle_instr_mute_all();
	mainWindow->handle_instr_rem();
	mainWindow->handle_instr_unmute_all();
}

void
MppReplayTab :: handleCheckMode()
{
	bool change;

	pthread_mutex_lock(&mainWindow->mtx);
	change = (mainWindow->scores_main[0]->keyMode == MM_PASS_ALL);
	if (change)
		mainWindow->scores_main[0]->keyMode = MM_PASS_NONE_FIXED;
	pthread_mutex_unlock(&mainWindow->mtx);

	if (change)
		mainWindow->handle_mode(0, 0);
}
