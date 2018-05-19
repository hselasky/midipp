/*-
 * Copyright (c) 2010-2018 Hans Petter Selasky. All rights reserved.
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

#include "midipp_buttonmap.h"
#include "midipp_chords.h"
#include "midipp_mainwindow.h"
#include "midipp_scores.h"
#include "midipp_button.h"
#include "midipp_element.h"
#include "midipp_groupbox.h"

Q_DECL_EXPORT const QString
MppKeyStr(int key)
{
	int rem = MPP_BAND_REM(key);
	int off;
	int sub;
	int oct;

	off = rem / MPP_MAX_SUBDIV;
	sub = rem % MPP_MAX_SUBDIV;
	oct = (key - rem) / MPP_MAX_BANDS;

	if (sub != 0) {
		const char *off_map[12] = {
			"C%1.%2", "D%1B.%2", "D%1.%2", "E%1B.%2",
			"E%1.%2", "F%1.%2", "G%1B.%2", "G%1.%2",
			"A%1B.%2", "A%1.%2", "H%1B.%2", "H%1.%2"
		};
		sub = MPP_BAND_REM_BITREV(sub);
		return (QString(off_map[off]).arg(oct).arg(sub));
	} else {
	  	const char *off_map[12] = {
			"C%1", "D%1B", "D%1", "E%1B",
			"E%1", "F%1", "G%1B", "G%1",
			"A%1B", "A%1", "H%1B", "H%1"
		};
		return (QString(off_map[off]).arg(oct));
	}
}

Q_DECL_EXPORT const QString
MppKeyStrNoOctave(int key)
{
	int rem = MPP_BAND_REM(key);
	int off;
	int sub;
	int oct;

	off = rem / MPP_MAX_SUBDIV;
	sub = rem % MPP_MAX_SUBDIV;
	oct = (key - rem) / MPP_MAX_BANDS;

	if (sub != 0) {
		const char *off_map[12] = {
			"C.%1", "Db.%1", "D.%1", "Eb.%1",
			"E.%1", "F.%1", "Gb.%1", "G.%1",
			"Ab.%1", "A.%1", "Hb.%1", "H.%1"
		};
		sub = MPP_BAND_REM_BITREV(sub);
		return (QString(off_map[off]).arg(sub));
	} else {
	  	const char *off_map[12] = {
			"C", "Db", "D", "Eb",
			"E", "F", "Gb", "G",
			"Ab", "A", "Hb", "H"
		};
		return (QString(off_map[off]));
	}
}

Q_DECL_EXPORT const QString
MppBitsToString(const MppChord_t &mask, int off)
{
	QString temp;
	int x;

	for (x = 0; x != MPP_MAX_BANDS; x++) {
		if (mask.test(x) == 0)
			continue;
		temp += MppKeyStrNoOctave(x + off);
		temp += " ";
	}
	return (temp);
}

void
MppScoreVariantInit(void)
{
	const int rk = 0;
	const int s = MPP_BAND_STEP_24;
	MppChord_t mask;
	QString str;
	uint32_t x;
	uint32_t y;
	uint32_t z[2];

	Mpp.VariantList += QString("\n/* Unique chords with two keys */\n\n");

	for (z[0] = z[1] = 0, x = s; x != MPP_MAX_BANDS; x += s) {
		mask.zero();
		mask.set(0);
		mask.set(x);

		if (MppFindChordRoot(mask) != mask)
			continue;

		MppChordToStringGeneric(mask, rk, rk, 0, MPP_BAND_STEP_24, str);
		if (str.isEmpty())
			str = "  ";
		else {
			if ((x % MPP_BAND_STEP_12) == 0)
				z[0]++;
			z[1]++;
		}
		Mpp.VariantList += str;
		Mpp.VariantList += " = ";
		Mpp.VariantList += MppBitsToString(mask, rk);
		Mpp.VariantList += "\n";
	}

	Mpp.VariantList += QString("\n/* Total number of variants is %1 and %2 */\n\n").arg(z[0]).arg(z[1]);
	Mpp.VariantList += QString("/* Unique chords with three keys */\n\n");

	for (z[0] = z[1] = 0, x =  s; x != MPP_MAX_BANDS; x += s) {
		for (y = x +  s; y != MPP_MAX_BANDS; y += s) {
			mask.zero();
			mask.set(0);
			mask.set(x);
			mask.set(y);

			if (MppFindChordRoot(mask) != mask)
				continue;

			MppChordToStringGeneric(mask, rk, rk, 0, MPP_BAND_STEP_24, str);
			if (str.isEmpty())
				str = "   ";
			else {
				if ((x % MPP_BAND_STEP_12) == 0 &&
				    (y % MPP_BAND_STEP_12) == 0)
					z[0]++;
				z[1]++;
			}
			Mpp.VariantList += str;
			Mpp.VariantList += " = ";
			Mpp.VariantList += MppBitsToString(mask, rk);
			Mpp.VariantList += "\n";
		}
	}

	Mpp.VariantList += QString("\n/* Total number of variants is %1 and %2 */\n\n").arg(z[0]).arg(z[1]);
}

