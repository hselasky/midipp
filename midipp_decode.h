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

#ifndef _MIDIPP_DECODE_H_
#define	_MIDIPP_DECODE_H_

#include "midipp_chords.h"

class MppDecodeEditor : public QPlainTextEdit
{
public:
	MppDecodeEditor(MppMainWindow *_mw) { mw = _mw; };
	~MppDecodeEditor() {};

	MppMainWindow *mw;
	void mouseDoubleClickEvent(QMouseEvent *);
};

class MppDecodeTab : public QWidget
{
	Q_OBJECT;

public:
	MppDecodeTab(MppMainWindow *);
	~MppDecodeTab();

	QString getText();
	void setText(QString);
	uint8_t parseScoreChord(MppChordElement *);
	void keyPressEvent(QKeyEvent *);

	MppMainWindow *mw;

	QGridLayout *gl;

	/* Chord Selector */
	MppGroupBox *gb;

	QLineEdit *lin_edit;
	QLineEdit *lin_out;

	QPushButton *but_rol_up;
	QPushButton *but_rol_down;

	QPushButton *but_mod_up;
	QPushButton *but_mod_down;

	QPushButton *but_step_up;
	QPushButton *but_step_down;

	MppButtonMap *but_map_step;
	MppButtonMap *but_map_volume;
	MppButtonMap *but_map_view;

	QPushButton *but_play;
	QPushButton *but_insert;

	/* Chord Scratch Area */
	MppGroupBox *gb_gen;
	MppDecodeEditor *editor;

	int chord_key;
	int chord_bass;
	int chord_step;
	int chord_sharp;
	int delta_v;

	MppChord_t chord_mask;

public slots:
	void handle_rol_up();
	void handle_rol_down();
	void handle_mod_up();
	void handle_mod_down();
	void handle_step_up();
	void handle_step_down();
	void handle_play_press();
	void handle_play_release();
	void handle_insert();
	void handle_parse();
	void handle_refresh();
	void handle_stepping();
	void handle_align(int);
};

extern const QString MppKeyStr(int key);
extern const QString MppKeyStrNoOctave(int key);
extern const QString MppBitsToString(const MppChord_t &, int);

#endif		/* _MIDIPP_DECODE_H_ */
