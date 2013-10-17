/*-
 * Copyright (c) 2009-2013 Hans Petter Selasky. All rights reserved.
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

#include "midipp_mainwindow.h"
#include "midipp_scores.h"
#include "midipp_looptab.h"
#include "midipp_pattern.h"
#include "midipp_bpm.h"
#include "midipp_mode.h"
#include "midipp_decode.h"
#include "midipp_import.h"
#include "midipp_replace.h"
#include "midipp_spinbox.h"
#include "midipp_groupbox.h"
#include "midipp_button.h"
#include "midipp_show.h"

static int
MppCountNewline(const QString &str)
{
	int x;
	int y = 0;
	QChar ch = '\0';

	if (str.size() == 0)
		return (0);

	for (x = 0; x != str.size(); x++) {
		ch = str[x];
		if (ch == '\n')
			y++;
	}
	if (ch != '\n')
		y++;
	return (y);
}

MppScoreView :: MppScoreView(MppScoreMain *parent)
{
	pScores = parent;
	setFocusPolicy(Qt::ClickFocus);
}

void
MppScoreView :: mousePressEvent(QMouseEvent *e)
{
	pScores->viewMousePressEvent(e);
}

void
MppScoreView :: wheelEvent(QWheelEvent *event)
{
	QScrollBar *ps = pScores->viewScroll;
	int value;

	value = ps->value();

	if (event->delta() < 0)
		value ++;
	else if (event->delta() > 0)
		value --;
	else
		goto done;

	if (value >= ps->minimum() && value <= ps->maximum())
		ps->setValue(value);
done:
	event->accept();
}

void
MppScoreView :: keyPressEvent(QKeyEvent *event)
{
	QScrollBar *ps = pScores->viewScroll;
	int value;

	value = ps->value();

	switch (event->key()) {
	case Qt::Key_PageDown:
	case Qt::Key_Down:
		value ++;
		break;
	case Qt::Key_Up:
	case Qt::Key_PageUp:
		value --;
		break;
	case Qt::Key_End:
		value = ps->maximum();
		break;
	case Qt::Key_Home:
		value = 0;
		break;
	default:
		goto done;
	}

	if (value >= ps->minimum() && value <= ps->maximum())
		ps->setValue(value);
 done:
	event->accept();
}

void
MppScoreView :: paintEvent(QPaintEvent *event)
{
	pScores->viewPaintEvent(event);
}

MppScoreTextEdit :: MppScoreTextEdit(MppScoreMain *parent)
{
	sm = parent;
}

MppScoreTextEdit :: ~MppScoreTextEdit(void)
{

}

void
MppScoreTextEdit :: mouseDoubleClickEvent(QMouseEvent *e)
{
	/* edit the line */
	if (sm->handleEditLine() == 0)
		sm->mainWindow->handle_compile();
}

MppScoreMain :: MppScoreMain(MppMainWindow *parent, int _unit)
{
	QString defaultText;
	char buf[32];

	if (_unit == 0) {
	  defaultText = tr(
	    "S\"L0 - verse: \"\n"
	    "\n"
	    "S\".(C)Welcome .(C)to .(D)MIDI .(E)Player .(C)Pro! \"\n"
	    "\n"
	    "L0:\n"
	    "U1 C3 C4 C5 E5 G5 /* C */\n"
	    "U1 C3 C4 C5 E5 G5 /* C */\n"
	    "U1 D3 D4 D5 G5B A5 /* D */\n"
	    "U1 E3 E4 E5 A5B H5 /* E */\n"
	    "U1 C3 C4 C5 E5 G5 /* C */\n"
	    "J0\n");
	}

	snprintf(buf, sizeof(buf), "%c-scores", 'A' + _unit);

	/* set memory default */

	memset(auto_zero_start, 0, auto_zero_end - auto_zero_start);

	/* all devices are input */

	devInputMask = 0;
	baseKey = MPP_DEFAULT_BASE_KEY;
	cmdKey = MPP_DEFAULT_CMD_KEY;
	delayNoise = 25;
	chordContrast = 128;
	chordNormalize = 1;
	unit = _unit;

	/* Set parent */

	mainWindow = parent;

	/* Buttons */

	butScoreFileNew = new QPushButton(tr("New"));
	butScoreFileOpen = new QPushButton(tr("Open"));
	butScoreFileSave = new QPushButton(tr("Save"));
	butScoreFileSaveAs = new QPushButton(tr("Save As"));
	butScoreFilePrint = new QPushButton(tr("Print"));
	butScoreFileAlign = new QPushButton(tr("Align"));
	spnScoreFileAlign = new MppSpinBox();
	spnScoreFileAlign->setValue(F5);
	butScoreFileScale = new QPushButton(tr("Scale"));
	spnScoreFileScale = new QSpinBox();
	spnScoreFileScale->setRange(0, 60000);
	spnScoreFileScale->setSuffix(" ms");
	spnScoreFileScale->setValue(1000);
	butScoreFileStepUp = new QPushButton(tr("Step Up"));
	butScoreFileStepDown = new QPushButton(tr("Step Down"));
	butScoreFileSetSharp = new QPushButton(tr("Set #"));
	butScoreFileSetFlat = new QPushButton(tr("Set b"));
	butScoreFileAutoMel[0] = new MppButton(tr("AutoMel 1"), 0);
	butScoreFileAutoMel[1] = new MppButton(tr("AutoMel 2"), 1);
	butScoreFileEditChord = new QPushButton(tr("Edit Chord"));
	butScoreFileInsertChord = new QPushButton(tr("Insert Chord"));
	butScoreFileReplaceAll = new QPushButton(tr("Replace All"));
	butScoreFileExport = new QPushButton(tr("To Lyrics"));

#ifdef QT_NO_PRINTER
	butScoreFilePrint->hide();
#endif
	gbScoreFile = new MppGroupBox(tr(buf));
	gbScoreFile->addWidget(butScoreFileNew, 0, 0, 1, 2);
	gbScoreFile->addWidget(butScoreFileOpen, 1, 0, 1, 2);
	gbScoreFile->addWidget(butScoreFileSave, 2, 0, 1, 2);
	gbScoreFile->addWidget(butScoreFileSaveAs, 3, 0, 1, 2);
	gbScoreFile->addWidget(butScoreFilePrint, 4, 0, 1, 2);
	gbScoreFile->addWidget(butScoreFileAlign, 5, 0, 1, 1);
	gbScoreFile->addWidget(spnScoreFileAlign, 5, 1, 1, 1);
	gbScoreFile->addWidget(butScoreFileScale, 6, 0, 1, 1);
	gbScoreFile->addWidget(spnScoreFileScale, 6, 1, 1, 1);
	gbScoreFile->addWidget(butScoreFileStepUp, 7, 0, 1, 1);
	gbScoreFile->addWidget(butScoreFileStepDown, 7, 1, 1, 1);
	gbScoreFile->addWidget(butScoreFileSetSharp, 8, 0, 1, 1);
	gbScoreFile->addWidget(butScoreFileSetFlat, 8, 1, 1, 1);
	gbScoreFile->addWidget(butScoreFileAutoMel[0], 9, 0, 1, 1);
	gbScoreFile->addWidget(butScoreFileAutoMel[1], 9, 1, 1, 1);
	gbScoreFile->addWidget(butScoreFileEditChord, 10, 0, 1, 1);
	gbScoreFile->addWidget(butScoreFileInsertChord, 10, 1, 1, 1);
	gbScoreFile->addWidget(butScoreFileReplaceAll, 11, 0, 1, 2);
	gbScoreFile->addWidget(butScoreFileExport, 12, 0, 1, 2);

	connect(butScoreFileNew, SIGNAL(released()), this, SLOT(handleScoreFileNew()));
	connect(butScoreFileOpen, SIGNAL(released()), this, SLOT(handleScoreFileOpen()));
	connect(butScoreFileSave, SIGNAL(released()), this, SLOT(handleScoreFileSave()));
	connect(butScoreFileSaveAs, SIGNAL(released()), this, SLOT(handleScoreFileSaveAs()));
	connect(butScoreFilePrint, SIGNAL(released()), this, SLOT(handleScorePrint()));
	connect(butScoreFileAlign, SIGNAL(released()), this, SLOT(handleScoreFileAlign()));
	connect(butScoreFileStepUp, SIGNAL(released()), this, SLOT(handleScoreFileStepUp()));
	connect(butScoreFileStepDown, SIGNAL(released()), this, SLOT(handleScoreFileStepDown()));
	connect(butScoreFileSetSharp, SIGNAL(released()), this, SLOT(handleScoreFileSetSharp()));
	connect(butScoreFileSetFlat, SIGNAL(released()), this, SLOT(handleScoreFileSetFlat()));
	connect(butScoreFileScale, SIGNAL(released()), this, SLOT(handleScoreFileScale()));
	connect(butScoreFileAutoMel[0], SIGNAL(released(int)), this, SLOT(handleScoreFileAutoMelody(int)));
	connect(butScoreFileAutoMel[1], SIGNAL(released(int)), this, SLOT(handleScoreFileAutoMelody(int)));
	connect(butScoreFileEditChord, SIGNAL(released()), this, SLOT(handleScoreFileEditChord()));
	connect(butScoreFileInsertChord, SIGNAL(released()), this, SLOT(handleScoreFileInsertChord()));
	connect(butScoreFileReplaceAll, SIGNAL(released()), this, SLOT(handleScoreFileReplaceAll()));
	connect(butScoreFileExport, SIGNAL(released()), this, SLOT(handleScoreFileExport()));

	/* Widget */

	viewWidget.setContentsMargins(0,0,0,0);

	/* Editor */

	editWidget = new MppScoreTextEdit(this);

	editWidget->setFont(parent->editFont);
	editWidget->setPlainText(defaultText);
	editWidget->setCursorWidth(4);
	editWidget->setLineWrapMode(QPlainTextEdit::NoWrap);

	/* GridLayout */

	viewGrid = new QGridLayout(&viewWidget);
	viewGrid->setSpacing(0);
	viewGrid->setContentsMargins(1,1,1,1);

	viewScroll = new QScrollBar(Qt::Vertical);
	viewScroll->setValue(0);
	viewScroll->setRange(0,0);
	viewScroll->setPageStep(1);

	connect(viewScroll, SIGNAL(valueChanged(int)), this, SLOT(handleScrollChanged(int)));

	/* Visual */

	viewWidgetSub = new MppScoreView(this);

	viewGrid->addWidget(viewWidgetSub, 0, 0, 1, 1);
	viewGrid->addWidget(viewScroll, 0, 1, 1, 1);

	/* Initial compile */

	handleCompile(1);

	QPainter paint;

	picChord[0] = new QPicture();
	paint.begin(picChord[0]);
	paint.setRenderHints(QPainter::Antialiasing, 1);
 	paint.setPen(QPen(color_black, 1));
	paint.setBrush(QColor(color_black));
	paint.drawEllipse(QRect(0,0,MPP_VISUAL_C_MAX,MPP_VISUAL_C_MAX));
	paint.end();

	picChord[1] = new QPicture();
	paint.begin(picChord[1]);
	paint.setRenderHints(QPainter::Antialiasing, 1);
 	paint.setPen(QPen(color_black, 1));
	paint.setBrush(QColor(color_black));
	paint.drawEllipse(QRect(MPP_VISUAL_C_MAX,0,MPP_VISUAL_C_MAX,MPP_VISUAL_C_MAX));
	paint.end();
}

