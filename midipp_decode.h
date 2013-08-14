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

struct score_variant {
	char keyword[12];
	uint8_t offset[MPP_MAX_VAR_OFF];
};

class MppDecode : public QDialog
{
	Q_OBJECT;

public:
	MppDecode(MppMainWindow *, int);
	~MppDecode();

	QString getText();
	void setText(QString);
	uint8_t parseScoreChord(struct MppScoreEntry *, const char *, uint8_t);
	void keyPressEvent(QKeyEvent *);
	void wheelEvent(QWheelEvent *);

	MppMainWindow *mw;

	QGridLayout *gl;

	QLineEdit *lin_edit;
	QLineEdit *lin_out;

	QLabel *lbl_format;
	QLabel *lbl_status;
	QLabel *lbl_rol;
	QLabel *lbl_base;
	QLabel *lbl_auto_base;

	MppCheckBox *cbx_auto_base;

	QSpinBox *spn_rol;
	QSpinBox *spn_base;

	MppButton *but_play[3];
	QPushButton *but_ok;
	QPushButton *but_cancel;

	uint8_t current_score[MPP_MAX_VAR_OFF];
	uint8_t auto_base[MPP_MAX_VAR_OFF];

public slots:

	void handle_play_press(int);
	void handle_play_release(int);
	void handle_ok();
	void handle_cancel();
	void handle_parse_int(int x);
	void handle_parse_text(const QString &x);
	void handle_parse(int = 0);

};

extern uint8_t mpp_find_chord(const char *input, uint8_t *pbase, uint8_t *pkey, uint8_t *pvar);
extern uint8_t mpp_parse_chord(const char *input, uint8_t trans, int8_t rol, uint8_t *pout, uint8_t *pn, uint8_t *, int);

#endif		/* _MIDIPP_DECODE_H_ */
