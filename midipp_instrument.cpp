/*-
 * Copyright (c) 2017-2019 Hans Petter Selasky. All rights reserved.
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

#include "midipp_gridlayout.h"
#include "midipp_buttonmap.h"
#include "midipp_spinbox.h"
#include "midipp_chansel.h"
#include "midipp_checkbox.h"
#include "midipp_groupbox.h"
#include "midipp_mainwindow.h"
#include "midipp_instrument.h"

MppInstrumentTab :: MppInstrumentTab(MppMainWindow *_mw)
{
	mw = _mw;

	gl = new MppGridLayout();

	but_instr_program = new QPushButton(tr("Program One"));
	but_instr_program_all = new QPushButton(tr("Program All"));
	but_instr_reset = new QPushButton(tr("Reset"));
	but_instr_rem = new QPushButton(tr("Delete muted"));
	but_instr_mute_all = new QPushButton(tr("Mute all"));
	but_instr_unmute_all = new QPushButton(tr("Unmute all"));

	spn_instr_curr_chan = new MppChanSel(mw, 0, 0);
	connect(spn_instr_curr_chan, SIGNAL(valueChanged(int)), this, SLOT(handle_instr_channel_changed(int)));

	spn_instr_curr_bank = new QSpinBox();
	spn_instr_curr_bank->setRange(0, 16383);
	spn_instr_curr_bank->setValue(0);

	spn_instr_curr_prog = new QSpinBox();
	spn_instr_curr_prog->setRange(0, 127);
	spn_instr_curr_prog->setValue(0);

	gb_instr_select = new MppGroupBox(tr("Synth and Recording Instrument Selector"));
	gb_instr_select->addWidget(new QLabel(tr("Channel")), 0, 0, 1, 1, Qt::AlignVCenter|Qt::AlignHCenter);
	gb_instr_select->addWidget(new QLabel(tr("Bank")), 0, 1, 1, 1, Qt::AlignVCenter|Qt::AlignHCenter);
	gb_instr_select->addWidget(new QLabel(tr("Program")), 0, 2, 1, 1, Qt::AlignVCenter|Qt::AlignHCenter);
	gb_instr_select->addWidget(spn_instr_curr_chan, 1, 0, 1, 1);
	gb_instr_select->addWidget(spn_instr_curr_bank, 1, 1, 1, 1);
	gb_instr_select->addWidget(spn_instr_curr_prog, 1, 2, 1, 1);
	gb_instr_select->addWidget(but_instr_program, 1, 3, 1, 3);
	gb_instr_select->addWidget(but_instr_program_all, 1, 6, 1, 2);

	gb_instr_table = new MppGroupBox(tr("Synth and Recording Instrument Table"));

	gb_instr_table->addWidget(new QLabel(tr("Bank")), 0, 1, 1, 1, Qt::AlignVCenter|Qt::AlignHCenter);
	gb_instr_table->addWidget(new QLabel(tr("Program")), 0, 2, 1, 1, Qt::AlignVCenter|Qt::AlignHCenter);
	gb_instr_table->addWidget(new QLabel(tr("Mute")), 0, 3, 1, 1, Qt::AlignVCenter|Qt::AlignHCenter);

	gb_instr_table->addWidget(new QLabel(tr("Bank")), 0, 5, 1, 1, Qt::AlignVCenter|Qt::AlignHCenter);
	gb_instr_table->addWidget(new QLabel(tr("Program")), 0, 6, 1, 1, Qt::AlignVCenter|Qt::AlignHCenter);
	gb_instr_table->addWidget(new QLabel(tr("Mute")), 0, 7, 1, 1, Qt::AlignVCenter|Qt::AlignHCenter);

	for (unsigned n = 0; n != 16; n++) {
		int y_off = (n & 8) ? 4 : 0;

		spn_instr_bank[n] = new QSpinBox();
		spn_instr_bank[n]->setRange(0, 16383);
		connect(spn_instr_bank[n], SIGNAL(valueChanged(int)), this, SLOT(handle_instr_changed(int)));

		spn_instr_prog[n] = new QSpinBox();
		spn_instr_prog[n]->setRange(0, 127);
		connect(spn_instr_prog[n], SIGNAL(valueChanged(int)), this, SLOT(handle_instr_changed(int)));

		cbx_instr_mute[n] = new MppCheckBox(n);
		connect(cbx_instr_mute[n], SIGNAL(stateChanged(int,int)), this, SLOT(handle_instr_changed(int)));

		gb_instr_table->addWidget(new QLabel(MppChanName(n)), (n & 7) + 1, 0 + y_off, 1, 1, Qt::AlignVCenter|Qt::AlignRight);
		gb_instr_table->addWidget(spn_instr_bank[n], (n & 7) + 1, 1 + y_off, 1, 1, Qt::AlignVCenter|Qt::AlignRight);
		gb_instr_table->addWidget(spn_instr_prog[n], (n & 7) + 1, 2 + y_off, 1, 1, Qt::AlignVCenter|Qt::AlignHCenter);
		gb_instr_table->addWidget(cbx_instr_mute[n], (n & 7) + 1, 3 + y_off, 1, 1, Qt::AlignVCenter|Qt::AlignHCenter);
	}

	gl->addWidget(gb_instr_select, 0, 0, 1, 8);
	gl->addWidget(gb_instr_table, 1, 0, 1, 8);

	gl->setRowStretch(3, 1);
	gl->setColumnStretch(8, 1);

	gl->addWidget(but_instr_mute_all, 4, 0, 1, 2);
	gl->addWidget(but_instr_unmute_all, 4, 2, 1, 2);
 	gl->addWidget(but_instr_rem, 4, 4, 1, 2);
	gl->addWidget(but_instr_reset, 4, 6, 1, 2);

	connect(but_instr_rem, SIGNAL(released()), this, SLOT(handle_instr_rem()));
	connect(but_instr_program, SIGNAL(released()), this, SLOT(handle_instr_program()));
	connect(but_instr_program_all, SIGNAL(released()), this, SLOT(handle_instr_program_all()));
	connect(but_instr_reset, SIGNAL(released()), this, SLOT(handle_instr_reset()));
	connect(but_instr_mute_all, SIGNAL(released()), this, SLOT(handle_instr_mute_all()));
	connect(but_instr_unmute_all, SIGNAL(released()), this, SLOT(handle_instr_unmute_all()));
}

MppInstrumentTab :: ~MppInstrumentTab()
{
	mw->atomic_lock();
	mw->tab_instrument = 0;
	mw->atomic_unlock();
}

void
MppInstrumentTab :: handle_instr_channel_changed(int chan)
{
	MppInstrumentTab * const ni = 0;

  	if (this == ni)
		return;

	int temp[2];

	temp[0] = spn_instr_bank[chan]->value();
	temp[1] = spn_instr_prog[chan]->value();

	spn_instr_curr_bank->setValue(temp[0]);
	spn_instr_curr_prog->setValue(temp[1]);

	MPP_BLOCKED(spn_instr_curr_chan,setValue(chan));
}

void
MppInstrumentTab :: handle_instr_program()
{
	int chan = spn_instr_curr_chan->value();
	int bank = spn_instr_curr_bank->value();
	int prog = spn_instr_curr_prog->value();

	MPP_BLOCKED(spn_instr_bank[chan],setValue(bank));
	MPP_BLOCKED(spn_instr_prog[chan],setValue(prog));
	MPP_BLOCKED(cbx_instr_mute[chan],setChecked(0));

	mw->atomic_lock();
	mw->instr[chan].updated |= 1;
	mw->atomic_unlock();

	handle_instr_changed(0);
}

void
MppInstrumentTab :: handle_instr_program_all()
{

	mw->atomic_lock();
	for (unsigned int x = 0; x != 16; x++)
		mw->instr[x].updated |= 1;
	mw->atomic_unlock();

	handle_instr_changed(0);
}

void 
MppInstrumentTab :: handle_instr_changed(int dummy)
{
  	MppInstrumentTab * const ni = 0;

	if (this == ni)
		return;

	struct mid_data *d = &mw->mid_data;
	int temp[3];
	uint8_t curr_chan;
	uint8_t x;
	uint8_t y;
	uint8_t z;
	uint8_t update_curr;
	uint8_t trig;

	curr_chan = spn_instr_curr_chan->value();

	for (x = 0; x != 16; x++) {

		mw->atomic_lock();

		temp[0] = spn_instr_bank[x]->value();
		temp[1] = spn_instr_prog[x]->value();
		temp[2] = cbx_instr_mute[x]->isChecked();

		update_curr = 0;

		if (mw->instr[x].bank != temp[0]) {
			if (mw->instr[x].updated & 2) {
				temp[0] = mw->instr[x].bank;
			} else {
				mw->instr[x].bank = temp[0];
				mw->instr[x].updated |= 1;
			}
			update_curr = 1;
		}
		if (mw->instr[x].prog != temp[1]) {
			if (mw->instr[x].updated & 2) {
				temp[1] = mw->instr[x].prog;
			} else {
				mw->instr[x].prog = temp[1];
				mw->instr[x].updated |= 1;
			}
			update_curr = 1;
		}
		if (mw->instr[x].muted != temp[2]) {
			if (mw->instr[x].updated & 2) {
				temp[2] = mw->instr[x].muted;
			} else {
				mw->instr[x].muted = temp[2];
				mw->instr[x].updated |= 1;
			}
			update_curr = 1;
		}
		mw->atomic_unlock();

		if (update_curr) {
			MPP_BLOCKED(spn_instr_bank[x],setValue(temp[0]));
			MPP_BLOCKED(spn_instr_prog[x],setValue(temp[1]));
			MPP_BLOCKED(cbx_instr_mute[x],setChecked(temp[2]));

			mw->atomic_lock();
			update_curr = (curr_chan == x);
			mw->atomic_unlock();
		}

		if (update_curr) {
			MPP_BLOCKED(spn_instr_curr_chan,setValue(x));
			MPP_BLOCKED(spn_instr_curr_bank,setValue(temp[0]));
			MPP_BLOCKED(spn_instr_curr_prog,setValue(temp[1]));
		}
	}

	/* Do the real programming */

	mw->atomic_lock();
	trig = mw->midiTriggered;
	mw->midiTriggered = 1;

	for (z = x = 0; x != 16; x++) {
		if (mw->instr[x].updated == 0)
			continue;

		mw->instr[x].updated = 0;
		for (y = 0; y != MPP_MAX_TRACKS; y++) {
			if (mw->check_mirror(y))
				continue;
			if (mw->check_play(y, x, 0) == 0)
				continue;
			mid_delay(d, z);
			mid_set_bank_program(d, x, 
			    mw->instr[x].bank,
			    mw->instr[x].prog);

			/* put some delay between the commands */
			z++;
		}
	}

	mw->midiTriggered = trig;
	mw->atomic_unlock();
}