MppScoreMain :: ~MppScoreMain()
{
	handleScoreFileNew();
}

void
MppScoreMain :: handlePrintSub(QPrinter *pd, QPoint orig, float scale_f)
{
	MppVisualDot *pdot;
	MppElement *ptr;
	MppElement *next;
	QPainter paint;
	QString linebuf;
	QString chord;
	QRectF box;
	qreal chord_x_max;
	qreal offset;
	int last_dot;
	int dur;
	int x;
	int y;
	int z;

	QFont fnt_a;
	QFont fnt_b;

	fnt_a = mainWindow->defaultFont;
	fnt_a.setPixelSize(mainWindow->defaultFont.pixelSize());

	fnt_b = mainWindow->defaultFont;
	fnt_b.setPixelSize(mainWindow->defaultFont.pixelSize() + 4);

	QFontMetricsF fm_b(fnt_b);

	if (pd != NULL) {
#ifndef QT_NO_PRINTER
		/* get biggest number of score lines in a page */
		x = visual_p_max;
		if (x != 0) {
			qreal scale_min = ((qreal)(pd->height() - (2 * orig.y()))) / 
			    (((qreal)x) * visual_y_max);

			if (scale_min < 0)
				scale_f = 0.5;	/* dummy */
			else if (scale_min < scale_f)
				scale_f = scale_min;
		}

		/* translate printing area */
		paint.begin(pd);
		paint.translate(orig);
		paint.scale(scale_f, scale_f);
		paint.translate(QPoint(-MPP_VISUAL_MARGIN,-MPP_VISUAL_MARGIN));
#endif
	}

	for (x = y = 0; x != visual_max; x++) {
		if (pd == NULL) {
			delete (pVisual[x].pic);
			pVisual[x].pic = new QPicture();
			paint.begin(pVisual[x].pic);
			paint.setRenderHints(QPainter::Antialiasing, 1);
		}

		paint.setPen(QPen(color_black, 1));
		paint.setBrush(QColor(color_black));

		linebuf = QString();
		z = 0;
		last_dot = 0;
		chord_x_max = 0;

		for (ptr = pVisual[x].start; ptr != pVisual[x].stop;
		    ptr = TAILQ_NEXT(ptr, entry)) {

			paint.setFont(fnt_a);

			if (ptr->type == MPP_T_STRING_DOT) {
				if (last_dot != 0)
					linebuf += ' ';
				last_dot = 1;
			} else {
				last_dot = 0;
			}
		retry:
			box = paint.boundingRect(QRectF(0,0,0,0),
			    Qt::TextSingleLine | Qt::AlignLeft, linebuf);

			if (ptr->type != MPP_T_STRING_DESC &&
			    box.width() < chord_x_max && linebuf.size() < 256) {
				int t;
				for (t = linebuf.size() - 1; t > -1; t--) {
					if (!linebuf[t].isSpace())
						break;
					if (linebuf[t] == '-') {
						/* insert space before last dash */
						linebuf = linebuf.mid(0,t) + ' ' +
						    linebuf.mid(t, linebuf.size() - t);
						break;
					}
				}
				linebuf += ' ';
				goto retry;
			}

			switch (ptr->type) {
			case MPP_T_STRING_DESC:
				linebuf += ptr->txt;
				break;

			case MPP_T_STRING_DOT:
				pdot = &pVisual[x].pdot[z++];
				if (z > pVisual[x].ndot)
					break;

				pdot->x_off = MPP_VISUAL_MARGIN + box.width();
				pdot->y_off = MPP_VISUAL_MARGIN + (visual_y_max / 3);

				for (next = ptr; next != pVisual[x].stop;
				     next = TAILQ_NEXT(next, entry)) {
					if (next->type == MPP_T_STRING_CHORD &&
					    next->txt.size() > 1 && next->txt[0] == '(')
						break;
				}
				if (next != pVisual[x].stop)
					pdot->x_off += (fm_b.width(next->txt[1]) - MPP_VISUAL_R_MAX - 2) / 2.0;

				dur = ptr->value[0];

				paint.drawEllipse(QRectF(pdot->x_off, pdot->y_off,
				    MPP_VISUAL_R_MAX, MPP_VISUAL_R_MAX));

				if (dur <= 0)
					break;
				if (dur > 5)
					dur = 5;

				offset = 0;

				paint.drawLine(
				    pdot->x_off + MPP_VISUAL_R_MAX,
				    pdot->y_off + (MPP_VISUAL_R_MAX / 2),
				    pdot->x_off + MPP_VISUAL_R_MAX,
				    pdot->y_off + (MPP_VISUAL_R_MAX / 2) -
				    (3 * MPP_VISUAL_R_MAX));

				while (dur--) {
					paint.drawLine(
					    pdot->x_off + MPP_VISUAL_R_MAX,
					    pdot->y_off + (MPP_VISUAL_R_MAX / 2) -
					    (3 * MPP_VISUAL_R_MAX) + offset,
					    pdot->x_off,
					    pdot->y_off + MPP_VISUAL_R_MAX -
					    (3 * MPP_VISUAL_R_MAX) + offset);

					offset += (MPP_VISUAL_R_MAX / 2.0);
				}
				break;

			case MPP_T_STRING_CHORD:
				chord = MppDeQuoteChord(ptr->txt);

				paint.setFont(fnt_b);
				paint.drawText(QPointF(MPP_VISUAL_MARGIN + box.width(),
				    MPP_VISUAL_MARGIN + (visual_y_max / 3) - (MPP_VISUAL_C_MAX / 4)),
				    chord);

				chord_x_max = box.width() +
					paint.boundingRect(QRectF(0,0,0,0), Qt::TextSingleLine | Qt::AlignLeft,
					chord + QChar(' ')).width();
				break;

			default:
				break;
			}
		}

		paint.setFont(fnt_a);
		paint.drawText(QPointF(MPP_VISUAL_MARGIN, MPP_VISUAL_MARGIN +
		    visual_y_max - (visual_y_max / 3)), linebuf);

		if (pd != NULL) {
#ifndef QT_NO_PRINTER
			if (pVisual[x].newpage != 0) {
				pd->newPage();
				paint.translate(QPoint(0, -visual_y_max * y));
				y = 0;
			} else {
				paint.translate(QPoint(0, visual_y_max));
				y++;
			}
#endif
		} else {
			paint.end();
		}
	}

	if (pd != NULL)
		paint.end();
}

