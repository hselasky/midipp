/*-
 * Copyright (c) 2018-2022 Hans Petter Selasky. All rights reserved.
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

#include "midipp_devsel.h"
#include "midipp_button.h"

MppDevSelDiag :: MppDevSelDiag(QWidget *_parent, int val, int have_any) :
	QDialog(_parent), QGridLayout(this)
{
	MppButton *pmb;
	int x;

	value.value = val;
	value.parent = this;

	setWindowIcon(QIcon(MppIconFile));
	setWindowTitle(QDialog::tr("Select Device Number"));

	for (x = 0; x != MPP_MAX_DEVS; x++) {
		pmb = new MppButton(MppDevName(x, have_any), x);
		QDialog :: connect(pmb, SIGNAL(released(int)), &value, SLOT(handle_released(int))); 
		addWidget(pmb, x / 4, x % 4, 1, 1);
		if (x == val)
			pmb->setFocus();
	}

	x = (x + 3) / 4;

	switch (have_any) {
	case MPP_DEV_ALL:
		pmb = new MppButton(MppDevName(-1, have_any), -1);
		QDialog :: connect(pmb, SIGNAL(released(int)), &value, SLOT(handle_released(int))); 
		addWidget(pmb, x, 0, 1, 1);
		if (val == -1)
			pmb->setFocus();
		break;
	case MPP_DEV_NONE:
		pmb = new MppButton(MppDevName(-1, have_any), -1);
		QDialog :: connect(pmb, SIGNAL(released(int)), &value, SLOT(handle_released(int))); 
		addWidget(pmb, x, 0, 1, 1);
		if (val == -1)
			pmb->setFocus();
		break;
	default:
		break;
	}

	pmb = new MppButton(QDialog :: tr("Cancel"), MPP_MAX_DEVS + 1);
	QDialog :: connect(pmb, SIGNAL(released(int)), &value, SLOT(handle_released(int))); 
	addWidget(pmb, x, 2, 1, 2);
}

MppDevSelDiag :: ~MppDevSelDiag()
{
}

void
MppDevSelDiagValue :: handle_released(int id)
{
	switch (id) {
	case MPP_MAX_DEVS + 1:
		parent->reject();
		break;
	default:
		value = id;
		parent->accept();
		break;
	}
}

MppDevSel :: MppDevSel(int val, int have_any) :
    QPushButton()
{
	device = val;
	haveAny = have_any;

	setText(MppDevName(val, haveAny));
	connect(this, SIGNAL(released()), this, SLOT(handle_released()));
}

void
MppDevSel :: setValue(int value)
{
	if (haveAny) {
		if (value < -1 || value >= MPP_MAX_DEVS)
			value = -1;
	} else if (value < 0 || value >= MPP_MAX_DEVS)
		value = 0;

	if (device != value) {
		device = value;
		setText(MppDevName(value, haveAny));
		valueChanged(value);
	}
}

int
MppDevSel :: value()
{
	return (device);
}

MppDevSel :: ~MppDevSel()
{
}

void
MppDevSel :: handle_released()
{
	MppDevSelDiag diag(this, device, haveAny);

	if (diag.exec() == QDialog::Accepted)
		setValue(diag.value.value);
}
