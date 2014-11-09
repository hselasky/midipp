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

#include "midipp_pianotab.h"
#include "midipp_mainwindow.h"

MppPianoTab :: MppPianoTab(MppMainWindow *parent)
  : QWidget(parent)
{
	memset(&state, 0, sizeof(state));

	mw = parent;

	setMinimumSize(QSize(50,50));
}

MppPianoTab :: ~MppPianoTab()
{

}

void
MppPianoTab :: mousePressEvent(QMouseEvent *event)
{
	QPoint p = event->pos();
	unsigned x;

	for (x = 0; x != 24; x++) {
		if (r_pressed[x].contains(p) == 0)
			continue;
		if (state.pressed[x] != 0) {
			state.pressed[x] = 0;
			int key = MPP_DEFAULT_BASE_KEY + x;
			mw->handle_play_release(key, state.view_index);
			update();
			continue;
		}
		uint8_t curr_octave = (x >= 12);
		if (state.last_octave != curr_octave) {
			unsigned y;
			state.last_octave = curr_octave;

			/* release keys, when shifting octave */
			for (y = 0; y != 24; y++) {
				if (state.pressed[y] == 0)
					continue;
				state.pressed[y] = 0;
				int key = MPP_DEFAULT_BASE_KEY + y;
				mw->handle_play_release(key, state.view_index);
			}
			if (state.sustain != 0) {
				mw->handle_sustain_release(state.view_index);
				mw->handle_sustain_press(state.view_index);
			}
		}
		state.pressed[x] = 1;
		int key = MPP_DEFAULT_BASE_KEY + x;
		mw->handle_play_press(key, state.view_index);
		update();
	}
	if (r_sustain_on.contains(p) != 0) {
		state.sustain = 1;
		mw->handle_sustain_press(state.view_index);
		update();
	}
	if (r_sustain_off.contains(p) != 0) {
		state.sustain = 0;
		mw->handle_sustain_release(state.view_index);
		update();
	}
	if (r_view_a.contains(p) != 0) {
		state.view_index = 0;
		update();
	}
	if (r_view_b.contains(p) != 0) {
		state.view_index = 1;
		update();
	}
	for (x = 0; x != MPP_PIANO_TAB_LABELS; x++) {
		if (r_label[x].contains(p) == 0)
			continue;
		state.last_jump = x;
		mw->handle_jump(x);
		if (state.sustain != 0) {
			mw->handle_sustain_release(state.view_index);
			mw->handle_sustain_press(state.view_index);
		}
		update();
	}
}

void
MppPianoTab :: mouseReleaseEvent(QMouseEvent *event)
{
	QPoint p = event->pos();
	unsigned x;

	for (x = 0; x != 24; x++) {
		if (r_pressed[x].contains(p) == 0)
			continue;
		if (state.pressed[x] == 0)
			continue;
		state.pressed[x] = 0;
		int key = MPP_DEFAULT_BASE_KEY + x;
		mw->handle_play_release(key, state.view_index);
		update();
	}
}

QRect
MppPianoTab :: drawText(QPainter &paint, const QColor &fg,
    const QColor &bg, qreal x_unit, qreal y_unit, qreal x, qreal y,
    const char *buf)
{
	QString buffer(buf);

	QRectF n(x,y,x_unit,y_unit);
	QRectF r = paint.boundingRect(n, Qt::AlignCenter | Qt::TextSingleLine, buffer);

	QRectF f(x + (x_unit - r.width()) / 2.0, y + (y_unit - r.height()) / 2.0, r.width(), r.height());

	paint.setPen(QPen());
	paint.setBrush(QBrush(bg));
	paint.drawRoundedRect(n, y_unit/2.0, y_unit/2.0);

	paint.setPen(QPen(fg,2));
	paint.setBrush(QBrush());
	paint.drawRoundedRect(n, y_unit/2.0, y_unit/2.0);

	paint.drawText(f, Qt::AlignCenter | Qt::TextSingleLine, buffer);

	return (QRect(n.x(),n.y(),n.width(),n.height()));
}

