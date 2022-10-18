/*-
 * Copyright (c) 2013 Hans Petter Selasky
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

#ifndef _MIDIPP_GROUPBOX_H_
#define	_MIDIPP_GROUPBOX_H_

#include "midipp_texture.h"

class MppGroupBox : public QWidget {
public:
	MppRounded t;
	QLabel l;
	QWidget w;
	QGridLayout gl_inner;
	QGridLayout gl;

	MppGroupBox(const QString & = QString(), QWidget * = 0);

	void paintEvent(QPaintEvent *);
	void setTitle(const QString &str) { l.setText(str); };
	QString title() const { return (l.text()); };

	void addWidget(QWidget *p, int r, int c,
	    int w = 1, int h = 1, Qt::Alignment a = Qt::Alignment()) {
		gl.addWidget(p, r, c, w, h, a);
	};
	void setColumnStretch(int a, int b) {
		gl.setColumnStretch(a, b);
	};
	void setRowStretch(int a, int b) {
		gl.setRowStretch(a, b);
	};
};

#endif		/* _MIDIPP_GROUPBOX_H_ */