void
MppScoreMain :: viewMousePressEvent(QMouseEvent *e)
{
	int yi;

	yi = ((e->y() - MPP_VISUAL_MARGIN) / visual_y_max) + picScroll;

	if (yi < 0 || yi >= visual_max)
		return;

	mainWindow->handle_tab_changed(1);

	pthread_mutex_lock(&mainWindow->mtx);
	head.jumpPointer(pVisual[yi].start);
	head.syncLast();
	mainWindow->handle_stop();
	pthread_mutex_unlock(&mainWindow->mtx);
}

void
MppScoreMain :: locateVisual(MppElement *ptr, int *pindex, MppVisualDot **ppdot)
{
	MppElement *start;
	MppElement *stop;
	MppElement *elem;
	int x;
	int y;

	for (x = y = 0; x != visual_max; x++) {
		if (ptr->compare(pVisual[x].start) < 0 ||
		    ptr->compare(pVisual[x].stop) >= 0)
			continue;

		for (start = stop = pVisual[x].start;
			head.foreachLine(&start, &stop); ) {

			if (start->compare(pVisual[x].stop) >= 0)
				break;
			if (ptr->compare(start) >= 0 &&
			    ptr->compare(stop) < 0)
				break;

			for (elem = start; elem != stop;
			    elem = TAILQ_NEXT(elem, entry)) {
				/* check if scores in line */
				if (elem->type == MPP_T_SCORE ||
				    elem->type == MPP_T_MACRO) {
					y++;
					break;
				}
			}
		}
		break;
	}

	*pindex = x;

	if (x != visual_max && y < pVisual[x].ndot)
		*ppdot = &pVisual[x].pdot[y];
	else
		*ppdot = 0;
}

void
MppScoreMain :: viewPaintEvent(QPaintEvent *event)
{
	QPainter paint(viewWidgetSub);
	MppVisualDot *pcdot;
	MppVisualDot *podot;
	MppElement *curr;
	MppElement *last;
	int scroll;
	int y_blocks;

	int y_div;
	int y_rem;

	int yc_div;
	int yc_rem;

	int yo_div;
	int yo_rem;

	int x;

	paint.fillRect(event->rect(), color_white);

	pthread_mutex_lock(&mainWindow->mtx);
	curr = head.state.curr_start;
	last = head.state.last_start;
	scroll = picScroll;
	pthread_mutex_unlock(&mainWindow->mtx);

	y_blocks = (viewWidgetSub->height() / visual_y_max);
	if (y_blocks == 0)
		y_blocks = 1;

	/* locate current play position */
	locateVisual(curr, &yc_rem, &pcdot);

	/* locate last play position */
	locateVisual(last, &yo_rem, &podot);

	y_div = 0;
	yc_div = 0;
	yo_div = 0;

	/* compute scrollbar */

	y_div = scroll / y_blocks;
	y_rem = scroll % y_blocks;

	/* align current position with scroll bar */

	if (yc_rem < y_rem) {
		yc_rem = 0;
		yc_div = visual_max;
	} else {
		yc_rem -= y_rem;
		yc_div = yc_rem / y_blocks;
		yc_rem = yc_rem % y_blocks;
	}

	/* align last position with scroll bar */

	if (yo_rem < y_rem) {
		yo_rem = 0;
		yo_div = visual_max;
	} else {
		yo_rem -= y_rem;
		yo_div = yo_rem / y_blocks;
		yo_rem = yo_rem % y_blocks;
	}

	/* paint pictures */

	for (x = 0; x != y_blocks; x++) {
		int y = ((y_div * y_blocks) + y_rem + x);
		if (y >= visual_max)
			break;
		paint.drawPicture(
		    QPoint(0, x * visual_y_max),
		    *(pVisual[y].pic));
	}

	/* overlay (last) */

	if ((curr != last) && (yo_div == y_div) && (podot != 0)) {
		paint.setPen(QPen(color_green, 4));
		paint.setBrush(QColor(color_green));
		paint.drawEllipse(QRect(podot->x_off,
		    podot->y_off + (yo_rem * visual_y_max),
		    MPP_VISUAL_R_MAX, MPP_VISUAL_R_MAX));

	}

	/* overlay (current) */

	if ((yc_div == y_div) && (pcdot != 0)) {
		paint.setPen(QPen(color_logo, 4));
		paint.setBrush(QColor(color_logo));
		paint.drawEllipse(QRect(pcdot->x_off,
		    pcdot->y_off + (yc_rem * visual_y_max),
		    MPP_VISUAL_R_MAX, MPP_VISUAL_R_MAX));
	}

	/* overlay (active chord) */

	if (keyMode == MM_PASS_NONE_CHORD) {
		int width = viewWidgetSub->width();
		int mask;

		pthread_mutex_lock(&mainWindow->mtx);
		mask = ((mainWindow->get_time_offset() % 1000) >= 500);
		pthread_mutex_unlock(&mainWindow->mtx);

		if (width >= (2 * MPP_VISUAL_C_MAX)) {
		  if (pressed_future != 1 || mask) {
			paint.drawPicture(QPoint(width -
			    (2 * MPP_VISUAL_C_MAX), 0),
			    *picChord[0]);
		  }
		  if (pressed_future != 0 || mask) {
			paint.drawPicture(QPoint(width -
			    (2 * MPP_VISUAL_C_MAX), 0),
			    *picChord[1]);
		  }
		}
	}
}

