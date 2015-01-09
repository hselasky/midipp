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

#ifndef _MIDIPP_DECODE_H_
#define	_MIDIPP_DECODE_H_

#include "midipp.h"

#define	MPP_MAX_VAR_OFF 12
#define	MPP_SCORE_KEYMAX 4

struct score_variant_initial {
	const char *pattern[MPP_SCORE_KEYMAX];
	uint32_t footprint;
};

class score_variant {
public:
	score_variant() { footprint = 0; duplicate = 0; pattern = QString(); }
	QString pattern;
	uint32_t footprint;
	uint32_t duplicate;
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
	void wheelEvent(QWheelEvent *);

	MppMainWindow *mw;

	QGridLayout *gl;
	MppGroupBox *gb;

	QLineEdit *lin_edit;
	QLineEdit *lin_out;

	QPushButton *but_rol_up;
	QPushButton *but_rol_down;

	QPushButton *but_mod_up;
	QPushButton *but_mod_down;

	QLabel *lbl_status;

	MppCheckBox *cbx_auto_base;

	MppButton *but_play[MPP_MAX_VIEWS][3];

	QPushButton *but_insert;

	int rol_value;

	uint8_t current_score[MPP_MAX_VAR_OFF];
	uint8_t auto_base[MPP_MAX_VAR_OFF];

public slots:

	void handle_rol_up();
	void handle_rol_down();
	void handle_mod_up();
	void handle_mod_down();
	void handle_play_press(int);
	void handle_play_release(int);
	void handle_insert();
	void handle_parse(int = 0);
};

extern uint8_t mpp_find_chord(QString input, uint8_t *pbase, uint8_t *pkey, uint32_t *pvar);
extern uint8_t mpp_parse_chord(const QString &input, int8_t rol, uint8_t *pout, uint8_t *pn, uint32_t *pvar, int);

#endif		/* _MIDIPP_DECODE_H_ */
