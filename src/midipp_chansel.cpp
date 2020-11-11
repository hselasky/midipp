/*-
 * Copyright (c) 2014-2019 Hans Petter Selasky. All rights reserved.
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

#include "midipp_chansel.h"
#include "midipp_button.h"
#include "midipp_mainwindow.h"

MppChanSelDiag :: MppChanSelDiag(QWidget *parent, int val, int mask_any, int mask_chan) :
	QDialog(parent), QGridLayout(this)
{
	MppButton *pmb;
	int x;
	int y;

	value.value = val;
	value.parent = this;

	setWindowIcon(QIcon(MppIconFile));
	setWindowTitle(QDialog::tr("Select MIDI Channel"));

	for (x = 0; x != 16; x++) {
		pmb = new MppButton(MppChanName(x), x);
		QDialog :: connect(pmb, SIGNAL(released(int)), &value, SLOT(handle_released(int))); 
		addWidget(pmb, x / 4, x % 4, 1, 1);
		if ((mask_chan >> x) & 1) {
			if (x == val)
				pmb->setFocus();
		} else {
			pmb->setDisabled(1);
		}
		butChannel[x] = pmb;
	}

	for (x = y = 0; x != 16; x++) {
		if (~(mask_any >> x) & 1)
			continue;

		switch (-x) {
		case MPP_CHAN_ANY:
			pmb = new MppButton(MppChanName(-x), -x);
			QDialog :: connect(pmb, SIGNAL(released(int)), &value, SLOT(handle_released(int))); 
			addWidget(pmb, 4, y++, 1, 1);
			if (val == -x)
				pmb->setFocus();
			break;
		case MPP_CHAN_NONE:
			pmb = new MppButton(MppChanName(-x), -x);
			QDialog :: connect(pmb, SIGNAL(released(int)), &value, SLOT(handle_released(int))); 
			addWidget(pmb, 4, y++, 1, 1);
			if (val == -x)
				pmb->setFocus();
			break;
		case MPP_CHAN_MPE:
			pmb = new MppButton(MppChanName(-x), -x);
			QDialog :: connect(pmb, SIGNAL(released(int)), &value, SLOT(handle_released(int))); 
			addWidget(pmb, 4, y++, 1, 1);
			if (val == -x)
				pmb->setFocus();
			break;
		default:
			break;
		}
	}

	pmb = new MppButton(QDialog :: tr("Cancel"), 17);
	QDialog :: connect(pmb, SIGNAL(released(int)), &value, SLOT(handle_released(int))); 
	addWidget(pmb, 4, 2, 1, 2);
}

MppChanSelDiag :: ~MppChanSelDiag()
{
}

void
MppChanSelDiagValue :: handle_released(int id)
{
	switch (id) {
	case 17:
		parent->reject();
		break;
	default:
		value = id;
		parent->accept();
		break;
	}
}

MppChanSel :: MppChanSel(MppMainWindow *_mw, int val, int mask_any) :
    QPushButton()
{
	channel = val;
	maskAny = mask_any;
	mw = _mw;
	channelMask = 0xFFFFU;

	setText(MppChanName(val));
	connect(this, SIGNAL(released()), this, SLOT(handle_released()));
}

void
MppChanSel :: setValue(int value)
{
	/* sanity checks first */
	if (value > 15)
		value = 15;
	else if (value < -15)
		value = 0;
	else if (value < 0 && ((maskAny >> -value) & 1) == 0)
		value = 0;

	if (channel != value) {
		channel = value;
		setText(MppChanName(value));
		valueChanged(value);
	}
}

void
MppChanSel :: setChannelMask(uint16_t mask)
{
	channelMask = mask;
}

int
MppChanSel :: value()
{
	return (channel);
}

MppChanSel :: ~MppChanSel()
{
}

void
MppChanSel :: handle_released()
{
	MppChanSelDiag diag(this, channel, maskAny, channelMask);

	if (diag.exec() == QDialog::Accepted)
		setValue(diag.value.value);
}