/* The following function must be called locked */

void
MppScoreMain :: handleParse(const QString &pstr)
{
	MppElement *start;
	MppElement *stop;
	MppElement *ptr;
	int auto_melody;
	int has_string;
	int index;
	int x;
	int step;
	int num_dot;

	/* reset head structure */
	head.clear();

	/* add string to input */
	head += pstr;

	/* flush last element, if any */
	head.flush();

	/* set initial mask for active channels */
	active_channels = 1;

	/* no automatic melody */
	auto_melody = 0;

	/* number of base keys */
	num_base = 2;

	/* cleanup visual entries */
	for (x = 0; x != visual_max; x++) {
		delete (pVisual[x].pic);
		free (pVisual[x].pdot);
	}

	visual_max = 0;
	visual_p_max = 0;
	index = 0;
	free(pVisual);
	pVisual = 0;

	for (start = stop = 0; head.foreachLine(&start, &stop); ) {

		has_string = 0;

		for (ptr = start; ptr != stop;
		    ptr = TAILQ_NEXT(ptr, entry)) {
			if (ptr->type == MPP_T_COMMAND) {
				switch (ptr->value[0]) {
				case MPP_CMD_BPM_REF:
					/* update BPM timer */
					pthread_mutex_lock(&mainWindow->mtx);
					mainWindow->dlg_bpm->ref = ptr->value[1];
					mainWindow->dlg_bpm->period = ptr->value[2];
					mainWindow->dlg_bpm->handle_update();
					pthread_mutex_unlock(&mainWindow->mtx);
					break;
				case MPP_CMD_AUTO_MELODY:
					auto_melody = ptr->value[1];
					break;
				case MPP_CMD_NUM_BASE:
					num_base = ptr->value[1];
					break;
				default:
					break;
				}
			} else if (ptr->type == MPP_T_CHANNEL) {
				if (ptr->value[0] > -1 && ptr->value[0] < 16)
					active_channels |= (1 << ptr->value[0]);
			} else if (ptr->type == MPP_T_STRING_DESC || 
			    ptr->type == MPP_T_STRING_DOT ||
			    ptr->type == MPP_T_STRING_CHORD) {
				has_string = 1;
			} else if (ptr->type == MPP_T_JUMP) {
				if (ptr->value[1] & MPP_FLAG_JUMP_PAGE)
					index = 0;
			}
		}
		/* compute maximum number of score lines */
		if (has_string) {
			index++;
			visual_max++;
			if (visual_p_max < index)
				visual_p_max = index;
		}
	}
	if (visual_max != 0) {
		size_t size = sizeof(MppVisualScore) * visual_max;
		pVisual = (MppVisualScore *)malloc(size);
		memset(pVisual, 0, size);
	}

	head.dotReorder();

	step = 0;
	index = 0;

	for (start = stop = 0; head.foreachLine(&start, &stop); ) {

		has_string = 0;
		num_dot = 0;

		for (ptr = start; ptr != stop;
		    ptr = TAILQ_NEXT(ptr, entry)) {
			if (ptr->type == MPP_T_STRING_DOT) {
				num_dot++;
				has_string = 1;
			} else if (ptr->type == MPP_T_STRING_DESC || 
			    ptr->type == MPP_T_STRING_CHORD) {
				has_string = 1;
			} else if (ptr->type == MPP_T_JUMP) {
				if ((ptr->value[1] & MPP_FLAG_JUMP_PAGE) &&
				    (index > 0 && index <= visual_max)) {
					pVisual[index - 1].newpage = 1;
				}
			}
		}
		if (has_string && (index < visual_max)) {
			/* extend region of previous visual */
			if (index > 0)
				pVisual[index - 1].stop = start;

			pVisual[index].start = start;
			pVisual[index].stop = stop;
			pVisual[index].ndot = num_dot;

			if (num_dot != 0) {
				size_t size = sizeof(MppVisualDot) * num_dot;
				pVisual[index].pdot = (MppVisualDot *)
				    malloc(size);
				memset(pVisual[index].pdot, 0, size);
			}
			index++;
		}
	}
	/* extend region of first and last visual */
	if (visual_max != 0) {
		pVisual[0].start = TAILQ_FIRST(&head.head);
		pVisual[visual_max - 1].stop = 0;
	}

	/* check if auto-melody should be applied */
	if (auto_melody > 0)
		head.autoMelody(auto_melody - 1);

	/* number all elements to make searching easier */
	head.sequence();

	/* get first line */
	head.currLine(&start, &stop);

	/* sync last */
	head.syncLast();

	/* create the graphics */
	handlePrintSub(0, QPoint(0,0), 1.0);

	/* update scrollbar */
	viewScroll->setMaximum((visual_max > 0) ? (visual_max - 1) : 0);

#ifndef HAVE_NO_SHOW
	MppShowControl *pshow = mainWindow->tab_show_control;

	/* create show lyrics, if any */
	head.toLyrics(pshow->labelTxt[unit]);

	/* force new transition */
	pshow->transition = 0;
#endif
}

void
MppScoreMain :: handleScoreFileNew()
{
	editWidget->setPlainText(QString());

	handleCompile();

	if (currScoreFileName != NULL) {
		delete (currScoreFileName);
		currScoreFileName = NULL;
	}

	mainWindow->handle_tab_changed(1);
	mainWindow->handle_make_scores_visible(this);
}

void
MppScoreMain :: handleScoreFileOpenRaw(char *data, uint32_t len)
{
	handleScoreFileNew();

	editWidget->setPlainText(QString::fromUtf8(QByteArray(data, len)));

	handleCompile();

	mainWindow->handle_tab_changed(1);
	mainWindow->handle_make_scores_visible(this);
}

int
MppScoreMain :: handleScoreFileOpenSub(QString fname)
{
	QString scores;

	handleScoreFileNew();

	currScoreFileName = new QString(fname);

	scores = MppReadFile(fname);

	editWidget->setPlainText(scores);

	handleCompile();

	mainWindow->handle_tab_changed(1);
	mainWindow->handle_make_scores_visible(this);

	return (scores.isNull() || scores.isEmpty());
}

void
MppScoreMain :: handleScoreFileOpen()
{
	QFileDialog *diag = 
	  new QFileDialog(mainWindow, tr("Select Score File"), 
		MppHomeDirTxt,
		QString("Score File (*.txt *.TXT)"));

	diag->setAcceptMode(QFileDialog::AcceptOpen);
	diag->setFileMode(QFileDialog::ExistingFile);

	if (diag->exec()) {
		MppHomeDirTxt = diag->directory().path();
		handleScoreFileOpenSub(diag->selectedFiles()[0]);
	}

	delete diag;
}

void
MppScoreMain :: handleScoreFileSave()
{
	if (currScoreFileName != NULL)
		MppWriteFile(*currScoreFileName, editWidget->toPlainText());
	else
		handleScoreFileSaveAs();
}

