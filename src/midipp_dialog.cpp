/*-
 * Copyright (c) 2022 Hans Petter Selasky. All rights reserved.
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

#include "midipp_dialog.h"
#include "midipp_mainwindow.h"

MppDialog :: MppDialog(MppMainWindow *_mw, const QString &title) :
    QGroupBox(title, _qw = new QWidget(_mw)), _gl(new QGridLayout(_qw))
{
	mw = _mw;
	_result = -1;

	_qw->hide();

	_gl->setRowStretch(0,1);
	_gl->setColumnStretch(0,1);
	_gl->setRowStretch(2,1);
	_gl->setColumnStretch(2,1);
	_gl->addWidget(this, 1,1,1,1);
}

MppDialog :: ~MppDialog()
{
	setParent(0);
	delete _qw;
}

int
MppDialog :: exec()
{
	QWidget *old = mw->currentWidget();

	mw->addWidget(_qw);
	mw->setCurrentWidget(_qw);
	eventLoop.exec(QEventLoop::DialogExec);
	mw->setCurrentWidget(old);
	mw->removeWidget(_qw);

	_qw->hide();

	return (_result);
}

void
MppDialog :: accept()
{
	_result = Accepted;
	eventLoop.exit();
}

void
MppDialog :: reject()
{
	_result = Rejected;
	eventLoop.exit();
}
