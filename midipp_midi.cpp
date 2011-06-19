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

#include <midipp_midi.h>

MppMidi :: MppMidi(uint32_t _mask, uint32_t _flags, uint32_t _thres)
    : QDialog()
{
	char line_buf[64];
	uint32_t x;
	uint32_t y;
	uint32_t z;
	uint32_t t;
	uint32_t u;

	chan_mask = _mask;
	flags = _flags;
	thres = _thres;

	gl = new QGridLayout(this);

	setWindowTitle(tr("MIDI import"));

	lbl_import[0] = new QLabel(tr("Select tracks\nto import"));
	lbl_import[0]->setAlignment(Qt::AlignCenter);

	lbl_import[1] = new QLabel(tr("Select tracks\nto import"));
	lbl_import[1]->setAlignment(Qt::AlignCenter);

	but_done = new QPushButton(tr("Done"));
	connect(but_done, SIGNAL(released()), this, SLOT(handle_done()));

	but_set_all = new QPushButton(tr("All tracks"));
	connect(but_set_all, SIGNAL(released()), this, SLOT(handle_set_all_track()));

	but_clear_all = new QPushButton(tr("No tracks"));
	connect(but_clear_all, SIGNAL(released()), this, SLOT(handle_clear_all_track()));

	y = 0;

	gl->addWidget(lbl_import[0],y,1,1,1);
	gl->addWidget(lbl_import[1],y,3,1,1);

	y++;

	for (z = x = 0; x != MIDI_MAX_TRACKS; x++) {
		if (_mask & (1 << x))
			z++;
	}

	for (t = u = x = 0; x != MIDI_MAX_TRACKS; x++) {
		if (_mask & (1 << x)) {

			if (t >= ((z + 1) / 2)) {
				t = 0;
				u = 2;
			}

			snprintf(line_buf, sizeof(line_buf),
			    "Track%d: <%s>", (int)x, (x == 9) ? "Drums" : "");

			cbx_import[x] = new QCheckBox();

			lbl_info[x] = new QLabel(tr(line_buf));

			gl->addWidget(cbx_import[x],t+y,1+u,1,1,Qt::AlignHCenter|Qt::AlignVCenter);
			gl->addWidget(lbl_info[x],t+y,0+u,1,1,Qt::AlignLeft|Qt::AlignVCenter);

			t++;
		}
	}

	y += ((z + 1) / 2);

	cbx_single_track = new QCheckBox();
	lbl_single_track = new QLabel(tr("Output like a single track"));

	if (!(flags & MIDI_FLAG_MULTI_CHAN))
		cbx_single_track->setChecked(1);

	gl->addWidget(cbx_single_track,y,3,1,1,Qt::AlignHCenter|Qt::AlignVCenter);
	gl->addWidget(lbl_single_track,y,0,1,3,Qt::AlignRight|Qt::AlignVCenter);

	y++;

	cbx_have_strings = new QCheckBox();
	lbl_have_strings = new QLabel(tr("Add tempo strings"));

	if (flags & MIDI_FLAG_STRING)
		cbx_have_strings->setChecked(1);

	gl->addWidget(cbx_have_strings,y,3,1,1,Qt::AlignHCenter|Qt::AlignVCenter);
	gl->addWidget(lbl_have_strings,y,0,1,3,Qt::AlignRight|Qt::AlignVCenter);

	y++;

	cbx_have_duration = new QCheckBox();
	lbl_have_duration = new QLabel(tr("Add duration to score lines"));

	if (flags & MIDI_FLAG_DURATION)
		cbx_have_duration->setChecked(1);

	gl->addWidget(cbx_have_duration,y,3,1,1,Qt::AlignHCenter|Qt::AlignVCenter);
	gl->addWidget(lbl_have_duration,y,0,1,3,Qt::AlignRight|Qt::AlignVCenter);

	y++;

	lbl_parse_thres = new QLabel(tr("New scores line threshold"));
	spn_parse_thres = new QSpinBox();
	spn_parse_thres->setRange(0, 10000);
	spn_parse_thres->setValue(thres);
	spn_parse_thres->setSuffix(tr(" ms"));

	gl->addWidget(spn_parse_thres,y,3,1,1,Qt::AlignHCenter|Qt::AlignVCenter);
	gl->addWidget(lbl_parse_thres,y,0,1,3,Qt::AlignRight|Qt::AlignVCenter);

	y++;

	gl->addWidget(but_set_all,y,0,1,1);
	gl->addWidget(but_clear_all,y,1,1,1);
	gl->addWidget(but_done,y,3,1,1);

	exec();

	for (x = y = 0; x != MIDI_MAX_TRACKS; x++) {
		if (_mask & (1 << x)) {
			if (cbx_import[x]->isChecked()) {
				chan_mask |= (1 << x);
			} else {
				chan_mask &= ~(1 << x);
			}
		}
	}

	if (cbx_single_track->isChecked())
		flags &= ~MIDI_FLAG_MULTI_CHAN;
	else
		flags |= MIDI_FLAG_MULTI_CHAN;

	if (cbx_have_strings->isChecked())
		flags |= MIDI_FLAG_STRING;
	else
		flags &= ~MIDI_FLAG_STRING;

	if (cbx_have_duration->isChecked())
		flags |= MIDI_FLAG_DURATION;
	else
		flags &= ~MIDI_FLAG_DURATION;

	thres = spn_parse_thres->value();
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
MppMidi :: handle_clear_all_track()
{
	uint32_t x;

	for (x = 0; x != MIDI_MAX_TRACKS; x++) {
		if (chan_mask & (1 << x)) {
			cbx_import[x]->setChecked(0);
		}
	}
}