void
MppScoreMain :: handleScoreFileSaveAs()
{
	QFileDialog *diag = 
	  new QFileDialog(mainWindow, tr("Select Score File"), 
		MppHomeDirTxt,
		QString("Score File (*.txt *.TXT)"));

	diag->setAcceptMode(QFileDialog::AcceptSave);
	diag->setFileMode(QFileDialog::AnyFile);
	diag->setDefaultSuffix(QString("txt"));

	if (diag->exec()) {
		MppHomeDirTxt = diag->directory().path();

		if (currScoreFileName != NULL)
			delete (currScoreFileName);

		currScoreFileName = new QString(diag->selectedFiles()[0]);

		if (currScoreFileName != NULL)
			handleScoreFileSave();

		mainWindow->handle_tab_changed(1);
	}

	delete diag;
}

int
MppScoreMain :: checkLabelJump(int pos)
{
	if ((pos < 0) || (pos >= 12) || (pos >= MPP_MAX_LABELS) ||
	    (head.state.label_start[pos] == 0))
		return (0);

	return (1);
}

int
MppScoreMain :: checkHalfPassThru(int key)
{
	static const uint8_t is_black[12] = {0,1,0,1,0,0,1,0,1,0,1,0};

	return ((key >= mid_next_key(baseKey, -1)) &&
	    (key <= mid_next_key(baseKey, +1)) &&
	    (is_black[((uint8_t)key) % 12U] ==
	     is_black[((uint8_t)baseKey) % 12U]));
}

/* must be called locked */
void
MppScoreMain :: handleLabelJump(int pos)
{
	if (checkLabelJump(pos) == 0)
		return;

	head.jumpLabel(pos);
	head.syncLast();

	mainWindow->cursorUpdate = 1;

	mainWindow->handle_stop(1);

	mainWindow->send_song_select_locked(pos);
}

void
MppScoreMain :: handleChordsLoad(void)
{
	MppElement *start;
	MppElement *stop;
	MppElement *ptr;
	int duration;
	uint8_t x;
	uint8_t ns;
	uint8_t nb;
	uint8_t nk;
	uint8_t chan;
	uint8_t score[12];
	uint8_t base[12];
	uint8_t key[12];

	memset(score_future, 0, sizeof(score_future));

	nb = 0;
	nk = 0;
	ns = 0;
	chan = 0;
	duration = 1;

	head.currLine(&start, &stop);

	for (ptr = start; ptr != stop;
	     ptr = TAILQ_NEXT(ptr, entry)) {
		switch (ptr->type) {
		case MPP_T_DURATION:
			duration = ptr->value[0];
			break;
		case MPP_T_SCORE:
			if (duration == 0)
				break;
			if (ns < 12)
				score[ns++] = ptr->value[0];
			break;
		case MPP_T_CHANNEL:
			chan = ptr->value[0];
			break;
		default:
			break;
		}
	}

	if (ns == 0)
		return;

	mid_sort(score, ns);

	for (x = 0; x != ns; x++) {
		if (x < num_base)
			base[nb++] = score[x];
		else
			key[nk++] = score[x];
	}

	if (nb != 0) {
		score_future[2].dur = 1;
		score_future[2].key = base[0];
		score_future[2].channel = chan;
		mid_trans(base, nb, 1);
		score_future[4].dur = 1;
		score_future[4].key = base[0];
		score_future[4].channel = chan;
		mid_trans(base, nb, 1);
		score_future[1].dur = 1;
		score_future[1].key = base[0];
		score_future[1].channel = chan;
		mid_trans(base, nb, 1);
		score_future[3].dur = 1;
		score_future[3].key = base[0];
		score_future[3].channel = chan;
		mid_trans(base, nb, 1);
	}

	if (nk != 0) {
		score_future[5].dur = 1;
		score_future[5].key = key[0];
		score_future[5].channel = chan;
		mid_trans(key, nk, 1);
		score_future[7].dur = 1;
		score_future[7].key = key[0];
		score_future[7].channel = chan;
		mid_trans(key, nk, 1);
		score_future[9].dur = 1;
		score_future[9].key = key[0];
		score_future[9].channel = chan;
		mid_trans(key, nk, 1);
		score_future[11].dur = 1;
		score_future[11].key = key[0];
		score_future[11].channel = chan;
		mid_trans(key, nk, 1);
		score_future[6].dur = 1;
		score_future[6].key = key[0];
		score_future[6].channel = chan;
		mid_trans(key, nk, 1);
		score_future[8].dur = 1;
		score_future[8].key = key[0];
		score_future[8].channel = chan;
		mid_trans(key, nk, 1);
		score_future[10].dur = 1;
		score_future[10].key = key[0];
		score_future[10].channel = chan;
		mid_trans(key, nk, 1);
	}

	head.syncLast();
	head.stepLine(&start, &stop);

	mainWindow->cursorUpdate = 1;
}

/* must be called locked */
uint8_t
MppScoreMain :: handleKeyRemovePast(MppScoreEntry *pn, uint32_t key_delay)
{
	int x;
	uint8_t chan;
	uint8_t retval = 0;

	chan = (synthChannel + pn->channel) & 0xF;

	for (x = 0; x != 24; x++) {
		if (score_past[x].dur != 0 &&
		    score_past[x].key == pn->key &&
		    score_past[x].channel == chan) {

			mainWindow->output_key(chan, pn->key, 0, key_delay, 0);

			/* kill past score */
			score_past[x].dur = 0;

			/* tell caller about leftover key presses */
			retval = 1;
		}
	}
	return (retval);
}

/* must be called locked */
void
MppScoreMain :: handleBeat(void)
{
	/* make sure we are triggered before sending beat events */
	mainWindow->handle_midi_trigger();
	mainWindow->send_song_event_locked(0xF8);
}

/* must be called locked */
void
MppScoreMain :: handleKeyPressChord(int in_key, int vel, uint32_t key_delay)
{
	MppScoreEntry *pn;
	int off;

	off = (int)in_key - (int)baseKey;

	if (off >= 1 && off < 12) {

		if (pressed_future == 0 || head.isFirst()) {
			pressed_future = 1;
			handleChordsLoad();
			handleBeat();
		}

		pn = &score_future[off];

		/* check for silent key */
		if (pn->dur == 0)
			return;

		/* remove key if already pressed */
		if (handleKeyRemovePast(pn, key_delay))
			key_delay++;

		score_past[off].channel = (synthChannel + pn->channel) & 0xF;
		score_past[off].key = pn->key;
		score_past[off].dur = 1;

		mainWindow->output_key(score_past[off].channel, score_past[off].key,
		    vel, key_delay, 0);

	} else if (off >= 13 && off < 24) {

		if (pressed_future == 1 || head.isFirst()) {
			pressed_future = 0;
			handleChordsLoad();
			handleBeat();
		}

		pn = &score_future[off - 12];

		/* check for silent key */
		if (pn->dur == 0)
			return;

		/* remove key if already pressed */
		if (handleKeyRemovePast(pn, key_delay))
			key_delay++;

		score_past[off].channel = (synthChannel + pn->channel) & 0xF;
		score_past[off].key = pn->key;
		score_past[off].dur = 1;

		mainWindow->output_key(score_past[off].channel, score_past[off].key,
		    vel, key_delay, 0);
	} else {
		/* silent keys */
		return;
	}

	if (mainWindow->currScoreMain() == this)
		mainWindow->do_update_bpm();

	mainWindow->cursorUpdate = 1;
}