uint8_t
MppDecodeTab :: parseScoreChord(MppChordElement *pinfo)
{
	QString out;
	uint32_t is_sharp;
	uint32_t any = 0;
	uint32_t key = 0;
	uint32_t bass;
	MppChord_t footprint;

	if (pinfo->chord != 0)
		is_sharp = (pinfo->chord->txt.indexOf('#') > -1);
	else
		is_sharp = 0;

	footprint.zero();

	for (int x = 0; x != MPP_MAX_BANDS; x++) {
		if (pinfo->stats[x] == 0)
			continue;
		footprint.set(x);
		any = 1;
	}
	if (!any)
		return (1);	/* not found */

	while (footprint.test(0) == 0) {
		footprint.shr();
		key++;
	}

	bass = MPP_BAND_REM(pinfo->key_base);
	
	MppChordToStringGeneric(footprint, key, bass, is_sharp, chord_step, out);

	if (out.isEmpty())
		return (1);	/* not found */

	chord_key = key;
	chord_bass = bass;
	chord_sharp = is_sharp;
	chord_mask = footprint;

	lin_edit->blockSignals(1);
	lin_edit->setText(out);
	lin_edit->blockSignals(0);

	handle_align(pinfo->key_max + 1);
	handle_refresh();

	return (0);
}

