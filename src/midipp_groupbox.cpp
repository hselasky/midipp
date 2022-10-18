/*-
 * Copyright (c) 2013-2022 Hans Petter Selasky
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

#include "midipp_groupbox.h"

MppGroupBox :: MppGroupBox(const QString &_str, QWidget *_parent) :
     t(QColor(192,192,192,128), 24), gl_inner(this), gl(&w)
{
	setParent(_parent);
	l.setText(_str);
	l.setContentsMargins(0,0,0,0);
	l.setMargin(0);
	l.setIndent(12);

	gl_inner.setContentsMargins(0,6,0,0);
	gl_inner.setSpacing(0);
	gl_inner.addWidget(&l, 0,0,1,1, Qt::AlignLeft | Qt::AlignHCenter);
	gl_inner.addWidget(&w, 1,0,1,1);
	gl_inner.setRowStretch(1,1);
}

void
MppGroupBox :: paintEvent(QPaintEvent *event)
{
	t.paintEvent(this, event);

	QWidget::paintEvent(event);
}