void 
MppInstrumentTab :: handle_instr_reset()
{
  	MppInstrumentTab * const ni = 0;

	if (this == ni)
		return;

	for (uint8_t x = 0; x != 16; x++) {
		MPP_BLOCKED(spn_instr_bank[x],setValue(0));
		MPP_BLOCKED(spn_instr_prog[x],setValue(0));
		MPP_BLOCKED(cbx_instr_mute[x],setChecked(0));

		mw->atomic_lock();
		mw->instr[x].updated = 1;
		mw->atomic_unlock();
	}

	MPP_BLOCKED(spn_instr_curr_chan,setValue(0));
	MPP_BLOCKED(spn_instr_curr_bank,setValue(0));
	MPP_BLOCKED(spn_instr_curr_prog,setValue(0));

	handle_instr_changed(0);
}

void
MppInstrumentTab :: handle_instr_mute_all()
{

	for (uint8_t n = 0; n != 16; n++)
		MPP_BLOCKED(cbx_instr_mute[n],setChecked(1));

	handle_instr_changed(0);
}

void
MppInstrumentTab :: handle_instr_unmute_all()
{

	for (uint8_t n = 0; n != 16; n++)
		MPP_BLOCKED(cbx_instr_mute[n],setChecked(0));

	handle_instr_changed(0);
}

void
MppInstrumentTab :: handle_instr_rem()
{
	struct umidi20_event *event;
	struct umidi20_event *event_next;

	handle_instr_changed(0);

	mw->atomic_lock();

	for (unsigned int x = 0; x != MPP_MAX_TRACKS; x++) {
		if (mw->track[x] == 0)
			continue;
	
		UMIDI20_QUEUE_FOREACH_SAFE(event, &mw->track[x]->queue, event_next) {
			if (umidi20_event_get_what(event) & UMIDI20_WHAT_CHANNEL) {
				uint8_t chan = umidi20_event_get_channel(event) & 0xF;
				if (mw->instr[chan].muted) {
					UMIDI20_IF_REMOVE(&mw->track[x]->queue, event);
					umidi20_event_free(event);
				}
			}
		}
	}

	mw->atomic_unlock();
}