MppDecodeTab :: MppDecodeTab(MppMainWindow *_mw)
  : QWidget()
{

	chord_key = 5 * 12 * MPP_BAND_STEP_12; /* C5 */
	chord_bass = 0;	/* C */
	chord_sharp = 0;
	chord_step = MPP_MAX_SUBDIV;
	chord_mask.zero();
	chord_mask.set(0);
	chord_mask.set(MPP_BAND_STEP_12 * 4);
	chord_mask.set(MPP_BAND_STEP_12 * 7);
	delta_v = 0;

	mw = _mw;

	gl = new QGridLayout(this);

	gb = new MppGroupBox(tr("Chord Selector"));
	gl->addWidget(gb, 0,0,1,1);
	gl->setRowStretch(1,1);
	gl->setColumnStretch(1,1);

	lin_edit = new QLineEdit(QString("C"));
	lin_edit->setMaxLength(256);

	lin_out = new QLineEdit();
	lin_out->setMaxLength(256);

	but_map_step = new MppButtonMap("Scale size\0"
					"12\0"
					"24\0"
					"48\0"
					"96\0"
					"192\0", 5, 5);

	but_map_volume = new MppButtonMap("Key volume\0"
					  "MAX\0"
					  "63\0"
					  "31\0"
					  "15\0"
					  "7\0", 5, 5);
	but_map_volume->setSelection(4);

	but_map_view = new MppButtonMap("View selection\0"
					"View-A\0"
					"View-B\0", 2, 2);

#if MPP_MAX_VIEWS != 2
#error "Please update code above"
#endif
	but_insert = new QPushButton(tr("&Insert"));
	but_rol_up = new QPushButton(tr("Rotate\n&up"));
	but_rol_down = new QPushButton(tr("Rotate\nd&own"));
	but_mod_up = new QPushButton(tr("&Next\nvariant"));
	but_mod_down = new QPushButton(tr("Pre&vious\nvariant"));
	but_step_up = new QPushButton(tr("Step up\nhalf"));
	but_step_down = new QPushButton(tr("Step down\nhalf"));

	but_play = new QPushButton(tr("&Play"));

	connect(but_play, SIGNAL(pressed()), this, SLOT(handle_play_press()));
	connect(but_play, SIGNAL(released()), this, SLOT(handle_play_release()));

	connect(but_insert, SIGNAL(released()), this, SLOT(handle_insert()));

	connect(but_rol_up, SIGNAL(pressed()), this, SLOT(handle_rol_up()));
	connect(but_rol_up, SIGNAL(released()), this, SLOT(handle_play_release()));
	connect(but_rol_down, SIGNAL(pressed()), this, SLOT(handle_rol_down()));
	connect(but_rol_down, SIGNAL(released()), this, SLOT(handle_play_release()));

	connect(but_mod_up, SIGNAL(pressed()), this, SLOT(handle_mod_up()));
	connect(but_mod_up, SIGNAL(released()), this, SLOT(handle_play_release()));
	connect(but_mod_down, SIGNAL(pressed()), this, SLOT(handle_mod_down()));
	connect(but_mod_down, SIGNAL(released()), this, SLOT(handle_play_release()));

	connect(but_step_up, SIGNAL(pressed()), this, SLOT(handle_step_up()));
	connect(but_step_up, SIGNAL(released()), this, SLOT(handle_play_release()));
	connect(but_step_down, SIGNAL(pressed()), this, SLOT(handle_step_down()));
	connect(but_step_down, SIGNAL(released()), this, SLOT(handle_play_release()));
	
	connect(lin_edit, SIGNAL(textChanged(const QString &)), this, SLOT(handle_parse()));
	connect(but_map_step, SIGNAL(selectionChanged(int)), this, SLOT(handle_stepping()));

	gb->addWidget(
	    new QLabel(tr("[CDEFGABH][#b][...][/CDEFGABH[#b]]")),
	    0,0,1,4, Qt::AlignHCenter|Qt::AlignVCenter);

	gb->addWidget(but_rol_down, 2,0,1,1);
	gb->addWidget(but_rol_up, 2,1,1,1);

	gb->addWidget(but_mod_down, 2,2,1,1);
	gb->addWidget(but_mod_up, 2,3,1,1);

	gb->addWidget(but_step_down, 3,0,1,2);
	gb->addWidget(but_step_up, 3,2,1,2);

	gb->addWidget(lin_edit, 4,0,1,4);
	gb->addWidget(lin_out, 5,0,1,4);

	gb->addWidget(but_map_step, 6,0,1,4);
	gb->addWidget(but_map_volume, 7,0,1,4);
	gb->addWidget(but_map_view, 8,0,1,4);

	gb->addWidget(but_insert, 9, 2, 1, 2);
	gb->addWidget(but_play, 9, 0, 1, 2);

	gb_gen = new MppGroupBox(tr("Chord Scratch Area"));
	gl->addWidget(gb_gen, 0,1,2,1);

	editor = new MppDecodeEditor(_mw);
	gb_gen->addWidget(editor, 0,0,1,1);

	handle_parse();
    
	but_insert->setFocus();
}

MppDecodeTab :: ~MppDecodeTab()
{

}

