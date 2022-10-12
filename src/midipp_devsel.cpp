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

#include "midipp_mainwindow.h"
#include "midipp_devsel.h"
#include "midipp_button.h"

MppDevSelDiag :: MppDevSelDiag(MppMainWindow *_mw, int val, int have_any) :
    MppDialog(_mw, QObject::tr("Select device number")), QGridLayout(this)
{
	MppDevSelDiagBase *d = this;
	MppDialog *dlg = this;
	MppButton *pmb;
	int x;

	value = val;

	for (x = 0; x != MPP_MAX_DEVS; x++) {
		pmb = new MppButton(MppDevName(x, have_any), x);
		d->connect(pmb, SIGNAL(released(int)), d, SLOT(handle_released(int)));
		dlg->connect(pmb, SIGNAL(released(int)), dlg, SLOT(accept()));
		addWidget(pmb, x / 4, x % 4, 1, 1);
		if (x == val)
			pmb->setFocus();
	}

	x = (x + 3) / 4;

	switch (have_any) {
	case MPP_DEV_ALL:
		pmb = new MppButton(MppDevName(-1, have_any), -1);
		d->connect(pmb, SIGNAL(released(int)), d, SLOT(handle_released(int)));
		dlg->connect(pmb, SIGNAL(released(int)), dlg, SLOT(accept()));
		addWidget(pmb, x, 0, 1, 1);
		if (val == -1)
			pmb->setFocus();
		x++;
		break;
	case MPP_DEV_NONE:
		pmb = new MppButton(MppDevName(-1, have_any), -1);
		d->connect(pmb, SIGNAL(released(int)), d, SLOT(handle_released(int)));
		dlg->connect(pmb, SIGNAL(released(int)), dlg, SLOT(accept()));
		addWidget(pmb, x, 0, 1, 1);
		if (val == -1)
			pmb->setFocus();
		x++;
		break;
	default:
		break;
	}

	pmb = new MppButton(QObject::tr("Done"), -1);
	dlg->connect(pmb, SIGNAL(released(int)), dlg, SLOT(reject()));
	addWidget(pmb, x, 0, 1, 4);

	setRowStretch(x + 1, 1);
	setColumnStretch(4, 1);
}

void
MppDevSelDiagBase :: handle_released(int id)
{
	value = id;
}

MppDevSel :: MppDevSel(MppMainWindow *_mw, int val, int have_any) :
    QPushButton()
{
	mw = _mw;
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

void
MppDevSel :: handle_released()
{
	MppDevSelDiag *diag = new MppDevSelDiag(mw, device, haveAny);

	if (diag->exec() == MppDialog::Accepted)
		setValue(diag->value);

	delete diag;
}
