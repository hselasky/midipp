/*-
 * Copyright (c) 2014 Hans Petter Selasky. All rights reserved.
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

#ifndef _MIDIPP_PIANOTAB_H_
#define	_MIDIPP_PIANOTAB_H_

#include "midipp.h"

class MppPianoTab : public QWidget
{
	Q_OBJECT;

public:
	MppPianoTab(MppMainWindow * = 0);
	~MppPianoTab();

	QRect drawText(QPainter &, const QColor &, const QColor &, qreal, qreal, qreal, qreal, const char *);

	MppMainWindow *mw;

	struct {
		uint8_t pressed[24];
		uint8_t sustain;
		uint8_t view_index;
		uint8_t last_jump;
		uint8_t last_octave;
	} state;

	QRect r_pressed[24];
	QRect r_view_a;
	QRect r_view_b;
	QRect r_sustain_on;
	QRect r_sustain_off;
	QRect r_label[MPP_PIANO_TAB_LABELS];

	void mousePressEvent(QMouseEvent *event);
	void mouseReleaseEvent(QMouseEvent *event);

	void paintEvent(QPaintEvent *);
};

#endif		/* _MIDIPP_PIANOTAB_H_ */

