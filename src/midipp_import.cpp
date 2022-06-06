/*-
 * Copyright (c) 2010-2022 Hans Petter Selasky. All rights reserved.
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

#include "midipp_chords.h"
#include "midipp_decode.h"
#include "midipp_mainwindow.h"
#include "midipp_scores.h"
#include "midipp_groupbox.h"
#include "midipp_import.h"
#include "midipp_button.h"
#include "midipp_spinbox.h"
#include "midipp_buttonmap.h"

static bool
midipp_import_find_chord(const QString &str)
{
	uint32_t rem;
	uint32_t bass;
	MppChord_t mask;

	MppStringToChordGeneric(mask, rem, bass, MPP_BAND_STEP_CHORD, str);

	return (mask.test(0) == 0);
}

static bool
midipp_import_is_chord(const QString &str)
{
	static const QString first_char("ABCDEFGH");

	if (str.size() < 1)
		return (false);

	/* Scan for valid characters first */
	for (int x = 0; x != first_char.size(); x++) {
		if (first_char[x] == str[0])
			return (midipp_import_find_chord(str) == 0);
	}
	return (false);
}

static uint8_t
midipp_import_flush(class midipp_import *ps, int i_txt, int i_score)
{
	QString out;
	QString lbl;

	int off;
	int x;
	int ai;
	int bi;

	uint8_t any;
	uint8_t output = 0;

	/* detect label marks like "LXXX" or [intro], in input text */
	if (i_txt > -1 && ps->n_words[i_txt] > 0) {
		QString &str = ps->d_word[i_txt][0].name;
		QString &end = ps->d_word[i_txt][ps->n_words[i_txt] - 1].name;

		if (str.size() > 1 && str[0] == QChar('L') && str[1].isDigit()) {
			/* import Lxx tag */
			int label = 0;
			for (bi = 1; bi != str.size(); bi++) {
				if (str[bi].isDigit() == 0)
					break;
				label *= 10;
				label += str[bi].digitValue();
			}

			/* output full text line */
			lbl += "S\"";
			for (ai = 0; ai != ps->n_words[i_txt]; ai++)
			    lbl += ps->d_word[i_txt][ai].name;
			lbl += "\"\n";
			lbl += QString("L%1:\n\n").arg(label);

			/* next label is current label added one */
			ps->n_label = label + 1;

			/* ignore current text line */
			i_txt = -1;
			output = 1;

		} else if (&str == &end && str.size() > 1 &&
			   str[0] == '[' && str[str.size() - 1] == ']' &&
			   midipp_import_is_chord(str.mid(1, str.size() - 2)) == 0) {
			/* import tag */

			/* update current string */
			str = QString("L%1 - ").arg(ps->n_label) + str.mid(1, str.size() - 2);

			/* output full text line */
			lbl += "S\"";
			lbl += str;
			lbl += "\"\n";
			lbl += QString("L%1:\n\n").arg(ps->n_label);

			/* advance label number */
			ps->n_label++;

			/* ignore current text line */
			i_txt = -1;
			output = 1;
		}
	}

	if (i_txt > -1 || i_score > -1) {
	    QString scs;

	    out += "S\"";

	    for (ai = bi = x = 0; x != ps->max_off; x++) {

		any = 0;

		if (i_score > -1 && ai < ps->n_words[i_score]) {

			any = 1;

			if (ps->d_word[i_score][ai].off == x) {
				const QString &chord = ps->d_word[i_score][ai].name;

				if (midipp_import_find_chord(chord) == 0) {
					MppDecodeTab *ptab = ps->sm->mainWindow->tab_chord_gl;

					output = 1;
					out += ".(";
					out += ps->d_word[i_score][ai].name;
					out += ")";

					ptab->setText(chord);
					ptab->handle_align(ps->sm->spnScoreFileAlign->value());
					ptab->handle_refresh();

					scs += ptab->getText() + "\n";
				}
				ai++;
			}
		}
	next_word:
		if (i_txt > -1 && bi < ps->n_words[i_txt]) {
			const QString &str = ps->d_word[i_txt][bi].name;

			off = ps->d_word[i_txt][bi].off;

			if (x >= off) {
				int y = x - off;

				if (y >= str.size()) {
					bi++;
					goto next_word;
				} else if (y == 0 && str.size() > 1 &&
				    str[0] == '[' && str[str.size() - 1] == ']') {
					/* Handle lyrics chord */
					const QString chord(str.mid(1, str.size() - 2));

					if (midipp_import_is_chord(chord) != 0) {
						MppDecodeTab *ptab = ps->sm->mainWindow->tab_chord_gl;

						output = 1;
						out += ".(";
						out += chord;
						out += ")";

						ptab->setText(chord);
						ptab->handle_align(ps->sm->spnScoreFileAlign->value());
						ptab->handle_refresh();

						scs += ptab->getText() + "\n";
						bi++;
						goto next_word;
					}
				} else if (y == 0 && str.size() > 1 &&
				    str[0] == '(' && str[str.size() - 1] == ')') {
					/* Handle lyrics comment */
					output = 1;
					scs += "V\"";
					scs += str;
					scs += "\"\n";
					bi++;
					goto next_word;
				}
				output = 1;
				out += str[y];
			} else {
				/* If there are any chords on top, we need to space. */
				if (any) {
					output = 1;
					if ((x + 1) != ps->max_off)
						out += " ";
				}
			}
		} else {
			/* If there are any chords on top, we need to space. */
			if (any) {
				output = 1;
				if ((x + 1) != ps->max_off)
					out += " ";
			}
		}
	    }
	    out += "\"\n\n";
	    out += scs;
	    out += "\n";
	}

	/* add label if any */
	out += lbl;

	if (output) {
		QTextCursor cursor(ps->sm->editWidget->textCursor());
		cursor.beginEditBlock();
		cursor.insertText(out);
		cursor.endEditBlock();
	}
	return (0);
}

