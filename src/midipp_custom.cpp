/*-
 * Copyright (c) 2013-2019 Hans Petter Selasky
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

#include "midipp.h"

#include "midipp_mainwindow.h"
#include "midipp_custom.h"
#include "midipp_button.h"
#include "midipp_groupbox.h"
#include "midipp_devsel.h"

MppCustomTab :: MppCustomTab(MppMainWindow *_mw)
  : QWidget(*_mw)
{
	int n;

	mw = _mw;

	gl = new QGridLayout(this);

	gb = new MppGroupBox(tr("Send custom hex-formatted MIDI command(s)"));
	gl->addWidget(gb, 0,0,1,4);

	gb_dev_sel = new MppGroupBox(tr("Select output device(s)"));
	gl->addWidget(gb_dev_sel, 1,0,1,1);

	but_dev_sel = new MppDevSel(_mw, -1, MPP_DEV_ALL);
	gb_dev_sel->addWidget(but_dev_sel, 0,0,1,1);

	for (n = 0; n != MPP_CUSTOM_MAX; n++) {
		custom[n].led_send = new QLineEdit(QString("FF FF"));
		custom[n].but_send = new MppButton(tr("SEND-%1").arg(n), n);

		gb->addWidget(custom[n].led_send, n, 0, 1, 1);
		gb->addWidget(custom[n].but_send, n, 1, 1, 1);

		connect(custom[n].but_send, SIGNAL(released(int)), this, SLOT(handle_send_custom(int)));
	}

	gl->setRowStretch(2, 1);
	gl->setColumnStretch(1, 1);
}

void
MppCustomTab :: handle_send_custom(int which)
{
	QString str = custom[which].led_send->text();
	uint8_t buf[str.size()];
	uint8_t trig;
	char ch;
	int dev;
 	int n;
	int x;
	int y;

	/* parse MIDI string */

	memset(buf, 0, sizeof(buf));

	for (x = y = 0; x != str.size(); x++) {
		ch = str[x].toLatin1();
		if (ch >= '0' && ch <= '9') {
			buf[y / 2] <<= 4;
			buf[y / 2] |= (ch - '0');
			y++;
		} else if (ch >= 'a' && ch <= 'f') {
			buf[y / 2] <<= 4;
			buf[y / 2] |= (ch - 'a' + 10);
			y++;
		} else if (ch >= 'A' && ch <= 'F') {
			buf[y / 2] <<= 4;
			buf[y / 2] |= (ch - 'A' + 10);
			y++;
		} else if (ch == ' ' || ch == '\t' || ch == '\n') {
			if (y & 1)
				y++;
		}
	}

	if (y & 1)
		y++;

	y /= 2;

	if (y == 0)
		return;

	/* get device number */
	dev = but_dev_sel->value();
	
	/* send MIDI data */

	mw->atomic_lock();

	trig = mw->midiTriggered;
	mw->midiTriggered = 1;

	for (n = 0; n != MPP_MAGIC_DEVNO; n++) {
		if (dev != -1 && dev != n)
			continue;
		if (mw->check_play(0, 0, 0, n) == 0)
			continue;
		mid_add_raw(&mw->mid_data, buf, y, 0);
	}
	mw->midiTriggered = trig;

	mw->atomic_unlock();
}

