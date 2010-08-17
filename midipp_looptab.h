/*-
 * Copyright (c) 2010 Hans Petter Selasky. All rights reserved.
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

#include <midipp.h>

#define	MIDIPP_LOOP_MAX		8U

class MppLoopTab : public QWidget
{
	Q_OBJECT;

public:
	enum {
		ST_IDLE,
		ST_REC,
		ST_DONE,
	};

	MppLoopTab(QWidget *parent, MppMainWindow *);
	~MppLoopTab();

	void add_key(uint8_t key, uint8_t vel);
	void add_pedal(uint8_t val);
	int checkLabelTrig(int off);
	int handle_trigN(int n, int key_off, int amp);
	void handle_clearN(int n);
	void fill_loop_data(int n, int vel, int key_off);

	uint8_t auto_zero_start[0];

	MppMainWindow *mw;

	QGridLayout *gl;

	QSpinBox *spn_chan[MIDIPP_LOOP_MAX];

	QLabel *lbl_rem[MIDIPP_LOOP_MAX];
	QLabel *lbl_dur[MIDIPP_LOOP_MAX];
	QLabel *lbl_loop[MIDIPP_LOOP_MAX];
	QLabel *lbl_state[MIDIPP_LOOP_MAX];

	QPushButton *but_loop_on;
	QPushButton *but_pedal_rec;
	QPushButton *but_clear[MIDIPP_LOOP_MAX];
	QPushButton *but_trig[MIDIPP_LOOP_MAX];

	QLabel *lbl_pedal_rec;
	QLabel *lbl_loop_on;
	QLabel *lbl_chn_title;
	QLabel *lbl_rem_title;
	QLabel *lbl_dur_title;
	QLabel *lbl_state_title;

	QPushButton *but_reset;

	uint32_t first_pos[MIDIPP_LOOP_MAX];
	uint32_t last_pos[MIDIPP_LOOP_MAX];
	uint32_t state[MIDIPP_LOOP_MAX];

	struct mid_data mid_data;
	struct umidi20_song *song;
	struct umidi20_track *track[MIDIPP_LOOP_MAX];

	uint8_t chan_val[MIDIPP_LOOP_MAX];
	uint8_t pedal_rec;
	uint8_t loop_on;
	uint8_t needs_update;
	uint8_t last_loop;

	uint8_t auto_zero_end[0];

public slots:

	void watchdog();
	void handle_pedal();
	void handle_loop();
	void handle_reset();
	void handle_clear();
	void handle_trig();
	void handle_value_changed(int);
};

#endif		/* _MIDIPP_LOOPTAB_H_ */