/* must be called locked */
void
MppScoreMain :: handleKeyPressureChord(int in_key, int vel, uint32_t key_delay)
{
	MppScoreEntry *pn;
	int off;

	off = (int)in_key - (int)baseKey;

	if (off == 0 || off == 12)
		return;

	if (off >= 0 && off < 24) {

		pn = &score_pressure[off];

		if (pn->dur != 0) {
			outputKeyPressure(pn->channel, pn->key, vel);
		}
	}
}

/* must be called locked */
void
MppScoreMain :: handleKeyReleaseChord(int in_key, uint32_t key_delay)
{
	MppScoreEntry *pn;
	int off;

	off = (int)in_key - (int)baseKey;

	if (off == 0 || off == 12)
		return;

	if (off >= 0 && off < 24) {

		pn = &score_past[off];

		if (pn->dur != 0) {

			/* store score for pressure */
			score_pressure[off] = score_past[off];

			mainWindow->output_key(pn->channel, pn->key, 0, key_delay, 0);

			/* only release once */
			pn->dur = 0;
		}
	}
}

/* must be called locked */
void
MppScoreMain :: handleKeyPressSub(int in_key, int vel,
    uint32_t key_delay, int key_trans, int allow_macro)
{
	MppElement *start;
	MppElement *stop;
	MppElement *ptr;
	int t_pre;
	int t_post;
	int channel;
	int duration;
	int nscore;
	int nfoot;
	int vel_other;
	int transpose;
	uint8_t footprint[12];

	head.currLine(&start, &stop);
	head.state.did_jump = 0;

	while (head.state.did_jump == 0) {
		t_pre = 0;
		t_post = 0;
		channel = 0;
		duration = 1;
		nfoot = 0;
		nscore = 0;
		transpose = key_trans;
		memset(footprint, 0, sizeof(footprint));

		decrementDuration();

		for (ptr = start; ptr != stop;
		    ptr = TAILQ_NEXT(ptr, entry)) {
			switch (ptr->type) {
			int temp;
			case MPP_T_SCORE:
				if (duration <= 0)
					break;
				nscore++;
				temp = ptr->value[0] % 12;
				if (footprint[temp] == 0) {
					footprint[temp] = 1;
					nfoot++;
				}
				break;
			case MPP_T_DURATION:
				duration = ptr->value[0];
				break;
			default:
				break;
			}
		}

		if (nfoot == 0) {
			vel_other = 0;
		} else {
			if (chordNormalize == 0) {
				vel_other = vel;
			} else {
				vel_other = vel +
				  ((vel * (nfoot - 1) * (128 - chordContrast)) /
				    (nfoot * 128));
				if (vel_other > 127)
					vel_other = 127;
				else if (vel_other < 0)
					vel_other = 0;
			}
		}

		duration = 1;

		for (ptr = start; ptr != stop;
		    ptr = TAILQ_NEXT(ptr, entry)) {
			switch (ptr->type) {
			int ch;
			int out_key;
			int out_vel;
			int delay;

			case MPP_T_TRANSPOSE:
				transpose = ptr->value[0] + key_trans;
				break;

			case MPP_T_MACRO:
				if (allow_macro == 0)
					break;

				/* push traversal state */
				head.pushLine();

				/* jump to target */
				head.jumpLabel(ptr->value[0]);

				/* set frozen keys */
				memcpy(frozenKeys, pressedKeys, sizeof(frozenKeys));

				handleKeyPressSub(in_key, vel,
				    key_delay, transpose, 0);

				/* pop traversal state */
				head.popLine();

				/* clear frozen keys */
				memset(frozenKeys, 0, sizeof(frozenKeys));
				break;

			case MPP_T_SCORE:
				if (duration <= 0)
					break;

				if (--nscore < 2)
					out_vel = vel;
				else
					out_vel = vel_other;

				out_key = ptr->value[0] + in_key + transpose;
				if (out_key < 0 || out_key > 127)
					break;

				ch = (synthChannel + channel) & 0xF;

				if (delayNoise != 0)
					delay = mainWindow->noise8(delayNoise);
				else
					delay = 0;

				if (setPressedKey(ch, out_key, duration, delay))
					break;

				mainWindow->output_key(ch, out_key, out_vel,
				    key_delay + delay, 0);
				break;

			case MPP_T_DURATION:
				duration = ptr->value[0];
				break;

			case MPP_T_CHANNEL:
				channel = ptr->value[0];
				break;

			case MPP_T_TIMER:
				t_pre += ptr->value[0];
				t_post += ptr->value[1];
				break;

			default:
				break;
			}
		}

		head.stepLine(&start, &stop);

		/* if no timer, we are done */
		if (t_pre == 0 && t_post == 0)
			break;

		key_delay += t_pre;
		decrementDuration(key_delay);
		key_delay += t_post;
	}
}

/* must be called locked */
void
MppScoreMain :: handleKeyPress(int in_key, int vel, uint32_t key_delay)
{
	/* check for key locking */

	if (head.state.key_lock < 0) {
		head.state.key_lock = 1;
		whatPlayKeyLocked = in_key;
	} else if (head.state.key_lock > 0) {
		if (in_key != (int)whatPlayKeyLocked) {
			return;
		}
	}

	/* generate beat event, if any */

	handleBeat();

	/* play sheet */

	handleKeyPressSub(in_key, vel, key_delay, -(int)baseKey, 1);

	/* update cursor, if any */

	mainWindow->cursorUpdate = 1;

	/* update bpm, if any */

	if (mainWindow->currScoreMain() == this)
		mainWindow->do_update_bpm();
}

/* must be called locked */
void
MppScoreMain :: decrementDuration(uint32_t timeout)
{
	uint8_t out_key;
	uint8_t chan;
	uint8_t delay;
	uint8_t x;

	for (x = 0; x != MPP_PRESSED_MAX; x++) {
		if (frozenKeys[x] != 0)
			continue;
		if ((pressedKeys[x] & 0xFF) == 1) {

			out_key = (pressedKeys[x] >> 8) & 0xFF;
			chan = (pressedKeys[x] >> 16) & 0xFF;
			delay = (pressedKeys[x] >> 24) & 0xFF;

			/* clear entry */
			pressedKeys[x] = 0;

			mainWindow->output_key(chan, out_key, 0,
			    timeout + delay, 0);
		}

		if (pressedKeys[x] != 0)
			pressedKeys[x] --;
	}
}

/* must be called locked */
void
MppScoreMain :: handleKeyRelease(int in_key, uint32_t key_delay)
{
	if (head.state.key_lock > 0) {
		if (in_key != (int)whatPlayKeyLocked) {
			return;
		}
	}

	decrementDuration(key_delay);

	head.syncLast();
}

void
MppScoreMain :: handleScorePrint(void)
{
#ifndef QT_NO_PRINTER
	QPrinter printer(QPrinter::HighResolution);
	QPrintDialog *dlg;
	QPoint orig;
	qreal scale_f;
	QString temp;

	/* make sure everything is up-to-date */

	handleCompile();

	printer.setFontEmbeddingEnabled(true);
	printer.setFullPage(true);
	printer.setResolution(600);

	if (currScoreFileName != NULL) {
		temp = *currScoreFileName;
		temp.replace(QString(".txt"), QString(".pdf"),
		     Qt::CaseInsensitive);

		printer.setOutputFileName(temp);
	} else {
		printer.setOutputFileName(MppHomeDirTxt + QString("/NewSong.pdf"));
	}

	printer.setColorMode(QPrinter::Color);
#ifdef __APPLE__
	printer.setOutputFormat(QPrinter::NativeFormat);
#else
	printer.setOutputFormat(QPrinter::PdfFormat);
#endif

	dlg = new QPrintDialog(&printer, mainWindow);

	if(dlg->exec() == QDialog::Accepted) {

		orig = QPoint(printer.logicalDpiX() * 0.5,
			      printer.logicalDpiY() * 0.5);

		scale_f = ((qreal)printer.logicalDpiY()) / (qreal)visual_y_max;

		handlePrintSub(&printer, orig, scale_f);
	}

	delete dlg;
#endif
}

