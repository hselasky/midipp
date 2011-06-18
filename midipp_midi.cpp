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

MppMidi :: MppMidi(uint32_t mask)
	: QDialog()
{
	char line_buf[64];
	uint32_t x;
	uint32_t y;

	chan_mask = mask;
	chan_merge = 0;
	have_strings = 0;

	gl = new QGridLayout(this);

	setWindowTitle(tr("MIDI import"));

	lbl_import = new QLabel(tr("Select tracks\nto import"));
	lbl_import->setAlignment(Qt::AlignCenter);

	but_done = new QPushButton(tr("Done"));
	connect(but_done, SIGNAL(released()), this, SLOT(handle_done()));

	y = 0;

	gl->addWidget(lbl_import,y,1,1,1);

	y++;

	for (x = 0; x != MIDI_MAX_TRACKS; x++) {
		if (mask & (1 << x)) {
			snprintf(line_buf, sizeof(line_buf),
			    "Track%d: <>", (int)x);

			cbx_import[x] = new QCheckBox();
			cbx_import[x]->setChecked(1);

			lbl_info[x] = new QLabel(tr(line_buf));

			gl->addWidget(cbx_import[x],y,1,1,1,Qt::AlignHCenter|Qt::AlignVCenter);
			gl->addWidget(lbl_info[x],y,0,1,1);

			y++;
		}
	}

	cbx_single_track = new QCheckBox();
	lbl_single_track = new QLabel(tr("Output like a single track"));

	gl->addWidget(cbx_single_track,y,1,1,1,Qt::AlignHCenter|Qt::AlignVCenter);
	gl->addWidget(lbl_single_track,y,0,1,1);

	y++;

	cbx_have_strings = new QCheckBox();
	lbl_have_strings = new QLabel(tr("Add tempo strings"));

	gl->addWidget(cbx_have_strings,y,1,1,1,Qt::AlignHCenter|Qt::AlignVCenter);
	gl->addWidget(lbl_have_strings,y,0,1,1);

	y++;

	gl->addWidget(but_done,y,1,1,1);

	exec();

	for (x = y = 0; x != MIDI_MAX_TRACKS; x++) {
		if (mask & (1 << x)) {
			if (cbx_import[x]->isChecked()) {
				chan_mask |= (1 << x);
			} else {
				chan_mask &= ~(1 << x);
			}
		}
	}

	if (cbx_single_track->isChecked())
		chan_merge = 1;

	if (cbx_have_strings->isChecked())
		have_strings = 1;
}

MppMidi :: ~MppMidi()
{


}

void
MppMidi :: handle_done()
{
	accept();
}
