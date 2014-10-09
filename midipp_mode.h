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

#ifndef _MIDIPP_MODE_H_
#define	_MIDIPP_MODE_H_

#include "midipp.h"

enum {
	MM_PASS_ALL,
	MM_PASS_ONE_MIXED,
	MM_PASS_NONE_FIXED,
	MM_PASS_NONE_TRANS,
	MM_PASS_NONE_CHORD_PIANO,
	MM_PASS_NONE_CHORD_GUITAR,
	MM_PASS_MAX,
};

class MppMode : public QDialog
{
	Q_OBJECT;

public:
	MppMode(MppScoreMain *_parent, uint8_t _vi);
	~MppMode();

	MppScoreMain *sm;

	void update_all(void);

	/* view number */
	uint8_t view_index;

public:
	QGridLayout *gl;

	MppGroupBox *gb_iconfig;
	MppGroupBox *gb_oconfig;
	MppGroupBox *gb_contrast;
	MppGroupBox *gb_delay;

	MppCheckBox *cbx_norm;

	MppButtonMap *but_song_events;
	MppButtonMap *but_mode;
	QPushButton *but_done;
	QPushButton *but_reset;

	QSlider *sli_contrast;
	QSlider *sli_delay;

	MppSpinBox *spn_base;

	MppChanSel *spn_input_chan;
	MppChanSel *spn_pri_chan;
	MppChanSel *spn_sec_base_chan;
	MppChanSel *spn_sec_treb_chan;

public slots:
	void handle_reset();
	void handle_changed();
	void handle_contrast_label(int v);
	void handle_delay_label(int v);
};

#endif		/* _MIDIPP_MODE_H_ */
