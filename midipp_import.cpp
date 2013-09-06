/*-
 * Copyright (c) 2010,2012-2013 Hans Petter Selasky. All rights reserved.
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
#include "midipp_scores.h"
#include "midipp_groupbox.h"
#include "midipp_import.h"

static uint8_t
midipp_import_flush(struct midipp_import *ps, int i_txt, int i_score)
{
	QByteArray out;
	QByteArray scs;

	uint16_t x;
	uint16_t ai;
	uint16_t bi;
	uint16_t off;

	uint8_t any;
	
	out += QByteArray("S\"");

	for (ai = bi = x = 0; x != MIDIPP_IMPORT_LB; x++) {

		any = 0;

		if (i_score > -1 && ai < ps->n_word[i_score]) {

			any = 1;

			if (ps->d_word[i_score][ai].off == x) {
				const char *ptr = ps->d_word[i_score][ai].name;

				if (mpp_find_chord(ptr, NULL, NULL, NULL) == 0) {
					out += QByteArray(".(") + QByteArray(ptr) +
					    QByteArray(")");

					ps->dlg->setText(ptr);

					scs += ps->sm->mainWindow->
					    led_config_insert->text().toUtf8() +
					    ps->dlg->getText().toUtf8() +
					    QByteArray("\n");
				}
				ai++;
			}
		}
	next_word:
		if (i_txt > -1 && bi < ps->n_word[i_txt]) {

			off = ps->d_word[i_txt][bi].off;

			if (x >= off) {
				char ch[2];
				ch[0] = ps->d_word[i_txt][bi].name[x - off];
				ch[1] = 0;
				if (ch[0] == 0) {
					bi++;
					goto next_word;
				} else {
					out += QByteArray(ch);
				}
			} else {
				if (!any)
					break;

				out += QByteArray(" ");
			}
		} else {
			if (!any)
				break;

			out += QByteArray(" ");
		}
	}
	out += QByteArray("\"\n\n") + scs + QByteArray("\n");

	QTextCursor cursor(ps->sm->editWidget->textCursor());
	cursor.beginEditBlock();
	cursor.insertText(QString::fromUtf8(out));
	cursor.endEditBlock();

	return (0);
}

static uint8_t
midipp_import_is_chord(char c)
{
	if (c >= 'A' && c <= 'H')
		return (1);
	if (c >= '0' && c <= '9')
		return (1);
	if (c >= 'a' && c <= 'z')
		return (1);
	if (c == '+')
		return (1);
	if (c == '/')
		return (1);
	if (c == '#')
		return (1);
	if (c == ' ')
		return (2);
	return (0);
}

static uint8_t
midipp_import_parse(struct midipp_import *ps)
{
	uint16_t n_word;
	uint16_t n_off;
	uint16_t n_cof;
	uint8_t state;
	uint8_t next;
	uint8_t nchord;
	uint8_t fchord;
	int c;

	n_word = 0;
	n_off = 0;
	n_cof = 0;
	state = midipp_import_is_chord(ps->line_buffer[0]);
	nchord = (state == 1) ? 1 : 0;
	fchord = 0;

	while (1) {
		c = ps->line_buffer[n_off];

		next = midipp_import_is_chord(c);
		if (next != state || n_cof == 31 || c == 0) {

			ps->d_word[ps->index][n_word].name[n_cof] = 0;
			ps->d_word[ps->index][n_word].off = n_off - n_cof;

			if (state == 1 &&
			    mpp_find_chord(ps->d_word[ps->index][n_word].name,
			    NULL, NULL, NULL) == 0) {
				fchord++;
			}

			n_word ++;
			n_cof = 0;

			if (n_word == (MIDIPP_IMPORT_MW - 1) || c == 0)
				break;

			state = next;
			if (state == 1)
				nchord++;
		}
		ps->d_word[ps->index][n_word].name[n_cof] = c;
		n_off ++;
		n_cof ++;
	}

	ps->n_word[ps->index] = n_word;
	ps->d_chords[ps->index] = ((nchord != 0) && (nchord == fchord));

	if (ps->load_more != 0) {
		/* load another line */
		ps->index ^= 1;
		ps->load_more--;
		return (0);
	}

	c = 0;
	if (ps->d_chords[0])
		c |= 1;
	if (ps->d_chords[1])
		c |= 2;

	switch (c) {
	case 0:
		if (ps->index == 0) {
			if (midipp_import_flush(ps, 1, -1))
				return (1);
		} else {
			if (midipp_import_flush(ps, 0, -1))
				return (1);
		}
		break;
	case 1:
		if (ps->index == 0) {
			if (midipp_import_flush(ps, 1, -1))
				return (1);
		} else {
			if (midipp_import_flush(ps, 1, 0))
				return (1);
			ps->load_more = 1;
		}
		break;
	case 2:
		if (ps->index == 0) {
			if (midipp_import_flush(ps, 0, 1))
				return (1);
			ps->load_more = 1;
		} else {
			if (midipp_import_flush(ps, 0, -1))
				return (1);
		}
		break;
	default:
		if (ps->index == 0) {
			if (midipp_import_flush(ps, -1, 1))
				return (1);
		} else {
			if (midipp_import_flush(ps, -1, 0))
				return (1);
		}
		break;
	}
	ps->index ^= 1;
	return (0);
}

