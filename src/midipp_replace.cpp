/*-
 * Copyright (c) 2012 Hans Petter Selasky
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

#include "midipp_decode.h"
#include "midipp_mainwindow.h"
#include "midipp_replace.h"

MppReplace :: MppReplace(MppMainWindow *_mw, MppScoreMain *_sm, QString _match,
    QString _replace) : MppDialog(_mw, QObject::tr("Replace text dialog"))
{
	MppDialog *d = this;

	mw = _mw;
	sm = _sm;

	lbl_replace = new QLabel(tr("Replace: "));
	lbl_with = new QLabel(tr("With: "));

	led_replace = new QLineEdit(_match);
	led_replace->setMaxLength(1024);

	led_with = new QLineEdit(_replace);
	led_with->setMaxLength(1024);

	but_ok = new QPushButton(tr("Ok"));
	but_cancel = new QPushButton(tr("Cancel"));
	but_edit = new QPushButton(tr("Insert Chord"));

	d->connect(but_ok, SIGNAL(released()), d, SLOT(accept()));
	d->connect(but_cancel, SIGNAL(released()), d, SLOT(reject()));
	connect(but_edit, SIGNAL(released()), this, SLOT(edit()));

	addWidget(lbl_replace, 0,0,1,1, Qt::AlignCenter);
	addWidget(led_replace, 0,1,1,3);

	addWidget(lbl_with, 1,0,1,1, Qt::AlignCenter);
	addWidget(led_with, 1,1,1,3);

	addWidget(but_edit, 2,1,1,1, Qt::AlignCenter);
	addWidget(but_ok, 2,2,1,1, Qt::AlignCenter);
	addWidget(but_cancel, 2,3,1,1, Qt::AlignCenter);
}

void
MppReplace :: accept(void)
{
	match = led_replace->text();
	replace = led_with->text();

	if (match.isEmpty())
		MppDialog :: reject();
	else
		MppDialog :: accept();
}

void
MppReplace :: edit(void)
{
	led_with->setText(mw->tab_chord_gl->getText());
}
