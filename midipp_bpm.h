/*-
 * Copyright (c) 2011 Hans Petter Selasky. All rights reserved.
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

#ifndef _MIDIPP_BPM_H_
#define	_MIDIPP_BPM_H_

#include <midipp.h>

class MppBpm : public QDialog
{
	Q_OBJECT;

public:
	MppBpm(MppMainWindow *parent);
	~MppBpm();

	void handle_update();

	MppMainWindow *mw;

	QGridLayout *gl;

	QLabel *lbl_bpm_pattern;
	QLabel *lbl_bpm_value;
	QLabel *lbl_bpm_duty;
	QLabel *lbl_bpm_amp;
	QLabel *lbl_bpm_key;
	QLabel *lbl_view[MPP_MAX_VIEWS];
	QLabel *lbl_bpm_ref;
	QLabel *lbl_bpm_period;

	MppPattern *led_bpm_pattern;
	QSpinBox *spn_bpm_value;
	QSpinBox *spn_bpm_duty;
	QSpinBox *spn_bpm_amp;
	MppSpinBox *spn_bpm_key;
	QSpinBox *spn_bpm_ref;
	QSpinBox *spn_bpm_period;
	QCheckBox *cbx_view[MPP_MAX_VIEWS];

	QPushButton *but_bpm_enable;
	QPushButton *but_reset_all;
	QPushButton *but_done_all;

	uint32_t time;
	uint32_t enabled;
	uint32_t bpm;
	uint32_t duty;
	uint32_t duty_ticks;
	uint32_t ref;
	uint32_t period;
	uint8_t key;
	uint8_t view[MPP_MAX_VIEWS];
	uint8_t amp;
	uint8_t skip_bpm;

public slots:
	void handle_reload_all();
	void handle_view_all(int);
	void handle_bpm_enable();
	void handle_reset_all();
	void handle_done_all();
	void handle_bpm_value(int);
	void handle_bpm_duty(int);
	void handle_bpm_amp(int);
	void handle_bpm_key(int);
	void handle_bpm_ref(int);
	void handle_bpm_period(int);
};

#endif		/* _MIDIPP_BPM_H_ */
