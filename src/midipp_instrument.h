/*-
 * Copyright (c) 2017-2022 Hans Petter Selasky
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

#ifndef _MIDIPP_INSTRUMENT_H_
#define	_MIDIPP_INSTRUMENT_H_

#include "midipp.h"

class MppInstrumentTab : public QObject
{
	Q_OBJECT

public:
	MppInstrumentTab(MppMainWindow * = 0);
	~MppInstrumentTab();

	MppMainWindow *mw;

	MppGridLayout *gl;

	MppGroupBox *gb_instr_select;
	MppGroupBox *gb_instr_table;

	MppDevSel *spn_instr_curr_dev;
	MppChanSel *spn_instr_curr_chan;
	QSpinBox *spn_instr_curr_bank;
	QSpinBox *spn_instr_curr_prog;
	QSpinBox *spn_instr_bank[16];
	QSpinBox *spn_instr_prog[16];

	MppCheckBox *cbx_instr_mute[16];

	QPushButton *but_instr_rem;
	QPushButton *but_instr_program;
	QPushButton *but_instr_program_all;
	QPushButton *but_instr_reset;
	QPushButton *but_instr_mute_all;
	QPushButton *but_instr_unmute_all;

public slots:
	void handle_instr_changed();
	void handle_instr_reset();
	void handle_instr_program();
	void handle_instr_program_all();
	void handle_instr_channel_changed(int);
	void handle_instr_rem();
	void handle_instr_mute_all();
	void handle_instr_unmute_all();
};

#endif		/* _MIDIPP_INSTRUMENT_H_ */
