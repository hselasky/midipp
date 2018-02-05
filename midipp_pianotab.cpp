/*-
 * Copyright (c) 2014-2017 Hans Petter Selasky. All rights reserved.
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
#include "midipp_scores.h"
#include "midipp_mode.h"
#include "midipp_mainwindow.h"

MppPianoTab :: MppPianoTab(MppMainWindow *parent)
  : QWidget(parent)
{
	memset(&state, 0, sizeof(state));

	mw = parent;

	setMinimumSize(QSize(50,50));

	setFocusPolicy(Qt::StrongFocus);
}

MppPianoTab :: ~MppPianoTab()
{

}

void
MppPianoTab :: mousePressEvent(QMouseEvent *event)
{
	QPoint p = event->pos();
	unsigned x;

	for (x = 0; x != 2 * MPP_MAX_BANDS; x++) {
		if (r_pressed[x].contains(p) == 0)
			continue;
		if (state.pressed[x] != 0) {
			state.pressed[x] = 0;
			unsigned key = getBaseKey() + x;
			mw->handle_play_release(key, state.view_index);
			update();
			continue;
		}
		uint8_t curr_octave = (x >= MPP_MAX_BANDS);
		if (state.last_octave != curr_octave) {
			state.last_octave = curr_octave;
			releaseAll();
		}
		state.pressed[x] = 1;
		unsigned key = getBaseKey() + x;
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

	for (x = 0; x != 2 * MPP_MAX_BANDS; x++) {
		if (r_pressed[x].contains(p) == 0)
			continue;
		if (state.pressed[x] == 0)
			continue;
		state.pressed[x] = 0;
		unsigned key = getBaseKey() + x;
		mw->handle_play_release(key, state.view_index);
		update();
	}
}

int
MppPianoTab :: getBaseKey()
{
	int baseKey;

	mw->atomic_lock();
  	baseKey = mw->scores_main[state.view_index]->baseKey;
	baseKey -= MPP_BAND_REM(baseKey);
	mw->atomic_unlock();

	return (baseKey);
}

void
MppPianoTab :: releaseAll()
{
	unsigned y;

	/* release keys, when shifting octave */
	for (y = 0; y != 2 * MPP_MAX_BANDS; y++) {
		if (state.pressed[y] == 0)
			continue;
		state.pressed[y] = 0;
		unsigned key = getBaseKey() + y;
		mw->handle_play_release(key, state.view_index);
	}
	if (state.sustain != 0) {
		mw->handle_sustain_release(state.view_index);
		mw->handle_sustain_press(state.view_index);
	}
}

void
MppPianoTab :: processKey(uint8_t release, char which)
{
	unsigned keyMode;
	unsigned key;

	mw->atomic_lock();
	keyMode = mw->scores_main[state.view_index]->keyMode;
	mw->atomic_unlock();

	switch (keyMode) {
	case MM_PASS_NONE_CHORD_PIANO:
		switch (which) {
		case ' ':
			key = MPP_D0;
			break;
		case 'g':
			key = MPP_F0;
			break;
		case 'h':
			key = MPP_G0;
			break;
		case 'j':
			key = MPP_A0;
			break;
		case 'k':
			key = MPP_H0;
			break;
		case 'l':
			key = MPP_G0B;
			break;
		default:
			return;
		}
		key *= MPP_BAND_STEP_12;
		if (state.last_key)
			key += MPP_MAX_BANDS;
		break;

	case MM_PASS_NONE_FIXED:
	case MM_PASS_NONE_TRANS:
		key = 0;
		break;

	default:
		return;
	}

	if (release) {
		if (state.pressed[key] == 0)
			return;
		state.pressed[key] = 0;
		mw->handle_play_release(getBaseKey() + key, state.view_index);
	} else {
		uint8_t curr_octave = (key >= MPP_MAX_BANDS);
		if (state.last_octave != curr_octave) {
			state.last_octave = curr_octave;
			releaseAll();
		}
		if (state.pressed[key] != 0)
			return;
		state.pressed[key] = 1;
		mw->handle_play_press(getBaseKey() + key, state.view_index);
	}
	update();
}

