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

#ifndef _MIDIPP_DEVSEL_H_
#define	_MIDIPP_DEVSEL_H_

#include "midipp_dialog.h"

class MppDevSelDiagBase : public QObject
{
	Q_OBJECT
public:
	int value;

public slots:
	void handle_released(int);
};

class MppDevSelDiag : public MppDialog, public QGridLayout, public MppDevSelDiagBase
{
public:
	MppDevSelDiag(MppMainWindow *, int, int);
};

class MppDevSel : public QPushButton
{
	Q_OBJECT
public:
	MppDevSel(MppMainWindow *, int, int);

	void setValue(int);
	int value();
private:
	MppMainWindow *mw;
	int device;
	int haveAny;

public slots:
	void handle_released();
signals:
	void valueChanged(int);
};

#endif		/* _MIDIPP_DEVSEL_H_ */