static uint8_t
midipp_import_parse(class midipp_import *ps)
{
	int n_off;
	int n_word;
	int n_chord;
	int n_space;
	int temp;
	uint8_t flushword;
	QChar c;
	QChar lastch;
	QChar endch;

	n_word = 0;
	n_chord = 0;
	n_space = 0;
	flushword = 0;
	endch = '\0';
	lastch = '\0';

	/* reset all the "name" strings */
	for (n_off = 0; n_off != MIDIPP_IMPORT_MW; n_off++)
		ps->d_word[ps->index][n_off].name = QString();

	/*
	 * The supported syntax is:
	 *
	 * [Chord]
	 *
	 * [Tag]
	 *
	 * (One-line comments ....)
	 *
	 * Lyrics lyrics lyrics
	 *
	 * Chord       Chord
	 * Lyrics lyrics lyrics
	 *
	 * Lyrics [Chord]lyrics [Chord]lyrics
	 */
	for (n_off = 0; ; n_off++) {
		uint8_t is_done = (n_off == ps->line_buffer.size());
		uint8_t can_flush = ps->d_word[ps->index][n_word].name.size() != 0;

		if (is_done == 0) {
			c = ps->line_buffer[n_off];
			if (endch == '\0') {
				/* check for word delimiters */
				if (c == '[') {
					flushword = can_flush;
					endch = ']';
				} else if (c == '(') {
					flushword = can_flush;
					endch = ')';
				} else if (c.isSpace()) {
					flushword = (c != lastch || flushword == 2) ? can_flush : 0;
				} else {
					flushword = (lastch.isSpace() || flushword == 2) ? can_flush : 0;
				}
			} else if (c == endch) {
				/* make sure the end character gets appended to the current word */
				flushword = can_flush ? 2 : 0;
				endch = '\0';
			} else {
				/* keep on appending until the end character */
				flushword = 0;
			}
		} else {
			c = ' ';
			flushword = 0;
		}

		if (is_done != 0 || flushword == 1) {
			ps->d_word[ps->index][n_word].off = n_off -
			    ps->d_word[ps->index][n_word].name.size();

			if (midipp_import_is_chord(
			    ps->d_word[ps->index][n_word].name) != 0)
				n_chord++;

			n_word ++;
			if (is_done != 0 || n_word == MIDIPP_IMPORT_MW)
				break;
		}
		ps->d_word[ps->index][n_word].name += c;

		/* keep the last character */
		lastch = c;
	}
	if (n_off > ps->max_off)
		ps->max_off = n_off;
	/* count number of words containing spaces only */
	for (n_off = 0; n_off != n_word; n_off++) {
		if (ps->d_word[ps->index][n_off].name.size() > 0 &&
		    ps->d_word[ps->index][n_off].name[0].isSpace())
			n_space++;
	}
	ps->n_spaces[ps->index] = n_space;
	ps->n_words[ps->index] = n_word;
	ps->n_chords[ps->index] = n_chord;

	if (ps->load_more != 0) {
		/* load another line */
		ps->index ^= 1;
		ps->load_more = 0;
		return (0);
	}

	temp = 0;
	if (ps->n_chords[0])
		temp |= 1;
	if (ps->n_chords[1])
		temp |= 2;

	switch (temp) {
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
			/* both lines were consumed */
			ps->load_more = 1;
		}
		break;
	case 2:
		if (ps->index == 0) {
			if (midipp_import_flush(ps, 0, 1))
				return (1);
			/* both lines were consumed */
			ps->load_more = 1;
		} else {
			if (midipp_import_flush(ps, 0, -1))
				return (1);
		}
		break;
	default:
		/*
		 * If the text line contains only chords, print it as
		 * chords and not text:
		 */
		if ((ps->n_spaces[ps->index] + ps->n_chords[ps->index]) ==
		    ps->n_words[ps->index]) {
			if (ps->index == 0) {
				if (midipp_import_flush(ps, -1, 1))
					return (1);
			} else {
				if (midipp_import_flush(ps, -1, 0))
					return (1);
			}
		} else {
			if (ps->index == 0) {
				if (midipp_import_flush(ps, 0, 1))
					return (1);
			} else {
				if (midipp_import_flush(ps, 1, 0))
					return (1);
			}
			/* both lines were consumed */
			ps->load_more = 1;
		}
		break;
	}
	ps->index ^= 1;
	return (0);
}