void
MppPianoTab :: keyPressEvent(QKeyEvent *event)
{
	if (event->isAutoRepeat())
		return;
	if (event->key() >= Qt::Key_0 && event->key() <= Qt::Key_9) {
		unsigned x = event->key() - Qt::Key_0;
		state.last_jump = x;
		mw->handle_jump(x);
		if (state.sustain != 0) {
			mw->handle_sustain_release(state.view_index);
			mw->handle_sustain_press(state.view_index);
		}
		update();
	} else if (event->key() == Qt::Key_Space) {
		processKey(0, ' ');
	} else if (event->key() >= Qt::Key_A && event->key() <= Qt::Key_Z) {
		processKey(0, event->key() - Qt::Key_A + 'a');
	} else if (event->key() == Qt::Key_Shift) {
		state.sustain = 1;
		mw->handle_sustain_press(state.view_index);
		update();
	}
}

void
MppPianoTab :: keyReleaseEvent(QKeyEvent *event)
{
	if (event->isAutoRepeat())
		return;
	if (event->key() == Qt::Key_Space) {
		processKey(1, ' ');
	} else if (event->key() >= Qt::Key_A && event->key() <= Qt::Key_Z) {
		processKey(1, event->key() - Qt::Key_A + 'a');
	} else if (event->key() == Qt::Key_Shift) {
		state.sustain = 0;
		state.last_key ^= 1;
		mw->handle_sustain_release(state.view_index);
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
MppPianoTab :: drawTextBox(QPainter &paint, int index, qreal w, qreal h, qreal x, qreal y, const char *txt)
{
	r_pressed[index] = drawText(paint, Mpp.ColorBlack, state.pressed[index] ?
	    Mpp.ColorGrey : Mpp.ColorWhite, w, h, x, y, txt);
}

void
MppPianoTab :: paintEvent(QPaintEvent *event)
{
	QPainter paint(this);
	QFont fnt;
	qreal unit;
	qreal small;
	qreal ss;
	qreal dx;
	qreal sh;
	qreal uh;
	qreal uf;
	qreal uq;
	int len;
	qreal xpos;
	qreal ypos;
	int base = getBaseKey() / MPP_MAX_BANDS;
	int w = width();
	int h = height();
	int z;
	char buffer[16];
	const char *buf;

	paint.fillRect(QRectF(0,0,w,h), Mpp.ColorWhite);

	paint.setRenderHints(QPainter::Antialiasing, 1);

	z = (h + (2 * h / 3));
	if (w > z)
		w = z;

	dx = 3.0 * w / 25.0;
	ss = 3.0 * w / 75.0;

	unit = 2.0 * w / 25.0;
	small = 3.0 * w / 50.0;
	sh = 3.0 * w / 100.0;
	uh = w / 25.0;
	uf = w / 25.0;
	uq = w / 50.0;
	h /= 2;

	fnt.setPixelSize(uf);
	paint.setFont(fnt);

	ypos = h - 2 * unit + uh;
	drawTextBox(paint, MPP_D0B * MPP_BAND_STEP_12, unit, unit, unit / 2.0 + uq + dx * 0.5 - unit / 2.0, ypos, "#");
	drawTextBox(paint, MPP_E0B * MPP_BAND_STEP_12, unit, unit, unit / 2.0 + uq + dx * 1.5 - unit / 2.0, ypos, "#");
	drawTextBox(paint, MPP_G0B * MPP_BAND_STEP_12, unit, unit, unit / 2.0 + uq + dx * 3.5 - unit / 2.0, ypos, "#");
	drawTextBox(paint, MPP_A0B * MPP_BAND_STEP_12, unit, unit, unit / 2.0 + uq + dx * 4.5 - unit / 2.0, ypos, "#");
	drawTextBox(paint, MPP_H0B * MPP_BAND_STEP_12, unit, unit, unit / 2.0 + uq + dx * 5.5 - unit / 2.0, ypos, "#");

	ypos = h - unit + uh;
	snprintf(buffer, sizeof(buffer), "C%d", base);
	drawTextBox(paint, MPP_C0 * MPP_BAND_STEP_12, unit, unit, uq + dx * 0.0, ypos, buffer);
	snprintf(buffer, sizeof(buffer), "D%d", base);
	drawTextBox(paint, MPP_D0 * MPP_BAND_STEP_12, unit, unit, uq + dx * 1.0, ypos, buffer);
	snprintf(buffer, sizeof(buffer), "E%d", base);
	drawTextBox(paint, MPP_E0 * MPP_BAND_STEP_12, unit, unit, uq + dx * 2.0, ypos, buffer);
	snprintf(buffer, sizeof(buffer), "F%d", base);
	drawTextBox(paint, MPP_F0 * MPP_BAND_STEP_12, unit, unit, uq + dx * 3.0, ypos, buffer);
	snprintf(buffer, sizeof(buffer), "G%d", base);
	drawTextBox(paint, MPP_G0 * MPP_BAND_STEP_12, unit, unit, uq + dx * 4.0, ypos, buffer);
	snprintf(buffer, sizeof(buffer), "A%d", base);
	drawTextBox(paint, MPP_A0 * MPP_BAND_STEP_12, unit, unit, uq + dx * 5.0, ypos, buffer);
	snprintf(buffer, sizeof(buffer), "H%d", base);
	drawTextBox(paint, MPP_H0 * MPP_BAND_STEP_12, unit, unit, uq + dx * 6.0, ypos, buffer);

	ypos = h + unit;
	drawTextBox(paint, MPP_D0B * MPP_BAND_STEP_12 + MPP_MAX_BANDS, unit, unit, unit / 2.0 + uq + dx * 0.5 - unit / 2.0, ypos, "#");
	drawTextBox(paint, MPP_E0B * MPP_BAND_STEP_12 + MPP_MAX_BANDS, unit, unit, unit / 2.0 + uq + dx * 1.5 - unit / 2.0, ypos, "#");
	drawTextBox(paint, MPP_G0B * MPP_BAND_STEP_12 + MPP_MAX_BANDS, unit, unit, unit / 2.0 + uq + dx * 3.5 - unit / 2.0, ypos, "#");
	drawTextBox(paint, MPP_A0B * MPP_BAND_STEP_12 + MPP_MAX_BANDS, unit, unit, unit / 2.0 + uq + dx * 4.5 - unit / 2.0, ypos, "#");
	drawTextBox(paint, MPP_H0B * MPP_BAND_STEP_12 + MPP_MAX_BANDS, unit, unit, unit / 2.0 + uq + dx * 5.5 - unit / 2.0, ypos, "#");

	ypos = h + 2*unit;
	snprintf(buffer, sizeof(buffer), "C%d", base + 1);
	drawTextBox(paint, MPP_C0 * MPP_BAND_STEP_12 + MPP_MAX_BANDS, unit, unit, uq + dx * 0.0, ypos, buffer);
	snprintf(buffer, sizeof(buffer), "D%d", base + 1);
	drawTextBox(paint, MPP_D0 * MPP_BAND_STEP_12 + MPP_MAX_BANDS, unit, unit, uq + dx * 1.0, ypos, buffer);
	snprintf(buffer, sizeof(buffer), "E%d", base + 1);
	drawTextBox(paint, MPP_E0 * MPP_BAND_STEP_12 + MPP_MAX_BANDS, unit, unit, uq + dx * 2.0, ypos, buffer);
	snprintf(buffer, sizeof(buffer), "F%d", base + 1);
	drawTextBox(paint, MPP_F0 * MPP_BAND_STEP_12 + MPP_MAX_BANDS, unit, unit, uq + dx * 3.0, ypos, buffer);
	snprintf(buffer, sizeof(buffer), "G%d", base + 1);
	drawTextBox(paint, MPP_G0 * MPP_BAND_STEP_12 + MPP_MAX_BANDS, unit, unit, uq + dx * 4.0, ypos, buffer);
	snprintf(buffer, sizeof(buffer), "A%d", base + 1);
	drawTextBox(paint, MPP_A0 * MPP_BAND_STEP_12 + MPP_MAX_BANDS, unit, unit, uq + dx * 5.0, ypos, buffer);
	snprintf(buffer, sizeof(buffer), "H%d", base + 1);
	drawTextBox(paint, MPP_H0 * MPP_BAND_STEP_12 + MPP_MAX_BANDS, unit, unit, uq + dx * 6.0, ypos, buffer);

	uf /= 3.0;
	fnt.setPixelSize(uf);
	paint.setFont(fnt);

	xpos = 0;
	ypos = h - (3*unit);
	r_label[0] = drawText(paint, Mpp.ColorBlack, (state.last_jump == 0) ? Mpp.ColorYellow : Mpp.ColorWhite, 2.0 * uf, unit, uq + xpos * uf, ypos, "L0");
	xpos += 2;
	r_label[1] = drawText(paint, Mpp.ColorBlack, (state.last_jump == 1) ? Mpp.ColorYellow : Mpp.ColorWhite, 2.0 * uf, unit, uq + xpos * uf, ypos, "L1");
	xpos += 2;
	r_label[2] = drawText(paint, Mpp.ColorBlack, (state.last_jump == 2) ? Mpp.ColorYellow : Mpp.ColorWhite, 2.0 * uf, unit, uq + xpos * uf, ypos, "L2");
	xpos += 2;
	r_label[3] = drawText(paint, Mpp.ColorBlack, (state.last_jump == 3) ? Mpp.ColorYellow : Mpp.ColorWhite, 2.0 * uf, unit, uq + xpos * uf, ypos, "L3");
	xpos += 2;
	r_label[4] = drawText(paint, Mpp.ColorBlack, (state.last_jump == 4) ? Mpp.ColorYellow : Mpp.ColorWhite, 2.0 * uf, unit, uq + xpos * uf, ypos, "L4");
	xpos += 2;
	r_label[5] = drawText(paint, Mpp.ColorBlack, (state.last_jump == 5) ? Mpp.ColorYellow : Mpp.ColorWhite, 2.0 * uf, unit, uq + xpos * uf, ypos, "L5");
	xpos += 2;
	r_label[6] = drawText(paint, Mpp.ColorBlack, (state.last_jump == 6) ? Mpp.ColorYellow : Mpp.ColorWhite, 2.0 * uf, unit, uq + xpos * uf, ypos, "L6");
	xpos += 2;
	r_label[7] = drawText(paint, Mpp.ColorBlack, (state.last_jump == 7) ? Mpp.ColorYellow : Mpp.ColorWhite, 2.0 * uf, unit, uq + xpos * uf, ypos, "L7");
	xpos += 2;
	r_label[8] = drawText(paint, Mpp.ColorBlack, (state.last_jump == 8) ? Mpp.ColorYellow : Mpp.ColorWhite, 2.0 * uf, unit, uq + xpos * uf, ypos, "L8");
	xpos += 2;
	r_label[9] = drawText(paint, Mpp.ColorBlack, (state.last_jump == 9) ? Mpp.ColorYellow : Mpp.ColorWhite, 2.0 * uf, unit, uq + xpos * uf, ypos, "L9");
	xpos += 2;

#if MPP_PIANO_TAB_LABELS != 10
#error "MPP_PIANO_TAB_LABELS != 10"
#endif
	buf = "A-View";
	len = strlen(buf) + 2;
	r_view_a = drawText(paint, Mpp.ColorBlack, (state.view_index == 0) ? Mpp.ColorGrey : Mpp.ColorWhite, uf * len, unit, uq + xpos * uf, ypos, buf);
	xpos += len;

	buf = "B-View";
	len = strlen(buf) + 2;
	r_view_b = drawText(paint, Mpp.ColorBlack, (state.view_index == 1) ? Mpp.ColorGrey : Mpp.ColorWhite, uf * len, unit, uq + xpos * uf, ypos, buf);
	xpos += len;

	buf = "Sustain-OFF";
	len = strlen(buf) + 2;
	r_sustain_off = drawText(paint, Mpp.ColorBlack, state.sustain ? Mpp.ColorWhite : Mpp.ColorGrey, uf * len, unit, uq + xpos * uf, ypos, buf);
	xpos += len;

	buf = "Sustain-ON";
	len = strlen(buf) + 2;
	r_sustain_on = drawText(paint, Mpp.ColorBlack, state.sustain ? Mpp.ColorGrey : Mpp.ColorWhite , uf * len, unit, uq + xpos * uf, ypos, buf);
	xpos += len;
}
