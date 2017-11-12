/*-
 * Copyright (c) 2016 Hans Petter Selasky. All rights reserved.
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

#include "midipp_sheet.h"
#include "midipp_element.h"
#include "midipp_scores.h"
#include "midipp_mainwindow.h"
#include "midipp_gridlayout.h"
#include "midipp_buttonmap.h"

MppSheet::MppSheet(MppMainWindow * parent, int _unit)
{
	mw = parent;
	unit = _unit;
	num_rows = 0;
	num_cols = 0;
	entries_ptr = 0;
	entries_rows = 0;
	entries_cols = 0;
	mode = 0;
	delta_h = 0;
	delta_v = 0;

	sizeInit();

	gl_sheet = new MppGridLayout();
	gl_sheet->setSpacing(0);

	vs_vert = new QScrollBar(Qt::Vertical);
	vs_vert->setValue(0);
	vs_vert->setRange(0, 0);
	vs_vert->setPageStep(1);
	connect(vs_vert, SIGNAL(valueChanged(int)), this, SLOT(handleScrollChanged(int)));

	vs_horiz = new QScrollBar(Qt::Horizontal);
	vs_horiz->setValue(0);
	vs_horiz->setRange(0, 0);
	vs_horiz->setPageStep(1);
	connect(vs_horiz, SIGNAL(valueChanged(int)), this, SLOT(handleScrollChanged(int)));

        mode_map = new MppButtonMap("Mouse press mode\0" "Toggle ON or OFF\0"
	    "Increase duration\0" "Decrease duration\0", 3, 3);
        connect(mode_map, SIGNAL(selectionChanged(int)), this, SLOT(handleModeChanged(int)));

	gl_sheet->addWidget(mode_map, 0, 0, 1, 2);
	gl_sheet->addWidget(this, 1, 0, 1, 1);
	gl_sheet->addWidget(vs_vert, 1, 1, 2, 1);
	gl_sheet->addWidget(vs_horiz, 2, 0, 1, 2);
	gl_sheet->setRowStretch(1, 1);
	gl_sheet->setColumnStretch(1, 1);
}

MppSheet::~MppSheet()
{
	free(entries_ptr);
	entries_ptr = 0;
	free(entries_rows);
	entries_rows = 0;
	free(entries_cols);
	entries_cols = 0;
	num_rows = 0;
	num_cols = 0;
}

void
MppSheet::sizeInit()
{
	boxs = 1.75 * QFontInfo(mw->editFont).pixelSize();
	xoff = boxs * 6;
	yoff = boxs * 2;
}

static const QString
MppTransToString(int trans_number, int trans_mode)
{
	if (trans_mode != 0) {
		return QString("X%1.%2 ").arg(trans_number).arg(trans_mode);
	} else {
		return QString("X%1 ").arg(trans_number);
	}
}

static const QString
MppSheetRowToString(MppSheetRow * ptr)
{
	QString ret;

	switch (ptr->type) {
	case MPP_T_MACRO:
		if (ptr->u.macro.chan != 0)
			ret += QString("T%1 ").arg(ptr->u.macro.chan);
		if (ptr->u.macro.trans_mode != 0 || ptr->u.macro.trans_number != 0) {
			ret += MppTransToString(ptr->u.macro.trans_number,
			    ptr->u.macro.trans_mode);
		}
		ret += QString("M%1").arg(ptr->u.macro.num);
		break;
	case MPP_T_SCORE:
		if (ptr->u.score.chan != 0)
			ret += QString("T%1 ").arg(ptr->u.score.chan);
		if (ptr->u.score.trans_mode != 0 || ptr->u.score.trans_number != 0) {
			ret += MppTransToString(ptr->u.score.trans_number,
			    ptr->u.score.trans_mode);
		}
		ret += QString("%1").arg(mid_key_str[ptr->u.score.num]);
		break;
	default:
		break;
	}
	return (ret);
}

static int
MppSheetRowCompareType(const void *_pa, const void *_pb)
{
	MppSheetRow *pa = (MppSheetRow *) _pa;
	MppSheetRow *pb = (MppSheetRow *) _pb;
	int ret;

	ret = pb->type - pa->type;
	if (ret != 0)
		return (ret);
	switch (pa->type) {
	case MPP_T_MACRO:
		ret = pa->u.macro.chan - pb->u.macro.chan;
		if (ret != 0)
			return (ret);
		ret = pa->u.macro.trans_number - pb->u.macro.trans_number;
		if (ret != 0)
			return (ret);
		ret = pa->u.macro.trans_mode - pb->u.macro.trans_mode;
		if (ret != 0)
			return (ret);
		ret = pa->u.macro.num - pb->u.macro.num;
		if (ret != 0)
			return (ret);
		break;
	case MPP_T_SCORE:
		ret = pa->u.score.chan - pb->u.score.chan;
		if (ret != 0)
			return (ret);
		ret = pa->u.score.trans_number - pb->u.score.trans_number;
		if (ret != 0)
			return (ret);
		ret = pa->u.score.trans_mode - pb->u.score.trans_mode;
		if (ret != 0)
			return (ret);
		ret = pa->u.score.num - pb->u.score.num;
		if (ret != 0)
			return (ret);
		break;
	default:
		break;
	}
	return (0);
}

static int
MppSheetRowCompare(const void *_pa, const void *_pb)
{
	MppSheetRow *pa = (MppSheetRow *) _pa;
	MppSheetRow *pb = (MppSheetRow *) _pb;
	int ret;

	ret = MppSheetRowCompareType(_pa, _pb);
	if (ret != 0)
		return (ret);

	ret = pa->col - pb->col;
	if (ret != 0)
		return (ret);
	return (0);
}

const QString
MppSheet::outputColumn(ssize_t col)
{
	QString ret;
	int chan = 0;
	int dur = 1;
	int trans_number = 0;
	int trans_mode = 0;
	int any = 0;
	ssize_t x;

	for (x = 0; x != num_rows; x++) {
		int ndur = entries_ptr[col + (x * num_cols)];
		if (ndur < 1)
			continue;
		switch (entries_rows[x].type) {
		case MPP_T_MACRO:
			if (chan != entries_rows[x].u.macro.chan) {
				chan = entries_rows[x].u.macro.chan;
				ret += QString("T%1 ").arg(chan);
			}
			if (trans_number != entries_rows[x].u.macro.trans_number ||
			    trans_mode != entries_rows[x].u.macro.trans_mode) {
				trans_number = entries_rows[x].u.macro.trans_number;
				trans_mode = entries_rows[x].u.macro.trans_mode;
				ret += MppTransToString(trans_number, trans_mode);
			}
			ret += QString("M%1 ").arg(entries_rows[x].u.macro.num);
			break;
		case MPP_T_SCORE:
			if (any == 0 || ndur != dur) {
				dur = ndur;
				ret += QString("U%1%2 ").arg((dur + 1)/2).arg((dur & 1) ? "" : ".");
				any = 1;
			}
			if (chan != entries_rows[x].u.score.chan) {
				chan = entries_rows[x].u.score.chan;
				ret += QString("T%1 ").arg(chan);
			}
			if (trans_number != entries_rows[x].u.score.trans_number ||
			    trans_mode != entries_rows[x].u.score.trans_mode) {
				trans_number = entries_rows[x].u.score.trans_number;
				trans_mode = entries_rows[x].u.score.trans_mode;
				ret += MppTransToString(trans_number, trans_mode);
			}
			ret += QString("%1 ").arg(
			    mid_key_str[entries_rows[x].u.score.num]);
			break;
		default:
			break;
		}
	}
	if (entries_cols[col].pre_timer > -1 &&
	    entries_cols[col].post_timer > -1) {
		ret += QString("W%1.%2 ").arg(entries_cols[col].pre_timer)
		  .arg(entries_cols[col].post_timer);
	}
	return (ret.trimmed());
}

void
MppSheet::compile(MppHead & head)
{
	MppSheetRow *ptemp;
	MppElement *ptr;
	MppElement *start;
	MppElement *stop;
	int label;
	size_t n;
	size_t x;

	free(entries_ptr);
	entries_ptr = 0;
	free(entries_rows);
	entries_rows = 0;
	free(entries_cols);
	entries_cols = 0;
	num_rows = 0;
	num_cols = 0;
	vs_horiz->setMaximum(0);
	vs_vert->setMaximum(0);

	start = stop = 0;
	n = 0;
	while (head.foreachLine(&start, &stop)) {
		int any = 0;
		for (ptr = start; ptr != stop; ptr = ptr->next()) {
			switch (ptr->type) {
			case MPP_T_MACRO:
			case MPP_T_SCORE:
				n++;
				any = 1;
				break;
			case MPP_T_TIMER:
				any = 1;
				break;
			default:
				break;
			}
		}
		if (any)
			num_cols++;
	}
	if (n == 0)
		return;

	x = n * sizeof(struct MppSheetRow);
	ptemp = (struct MppSheetRow *)malloc(x);
	memset(ptemp, 0, x);

	x = num_cols * sizeof(struct MppSheetCol);
	entries_cols = (struct MppSheetCol *)malloc(x);
	memset(entries_cols, 255, x);

	num_cols = 0;
	label = -1;
	start = stop = 0;
	n = 0;
	while (head.foreachLine(&start, &stop)) {
		int chan = 0;
		int dur = 1;
		int trans_number = 0;
		int trans_mode = 0;
		int any = 0;

		for (ptr = start; ptr != stop; ptr = ptr->next()) {
			switch (ptr->type) {
			case MPP_T_LABEL:
				label = ptr->value[0];
				break;
			case MPP_T_CHANNEL:
				chan = ptr->value[0];
				break;
			case MPP_T_DURATION:
				dur = ptr->value[0];
				break;
			case MPP_T_TRANSPOSE:
				trans_number = ptr->value[0];
				trans_mode = ptr->value[1];
				break;
			case MPP_T_MACRO:
				entries_cols[num_cols].label = label;
				entries_cols[num_cols].line = ptr->line;
				ptemp[n].type = ptr->type;
				ptemp[n].label = label;
				ptemp[n].col = num_cols;
				ptemp[n].u.macro.chan = chan;
				ptemp[n].u.macro.trans_number = trans_number;
				ptemp[n].u.macro.trans_mode = trans_mode;
				ptemp[n].u.macro.num = ptr->value[0];
				n++;
				any = 1;
				break;
			case MPP_T_SCORE:
				entries_cols[num_cols].label = label;
				entries_cols[num_cols].line = ptr->line;
				ptemp[n].type = ptr->type;
				ptemp[n].label = label;
				ptemp[n].col = num_cols;
				ptemp[n].u.score.chan = chan;
				ptemp[n].u.score.trans_number = trans_number;
				ptemp[n].u.score.trans_mode = trans_mode;
				ptemp[n].u.score.dur = dur;
				ptemp[n].u.score.num = ptr->value[0];
				n++;
				any = 1;
				break;
			case MPP_T_TIMER:
				entries_cols[num_cols].label = label;
				entries_cols[num_cols].line = ptr->line;
				entries_cols[num_cols].pre_timer = ptr->value[0];
				entries_cols[num_cols].post_timer = ptr->value[1];
				any = 1;
				break;
			default:
				break;
			}
		}
		if (any)
			num_cols++;
	}

	qsort(ptemp, n, sizeof(ptemp[0]), &MppSheetRowCompare);

	num_rows = 0;
	for (x = 0; x != n; x++) {
		if (x == 0 ||
		    MppSheetRowCompareType(ptemp + x - 1,
					     ptemp + x))
			num_rows++;
	}
	
	x = num_rows * sizeof(struct MppSheetRow);
	entries_rows = (struct MppSheetRow *)malloc(x);
	memset(entries_rows, 0, x);

	x = num_rows * num_cols * sizeof(int);
	entries_ptr = (int *)malloc(x);
	memset(entries_ptr, 0, x);

	num_rows = 0;
	for (x = 0; x != n; x++) {
		if (x == 0 ||
		    MppSheetRowCompareType(ptemp + x - 1,
					     ptemp + x)) {
			entries_rows[num_rows] = ptemp[x];
			num_rows++;
		}
		switch (ptemp[x].type) {
		case MPP_T_MACRO:
			entries_ptr[ptemp[x].col + ((num_rows - 1) * num_cols)] = 1;
			break;
		case MPP_T_SCORE:
			entries_ptr[ptemp[x].col + ((num_rows - 1) * num_cols)] =
			    ptemp[x].u.score.dur;
			break;
		default:
			break;
		}
	}	
	free(ptemp);

	vs_horiz->setMaximum(num_cols - 1);
	vs_vert->setMaximum(num_rows - 1);

	update();
}

void
MppSheet::paintEvent(QPaintEvent * event)
{
	MppScoreMain *sm = mw->scores_main[unit];
  	MppElement *curr;
	MppElement *last;
	QPainter paint(this);
	ssize_t x;
	ssize_t y;
	ssize_t xstart;
	ssize_t ystart;
	ssize_t xstop;
	ssize_t ystop;
	qreal ypos;
	qreal xpos;
	ssize_t curr_line;
	ssize_t last_line;
	int label;

	paint.fillRect(QRectF(0, 0, width(), height()), Mpp.ColorWhite);

	if (entries_ptr == 0 || entries_rows == 0 ||
	    entries_cols == 0 || num_cols == 0 || num_rows == 0)
		return;

	sizeInit();
	paint.setFont(mw->editFont);
	
	mw->atomic_lock();
	curr = sm->head.state.curr_start;
	if (curr != 0)
		curr_line = curr->line;
	else
		curr_line = -1;
	last = sm->head.state.last_start;
	if (last != 0)
		last_line = last->line;
	else
		last_line = -1;
	mw->atomic_unlock();

	if (curr_line > -1) {
		for (x = 0; x != num_cols; x++) {
			if (curr_line == entries_cols[x].line)
				break;
		}
		curr_line = x;
	}
	if (last_line > -1) {
		for (x = 0; x != num_cols; x++) {
			if (last_line == entries_cols[x].line)
				break;
		}
		last_line = x;
	}
	paint.setRenderHints(QPainter::Antialiasing, 1);

	xstart = vs_horiz->value();
	ystart = vs_vert->value();

	xstop = xstart + ((width() - xoff + boxs - 1) / boxs);
	if (xstop > num_cols)
		xstop = num_cols;
	ystop = ystart + ((height() - yoff + boxs - 1) / boxs);
	if (ystop > num_rows)
		ystop = num_rows;

	/* print row labels */
	for (y = ystart; y < ystop; y++) {
		ypos = boxs * (y - ystart);
		paint.setPen(QPen(Mpp.ColorBlack, 0));
		paint.setBrush(Mpp.ColorBlack);
		paint.drawText(QRectF(0, yoff + ypos, xoff, boxs),
		    Qt::AlignCenter | Qt::TextSingleLine,
		    MppSheetRowToString(entries_rows + y));
	}

	/* print column labels */
	label = -1;
	for (x = xstart; x < xstop; x++) {
		if (label == entries_cols[x].label)
			continue;
		label = entries_cols[x].label;
		xpos = boxs * (x - xstart);
		paint.setPen(QPen(Mpp.ColorBlack, 0));
		paint.setBrush(Mpp.ColorBlack);
		paint.drawText(QRectF(xoff + xpos, yoff - boxs, boxs * 8.0, boxs),
		    Qt::AlignLeft | Qt::TextSingleLine,
		    QString("L%1").arg(label));
	}

	/* draw empty boxes */
	for (x = xstart; x < xstop; x++) {
		for (y = ystart; y < ystop; y++) {
			ypos = boxs * (y - ystart);

			paint.setPen(QPen(Mpp.ColorBlack, 4));
			if (x == curr_line)
				paint.setBrush(Mpp.ColorLogo);
			else if (x == last_line)
				paint.setBrush(Mpp.ColorGreen);
			else
				paint.setBrush(Mpp.ColorGrey);

			paint.drawRect(QRectF(
			    xoff + (x - xstart) * boxs,
			    yoff + ypos,
			    boxs, boxs));
		}
	}
	
	/* draw filled boxes, if any */
	for (x = xstart; x < xstop; x++) {
		for (y = ystart; y < ystop; y++) {
			int dur = entries_ptr[x + (y * num_cols)];
			if (dur < 1)
				continue;
			ypos = boxs * (y - ystart);
			xpos = xoff + boxs * (x - xstart);
			paint.setPen(QPen(Mpp.ColorGrey, 0));
			paint.setBrush(Mpp.ColorBlack);
			paint.drawEllipse(QRectF(xpos + 3, yoff + ypos + 3,
			    (boxs * (dur + 1)) / 2 - 6, boxs - 6));
		}
	}
}