void
MppDecodeTab :: handle_play_press()
{
	MppScoreMain *sm = mw->scores_main[but_map_view->currSelection];
	uint8_t vel = (127 >> but_map_volume->currSelection);
	int key_bass;

	mw->atomic_lock();
	key_bass = chord_key - MPP_BAND_REM(chord_key) + chord_bass;
	if (key_bass >= chord_key)
		key_bass -= 2 * MPP_MAX_BANDS;
	else
		key_bass -= 1 * MPP_MAX_BANDS;

	mw->output_key(MPP_DEFAULT_TRACK(sm->unit), sm->synthChannel, key_bass, vel, 0, 0);
	mw->output_key(MPP_DEFAULT_TRACK(sm->unit), sm->synthChannel, key_bass + MPP_MAX_BANDS, vel, 0, 0);

	for (int x = 0; x != MPP_MAX_BANDS; x++) {
		if (chord_mask.test(x))
			mw->output_key(MPP_DEFAULT_TRACK(sm->unit), sm->synthChannel, chord_key + x, vel, 0, 0);
	}
	mw->atomic_unlock();
}

void
MppDecodeTab :: handle_play_release()
{
  	MppScoreMain *sm = mw->scores_main[but_map_view->currSelection];
	int key_bass;

	mw->atomic_lock();
	key_bass = chord_key - MPP_BAND_REM(chord_key) + chord_bass;
	if (key_bass >= chord_key)
		key_bass -= 2 * MPP_MAX_BANDS;
	else
		key_bass -= 1 * MPP_MAX_BANDS;

	mw->output_key(MPP_DEFAULT_TRACK(sm->unit), sm->synthChannel, key_bass, 0, 0, 0);
	mw->output_key(MPP_DEFAULT_TRACK(sm->unit), sm->synthChannel, key_bass + MPP_MAX_BANDS, 0, 0, 0);

	for (int x = 0; x != MPP_MAX_BANDS; x++) {
		if (chord_mask.test(x))
			mw->output_key(MPP_DEFAULT_TRACK(sm->unit), sm->synthChannel, chord_key + x, 0, 0, 0);
	}
	mw->atomic_unlock();
}

void
MppDecodeTab :: handle_insert()
{
	QPlainTextEdit *qedit = mw->currEditor();
	if (qedit == 0)
		return;

	QTextCursor cursor(qedit->textCursor());
	MppChordElement info;
	MppElement *ptr;
	MppHead temp;
	int row;

	temp += qedit->toPlainText();
	temp.flush();

	row = cursor.blockNumber();

	cursor.beginEditBlock();

	/* check if the chord is valid */
	if (temp.getChord(row, &info) != 0) {
		if (info.chord != 0) {
			info.chord->txt = QChar('(') + lin_edit->text().trimmed() + QChar(')');

			cursor.movePosition(QTextCursor::Start, QTextCursor::MoveAnchor, 1);
			cursor.movePosition(QTextCursor::Down, QTextCursor::MoveAnchor, info.chord->line);
			cursor.movePosition(QTextCursor::EndOfLine, QTextCursor::MoveAnchor, 1);
			cursor.movePosition(QTextCursor::StartOfLine, QTextCursor::KeepAnchor, 1);
			cursor.removeSelectedText();
			cursor.insertText(temp.toPlain(info.chord->line).replace("\n", ""));
		}
		if (info.start != 0) {
			for (ptr = info.start; ptr != info.stop;
			    ptr = TAILQ_NEXT(ptr, entry)) {
				ptr->type = MPP_T_UNKNOWN;
				ptr->txt = "";
			}

			info.start->txt = mw->led_config_insert->text() +
			    getText();

			cursor.movePosition(QTextCursor::Start, QTextCursor::MoveAnchor, 1);
			cursor.movePosition(QTextCursor::Down, QTextCursor::MoveAnchor, row);
			cursor.movePosition(QTextCursor::EndOfLine, QTextCursor::MoveAnchor, 1);
			cursor.movePosition(QTextCursor::StartOfLine, QTextCursor::KeepAnchor, 1);
			cursor.removeSelectedText();
			cursor.insertText(temp.toPlain(info.start->line).replace("\n", ""));
		}
	} else {
		cursor.removeSelectedText();
		cursor.insertText(mw->led_config_insert->text() + getText() + QChar('\n'));
	}
	cursor.endEditBlock();

	mw->handle_compile();
}

