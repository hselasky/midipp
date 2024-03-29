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

#ifndef _MIDIPP_MODE_H_
#define	_MIDIPP_MODE_H_

#include "midipp_dialog.h"

enum {
	MM_NOTEMODE_NORMAL,
	MM_NOTEMODE_SYSEX,
	MM_NOTEMODE_MAX,
};

enum {
	MM_PASS_ALL,
	MM_PASS_NONE_FIXED,
	MM_PASS_NONE_TRANS,
	MM_PASS_NONE_CHORD_PIANO,
	MM_PASS_NONE_CHORD_AUX,
	MM_PASS_NONE_CHORD_TRANS,
	MM_PASS_MAX,
};

class MppModeBase : public QObject
{
	Q_OBJECT
public:
	MppScoreMain *sm;

	void sanity_check(void);
	void update_all(void);

	/* view number */
	uint8_t view_index;

	MppGroupBox *gb_iconfig;
	MppGroupBox *gb_oconfig;
	MppGroupBox *gb_contrast;
	MppGroupBox *gb_delay;

	MppCheckBox *cbx_norm;

	MppButtonMap *but_note_mode;
	MppButtonMap *but_song_events;
	MppButtonMap *but_mode;
	QPushButton *but_done;
	QPushButton *but_reset;

	QSlider *sli_contrast;
	QSlider *sli_delay;

	MppSpinBox *spn_base;

	MppChanSel *spn_input_chan;

	MppChanSel *spn_pri_chan;
	MppDevSel *spn_pri_dev;
	MppVolume *spn_pri_volume;

	MppChanSel *spn_sec_base_chan;
	MppDevSel *spn_sec_base_dev;
	MppVolume *spn_sec_base_volume;

	MppChanSel *spn_sec_treb_chan;
	MppDevSel *spn_sec_treb_dev;
	MppVolume *spn_sec_treb_volume;

	MppChanSel *spn_aux_chan;
	MppChanSel *spn_aux_base_chan;
	MppChanSel *spn_aux_treb_chan;

public slots:
	void handle_reset();
	void handle_changed();
	void handle_contrast_label(int v);
	void handle_delay_label(int v);
};

class MppMode : public MppDialog, public MppModeBase
{
public:
	MppMode(MppScoreMain *_parent, uint8_t _vi);
};

#endif		/* _MIDIPP_MODE_H_ */