int
MppSheet :: getTranspose(int trans_mode)
{
	int temp;

	switch (trans_mode) {
	case 1:
		temp = mw->getCurrTransposeScore();
		if (temp >= 0)
			return (temp);
		else
			return (MPP_INVALID_TRANSPOSE);
	case 2:
		temp = mw->getCurrTransposeScore();
		if (temp >= 0)
			return (temp % 12);
		else
			return (MPP_INVALID_TRANSPOSE);
	default:
		break;
	}
	return (0);
}

void
MppSheet::mousePressEvent(QMouseEvent * event)
{
	MppScoreMain *sm = mw->scores_main[unit];
	QPoint p = event->pos();
	ssize_t x = ((p.x() - xoff + boxs - 1) / boxs) - 1;
	ssize_t y = ((p.y() - yoff + boxs - 1) / boxs) - 1;

	if (x >= 0)
		x += vs_horiz->value();
	if (y >= 0)
		y += vs_vert->value();

	if (x >= 0 && x < num_cols &&
	    y >= 0 && y < num_rows) {
		ssize_t z = x + y * num_cols;

		switch (mode) {
		case 0:
			entries_ptr[z] = -entries_ptr[z];
			if (entries_ptr[z] == 0)
				entries_ptr[z] = 1;
			break;
		case 1:
			if (entries_ptr[z] > 0 && entries_ptr[z] < MPP_MAX_DURATION)
				entries_ptr[z]++;
			break;
		case 2:
			if (entries_ptr[z] > 1)
				entries_ptr[z]--;
			break;
		default:
			break;
		}

		QString result = outputColumn(x) + QString("\n");
		QTextCursor cursor(sm->editWidget->textCursor());

		cursor.beginEditBlock();
		cursor.movePosition(QTextCursor::Start, QTextCursor::MoveAnchor, 1);
		cursor.movePosition(QTextCursor::Down, QTextCursor::MoveAnchor, entries_cols[x].line);
		cursor.movePosition(QTextCursor::Down, QTextCursor::KeepAnchor, 1);
		cursor.removeSelectedText();
		cursor.insertText(result);
		cursor.endEditBlock();

		update();
	}
	if (x >= 0 && x < num_cols) {
		for (y = 0; y != num_rows; y++) {
			int num;
			int chan;
			if (entries_ptr[x + (y * num_cols)] < 1)
				continue;
			switch (entries_rows[y].type) {
			case MPP_T_SCORE:
				num = entries_rows[y].u.score.num +
				    entries_rows[y].u.score.trans_number +
				    getTranspose(entries_rows[y].u.score.trans_mode);
				if (num < 0 || num > 127)
					break;
				chan = (sm->synthChannel +
				    entries_rows[y].u.score.chan) & 0xF;
				mw->output_key(chan, num, 75, 0, 0);
				entries_rows[y].playing = 1;
				entries_rows[y].playkey = num;
				entries_rows[y].playchan = chan;
				break;
			default:
				break;
			}
		}
	} else if (y >= 0 && y < num_rows) {
		int num;
		int chan;
		switch (entries_rows[y].type) {
		case MPP_T_SCORE:
			num = entries_rows[y].u.score.num +
			    entries_rows[y].u.score.trans_number +
			    getTranspose(entries_rows[y].u.score.trans_mode);
			if (num < 0 || num > 127)
				break;
			chan = (sm->synthChannel +
			    entries_rows[y].u.score.chan) & 0xF;
			mw->output_key(chan, num, 75, 0, 0);
			entries_rows[y].playing = 1;
			entries_rows[y].playkey = num;
			entries_rows[y].playchan = chan;
			break;
		default:
			break;
		}
	}
	event->accept();
}