int
MppScoreMain :: setPressedKey(int chan, int out_key, int dur, int delay)
{
	uint32_t temp;
	uint8_t y;

	dur &= 0xFF;
	out_key &= 0xFF;
	chan &= 0xFF;
	delay &= 0xFF;

	temp = dur | (out_key << 8) | (chan << 16) | (delay << 24);

	if (dur == 0) {
		/* release key */
		for (y = 0; y != MPP_PRESSED_MAX; y++) {
			if (frozenKeys[y] != 0)
				continue;
			if ((pressedKeys[y] & 0x00FFFF00U) == (temp & 0x00FFFF00U)) {
				/* key information matches */
				/* clear key */
				pressedKeys[y] = 0;
			}
		}
		return (0);
	} else {
		/* pre-press key */
		for (y = 0; y != MPP_PRESSED_MAX; y++) {
			if (frozenKeys[y] != 0)
				continue;
			if ((pressedKeys[y] & 0x00FFFF00U) == (temp & 0x00FFFF00U)) {
				/* key already set */
				return (1);
			}
		}

		/* press key */
		for (y = 0; y != MPP_PRESSED_MAX; y++) {
			if (frozenKeys[y] != 0)
				continue;
			if (pressedKeys[y] != 0)
				continue;	/* key in use */

			pressedKeys[y] = temp;	/* set key */

			return (0);
		}
		return (1);
	}
}

int
MppScoreMain :: handleCompile(int force)
{
	QString temp;

	temp = editWidget->toPlainText();

	if (temp != editText || force != 0) {
		editText = temp;

		visual_y_max = MPP_VISUAL_C_MAX +
		  (3 * mainWindow->defaultFont.pixelSize());

		pthread_mutex_lock(&mainWindow->mtx);

		handleParse(editText);

		pthread_mutex_unlock(&mainWindow->mtx);

		return (1);
	}
	return (0);
}

void
MppScoreMain :: watchdog()
{
	MppElement *curr;
	MppVisualDot *pdot;
	int off;
	int y_blocks;

	QTextCursor cursor(editWidget->textCursor());

	QTextEdit::ExtraSelection format;

	/* Highlight the next line to be played */

	cursor.movePosition(QTextCursor::EndOfLine, QTextCursor::KeepAnchor, 1);

	format.cursor = cursor;
	format.format.setForeground(color_black);
	format.format.setBackground(color_grey);

	QList<QTextEdit::ExtraSelection> extras;
	extras << format;

	editWidget->setExtraSelections(extras);

	/* Compute scrollbar */

	pthread_mutex_lock(&mainWindow->mtx);
	curr = head.state.curr_start;
	pthread_mutex_unlock(&mainWindow->mtx);

	/* Compute alignment factor */

	y_blocks = (viewWidgetSub->height() / visual_y_max);
	if (y_blocks == 0)
		y_blocks = 1;

	/* locate current play position */
	locateVisual(curr, &off, &pdot);

	if (off < visual_max) {
		viewScroll->setValue(off - (off % y_blocks));
	} else {
		viewScroll->setValue(0);
	}
}

void
MppScoreMain :: handleScrollChanged(int value)
{
	pthread_mutex_lock(&mainWindow->mtx);
	picScroll = value;
	pthread_mutex_unlock(&mainWindow->mtx);

	viewWidgetSub->repaint();
}

void
MppScoreMain :: handleScoreFileEffect(int which, int parm, int flag)
{
	QTextCursor cursor(editWidget->textCursor());
	MppHead temp;
	int re_select;
	int pos;

	cursor.beginEditBlock();

	re_select = cursor.hasSelection();

	if (re_select == 0) {
		cursor.movePosition(QTextCursor::End, QTextCursor::MoveAnchor, 1);
		cursor.movePosition(QTextCursor::Start, QTextCursor::KeepAnchor, 1);
	}

	QString sel = cursor.selectedText();

	if (sel.size() == 0)
		goto done;

	temp += sel;
	temp.flush();

	switch (which) {
	case 0:
		temp.limitScore(parm);
		break;
	case 1:
		temp.transposeScore(parm, flag);
		break;
	case 2:
		temp.scaleTime(parm);
		break;
	case 3:
		temp.autoMelody(parm);
		temp.optimise();
		break;
	default:
		break;
	}

	sel = temp.toPlain();

	cursor.removeSelectedText();
	pos = cursor.blockNumber();
	cursor.insertText(sel);

	if (re_select != 0) {
		cursor.movePosition(QTextCursor::Start, QTextCursor::MoveAnchor, 1);
		cursor.movePosition(QTextCursor::Down, QTextCursor::MoveAnchor, pos);
		cursor.movePosition(QTextCursor::Down, QTextCursor::KeepAnchor, MppCountNewline(sel));
		editWidget->setTextCursor(cursor);
	}
done:
	cursor.endEditBlock();

	handleCompile();
}

void
MppScoreMain :: handleScoreFileAlign(void)
{
	handleScoreFileEffect(0, spnScoreFileAlign->value(), 0);
}

void
MppScoreMain :: handleScoreFileScale(void)
{
	handleScoreFileEffect(2, spnScoreFileScale->value(), 0);
}

void
MppScoreMain :: handleScoreFileStepUp(void)
{
	handleScoreFileEffect(1, 1, 0);
}

void
MppScoreMain :: handleScoreFileStepDown(void)
{
	handleScoreFileEffect(1, -1, 0);
}

void
MppScoreMain :: handleScoreFileSetSharp(void)
{
	handleScoreFileEffect(1, 0, 1);
}

void
MppScoreMain :: handleScoreFileSetFlat(void)
{
	handleScoreFileEffect(1, 0, -1);
}

void
MppScoreMain :: handleScoreFileAutoMelody(int which)
{
	handleScoreFileEffect(3, which, 0);
}

void
MppScoreMain :: handleScoreFileExport(void)
{
	QTextCursor input(editWidget->textCursor());
	QString sel;

	if (input.hasSelection() == 0)
		sel = editWidget->toPlainText();
	else
		sel = input.selectedText();

	MppHead temp;

	temp += sel;
	temp.flush();

	QTextCursor cursor(mainWindow->tab_import->editWidget->textCursor());

	cursor.beginEditBlock();
	cursor.insertText(temp.toLyrics());
	cursor.endEditBlock();

	mainWindow->handle_make_tab_visible(mainWindow->tab_import->editWidget);
}