static uint8_t
midipp_import(QString &str, class midipp_import *ps, MppScoreMain *sm, bool removeTabXML = true)
{
	QChar ch;
	int off;

	/* reset parse state */
	for (off = 0; off != MIDIPP_IMPORT_MW; off++) {
		ps->d_word[0][off].off = 0;
		ps->d_word[1][off].off = 0;
	}

	ps->sm = sm;
	ps->n_spaces[0] = 0;
	ps->n_spaces[1] = 0;
	ps->n_chords[0] = 0;
	ps->n_chords[1] = 0;
	ps->n_words[0] = 0;
	ps->n_words[1] = 0;
	ps->n_label = 0;
	ps->max_off = 0;
	ps->index = 0;
	ps->load_more = 1;

	if (removeTabXML) {
		int level = 0;
		int isTabXML = 0;

		for (off = 0; off != str.size(); off++) {
			if (str.size() > 4 &&
			    str[off] == '[' &&
			    str[off + 1] == 't' &&
			    str[off + 2] == 'a' &&
			    str[off + 3] == 'b' &&
			    str[off + 4] == ']') {
				isTabXML |= 1;
				level++;
			} else if (str.size() > 5 &&
			    str[off] == '[' &&
			    str[off + 1] == '/' &&
			    str[off + 2] == 't' &&
			    str[off + 3] == 'a' &&
			    str[off + 4] == 'b' &&
			    str[off + 5] == ']') {
				isTabXML |= 2;
				level--;
			} else if (str.size() > 2 &&
			    str[off] == '[' &&
			    str[off + 1] != '/') {
				level++;
			} else if (str.size() > 2 &&
			    str[off] == '[' &&
			    str[off + 1] == '/') {
				level--;
			}
		}
		if (isTabXML == 3 && level == 0) {
			int noff = 0;

			/* Remove all formatting tags */
			for (off = 0; off != str.size(); off++) {
				if (str[off] == '[') {
					level++;
				} else if (str[off] == ']') {
					level--;
				} else if (level == 0) {
					str[noff++] = str[off];
				}
			}
			str.truncate(noff);
		}
	}

	for (off = 0; off != str.size(); off++) {
		ch = str[off];

		if (ch == '\r')
			continue;

		/*
		 * Remove some formatting characters we don't support,
		 * or which are internal:
		 */
		if (ch == '"')
			ch = '\'';
		else if (ch == '.')
			ch = ' ';
		else if (ch == '\0')
			ch = ' ';

		/* expand tabs to 8 spaces */
		if (ch == '\t')
			ps->line_buffer += "       ";

		if (ch == '\n') {
			/* remove spaces from end of line */
			ps->line_buffer =
			    ps->line_buffer.replace(QRegExp("\\s*$"), "");
			if (ps->line_buffer.size() != 0 &&
			    midipp_import_parse(ps) != 0)
				goto done;
			ps->line_buffer = QString();
		} else {
			ps->line_buffer += ch;
		}
	}
	/* remove spaces from end of line */
	ps->line_buffer =
	    ps->line_buffer.replace(QRegExp("\\s*$"), "");
	if (midipp_import_parse(ps) != 0)
		goto done;

	if (ps->line_buffer.size() != 0 && ps->load_more == 0) {
		ps->line_buffer = QString();
		if (midipp_import_parse(ps) != 0)
			goto done;
	}

 done:
	return (0);
}