void
MppSheet::mouseReleaseEvent(QMouseEvent * event)
{
	ssize_t y;

	for (y = 0; y != num_rows; y++) {
		int num;
		int chan;
		if (entries_rows[y].playing == 0)
			continue;
		entries_rows[y].playing = 0;

		switch (entries_rows[y].type) {
		case MPP_T_SCORE:
			num = entries_rows[y].playkey;
			chan = entries_rows[y].playchan;
			mw->output_key(chan, num, 0, 0, 0);
			break;
		default:
			break;
		}
	}
	event->accept();
}

void
MppSheet::handleScrollChanged(int value)
{
	update();
}

void
MppSheet::handleModeChanged(int value)
{
	mode = value;
}

void
MppSheet :: wheelEvent(QWheelEvent *event)
{
	if (event->orientation() == Qt::Horizontal) {
		delta_h -= event->delta();
		int delta = delta_h / MPP_WHEEL_STEP;
		delta_h %= MPP_WHEEL_STEP;
		if (delta != 0) {
			delta += vs_horiz->value();
			if (delta < 0)
				delta = 0;
			else if (delta > vs_horiz->maximum())
				delta = vs_horiz->maximum();
			vs_horiz->setValue(delta);
		}
	} else if (event->orientation() == Qt::Vertical) {
		delta_v -= event->delta();
		int delta = delta_v / MPP_WHEEL_STEP;
		delta_v %= MPP_WHEEL_STEP;
		if (delta != 0) {
			delta += vs_vert->value();
			if (delta < 0)
				delta = 0;
			else if (delta > vs_vert->maximum())
				delta = vs_vert->maximum();
			vs_vert->setValue(delta);
		}
	}
	event->accept();
}

void
MppSheet :: watchdog()
{
	MppScoreMain *sm = mw->scores_main[unit];
	int delta = (width() - xoff) / boxs;
	MppElement *curr;
	ssize_t x;
	int y;

	mw->atomic_lock();
	curr = sm->head.state.curr_start;
	mw->atomic_unlock();

	/* range check */
	if (delta < 1)
		delta = 1;

	if (curr != 0) {
		int line = curr->line;
		for (x = 0; x != num_cols; x++) {
			if (entries_cols[x].line == line)
				break;
		}
		if (x != num_cols) {
			y = vs_horiz->value();
			if (x > y)
				y += ((x - y) / delta) * delta;
			else
				y -= ((y - x + delta - 1) / delta) * delta;
			if (y < 0)
				y = 0;
			else if (y > vs_horiz->maximum())
				y = vs_horiz->maximum();

			vs_horiz->setValue(y);
		}
	}
}
