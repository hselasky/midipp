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

#include "midipp_midi.h"
#include "midipp_checkbox.h"
#include "midipp_groupbox.h"

MppMidi :: MppMidi(uint32_t _mask, uint32_t _flags, uint32_t _thres)
    : QDialog()
{
	uint32_t x;
	uint32_t t;
	uint32_t u;

	chan_mask = _mask;
	flags = _flags;
	thres = _thres;

	gl = new QGridLayout(this);

	setWindowTitle(tr("MIDI import"));
	setWindowIcon(QIcon(QString(MPP_ICON_FILE)));

	gb_import = new MppGroupBox(tr("Select MIDI channel(s) to import"));

	but_done = new QPushButton(tr("Done"));
	connect(but_done, SIGNAL(released()), this, SLOT(handle_done()));

	but_set_all = new QPushButton(tr("All channels"));
	connect(but_set_all, SIGNAL(released()), this, SLOT(handle_set_all_track()));

	but_clear_all = new QPushButton(tr("No channels"));
	connect(but_clear_all, SIGNAL(released()), this, SLOT(handle_clear_all_track()));

	gl->addWidget(gb_import, 0, 0, 1, 4);

	for (x = 0; x != MIDI_MAX_TRACKS; x++) {
		const char *str;

		u = (x >= (MIDI_MAX_TRACKS / 2)) ? 2 : 0;
		t = x % (MIDI_MAX_TRACKS / 2);

		if (_mask & (1 << x)) {
			if (x == 9)
				str = "drums";
			else
				str = "valid";
		} else {
			str = "empty";
		}
		gb_import->addWidget(new QLabel(MppChanName(x,0) + QString(" is ") + QString(str)),
		    t,0+u,1,1,Qt::AlignLeft|Qt::AlignVCenter);

		cbx_import[x] = new MppCheckBox();
		connect(cbx_import[x], SIGNAL(stateChanged(int,int)), this, SLOT(handle_checkboxes()));
		gb_import->addWidget(cbx_import[x],t,1+u,1,1,Qt::AlignHCenter|Qt::AlignVCenter);
	}

	gb_import->setColumnStretch(1, 1);
	gb_import->setColumnStretch(3, 1);

	cbx_single_track = new MppCheckBox();

	gl->addWidget(cbx_single_track,1,3,1,1,Qt::AlignHCenter|Qt::AlignVCenter);
	gl->addWidget(new QLabel(tr("Add channel number to score lines")),1,0,1,3,Qt::AlignRight|Qt::AlignVCenter);

	cbx_have_duration = new MppCheckBox();

	if (flags & MIDI_FLAG_DURATION)
		cbx_have_duration->setChecked(1);

	gl->addWidget(cbx_have_duration,2,3,1,1,Qt::AlignHCenter|Qt::AlignVCenter);
	gl->addWidget(new QLabel(tr("Add autoplay timeout to score lines")),2,0,1,3,Qt::AlignRight|Qt::AlignVCenter);

	cbx_have_strings = new MppCheckBox();

	if (flags & MIDI_FLAG_STRING)
		cbx_have_strings->setChecked(1);

	gl->addWidget(cbx_have_strings,3,3,1,1,Qt::AlignHCenter|Qt::AlignVCenter);
	gl->addWidget(new QLabel(tr("Add separate tempo strings")),3,0,1,3,Qt::AlignRight|Qt::AlignVCenter);

	spn_parse_thres = new QSpinBox();
	spn_parse_thres->setRange(0, 10000);
	spn_parse_thres->setValue(thres);
	spn_parse_thres->setSuffix(tr(" ms"));

	gl->addWidget(new QLabel(tr("New scores line threshold")),4,0,1,3,Qt::AlignRight|Qt::AlignVCenter);
	gl->addWidget(spn_parse_thres,4,3,1,1);

	led_prefix = new QLineEdit();
	led_prefix->setMaxLength(256);

	gl->addWidget(new QLabel(tr("Line prefix")),5,0,1,3,Qt::AlignRight|Qt::AlignVCenter);
	gl->addWidget(led_prefix,5,3,1,1);

	gl->addWidget(but_set_all,6,0,1,1);
	gl->addWidget(but_clear_all,6,1,1,1);
	gl->addWidget(but_done,6,3,1,1);

	gl->setColumnStretch(2, 1);

	handle_checkboxes();

	exec();

	for (x = 0; x != MIDI_MAX_TRACKS; x++) {
		if (_mask & (1 << x)) {
			if (cbx_import[x]->isChecked()) {
				chan_mask |= (1 << x);
			} else {
				chan_mask &= ~(1 << x);
			}
		}
	}

	if (cbx_single_track->isChecked())
		flags |= MIDI_FLAG_MULTI_CHAN;
	else
		flags &= ~MIDI_FLAG_MULTI_CHAN;

	if (cbx_have_strings->isChecked())
		flags |= MIDI_FLAG_STRING;
	else
		flags &= ~MIDI_FLAG_STRING;

	if (cbx_have_duration->isChecked())
		flags |= MIDI_FLAG_DURATION;
	else
		flags &= ~MIDI_FLAG_DURATION;

	thres = spn_parse_thres->value();

	prefix = led_prefix->text().trimmed();
}

MppMidi :: ~MppMidi()
{


}

void
MppMidi :: handle_done()
{
	accept();
}

void
MppMidi :: handle_set_all_track()
{
	uint32_t x;

	for (x = 0; x != MIDI_MAX_TRACKS; x++) {
		if (chan_mask & (1 << x)) {
			cbx_import[x]->setChecked(1);
		}
	}
}

void
MppMidi :: handle_checkboxes()
{
	uint32_t x;
	uint32_t y;

	for (x = y = 0; x != MIDI_MAX_TRACKS; x++) {
		if (cbx_import[x]->isChecked())
			y++;
	}
	cbx_single_track->setChecked(y >= 2);
}

void
MppMidi :: handle_clear_all_track()
{
	uint32_t x;

	for (x = 0; x != MIDI_MAX_TRACKS; x++) {
		cbx_import[x]->setChecked(0);
	}
}
