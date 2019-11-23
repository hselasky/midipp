/*-
 * Copyright (c) 2010-2019 Hans Petter Selasky. All rights reserved.
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

#ifndef _MIDIPP_LOOPTAB_H_
#define	_MIDIPP_LOOPTAB_H_

#include "midipp.h"

struct MppLoopEntry {
	MppButton *but_trig;
	struct umidi20_track *track[MPP_MAX_TRACKS];
	qreal scale_factor;
	qreal repeat_factor;
	uint32_t period;
	uint32_t first;
	uint32_t last;
	uint32_t state;
};

class MppLoopTab : public QWidget
{
	Q_OBJECT;

public:
	enum {
		ST_IDLE,
		ST_REC,
		ST_PLAYING,
		ST_STOPPED,
	};

	MppLoopTab(QWidget *parent, MppMainWindow *);
	~MppLoopTab();

	bool check_record(uint8_t index, uint8_t chan, uint8_t n);
	void handle_clearN(int);
	void handle_recordN(int);
	void handle_timer_sync();

	uint8_t auto_zero_start[0];

	MppMainWindow *mw;

	QGridLayout *gl;

	MppGroupBox *gb_control;

	MppButtonMap *mbm_arm_import;
	MppButtonMap *mbm_arm_reset;
	MppButtonMap *mbm_pedal_rec;

	QPushButton *but_reset;
	QSlider *sli_progress;

	struct MppLoopEntry loop[MPP_LOOP_MAX];

	uint32_t pos_align;
	uint32_t cur_period;
  
	uint8_t pedal_rec;
	uint8_t needs_update;

	uint8_t auto_zero_end[0];

public slots:

	bool handle_import(int);
	bool handle_clear(int);
	void handle_trigger(int);
	void watchdog();
	void handle_pedal_rec(int);
	void handle_reset();
	void handle_value_changed(int);
};

#endif		/* _MIDIPP_LOOPTAB_H_ */
