/*-
 * Copyright (c) 2013 Hans Petter Selasky. All rights reserved.
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

#include "midipp_checkbox.h"

MppCheckBox :: MppCheckBox(int _id)
  : QWidget()
{
	setFixedSize(24,24);

	state = Qt::Unchecked;
	other = Qt::Unchecked;
	id = _id;
}

MppCheckBox :: ~MppCheckBox()
{

}

void
MppCheckBox :: paintEvent(QPaintEvent *event)
{
	QPainter paint(this);
	int w = width();
	int h = height();

	paint.setRenderHints(QPainter::Antialiasing, 1);

	QColor black(0,0,0);
	QColor grey(192,192,192);
	QColor white(255,255,255);

	paint.setPen(QPen(black, 4));
	paint.setBrush(grey);
	paint.drawRect(QRectF(0,0,w,h));

	if (state != Qt::Unchecked) {
		paint.setPen(QPen(grey, 0));
		paint.setBrush(black);
		paint.drawEllipse(QRectF(3,3,w-6,h-6));
	}
}

void
MppCheckBox :: mousePressEvent(QMouseEvent *event)
{
	if (event->button() != Qt::LeftButton) {
		event->ignore();
		return;
	}
	if (state == Qt::Unchecked)
		state = Qt::Checked;
	else
		state = Qt::Unchecked;

	if (other != state) {
		other = state;
		stateChanged(other, id);
		update();
	}
	event->accept();
}

void
MppCheckBox :: setCheckState(Qt::CheckState _state)
{
	state = _state;
	if (other != state) {
		other = state;
		stateChanged(other, id);
		update();
	}
}

Qt::CheckState
MppCheckBox :: checkState(void)
{
	return (state);
}

void
MppCheckBox :: setChecked(bool _enable)
{
	if (_enable)
		state = Qt::Checked;
	else
		state = Qt::Unchecked;
	if (other != state) {
		other = state;
		stateChanged(other, id);
		update();
	}
}

bool
MppCheckBox :: isChecked(void)
{
	return (state == Qt::Checked);
}