MppImportTab :: MppImportTab(MppMainWindow *parent)
{
	int x;

	mainWindow = parent;

	editWidget = new QPlainTextEdit();
	editWidget->setFont(mainWindow->editFont);
	editWidget->setLineWrapMode(QPlainTextEdit::NoWrap);
	editWidget->setPlainText(tr(
	    "[Intro tag]" "\n\n"
	    
	    "C  G  Am" "\n"
	    "Welcome!" "\n"
	    "\n"
	    "[C]Wel[C]com[Am]e!\n"
	    "\n"
	    "(Repeat 2x)\n"));
	editWidget->setTabChangesFocus(true);

	butImportFileNew = new QPushButton(tr("New"));
	butImportFileOpen = new QPushButton(tr("Open"));
	butImportFileSaveAs = new QPushButton(tr("Save As"));

	for (x = 0; x != MPP_MAX_VIEWS; x++) {
		butImport[x] = new MppButton(QString("To %1-Scores").arg(QChar('A' + x)), x);
		connect(butImport[x], SIGNAL(released(int)), this, SLOT(handleImport(int)));
	}
	gbImport = new MppGroupBox(tr("Lyrics"));
	gbImport->addWidget(butImportFileNew, 0, 0, 1, 1);
	gbImport->addWidget(butImportFileOpen, 1, 0, 1, 1);
	gbImport->addWidget(butImportFileSaveAs, 2, 0, 1, 1);

	for (x = 0; x != MPP_MAX_VIEWS; x++)
		gbImport->addWidget(butImport[x], 3 + x, 0, 1, 1);

	connect(butImportFileNew, SIGNAL(released()), this, SLOT(handleImportNew()));
	connect(butImportFileOpen, SIGNAL(released()), this, SLOT(handleImportOpen()));
	connect(butImportFileSaveAs, SIGNAL(released()), this, SLOT(handleImportSaveAs()));
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
		Mpp.HomeDirTxt,
		QString("Chord Tabular File (*.txt; *.TXT)"));
	QString scores;

	diag->setAcceptMode(QFileDialog::AcceptOpen);
	diag->setFileMode(QFileDialog::ExistingFile);

	if (diag->exec()) {

		Mpp.HomeDirTxt = diag->directory().path();

		handleImportNew();

		scores = MppReadFile(diag->selectedFiles()[0]);

		editWidget->setPlainText(scores);

		mainWindow->handle_make_tab_visible(editWidget);
	}
	delete diag;
}

void
MppImportTab :: handleImportSaveAs()
{
	QFileDialog *diag = 
	  new QFileDialog(mainWindow, tr("Select Chord Tabular File"), 
		Mpp.HomeDirTxt,
		QString("Score File (*.txt *.TXT)"));

	diag->setAcceptMode(QFileDialog::AcceptSave);
	diag->setFileMode(QFileDialog::AnyFile);
	diag->setDefaultSuffix(QString("txt"));

	if (diag->exec()) {
		Mpp.HomeDirTxt = diag->directory().path();
		MppWriteFile(diag->selectedFiles()[0], editWidget->toPlainText());
	}

	delete diag;
}

void
MppImportTab :: handleImport(int n)
{
	class midipp_import ps;

	QString str(editWidget->toPlainText());

	midipp_import(str, &ps, mainWindow->scores_main[n]);

	mainWindow->handle_compile();
	mainWindow->handle_make_scores_visible(mainWindow->scores_main[n]);
}
