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

#include <midipp_decode.h>
#include <midipp_mainwindow.h>
#include <midipp_scores.h>
#include <midipp_import.h>
#include <fcntl.h>
#include <stdio.h>

static uint8_t
midipp_import_flush(struct midipp_import *ps)
{
	QString out;

	uint16_t x;
	uint16_t ai;
	uint16_t bi;
	
	out += QString("S\"");

	for (ai = bi = x = 0; x != MIDIPP_IMPORT_LB; x++) {
		if (ai < ps->n_word[0] && ps->index != 0) {
			if (ps->d_word[0][ai].off == x) {
				out += QString(".(") +
				    QString(ps->d_word[0][ai].name) +
				    QString(")");
				ai++;
			}
		}
	retry_word:
		if (bi < ps->n_word[ps->index]) {
			uint16_t off;
			off = ps->d_word[ps->index][bi].off;
			if (x >= off) {
				char ch;
				ch = ps->d_word[ps->index][bi].name[x - off];
				if (ch == 0) {
					bi++;
					goto retry_word;
				} else {
					out += QString(ch);
				}
			} else {
				out += QString(" ");
			}
		} else {
			out += QString(" ");
		}

		if (((ps->index == 0) || (ai == ps->n_word[0])) &&
		    bi == ps->n_word[ps->index])
			break;
	}
	out += QString("\"\n\n");

	if (ps->index != 0) {
		for (ai = 0; ai < ps->n_word[0]; ai++) {

			ps->dlg->setText(ps->d_word[0][ai].name);

			if(ps->dlg->exec() != QDialog::Accepted)
				return (1);

			out += ps->sm->mainWindow->led_config_insert->text() +
			    ps->dlg->getText() + QString("\n");
		}
	}

	out += QString("\n");

	QTextCursor cursor(ps->sm->editWidget->textCursor());
	cursor.beginEditBlock();
	cursor.insertText(out);
	cursor.endEditBlock();

	return (0);
}

static uint8_t
midipp_import_parse(struct midipp_import *ps)
{
	uint16_t n_word;
	uint16_t n_off;
	uint16_t n_cof;
	uint16_t x;
	uint16_t y;
	uint8_t out[MPP_MAX_VAR_OFF];

	n_word = 0;
	n_off = 0;

	while (1) {

		if (ps->line_buffer[n_off] == '\0')
			break;

		if (ps->line_buffer[n_off] != ' ') {

			ps->d_word[ps->index][n_word].off = n_off;

			n_cof = 0;
			while (ps->line_buffer[n_off] != ' ' &&
			       ps->line_buffer[n_off] != '\0') {
				ps->d_word[ps->index][n_word].name[n_cof] = ps->line_buffer[n_off];
				n_off++;
				n_cof++;
				if (n_cof == 31)
					break;
			}
			ps->d_word[ps->index][n_word].name[n_cof] = 0;
			n_word ++;
			if (n_word == (MIDIPP_IMPORT_MW-1))
				break;
			continue;
		}

		n_off++;
	}
	ps->n_word[ps->index] = n_word;

	if (ps->index == 0) {
		for (y = x = 0; x != n_word; x++) {
			y += mpp_parse_chord(ps->d_word[0][x].name, C4, 0, out);
		}
		if (y == 0 && n_word != 0) {
			/* get one more line */
			ps->index = 1;
			ps->n_word[1] = 0;
		} else {
			if (midipp_import_flush(ps))
				return (1);
		}
	} else {
		if (midipp_import_flush(ps))
			return (1);

		/* reset - get more chords */
		ps->index = 0;
	}
	return (0);
}

uint8_t
midipp_import(QString str, struct midipp_import *ps, MppScoreMain *sm)
{
	MppDecode dlg(sm->mainWindow, sm->mainWindow, 0);
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

	while ((ch = *ptr_curr) != 0) {
		ptr_curr++;

		if (ch == '\r')
			continue;

		/* remove formatting characters */
		if (ch == '(')
			ch = '[';
		if (ch == ')')
			ch = ']';
		if (ch == '"' || ch == '.')
			ch = ' ';

		/* expand tabs to 8 spaces */
		if (ch == '\t') {
			int n;
			ch = ' ';
			for (n = 0; n != 7; n++) {
				if (off != (MIDIPP_IMPORT_LB-1)) {
					ps->line_buffer[off] = ch;
					off++;
				}
			}
		}

		ps->line_buffer[off] = ch;
		if (ch == '\n' || ch == '\0' || off == (MIDIPP_IMPORT_LB-1)) {
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

 done:
	free(ptr);

	return (0);
}

MppImportTab :: MppImportTab(MppMainWindow *parent)
{
	mainWindow = parent;

	editWidget = new QPlainTextEdit();
	editWidget->setFont(font_fixed);
	editWidget->setLineWrapMode(QPlainTextEdit::NoWrap);
	editWidget->setPlainText(tr(
	    "Example song:" "\n\n"
	    "C  G  Am" "\n"
	    "Welcome!" "\n"));

	butImportFileNew = new QPushButton(tr("New"));
	butImportFileOpen = new QPushButton(tr("Open"));
	butImport = new QPushButton(tr("Import"));

	connect(butImportFileNew, SIGNAL(pressed()), this, SLOT(handleImportNew()));
	connect(butImportFileOpen, SIGNAL(pressed()), this, SLOT(handleImportOpen()));
	connect(butImport, SIGNAL(pressed()), this, SLOT(handleImport()));
}

MppImportTab :: ~MppImportTab()
{

}

void
MppImportTab :: handleImportNew()
{
	editWidget->setPlainText(QString());
	mainWindow->lbl_file_status->setText(QString());
}

void
MppImportTab :: handleImportOpen()
{
	QFileDialog *diag = 
	  new QFileDialog(mainWindow, tr("Select Chord Tabular File"), 
		QString(), QString("Chord Tabular File (*.txt; *.TXT)"));
	QString scores;
	QString status;

	diag->setAcceptMode(QFileDialog::AcceptOpen);
	diag->setFileMode(QFileDialog::ExistingFile);

	if (diag->exec()) {

		handleImportNew();

		scores = MppReadFile(diag->selectedFiles()[0], &status);

		editWidget->setPlainText(scores);

		mainWindow->lbl_file_status->setText(status);
	}
	delete diag;
}

void
MppImportTab :: handleImport()
{
	struct midipp_import ps;

	midipp_import(editWidget->toPlainText(), &ps, mainWindow->currScoreMain());
}