void
MppDecodeTab :: handle_align(int key)
{
	if (key < (2 * MPP_MAX_BANDS) ||
	    key > (128 * MPP_MAX_SUBDIV - MPP_MAX_BANDS))
		return;

	chord_key = key - MPP_BAND_REM(key) +
	    MPP_MAX_BANDS + MPP_BAND_REM(chord_key);

	for (int x = MPP_MAX_BANDS; x--; ) {
		if (chord_mask.test(x) == 0)
			continue;
		if ((chord_key + x) < key)
			break;
		int rols = 0;
		MppRolUpChord(chord_mask, rols);
		chord_key -= rols;
		x = MPP_MAX_BANDS;
	}
}

void
MppDecodeTab :: handle_rol_up()
{
	int rols;

	if (chord_key > (128 * MPP_MAX_SUBDIV - MPP_MAX_BANDS))
		return;
	handle_play_release();
	rols = 0;
	MppRolDownChord(chord_mask, rols);
	chord_key += rols;
	handle_refresh();
	handle_play_press();
}

void
MppDecodeTab :: handle_rol_down()
{
	int rols;

	if (chord_key < 2 * MPP_MAX_BANDS)
		return;
	handle_play_release();
	rols = 0;
	MppRolUpChord(chord_mask, rols);
	chord_key -= rols;
	handle_refresh();
	handle_play_press();
}

void
MppDecodeTab :: handle_mod_up()
{
	uint32_t rols;
	handle_play_release();
	chord_mask = MppFindChordRoot(chord_mask, &rols);
	chord_key += rols;
	MppNextChordRoot(chord_mask, chord_step);
	handle_refresh();
	handle_play_press();
}

void
MppDecodeTab :: handle_mod_down()
{
  	uint32_t rols;
	handle_play_release();
	chord_mask = MppFindChordRoot(chord_mask, &rols);
	chord_key += rols;
	MppPrevChordRoot(chord_mask, chord_step);
	handle_refresh();
	handle_play_press();
}

void
MppDecodeTab :: handle_step_up()
{
	handle_play_release();
	if (chord_key < (128 * MPP_BAND_STEP_12)) {
		chord_key += MPP_BAND_STEP_12;
		chord_bass += MPP_BAND_STEP_12;
		chord_bass %= MPP_MAX_BANDS;
		handle_refresh();
	}
	handle_play_press();
}

void
MppDecodeTab :: handle_step_down()
{
	handle_play_release();
	if (chord_key >= (25 * MPP_BAND_STEP_12)) {
		chord_key -= MPP_BAND_STEP_12;
		chord_bass += 11 * MPP_BAND_STEP_12;
		chord_bass %= MPP_MAX_BANDS;
		handle_refresh();
	}
	handle_play_press();
}

void
MppDecodeTab :: handle_stepping()
{
	chord_step = (MPP_MAX_SUBDIV >> but_map_step->currSelection);
	handle_refresh();
}