uint8_t
MppScoreMain :: handleEditLine(void)
{
	MppHead temp;
	MppChordElement info;
	MppElement *ptr;
	QTextCursor cursor(editWidget->textCursor());
	int row;
	int retval = 1;

	cursor.movePosition(QTextCursor::EndOfLine, QTextCursor::MoveAnchor, 1);
	cursor.movePosition(QTextCursor::StartOfLine, QTextCursor::KeepAnchor, 1);

	row = cursor.blockNumber();

	temp += editWidget->toPlainText();
	temp.flush();

	if (temp.getChord(row, &info) != 0) {

		MppDecode dlg(mainWindow, this, 1);

		if (dlg.parseScoreChord(&info) == 0) {

			if (dlg.exec() == QDialog::Accepted) {

				cursor.beginEditBlock();

				if (info.chord != 0) {

					info.chord->txt = QChar('(') + dlg.lin_edit->text().trimmed() + QChar(')');

					cursor.movePosition(QTextCursor::Start, QTextCursor::MoveAnchor, 1);
					cursor.movePosition(QTextCursor::Down, QTextCursor::MoveAnchor, info.chord->line);
					cursor.movePosition(QTextCursor::EndOfLine, QTextCursor::MoveAnchor, 1);
					cursor.movePosition(QTextCursor::StartOfLine, QTextCursor::KeepAnchor, 1);
					cursor.removeSelectedText();
					cursor.insertText(temp.toPlain(info.chord->line).replace("\n", ""));
				}

				if (info.start != 0) {
					for (ptr = info.start; ptr != info.stop;
					    ptr = TAILQ_NEXT(ptr, entry)) {
						ptr->txt = QString();
					}

					info.start->txt = mainWindow->led_config_insert->text() +
					    dlg.getText();

					cursor.movePosition(QTextCursor::Start, QTextCursor::MoveAnchor, 1);
					cursor.movePosition(QTextCursor::Down, QTextCursor::MoveAnchor, row);
					cursor.movePosition(QTextCursor::EndOfLine, QTextCursor::MoveAnchor, 1);
					cursor.movePosition(QTextCursor::StartOfLine, QTextCursor::KeepAnchor, 1);
					cursor.removeSelectedText();
					cursor.insertText(temp.toPlain(info.start->line).replace("\n", ""));
				}
				cursor.endEditBlock();
				retval = 0;
			}
		}
	}
	return (retval);
}

void
MppScoreMain :: handleScoreFileReplaceAll(void)
{
	QTextCursor cursor(editWidget->textCursor());

	MppReplace dlg(mainWindow, this, cursor.selectedText(),
	    cursor.selectedText());

	if (dlg.exec() == QDialog::Accepted) {

		editWidget->moveCursor(QTextCursor::Start, QTextCursor::MoveAnchor);

		while (editWidget->find(dlg.match)) {

			cursor = editWidget->textCursor();

			cursor.beginEditBlock();
			cursor.removeSelectedText();
			cursor.insertText(dlg.replace);
			cursor.endEditBlock();
		}
	}
}

/* must be called locked */
void
MppScoreMain :: outputControl(uint8_t ctrl, uint8_t val)
{
	MppMainWindow *mw = mainWindow;
	struct mid_data *d = &mw->mid_data;
	uint16_t mask;
	uint8_t x;
	uint8_t chan;

	chan = synthChannel;

	if (keyMode == MM_PASS_ALL)
		mask = 1;
	else
		mask = active_channels;

	/* the control event is distributed to all active channels */
	while (mask) {
		if (mask & 1) {
			for (x = 0; x != MPP_MAX_DEVS; x++) {
				if (ctrl == 0x40 && mw->mutePedal[x] != 0)
					continue;
				if (mw->muteAllControl[x] != 0)
					continue;
				if (mw->check_synth(x, chan, 0) == 0)
					continue;
				mid_control(d, ctrl, val);
			}
			if (mw->check_record(chan, 0))
				mid_control(d, ctrl, val);
		}
		mask /= 2;
		chan++;
		chan &= 0xF;
	}

	if (ctrl == 0x40)
		mw->tab_loop->add_pedal(val);
}

/* must be called locked */
void
MppScoreMain :: outputKeyPressure(uint8_t chan, uint8_t key, uint8_t pressure)
{
	MppMainWindow *mw = mainWindow;
	struct mid_data *d = &mw->mid_data;
	uint16_t mask;
	uint8_t x;
	uint8_t buf[4];

	if (keyMode == MM_PASS_ALL)
		mask = 1;
	else
		mask = active_channels;

	buf[0] = 0xA0;
	buf[1] = key & 0x7F;
	buf[2] = pressure & 0x7F;
	buf[3] = 0;

	/* the pressure event is distributed to all active channels */
	while (mask) {
		if (mask & 1) {
			for (x = 0; x != MPP_MAX_DEVS; x++) {
				if (mw->check_synth(x, chan, 0))
					mid_add_raw(d, buf, 3, 0);
			}
			if (mw->check_record(chan, 0))
				mid_add_raw(d, buf, 3, 0);
		}
		mask /= 2;
		chan++;
		chan &= 0xF;
	}
}

/* must be called locked */
void
MppScoreMain :: outputChanPressure(uint8_t chan, uint8_t pressure)
{
	MppMainWindow *mw = mainWindow;
	struct mid_data *d = &mw->mid_data;
	uint16_t mask;
	uint8_t x;
	uint8_t buf[4];

	if (keyMode == MM_PASS_ALL)
		mask = 1;
	else
		mask = active_channels;

	buf[0] = 0xD0;
	buf[1] = pressure & 0x7F;
	buf[2] = 0;
	buf[3] = 0;

	/* the pressure event is distributed to all active channels */
	while (mask) {
		if (mask & 1) {
			for (x = 0; x != MPP_MAX_DEVS; x++) {
				if (mw->check_synth(x, chan, 0))
					mid_add_raw(d, buf, 2, 0);
			}
			if (mw->check_record(chan, 0))
				mid_add_raw(d, buf, 2, 0);
		}
		mask /= 2;
		chan++;
		chan &= 0xF;
	}
}

/* must be called locked */
void
MppScoreMain :: outputPitch(uint16_t val)
{
	MppMainWindow *mw = mainWindow;
	struct mid_data *d = &mw->mid_data;
	uint16_t mask;
	uint8_t x;
	uint8_t chan;

	chan = synthChannel;

	if (keyMode == MM_PASS_ALL)
		mask = 1;
	else
		mask = active_channels;

	/* the control event is distributed to all active channels */
	while (mask) {
		if (mask & 1) {
			for (x = 0; x != MPP_MAX_DEVS; x++) {
				if (mw->muteAllControl[x] != 0)
					continue;
				if (mw->check_synth(x, chan, 0) == 0)
					continue;
				mid_pitch_bend(d, val);
			}
			if (mw->check_record(chan, 0))
				mid_pitch_bend(d, val);
		}
		mask /= 2;
		chan++;
		chan &= 0xF;
	}
}

int
MppScoreMain :: getCurrLabel(void)
{
	int retval = 0;
	int first = 1;
	int min = 0;
	int delta;
	int seq;
	int x;

	pthread_mutex_lock(&mainWindow->mtx);
	if (head.state.curr_start != 0)
		seq = head.state.curr_start->sequence;
	else
		seq = 0;
	pthread_mutex_unlock(&mainWindow->mtx);

	for (x = 0; x != MPP_MAX_LABELS; x++) {
		if (head.state.label_start[x] == 0)
			continue;
		delta = seq - head.state.label_start[x]->sequence;
		if (delta < 0)
			continue;
		if (delta < min || first != 0) {
			min = delta;
			retval = x;
			first = 0;
		}
	}
	return (retval);
}

void
MppScoreMain :: handleScoreFileInsertChord(void)
{
	MppDecode dlg(mainWindow, this, 0);

        if(dlg.exec() == QDialog::Accepted) {
		QTextCursor cursor(editWidget->textCursor());
		cursor.beginEditBlock();
		cursor.insertText(mainWindow->led_config_insert->text());
		cursor.insertText(dlg.getText());
		cursor.insertText(QString("\n"));
		cursor.endEditBlock();
		mainWindow->handle_compile();
		mainWindow->handle_make_tab_visible(editWidget);
	}
}

void
MppScoreMain :: handleScoreFileEditChord(void)
{
	if (handleEditLine() == 0)
		mainWindow->handle_compile();
}
