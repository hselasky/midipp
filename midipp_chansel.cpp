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

MppChanSelDiag :: MppChanSelDiag(QWidget *parent, int val, int have_any, int mask) :
	QDialog(parent), QGridLayout(this)
{
	MppButton *pmb;
	int x;

	value.value = val;
	value.parent = this;

	setWindowIcon(QIcon(MppIconFile));
	setWindowTitle(QDialog::tr("Select MIDI Channel"));

	for (x = 0; x != 16; x++) {
		pmb = new MppButton(MppChanName(x, have_any), x);
		QDialog :: connect(pmb, SIGNAL(released(int)), &value, SLOT(handle_released(int))); 
		addWidget(pmb, x / 4, x % 4, 1, 1);
		if ((mask >> x) & 1) {
			if (x == val)
				pmb->setFocus();
		} else {
			pmb->setDisabled(1);
		}
		butChannel[x] = pmb;
	}

	switch (have_any) {
	case MPP_CHAN_ANY:
		pmb = new MppButton(MppChanName(-1, have_any), -1);
		QDialog :: connect(pmb, SIGNAL(released(int)), &value, SLOT(handle_released(int))); 
		addWidget(pmb, 4, 0, 1, 1);
		if (val == -1)
			pmb->setFocus();
		break;
	case MPP_CHAN_NONE:
		pmb = new MppButton(MppChanName(-1, have_any), -1);
		QDialog :: connect(pmb, SIGNAL(released(int)), &value, SLOT(handle_released(int))); 
		addWidget(pmb, 4, 0, 1, 1);
		if (val == -1)
			pmb->setFocus();
		break;
	default:
		break;
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

MppChanSel :: MppChanSel(MppMainWindow *_mw, int val, int have_any) :
    QPushButton()
{
	channel = val;
	haveAny = have_any;
	mw = _mw;
	channelMask = 0xFFFFU;

	setText(MppChanName(val, haveAny));
	connect(this, SIGNAL(released()), this, SLOT(handle_released()));
}

void
MppChanSel :: setValue(int value)
{
	if (haveAny) {
		if (value < -1 || value > 15)
			value = -1;
	} else if (value < 0 || value > 15)
		value = 0;

	if (channel != value) {
		channel = value;
		setText(MppChanName(value, haveAny));
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
	MppChanSelDiag diag(this, channel, haveAny, channelMask);

	if (diag.exec() == QDialog::Accepted)
		setValue(diag.value.value);
}