void
MppPianoTab :: paintEvent(QPaintEvent *event)
{
	QPainter paint(this);
	QFont fnt;
	qreal unit;
	qreal uh;
	qreal uf;
	qreal uq;
	int len;
	qreal xpos;
	qreal ypos;
	int w = width();
	int h = height();
	int z;
	const char *buf;

	QColor black(0,0,0);
	QColor white(255,255,255);
	QColor grey(192,192,192);
	QColor yellow(192,192,0);
	QColor red(255,0,0);

	paint.fillRect(QRectF(0,0,w,h), white);

	paint.setRenderHints(QPainter::Antialiasing, 1);

	z = (h + (2 * h / 3));
	if (w > z)
		w = z;

	unit = 2.0 * w / 21.0;
	uh = 1.0 * w / 21.0;
	uf = w / 21.0;
	uq = 1.0 * w / 42.0;
	h /= 2;

	fnt.setPixelSize(uf);
	paint.setFont(fnt);

	ypos = h - 2 * unit + uh;
	r_pressed[1] = drawText(paint, black, state.pressed[1] ? grey : white, unit, unit, uq + uh + uq + (unit + uh) * 0.0, ypos, "C#");
	r_pressed[3] = drawText(paint, black, state.pressed[3] ? grey : white, unit, unit, uq + uh + uq + (unit + uh) * 1.0, ypos, "D#");
	r_pressed[6] = drawText(paint, black, state.pressed[6] ? grey : white, unit, unit, uq + uh + uq + (unit + uh) * 3.0, ypos, "F#");
	r_pressed[8] = drawText(paint, black, state.pressed[8] ? grey : white, unit, unit, uq + uh + uq + (unit + uh) * 4.0, ypos, "G#");
	r_pressed[10] = drawText(paint, black, state.pressed[10] ? grey : white, unit, unit, uq + uh + uq + (unit + uh) * 5.0, ypos, "A#");

	ypos = h - unit + uh;
	r_pressed[0] = drawText(paint, black, state.pressed[0] ? grey : white, unit, unit, uq + (unit + uh) * 0.0, ypos, "C5");
	r_pressed[2] = drawText(paint, black, state.pressed[2] ? grey : white, unit, unit, uq + (unit + uh) * 1.0, ypos, "D5");
	r_pressed[4] = drawText(paint, black, state.pressed[4] ? grey : white, unit, unit, uq + (unit + uh) * 2.0, ypos, "E5");
	r_pressed[5] = drawText(paint, black, state.pressed[5] ? grey : white, unit, unit, uq + (unit + uh) * 3.0, ypos, "F5");
	r_pressed[7] = drawText(paint, black, state.pressed[7] ? grey : white, unit, unit, uq + (unit + uh) * 4.0, ypos, "G5");
	r_pressed[9] = drawText(paint, black, state.pressed[9] ? grey : white, unit, unit, uq + (unit + uh) * 5.0, ypos, "A5");
	r_pressed[11] = drawText(paint, black, state.pressed[11] ? grey : white, unit, unit, uq + (unit + uh) * 6.0, ypos, "H5");

	ypos = h + unit;
	r_pressed[12+1] = drawText(paint, black, state.pressed[12+1] ? grey : white, unit, unit, uq + uh + uq + (unit + uh) * 0.0, ypos, "C#");
	r_pressed[12+3] = drawText(paint, black, state.pressed[12+3] ? grey : white, unit, unit, uq + uh + uq + (unit + uh) * 1.0, ypos, "D#");
	r_pressed[12+6] = drawText(paint, black, state.pressed[12+6] ? grey : white, unit, unit, uq + uh + uq + (unit + uh) * 3.0, ypos, "F#");
	r_pressed[12+8] = drawText(paint, black, state.pressed[12+8] ? grey : white, unit, unit, uq + uh + uq + (unit + uh) * 4.0, ypos, "G#");
	r_pressed[12+10] = drawText(paint, black, state.pressed[12+10] ? grey : white, unit, unit, uq + uh + uq + (unit + uh) * 5.0, ypos, "A#");

	ypos = h + 2*unit;
	r_pressed[12+0] = drawText(paint, black, state.pressed[12+0] ? grey : white, unit, unit, uq + (unit + uh) * 0.0, ypos, "C6");
	r_pressed[12+2] = drawText(paint, black, state.pressed[12+2] ? grey : white, unit, unit, uq + (unit + uh) * 1.0, ypos, "D6");
	r_pressed[12+4] = drawText(paint, black, state.pressed[12+4] ? grey : white, unit, unit, uq + (unit + uh) * 2.0, ypos, "E6");
	r_pressed[12+5] = drawText(paint, black, state.pressed[12+5] ? grey : white, unit, unit, uq + (unit + uh) * 3.0, ypos, "F6");
	r_pressed[12+7] = drawText(paint, black, state.pressed[12+7] ? grey : white, unit, unit, uq + (unit + uh) * 4.0, ypos, "G6");
	r_pressed[12+9] = drawText(paint, black, state.pressed[12+9] ? grey : white, unit, unit, uq + (unit + uh) * 5.0, ypos, "A6");
	r_pressed[12+11] = drawText(paint, black, state.pressed[12+11] ? grey : white, unit, unit, uq + (unit + uh) * 6.0, ypos, "H6");

	uf /= 3.0;
	fnt.setPixelSize(uf);
	paint.setFont(fnt);

	xpos = 0;
	ypos = h - (3*unit);
	r_label[0] = drawText(paint, black, (state.last_jump == 0) ? yellow : white, 2.0 * uf, unit, uq + xpos * uf, ypos, "L0");
	xpos += 2;
	r_label[1] = drawText(paint, black, (state.last_jump == 1) ? yellow : white, 2.0 * uf, unit, uq + xpos * uf, ypos, "L1");
	xpos += 2;
	r_label[2] = drawText(paint, black, (state.last_jump == 2) ? yellow : white, 2.0 * uf, unit, uq + xpos * uf, ypos, "L2");
	xpos += 2;
	r_label[3] = drawText(paint, black, (state.last_jump == 3) ? yellow : white, 2.0 * uf, unit, uq + xpos * uf, ypos, "L3");
	xpos += 2;
	r_label[4] = drawText(paint, black, (state.last_jump == 4) ? yellow : white, 2.0 * uf, unit, uq + xpos * uf, ypos, "L4");
	xpos += 2;
	r_label[5] = drawText(paint, black, (state.last_jump == 5) ? yellow : white, 2.0 * uf, unit, uq + xpos * uf, ypos, "L5");
	xpos += 2;
	r_label[6] = drawText(paint, black, (state.last_jump == 6) ? yellow : white, 2.0 * uf, unit, uq + xpos * uf, ypos, "L6");
	xpos += 2;
	r_label[7] = drawText(paint, black, (state.last_jump == 7) ? yellow : white, 2.0 * uf, unit, uq + xpos * uf, ypos, "L7");
	xpos += 2;
	r_label[8] = drawText(paint, black, (state.last_jump == 8) ? yellow : white, 2.0 * uf, unit, uq + xpos * uf, ypos, "L8");
	xpos += 2;
	r_label[9] = drawText(paint, black, (state.last_jump == 9) ? yellow : white, 2.0 * uf, unit, uq + xpos * uf, ypos, "L9");
	xpos += 2;

#if MPP_PIANO_TAB_LABELS != 10
#error "MPP_PIANO_TAB_LABELS != 10"
#endif
	buf = "A-View";
	len = strlen(buf) + 2;
	r_view_a = drawText(paint, black, (state.view_index == 0) ? grey : white, uf * len, unit, uq + xpos * uf, ypos, buf);
	xpos += len;

	buf = "B-View";
	len = strlen(buf) + 2;
	r_view_b = drawText(paint, black, (state.view_index == 1) ? grey : white, uf * len, unit, uq + xpos * uf, ypos, buf);
	xpos += len;

	buf = "Sustain-OFF";
	len = strlen(buf) + 2;
	r_sustain_off = drawText(paint, black, state.sustain ? white : grey, uf * len, unit, uq + xpos * uf, ypos, buf);
	xpos += len;

	buf = "Sustain-ON";
	len = strlen(buf) + 2;
	r_sustain_on = drawText(paint, black, state.sustain ? grey : white , uf * len, unit, uq + xpos * uf, ypos, buf);
	xpos += len;
}
