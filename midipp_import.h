/*-
 * Copyright (c) 2010 Hans Petter Selasky. All rights reserved.
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

#ifndef _MIDIPP_IMPORT_H_
#define	_MIDIPP_IMPORT_H_

#include "midipp.h"

#define	MIDIPP_IMPORT_MW	64

class midipp_word {
public:
	int off;
	QString name;
};

class midipp_import {
public:
	QString line_buffer;

	midipp_word d_word[2][MIDIPP_IMPORT_MW];
	uint8_t d_chords[2];

	MppScoreMain *sm;

	int n_word[2];
	int max_off;

	uint8_t index;
	uint8_t load_more;
};

class MppImportTab : public QObject
{
	Q_OBJECT;
public:

	MppImportTab(MppMainWindow *parent);
	~MppImportTab();

	MppMainWindow *mainWindow;

	QPlainTextEdit *editWidget;
	QPushButton *butImportFileNew;
	QPushButton *butImportFileOpen;
	QPushButton *butImportFileSaveAs;
	MppButton *butImport[MPP_MAX_VIEWS];
	MppGroupBox *gbImport;

public slots:

	void handleImportNew();
	void handleImportOpen();
	void handleImportSaveAs();
	void handleImport(int);
};

extern uint8_t midipp_import(QString str, class midipp_import *ps, MppScoreMain *sm);

#endif		/* _MIDIPP_IMPORT_H_ */
