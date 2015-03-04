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

#ifndef _MIDIPP_MIDI_H_
#define	_MIDIPP_MIDI_H_

#include "midipp.h"

#define	MIDI_MAX_TRACKS 16

#define	MIDI_FLAG_STRING 0x01
#define	MIDI_FLAG_DURATION 0x02
#define	MIDI_FLAG_DIALOG 0x04
#define	MIDI_FLAG_MULTI_CHAN 0x08

class MppMidi : public QDialog
{
	Q_OBJECT;

public:
	MppMidi(uint32_t _mask, uint32_t _flags, uint32_t _thres);
	~MppMidi();

	/* channel mask */
	uint32_t chan_mask;

	/* MIDI_FLAG_XXX */
	uint32_t flags;

	/* event threshold in milliseconds */
	uint32_t thres;

	/* prefix */
	QString prefix;

private:
	QGridLayout *gl;

	MppGroupBox *gb_import;

	MppCheckBox *cbx_import[MIDI_MAX_TRACKS];
	MppCheckBox *cbx_single_track;
	MppCheckBox *cbx_have_strings;
	MppCheckBox *cbx_have_duration;

	QLineEdit *led_prefix;

	QSpinBox *spn_parse_thres;

	QPushButton *but_done;
	QPushButton *but_set_all;
	QPushButton *but_clear_all;

public slots:
	void handle_checkboxes();
	void handle_done();
	void handle_set_all_track();
	void handle_clear_all_track();
};

#endif		/* _MIDIPP_MIDI_H_ */
