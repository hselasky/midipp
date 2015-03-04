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

#ifndef _MIDIPP_REPLAY_H_
#define	_MIDIPP_REPLAY_H_

#include "midipp.h"

class MppReplayTab : public QWidget
{
	Q_OBJECT;
public:
	MppReplayTab(MppMainWindow *parent);
	~MppReplayTab() { };

	MppMainWindow *mainWindow;

	QGridLayout *gl;

	MppGroupBox *gb_start;
	MppGroupBox *gb_configure;
	MppGroupBox *gb_rehearse;
	MppGroupBox *gb_record;

	MppMetronome *metronome;

	QPushButton *butEmptyFile;
	QPushButton *butUseExistingMidiFile;
	QPushButton *butStartRecordSomething;
	QPushButton *butStopRecordSomething;
	QPushButton *butViewMode;
	QPushButton *butStartRehearse;
	QPushButton *butRecordNew;
	QPushButton *butRecordTrigger;
	QPushButton *butRecordStop;
	QPushButton *butRecordPlayback;
	QPushButton *butRecordExport;

	int saved_mode;
	
public slots:
	void handleEmptyFile();
	void handleUseExistingMidiFile();
	void handleStartRecordSomething();
	void handleStopRecordSomething();
	void handleViewMode();
	void handleStartRehearse();
	void handleRecordNew();
	void handleRecordTrigger();
	void handleRecordStop();
	void handleRecordPlayback();
	void handleRecordExport();
	void handleRestoreMode();
	void handleEraseMidiTracks();
	void handleCheckMode();
};

#endif			/* _MIDIPP_REPLAY_H_ */