uint8_t
midipp_import(QString str, struct midipp_import *ps, MppScoreMain *sm)
{
	MppDecode dlg(sm->mainWindow, sm, 0);
	char ch;
	int off;
	char *ptr;
	char *ptr_curr;

	ptr = MppQStringToAscii(str);
	if (ptr == NULL)
		return (1);

	ptr_curr = ptr;

	off = 0;

	memset(ps, 0, sizeof(*ps));

	ps->sm = sm;
	ps->dlg = &dlg;
	ps->load_more = 1;

	while ((ch = *ptr_curr) != 0) {
		ptr_curr++;

		if (ch == '\r')
			continue;

		/* remove formatting characters */
		if (ch == '(')
			ch = '[';
		if (ch == ')')
			ch = ']';
		if (ch == '"')
			ch = '\'';
		if (ch == '.')
			ch = ' ';

		/* expand tabs to 8 spaces */
		if (ch == '\t') {
			int n;
			ch = ' ';
			for (n = 0; n != 7; n++) {
				if (off != (MIDIPP_IMPORT_LB - 1)) {
					ps->line_buffer[off] = ch;
					off++;
				}
			}
		}

		ps->line_buffer[off] = ch;
		if (ch == '\n' || ch == '\0' || off == (MIDIPP_IMPORT_LB - 1)) {
			ps->line_buffer[off] = 0;
			if (midipp_import_parse(ps))
				goto done;
			off = 0;
		} else {
			off++;
		}
	}

	ps->line_buffer[off] = 0;
	midipp_import_parse(ps);

	if (off != 0 && ps->load_more == 0) {
		ps->line_buffer[0] = 0;
		midipp_import_parse(ps);
	}

 done:
	free(ptr);

	return (0);
}

MppImportTab :: MppImportTab(MppMainWindow *parent)
{
	mainWindow = parent;

	editWidget = new QPlainTextEdit();
	editWidget->setFont(mainWindow->editFont);
	editWidget->setLineWrapMode(QPlainTextEdit::NoWrap);
	editWidget->setPlainText(tr(
	    "Example song:" "\n\n"
	    "C  G  Am" "\n"
	    "Welcome!" "\n"));

	butImportFileNew = new QPushButton(tr("New"));
	butImportFileOpen = new QPushButton(tr("Open"));
	butImportFileSaveAs = new QPushButton(tr("Save As"));
	butImport = new QPushButton();

	gbImport = new MppGroupBox(tr("Lyrics"));
	gbImport->addWidget(butImportFileNew, 0, 0, 1, 1);
	gbImport->addWidget(butImportFileOpen, 1, 0, 1, 1);
	gbImport->addWidget(butImportFileSaveAs, 2, 0, 1, 1);
	gbImport->addWidget(butImport, 3, 0, 1, 1);

	connect(butImportFileNew, SIGNAL(released()), this, SLOT(handleImportNew()));
	connect(butImportFileOpen, SIGNAL(released()), this, SLOT(handleImportOpen()));
	connect(butImportFileSaveAs, SIGNAL(released()), this, SLOT(handleImportSaveAs()));
	connect(butImport, SIGNAL(released()), this, SLOT(handleImport()));
}

MppImportTab :: ~MppImportTab()
{

}

void
MppImportTab :: handleImportNew()
{
	editWidget->setPlainText(QString());

	mainWindow->handle_make_tab_visible(editWidget);
}

void
MppImportTab :: handleImportOpen()
{
	QFileDialog *diag = 
	  new QFileDialog(mainWindow, tr("Select Chord Tabular File"), 
		MppHomeDirTxt,
		QString("Chord Tabular File (*.txt; *.TXT)"));
	QString scores;

	diag->setAcceptMode(QFileDialog::AcceptOpen);
	diag->setFileMode(QFileDialog::ExistingFile);

	if (diag->exec()) {

		MppHomeDirTxt = diag->directory().path();

		handleImportNew();

		scores = MppReadFile(diag->selectedFiles()[0]);

		editWidget->setPlainText(scores);
	}
	delete diag;
}

void
MppImportTab :: handleImportSaveAs()
{
	QFileDialog *diag = 
	  new QFileDialog(mainWindow, tr("Select Chord Tabular File"), 
		MppHomeDirTxt,
		QString("Score File (*.txt *.TXT)"));

	diag->setAcceptMode(QFileDialog::AcceptSave);
	diag->setFileMode(QFileDialog::AnyFile);
	diag->setDefaultSuffix(QString("txt"));

	if (diag->exec()) {
		MppHomeDirTxt = diag->directory().path();
		MppWriteFile(diag->selectedFiles()[0], editWidget->toPlainText());
	}

	delete diag;
}

void
MppImportTab :: handleImport()
{
	struct midipp_import ps;

	midipp_import(editWidget->toPlainText(), &ps, mainWindow->currScoreMain());

	mainWindow->handle_compile();
}
