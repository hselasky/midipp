/*-
 * Copyright (c) 2011-2022 Hans Petter Selasky
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

#include "midipp_dialog.h"

class MppBpm : public MppDialog
{
	Q_OBJECT

public:
	MppBpm(MppMainWindow *parent);
	~MppBpm();

	uint32_t bpm_get();
	void handle_beat_event_locked(int);
	void handle_jump_event_locked(int);

	void handle_update(int = 0);
	void handle_callback_locked();

	MppMainWindow *mw;

	QSpinBox *spn_bpm_value;
	QSpinBox *spn_bpm_amp;
	MppSpinBox *spn_bpm_key;
	QSpinBox *spn_bpm_period_ref;
	QSpinBox *spn_bpm_period_cur;
	MppCheckBox *cbx_out_view[MPP_MAX_VIEWS];
	MppCheckBox *cbx_sync_view[MPP_MAX_VIEWS];
	MppCheckBox *cbx_midi_beat;
  	MppButtonMap *mbm_generator;

	MppGroupBox *gb_ctrl;
	MppGroupBox *gb_scale;
	MppGroupBox *gb_io;

	QTimer *tim_config;

	QPushButton *but_reset_all;
	QPushButton *but_done_all;

	uint32_t enabled;
	uint32_t bpm_cur;
	uint32_t bpm_other;
	uint32_t period_ref;
	uint32_t period_cur;
	int key;
	uint8_t view_out[MPP_MAX_VIEWS];
	uint8_t view_sync[MPP_MAX_VIEWS];
	uint8_t amp;
	uint8_t beat;
	uint8_t toggle;

public slots:
	void handle_config_apply();
	void handle_config_change(int = 0);
	void handle_bpm_enable(int);
	void handle_reload_all();
	void handle_reset_all();
	void handle_done_all();
};

#endif		/* _MIDIPP_BPM_H_ */