void
MppDecodeTab :: handle_refresh()
{
	QString out_key;
	QString str;
	int key_bass;
	int log2_step;

	key_bass = chord_key - MPP_BAND_REM(chord_key) + chord_bass;
	if (key_bass >= chord_key)
		key_bass -= 2 * MPP_MAX_BANDS;
	else
		key_bass -= 1 * MPP_MAX_BANDS;

	MppChordToStringGeneric(chord_mask, chord_key, key_bass,
	    chord_sharp, chord_step, str);

	for (log2_step = 0; ((MPP_MAX_SUBDIV /
	    chord_step) >> log2_step) != 1; log2_step++)
		;

	out_key += MppKeyStr(key_bass);
	out_key += " ";
	out_key += MppKeyStr(key_bass + MPP_MAX_BANDS);
	out_key += " ";
	
	for (int x = 0; x != MPP_MAX_BANDS; x++) {
		if (chord_mask.test(x) == 0)
			continue;
		out_key += MppKeyStr(x + chord_key);
		out_key += " ";
	}

	if (str.isEmpty() == 0) {
		out_key += "/* ";
		out_key += str;
		out_key += " */";
	}

	lin_edit->blockSignals(1);
	lin_edit->setText(str);
	lin_edit->blockSignals(0);
	lin_out->blockSignals(1);
	lin_out->setText(out_key);
	lin_out->blockSignals(0);
	but_map_step->blockSignals(1);
	but_map_step->setSelection(log2_step);
	but_map_step->blockSignals(0);
}

void
MppDecodeTab :: handle_parse()
{
	QString out_key;
	MppChord_t mask;
	int key_bass;
	uint32_t rem;
	uint32_t bass;

	QString chord = lin_edit->text().trimmed();
	if (chord.isEmpty())
		goto error;

	MppStringToChordGeneric(mask, rem, bass, chord_step, chord);
	chord_sharp = (chord.indexOf('#') > -1);

	if (mask.test(0) == 0)
		goto error;

	chord_bass = MPP_BAND_REM(bass);
	chord_key = chord_key - MPP_BAND_REM(chord_key) + rem;
	chord_mask = mask;

	key_bass = chord_key - MPP_BAND_REM(chord_key) + chord_bass;
	if (key_bass >= chord_key)
		key_bass -= 2 * MPP_MAX_BANDS;
	else
		key_bass -= 1 * MPP_MAX_BANDS;

	out_key += MppKeyStr(key_bass);
	out_key += " ";
	out_key += MppKeyStr(key_bass + MPP_MAX_BANDS);
	out_key += " ";
	
	for (int x = 0; x != MPP_MAX_BANDS; x++) {
		if (chord_mask.test(x) == 0)
			continue;
		out_key += MppKeyStr(x + chord_key);
		out_key += " ";
	}

	out_key += "/* ";
	out_key += chord;
	out_key += " */";

	lin_out->setText(out_key);
	return;
error:
	chord_mask.zero();
	chord_mask.set(0);
	lin_out->setText("/* ERROR */");
}

QString
MppDecodeTab :: getText()
{
	return (lin_out->text());
}

void
MppDecodeTab :: setText(QString str)
{
  	lin_edit->blockSignals(1);
	lin_edit->setText(str);
	lin_edit->blockSignals(0);

	handle_parse();
}

void
MppDecodeTab :: keyPressEvent(QKeyEvent *event)
{
	switch (event->key()) {
	case Qt::Key_Up:
		handle_mod_up();
		break;
	case Qt::Key_Down:
		handle_mod_down();
		break;
	default:
		break;
	}
}

void
MppDecodeEditor :: mouseDoubleClickEvent(QMouseEvent *e)
{
	MppHead temp;
	MppChordElement info;
	QTextCursor cursor(textCursor());
	int row;

	cursor.movePosition(QTextCursor::EndOfLine, QTextCursor::MoveAnchor, 1);
	cursor.movePosition(QTextCursor::StartOfLine, QTextCursor::KeepAnchor, 1);

	/* check if the line is empty */
	if (cursor.selectedText().simplified().size() == 0)
		return;

	row = cursor.blockNumber();

	temp += toPlainText();
	temp.flush();

	/* check if the chord is valid */
	if (temp.getChord(row, &info) != 0)
		mw->tab_chord_gl->parseScoreChord(&info);

	setTextCursor(cursor);
}
