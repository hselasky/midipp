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

#ifndef _MIDIPP_METRONOME_H_
#define	_MIDIPP_METRONOME_H_

#include "midipp.h"

class MppMetronome : public QObject
{
	Q_OBJECT;
public:
	MppMetronome(MppMainWindow *parent);
	~MppMetronome();

	MppMainWindow *mainWindow;

	MppGroupBox *gb;

	MppVolume *spn_volume;
	QSpinBox *spn_bpm;
	MppButtonMap *but_onoff;
	MppChanSel *spn_chan;
	MppSpinBox *spn_key_bar;
	MppSpinBox *spn_key_beat;
	MppButtonMap *but_mode;
	QTimer *tim_config;

	int volume;
	int bpm;
	int enabled;
	int chan;
	int key_bar;
	int key_beat;
	int mode;
	
public slots:
	void handleVolumeChanged(int);
	void handleBPMChanged(int);
	void handleEnableChanged(int);
	void handleChanChanged(int);
	void handleKeyBarChanged(int);
	void handleKeyBeatChanged(int);
	void handleModeChanged(int);
	void handleUpdateLocked();
	void handleTimeout();
};

#endif			/* _MIDIPP_METRONOME_H_ */
