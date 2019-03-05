/*-
 * Copyright (c) 2009-2019 Hans Petter Selasky. All rights reserved.
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
#include "midipp_bpm.h"
#include "midipp_mode.h"
#include "midipp_decode.h"
#include "midipp_import.h"
#include "midipp_replace.h"
#include "midipp_spinbox.h"
#include "midipp_groupbox.h"
#include "midipp_button.h"
#include "midipp_show.h"
#include "midipp_tabbar.h"
#include "midipp_gridlayout.h"
#include "midipp_sheet.h"

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
	delta_v = 0;
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

	if (event->orientation() == Qt::Vertical) {
		delta_v -= event->delta();
		int delta = delta_v / MPP_WHEEL_STEP;
		delta_v %= MPP_WHEEL_STEP;
		if (delta != 0) {
			delta += ps->value();
			if (delta < 0)
				delta = 0;
			else if (delta > ps->maximum())
				delta = ps->maximum();
			ps->setValue(delta);
		}
	}
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
	sm->handleEditLine();
}

MppScoreMain :: MppScoreMain(MppMainWindow *parent, int _unit)
{
	QString defaultText;
	char buf[32];

	if (_unit == 0) {
	  defaultText = tr(
	    "K5.1 /* Micro tune the chords */\n\n"
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

	/* set valid non-zero value */

	visual_y_max = 1;

	/* all devices are input */

	baseKey = MPP_DEFAULT_BASE_KEY;
	delayNoise = 25;
	chordContrast = 128;
	chordNormalize = 1;
	inputChannel = -1;
	synthChannel = ((_unit == 1) ? 9 : 0);
	synthChannelBase = -1;
	synthChannelTreb = -1;
	synthDevice = -1;
	synthDeviceBase = -1;
	synthDeviceTreb = -1;
	unit = _unit;

	/* Set parent */

	mainWindow = parent;

	/* Buttons */

	butScoreFileNew = new QPushButton(tr("New"));
	butScoreFileOpen = new QPushButton(tr("Open"));
	butScoreFileSave = new QPushButton(tr("Save"));
	butScoreFileSaveAs = new QPushButton(tr("Save As"));
	butScoreFilePrint = new QPushButton(tr("Print"));
	butScoreFileAlign = new QPushButton(tr("Align\nScores"));
	spnScoreFileBassOffset = new QSpinBox();
	spnScoreFileBassOffset->setRange(0,15);
	spnScoreFileBassOffset->setValue(0);
	butScoreFileBassOffset = new QPushButton(tr("Bass\nOffset"));
	spnScoreFileAlign = new MppSpinBox(0,0);
	spnScoreFileAlign->setValue((MPP_F0 + 12 * 5) * MPP_BAND_STEP_12);
	butScoreFileScale = new QPushButton(tr("Scale"));
	spnScoreFileScale = new QSpinBox();
	spnScoreFileScale->setRange(0, 60000);
	spnScoreFileScale->setSuffix(" ms");
	spnScoreFileScale->setValue(1000);
	butScoreFileStepUpHalf = new QPushButton(tr("Step Up\n12 scale"));
	butScoreFileStepDownHalf = new QPushButton(tr("Step Down\n12 scale"));
	butScoreFileStepUpSingle = new QPushButton(tr("Step Up\n192 scale"));
	butScoreFileStepDownSingle = new QPushButton(tr("Step Down\n192 scale"));
	butScoreFileSetSharp = new QPushButton(tr("Set #"));
	butScoreFileSetFlat = new QPushButton(tr("Set b"));
	butScoreFileReplaceAll = new QPushButton(tr("Replace all"));
	butScoreFileExport = new QPushButton(tr("To Lyrics with chords"));
	butScoreFileExportNoChords = new QPushButton(tr("To Lyrics no chords"));

#ifndef HAVE_PRINTER
	butScoreFilePrint->hide();
#endif
	gbScoreFile = new MppGroupBox(tr(buf));
	gbScoreFile->addWidget(butScoreFileNew, 0, 0, 1, 2);
	gbScoreFile->addWidget(butScoreFileOpen, 1, 0, 1, 2);
	gbScoreFile->addWidget(butScoreFileSave, 2, 0, 1, 2);
	gbScoreFile->addWidget(butScoreFileSaveAs, 3, 0, 1, 2);
	gbScoreFile->addWidget(butScoreFilePrint, 4, 0, 1, 2);
	gbScoreFile->addWidget(butScoreFileBassOffset, 5, 0, 1, 1);
	gbScoreFile->addWidget(spnScoreFileBassOffset, 5, 1, 1, 1);
	gbScoreFile->addWidget(butScoreFileAlign, 6, 0, 1, 1);
	gbScoreFile->addWidget(spnScoreFileAlign, 6, 1, 1, 1);
	gbScoreFile->addWidget(butScoreFileScale, 7, 0, 1, 1);
	gbScoreFile->addWidget(spnScoreFileScale, 7, 1, 1, 1);
	gbScoreFile->addWidget(butScoreFileStepUpHalf, 8, 0, 1, 1);
	gbScoreFile->addWidget(butScoreFileStepDownHalf, 8, 1, 1, 1);
	gbScoreFile->addWidget(butScoreFileStepUpSingle, 9, 0, 1, 1);
	gbScoreFile->addWidget(butScoreFileStepDownSingle, 9, 1, 1, 1);

	gbScoreFile->addWidget(butScoreFileSetSharp, 10, 0, 1, 1);
	gbScoreFile->addWidget(butScoreFileSetFlat, 10, 1, 1, 1);
	gbScoreFile->addWidget(butScoreFileReplaceAll, 11, 0, 1, 2);
	gbScoreFile->addWidget(butScoreFileExport, 12, 0, 1, 2);
	gbScoreFile->addWidget(butScoreFileExportNoChords, 13, 0, 1, 2);

	connect(butScoreFileNew, SIGNAL(released()), this, SLOT(handleScoreFileNew()));
	connect(butScoreFileOpen, SIGNAL(released()), this, SLOT(handleScoreFileOpen()));
	connect(butScoreFileSave, SIGNAL(released()), this, SLOT(handleScoreFileSave()));
	connect(butScoreFileSaveAs, SIGNAL(released()), this, SLOT(handleScoreFileSaveAs()));
	connect(butScoreFilePrint, SIGNAL(released()), this, SLOT(handleScorePrint()));
	connect(butScoreFileBassOffset, SIGNAL(released()), this, SLOT(handleScoreFileBassOffset()));
	connect(butScoreFileAlign, SIGNAL(released()), this, SLOT(handleScoreFileAlign()));
	connect(butScoreFileStepUpHalf, SIGNAL(released()), this, SLOT(handleScoreFileStepUpHalf()));
	connect(butScoreFileStepDownHalf, SIGNAL(released()), this, SLOT(handleScoreFileStepDownHalf()));
	connect(butScoreFileStepUpSingle, SIGNAL(released()), this, SLOT(handleScoreFileStepUpSingle()));
	connect(butScoreFileStepDownSingle, SIGNAL(released()), this, SLOT(handleScoreFileStepDownSingle()));
	connect(butScoreFileSetSharp, SIGNAL(released()), this, SLOT(handleScoreFileSetSharp()));
	connect(butScoreFileSetFlat, SIGNAL(released()), this, SLOT(handleScoreFileSetFlat()));
	connect(butScoreFileScale, SIGNAL(released()), this, SLOT(handleScoreFileScale()));
	connect(butScoreFileReplaceAll, SIGNAL(released()), this, SLOT(handleScoreFileReplaceAll()));
	connect(butScoreFileExport, SIGNAL(released()), this, SLOT(handleScoreFileExport()));
	connect(butScoreFileExportNoChords, SIGNAL(released()), this, SLOT(handleScoreFileExportNoChords()));

	/* Editor */

	editWidget = new MppScoreTextEdit(this);

	editWidget->setFont(parent->editFont);
	editWidget->setPlainText(defaultText);
	editWidget->setCursorWidth(4);
	editWidget->setLineWrapMode(QPlainTextEdit::NoWrap);

	/* GridLayout */

	gl_view = new MppGridLayout();
	gl_view->setSpacing(0);

	viewScroll = new QScrollBar(Qt::Vertical);
	viewScroll->setValue(0);
	viewScroll->setRange(0,0);
	viewScroll->setPageStep(1);

	connect(viewScroll, SIGNAL(valueChanged(int)), this, SLOT(handleScrollChanged(int)));

	/* Visual */

	viewWidgetSub = new MppScoreView(this);

	gl_view->addWidget(viewWidgetSub, 0, 0, 1, 1);
	gl_view->addWidget(viewScroll, 0, 1, 1, 1);

	sheet = new MppSheet(parent, _unit);
	
	/* Initial compile */

	handleCompile(1);

	QPainter paint;

	picChord[0] = new QPicture();
	paint.begin(picChord[0]);
	paint.setRenderHints(QPainter::Antialiasing, 1);
 	paint.setPen(QPen(Mpp.ColorBlack, 1));
	paint.setBrush(QColor(Mpp.ColorBlack));
	paint.drawEllipse(QRect(0,0,MPP_VISUAL_C_MAX,MPP_VISUAL_C_MAX));
	paint.end();

	picChord[1] = new QPicture();
	paint.begin(picChord[1]);
	paint.setRenderHints(QPainter::Antialiasing, 1);
 	paint.setPen(QPen(Mpp.ColorBlack, 1));
	paint.setBrush(QColor(Mpp.ColorBlack));
	paint.drawEllipse(QRect(MPP_VISUAL_C_MAX,0,MPP_VISUAL_C_MAX,MPP_VISUAL_C_MAX));
	paint.end();
}

MppScoreMain :: ~MppScoreMain()
{
	handleScoreFileNew();
}

void
MppScoreMain :: handlePrintSub(QPrinter *pd, QPoint orig)
{
#ifdef HAVE_PRINTER
	enum { PAGES_MAX = 128 };
	int pageStart[PAGES_MAX];
	int pageNum;
	int pageLimit;
#endif
	MppVisualDot *pdot;
	MppElement *ptr;
	MppElement *next;
	QPainter paint;
	QString linebuf;
	QString chord;
	QRectF box;
	QFont fnt_a;
	QFont fnt_b;
	qreal chord_x_max;
	qreal offset;
	qreal scale_x;
	qreal scale_y;
	int cmax_y;
	int rmax_x;
	int rmax_y;
	int margin_x;
	int margin_y;
	int vmax_y;
	int last_dot;
	int dur;
	int x;
	int y;
	int z;

#ifdef HAVE_PRINTER
	if (pd != NULL) {
		fnt_a = mainWindow->printFont;
		fnt_a.setPointSize(mainWindow->printFont.pixelSize());

		fnt_b = mainWindow->printFont;
		fnt_b.setPointSize(mainWindow->printFont.pixelSize() + 2);

		scale_x = (qreal)pd->logicalDpiX() / (qreal)mainWindow->logicalDpiX();
		scale_y = (qreal)pd->logicalDpiY() / (qreal)mainWindow->logicalDpiY();

		/* translate printing area */
		paint.begin(pd);
		paint.translate(orig);
	}
#endif
	if (pd == NULL) {
		fnt_a = mainWindow->defaultFont;
		fnt_a.setPixelSize(mainWindow->defaultFont.pixelSize());

		fnt_b = mainWindow->defaultFont;
		fnt_b.setPixelSize(mainWindow->defaultFont.pixelSize() + 4);

		scale_x = 1.0;
		scale_y = 1.0;
	}
		
	/* extract all text */
	for (x = 0; x != visual_max; x++) {
		QString *pstr = pVisual[x].str;

		/* delete old and allocate a new string */
		delete pstr;
		pstr = new QString();

		/* parse through the text */
		for (ptr = pVisual[x].start; ptr != pVisual[x].stop;
		     ptr = TAILQ_NEXT(ptr, entry)) {
			switch (ptr->type) {
			case MPP_T_STRING_DESC:
				*pstr += ptr->txt;
				break;
			default:
				break;
			}
		}
		/* Trim string */
		*pstr = pstr->trimmed();

		/* store new string */
		pVisual[x].str = pstr;
	}

	rmax_x = MPP_VISUAL_R_MAX * scale_x;
	rmax_y = MPP_VISUAL_R_MAX * scale_y;

	margin_x = MPP_VISUAL_MARGIN * scale_x;
	margin_y = MPP_VISUAL_MARGIN * scale_y;

#ifdef HAVE_PRINTER
	if (pd != NULL) {
		paint.translate(QPoint(-margin_x, -margin_y));

		/* count all pages */
		pageNum = 0;
		pageStart[pageNum++] = 0;
		for (x = 0; x != visual_max; x++) {
			QString &str = *pVisual[x].str;

			if (pageNum < PAGES_MAX &&
			    str.length() > 1 && str[0] == 'L' && str[1].isDigit())
				pageStart[pageNum++] = x;
		}
		if (pageNum < PAGES_MAX)
			pageStart[pageNum++] = visual_max;

		pageNum = 0;
	}
#endif
	QFontMetricsF fm_b(fnt_b, paint.device());

	vmax_y = MPP_VISUAL_C_MAX * scale_y + 3 * fm_b.height();
	cmax_y = MPP_VISUAL_C_MAX * scale_y;

	/* sanity check */
	if (vmax_y < 1)
		vmax_y = 1;

	/* store copy of maximum Y value */
	if (pd == NULL)
		visual_y_max = vmax_y;
#ifdef HAVE_PRINTER
	if (pd != NULL) {
		pageLimit = (pd->height() - 2 * margin_y) / vmax_y;
		if (pageLimit < 1)
			pageLimit = 1;
	}
#endif

	for (x = y = 0; x != visual_max; x++) {
#ifdef HAVE_PRINTER
		if (pd != NULL) {
			while (pageNum < PAGES_MAX && pageStart[pageNum] == x) {
				pageNum++;
				if (pageNum < PAGES_MAX &&
				    (pageStart[pageNum] - x) >= (pageLimit - y) && y != 0) {
					pd->newPage();
					paint.translate(QPoint(0, -vmax_y * y));
					y = 0;
				}
			}
			if (y != 0 && (y >= pageLimit || pVisual[x].newpage != 0)) {
				pd->newPage();
				paint.translate(QPoint(0, -vmax_y * y));
				y = 0;
			}
		}
#endif
		if (pd == NULL) {
			delete (pVisual[x].pic);
			pVisual[x].pic = new QPicture();
			paint.begin(pVisual[x].pic);
			paint.setRenderHints(QPainter::Antialiasing, 1);
		}

		paint.setPen(QPen(Mpp.ColorBlack, 1));
		paint.setBrush(QColor(Mpp.ColorBlack));

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

				pdot->x_off = margin_x + box.width();
				pdot->y_off = margin_y + (vmax_y / 3);

				for (next = ptr; next != pVisual[x].stop;
				     next = TAILQ_NEXT(next, entry)) {
					if (next->type == MPP_T_STRING_CHORD &&
					    next->txt.size() > 1 && next->txt[0] == '(')
						break;
				}
				if (next != pVisual[x].stop)
					pdot->x_off += (fm_b.width(next->txt[1]) - rmax_x - 2.0 * scale_x) / 2.0;

				dur = ptr->value[0];

				paint.drawEllipse(QRectF(pdot->x_off, pdot->y_off, rmax_x, rmax_y));

				if (dur <= 0)
					break;
				if (dur > 5)
					dur = 5;

				offset = 0;

				paint.drawLine(
				    pdot->x_off + rmax_x, pdot->y_off + (rmax_y / 2),
				    pdot->x_off + rmax_x, pdot->y_off + (rmax_y / 2) - (3 * rmax_y));

				while (dur--) {
					paint.drawLine(
					    pdot->x_off + rmax_x, pdot->y_off + (rmax_y / 2) - (3 * rmax_y) + offset,
					    pdot->x_off, pdot->y_off + rmax_y - (3 * rmax_y) + offset);

					offset += (rmax_y / 2);
				}
				break;

			case MPP_T_STRING_CHORD:
				chord = MppDeQuoteChord(ptr->txt);

				paint.setFont(fnt_b);
				paint.drawText(QPointF(margin_x + box.width(),
				    margin_y + (vmax_y / 3) - (cmax_y / 4)), chord);

				chord_x_max = box.width() +
					paint.boundingRect(QRectF(0,0,0,0), Qt::TextSingleLine | Qt::AlignLeft,
					chord + QChar(' ')).width();
				break;

			default:
				break;
			}
		}

		paint.setFont(fnt_a);
		paint.drawText(QPointF(margin_x, margin_y + vmax_y - (vmax_y / 3) - (cmax_y / 4)), linebuf);

#ifdef HAVE_PRINTER
		if (pd != NULL) {
			paint.translate(QPoint(0, vmax_y));
			y++;
		}
#endif
		if (pd == NULL)
			paint.end();
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

	mainWindow->atomic_lock();
	head.jumpPointer(pVisual[yi].start);
	head.syncLast();
	mainWindow->handle_stop();
	mainWindow->atomic_unlock();
}

void
MppScoreMain :: locateVisual(MppElement *ptr, int *pindex,
    int *pnext, MppVisualDot **ppdot)
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
				if (elem->type == MPP_T_SCORE_SUBDIV ||
				    elem->type == MPP_T_MACRO) {
					y++;
					break;
				}
			}
		}
		break;
	}
	if (pindex != 0) {
		*pindex = x;
	}
	if (pnext != 0) {
		if (x != visual_max && (x + 1) != visual_max &&
		    pVisual[x].stop == pVisual[x + 1].start)
			*pnext = x + 1;
		else
			*pnext = x;
	}
	if (ppdot != 0) {
		if (x != visual_max && y < pVisual[x].ndot)
			*ppdot = &pVisual[x].pdot[y];
		else
			*ppdot = 0;
	}
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

	paint.fillRect(event->rect(), Mpp.ColorWhite);

	mainWindow->atomic_lock();
	curr = head.state.curr_start;
	last = head.state.last_start;
	scroll = picScroll;
	mainWindow->atomic_unlock();

	y_blocks = (viewWidgetSub->height() / visual_y_max);
	if (y_blocks == 0)
		y_blocks = 1;

	/* locate current play position */
	locateVisual(curr, &yc_rem, 0, &pcdot);

	/* locate last play position */
	locateVisual(last, &yo_rem, 0, &podot);

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
		paint.setPen(QPen(Mpp.ColorGreen, 4));
		paint.setBrush(QColor(Mpp.ColorGreen));
		paint.drawEllipse(QRect(podot->x_off,
		    podot->y_off + (yo_rem * visual_y_max),
		    MPP_VISUAL_R_MAX, MPP_VISUAL_R_MAX));

	}

	/* overlay (current) */

	if ((yc_div == y_div) && (pcdot != 0)) {
		paint.setPen(QPen(Mpp.ColorLogo, 4));
		paint.setBrush(QColor(Mpp.ColorLogo));
		paint.drawEllipse(QRect(pcdot->x_off,
		    pcdot->y_off + (yc_rem * visual_y_max),
		    MPP_VISUAL_R_MAX, MPP_VISUAL_R_MAX));
	}

	/* overlay (active chord) */

	if (keyMode == MM_PASS_NONE_CHORD_PIANO ||
	    keyMode == MM_PASS_NONE_CHORD_GUITAR) {
		int width = viewWidgetSub->width();
		int mask;

		mainWindow->atomic_lock();
		mask = ((mainWindow->get_time_offset() % 1000) >= 500);
		mainWindow->atomic_unlock();

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
	int key_mode;
	int auto_melody;
	int auto_utune;
	int has_string;
	int index;
	int x;
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

	/* no automatic micro tuning */
	auto_utune = -1;
	
	/* no key mode selection */
	key_mode = -1;

	/* cleanup visual entries */
	for (x = 0; x != visual_max; x++) {
		delete (pVisual[x].str);
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
					mainWindow->atomic_lock();
					mainWindow->dlg_bpm->period_ref = ptr->value[1];
					mainWindow->dlg_bpm->period_cur = ptr->value[2];
					mainWindow->dlg_bpm->handle_update();
					mainWindow->atomic_unlock();
					break;
				case MPP_CMD_AUTO_MELODY:
					auto_melody = ptr->value[1];
					break;
				case MPP_CMD_KEY_MODE:
					key_mode = ptr->value[1];
					break;
				case MPP_CMD_MICRO_TUNE:
					auto_utune = ptr->value[1];
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

	/* compile before auto-melody */
	sheet->compile(head);
	
	if (auto_utune > 0)
		head.tuneScore();

	/* check if key-mode should be applied */
	switch (key_mode) {
	case 0:
		keyMode = MM_PASS_ALL;
		mainWindow->keyModeUpdated = 1;
		break;
	case 2:
		keyMode = MM_PASS_NONE_FIXED;
		mainWindow->keyModeUpdated = 1;
		break;
	case 3:
		keyMode = MM_PASS_NONE_TRANS;
		mainWindow->keyModeUpdated = 1;
		break;
	case 4:
		keyMode = MM_PASS_NONE_CHORD_PIANO;
		mainWindow->keyModeUpdated = 1;
		break;
	case 5:
		keyMode = MM_PASS_NONE_CHORD_GUITAR;
		mainWindow->keyModeUpdated = 1;
		break;
	case 6:
		keyMode = MM_PASS_NONE_CHORD_ALL;
		mainWindow->keyModeUpdated = 1;
		break;
	default:
		break;
	}

	/* number all elements to make searching easier */
	head.sequence();

	/* get first line */
	head.currLine(&start, &stop);

	/* sync last */
	head.syncLast();

	/* create the graphics */
	handlePrintSub(0, QPoint(0,0));

	/* update scrollbar */
	viewScroll->setMaximum((visual_max > 0) ? (visual_max - 1) : 0);

#ifndef HAVE_NO_SHOW
	mainWindow->tab_show_control->handle_text_change();
	mainWindow->tab_show_control->handle_pict_change();
#endif
}

void
MppScoreMain :: handleScoreFileNew(int invisible)
{
	editWidget->setPlainText(QString());

	handleCompile();

	if (currScoreFileName != NULL) {
		delete (currScoreFileName);
		currScoreFileName = NULL;
	}

	if (invisible == 0) {
		mainWindow->handle_tab_changed(1);
		mainWindow->handle_make_scores_visible(this);
	}
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
		Mpp.HomeDirTxt,
		QString("Score File (*.txt *.TXT)"));

	diag->setAcceptMode(QFileDialog::AcceptOpen);
	diag->setFileMode(QFileDialog::ExistingFile);

	if (diag->exec()) {
		Mpp.HomeDirTxt = diag->directory().path();
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
		Mpp.HomeDirTxt,
		QString("Score File (*.txt *.TXT)"));

	diag->setAcceptMode(QFileDialog::AcceptSave);
	diag->setFileMode(QFileDialog::AnyFile);
	diag->setDefaultSuffix(QString("txt"));

	if (diag->exec()) {
		Mpp.HomeDirTxt = diag->directory().path();

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
	if ((pos < 0) || (pos >= MPP_MAX_LABELS) ||
	    (head.state.label_start[pos] == 0))
		return (0);

	return (1);
}

/* must be called locked */
void
MppScoreMain :: handleLabelJump(int pos)
{
	if (checkLabelJump(pos) == 0)
		return;

	mainWindow->dlg_bpm->handle_jump_event_locked(unit);

	head.jumpLabel(pos);
	head.syncLast();

	mainWindow->cursorUpdate = 1;

	mainWindow->handle_stop(1);

	if (songEventsOn != 0)
		mainWindow->send_song_select_locked(pos);
}

#define	MPP_CHORD_MAP_KEY 0x0FF
#define	MPP_CHORD_MAP_CUR 0x100
#define	MPP_CHORD_MAP_A	0x200
#define	MPP_CHORD_MAP_B	0x400
#define	MPP_CHORD_MAP_BASE 0x800
#define	MPP_CHORD_MAP_RELOAD 0x1000

#if C0 != 0
#error "C is not starting the scale"
#endif

static const uint16_t mpp_piano_chord_map[MPP_MAX_CHORD_MAP] = {
  /* 1st octave */
  /* [C0] = */ 0,	/* dead */
  /* [D0B] = */ MPP_CHORD_MAP_A + 2 + MPP_CHORD_MAP_BASE,
  /* [D0] = */ MPP_CHORD_MAP_A + 0 + MPP_CHORD_MAP_BASE,
  /* [E0B] = */ MPP_CHORD_MAP_A + 3 + MPP_CHORD_MAP_BASE,
  /* [E0] = */ MPP_CHORD_MAP_A + 1 + MPP_CHORD_MAP_BASE,

  /* [F0] = */ MPP_CHORD_MAP_A + 0,
  /* [G0B] = */ MPP_CHORD_MAP_A + 4,
  /* [G0] = */ MPP_CHORD_MAP_A + 1,
  /* [A0B] = */ MPP_CHORD_MAP_A + 5,
  /* [A0] = */ MPP_CHORD_MAP_A + 2,
  /* [H0B] = */ MPP_CHORD_MAP_A + 6,
  /* [H0] = */ MPP_CHORD_MAP_A + 3,

  /* 2nd octave */
  /* [C1] = */ 0,	/* dead */
  /* [D1B] = */ MPP_CHORD_MAP_B + 2 + MPP_CHORD_MAP_BASE,
  /* [D1] = */ MPP_CHORD_MAP_B + 0 + MPP_CHORD_MAP_BASE,
  /* [E1B] = */ MPP_CHORD_MAP_B + 3 + MPP_CHORD_MAP_BASE,
  /* [E1] = */ MPP_CHORD_MAP_B + 1 + MPP_CHORD_MAP_BASE,

  /* [F1] = */ MPP_CHORD_MAP_B + 0,
  /* [G1B] = */ MPP_CHORD_MAP_B + 4,
  /* [G1] = */ MPP_CHORD_MAP_B + 1,
  /* [A1B] = */ MPP_CHORD_MAP_B + 5,
  /* [A1] = */ MPP_CHORD_MAP_B + 2,
  /* [H1B] = */ MPP_CHORD_MAP_B + 6,
  /* [H1] = */ MPP_CHORD_MAP_B + 3,
};

static const uint16_t mpp_guitar_chord_map[MPP_MAX_CHORD_MAP] = {
  /* E - string */
  /* [C0] = */ MPP_CHORD_MAP_CUR + 0 + MPP_CHORD_MAP_BASE,
  /* [D0B] = */ MPP_CHORD_MAP_A + 0 + MPP_CHORD_MAP_BASE,
  /* [D0] = */ MPP_CHORD_MAP_A + 1 + MPP_CHORD_MAP_BASE,
  /* [E0B] = */ MPP_CHORD_MAP_B + 0 + MPP_CHORD_MAP_BASE,
  /* [E0] = */ MPP_CHORD_MAP_B + 1 + MPP_CHORD_MAP_BASE,

  /* A - string */
  /* [F0] = */ MPP_CHORD_MAP_CUR + 0,
  /* [G0B] = */ MPP_CHORD_MAP_A + 0,
  /* [G0] = */ MPP_CHORD_MAP_A + 5,
  /* [A0B] = */ MPP_CHORD_MAP_B + 0,
  /* [A0] = */ MPP_CHORD_MAP_B + 5,

  /* D - string */
  /* [H0B] = */ MPP_CHORD_MAP_CUR + 1,
  /* [H0] = */ MPP_CHORD_MAP_A + 1,
  /* [C1] = */ MPP_CHORD_MAP_A + 6,
  /* [D1B] = */ MPP_CHORD_MAP_B + 1,
  /* [D1] = */ MPP_CHORD_MAP_B + 6,

  /* G - string */
  /* [E1B] = */ MPP_CHORD_MAP_CUR + 2,
  /* [E1] = */ MPP_CHORD_MAP_A + 2,
  /* [F1] = */ MPP_CHORD_MAP_A + 7,
  /* [G1B] = */ MPP_CHORD_MAP_B + 2,
  /* [G1] = */ MPP_CHORD_MAP_B + 7,

  /* B - string */
  /* [A1B] = */ MPP_CHORD_MAP_CUR + 3,
  /* [A1] = */ MPP_CHORD_MAP_A + 3,
  /* [H1B] = */ MPP_CHORD_MAP_A + 8,
  /* [H1] = */ MPP_CHORD_MAP_B + 3,
  /* [C2] = */ MPP_CHORD_MAP_B + 8,

  /* E - string */
  /* [D2B] = */ MPP_CHORD_MAP_CUR + 4,
  /* [D2] = */ MPP_CHORD_MAP_A + 4,
  /* [E2B] = */ MPP_CHORD_MAP_A + 9,
  /* [E2] = */ MPP_CHORD_MAP_B + 4,
  /* [F2] = */ MPP_CHORD_MAP_B + 9,
};

static const uint16_t mpp_piano_chord_all_map[MPP_MAX_CHORD_MAP] = {
  /* 1st octave */
  /* [C0] = */ 0,	/* dead */
  /* [D0B] = */ MPP_CHORD_MAP_A + 2 + MPP_CHORD_MAP_BASE,
  /* [D0] = */ MPP_CHORD_MAP_A + 0 + MPP_CHORD_MAP_BASE,
  /* [E0B] = */ MPP_CHORD_MAP_A + 3 + MPP_CHORD_MAP_BASE,
  /* [E0] = */ MPP_CHORD_MAP_A + 1 + MPP_CHORD_MAP_BASE,

  /* [F0] = */ MPP_CHORD_MAP_A + 0,
  /* [G0B] = */ MPP_CHORD_MAP_A + 4,
  /* [G0] = */ MPP_CHORD_MAP_A + 1,
  /* [A0B] = */ MPP_CHORD_MAP_A + 5,
  /* [A0] = */ MPP_CHORD_MAP_A + 2,
  /* [H0B] = */ MPP_CHORD_MAP_A + 6,
  /* [H0] = */ MPP_CHORD_MAP_A + 3,

  /* 2nd octave */
  /* [C0] = */ MPP_CHORD_MAP_A + 7,
  /* [D0B] = */ MPP_CHORD_MAP_A + 10,
  /* [D0] = */ MPP_CHORD_MAP_A + 8,
  /* [E0B] = */ MPP_CHORD_MAP_A + 11,
  /* [E0] = */ MPP_CHORD_MAP_A + 9,

  /* [F0] = */ MPP_CHORD_MAP_A + 12,
  /* [G0B] = */ MPP_CHORD_MAP_A + 16,
  /* [G0] = */ MPP_CHORD_MAP_A + 13,
  /* [A0B] = */ MPP_CHORD_MAP_A + 17,
  /* [A0] = */ MPP_CHORD_MAP_A + 14,
  /* [H0B] = */ MPP_CHORD_MAP_A + 18,
  /* [H0] = */ MPP_CHORD_MAP_A + 15,

  /* 4th octave */
  /* [C0] = */ 0,	/* dead */
  /* [D0B] = */ MPP_CHORD_MAP_B + 2 + MPP_CHORD_MAP_BASE,
  /* [D0] = */ MPP_CHORD_MAP_B + 0 + MPP_CHORD_MAP_BASE,
  /* [E0B] = */ MPP_CHORD_MAP_B + 3 + MPP_CHORD_MAP_BASE,
  /* [E0] = */ MPP_CHORD_MAP_B + 1 + MPP_CHORD_MAP_BASE,

  /* [F0] = */ MPP_CHORD_MAP_B + 0,
  /* [G0B] = */ MPP_CHORD_MAP_B + 4,
  /* [G0] = */ MPP_CHORD_MAP_B + 1,
  /* [A0B] = */ MPP_CHORD_MAP_B + 5,
  /* [A0] = */ MPP_CHORD_MAP_B + 2,
  /* [H0B] = */ MPP_CHORD_MAP_B + 6,
  /* [H0] = */ MPP_CHORD_MAP_B + 3,

  /* 2nd octave */
  /* [C0] = */ MPP_CHORD_MAP_B + 7,
  /* [D0B] = */ MPP_CHORD_MAP_B + 10,
  /* [D0] = */ MPP_CHORD_MAP_B + 8,
  /* [E0B] = */ MPP_CHORD_MAP_B + 11,
  /* [E0] = */ MPP_CHORD_MAP_B + 9,

  /* [F0] = */ MPP_CHORD_MAP_B + 12,
  /* [G0B] = */ MPP_CHORD_MAP_B + 16,
  /* [G0] = */ MPP_CHORD_MAP_B + 13,
  /* [A0B] = */ MPP_CHORD_MAP_B + 17,
  /* [A0] = */ MPP_CHORD_MAP_B + 14,
  /* [H0B] = */ MPP_CHORD_MAP_B + 18,
  /* [H0] = */ MPP_CHORD_MAP_B + 15,
};

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
	int score[24];
	int base[24];
	int key[24];

	memset(score_future_base, 0, sizeof(score_future_base));
	memset(score_future_treble, 0, sizeof(score_future_treble));

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
		case MPP_T_SCORE_SUBDIV:
			if (duration == 0)
				break;
			if (ns < 24)
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

	MppSort(score, ns);

	MppSplitBaseTreble(score, ns, base, &nb, key, &nk);
	
	if (nb != 0) {
		MppSort(base, nb);
		for (x = 0; x != MPP_MAX_CHORD_FUTURE; x++) {
			score_future_base[x].dur = 1;
			score_future_base[x].key = base[0];
			score_future_base[x].channel = chan;
			MppTrans(base, nb, 1);
		}
	}

	if (nk != 0) {
		MppSort(key, nk);
		for (x = 0; x != MPP_MAX_CHORD_FUTURE; x++) {
			score_future_treble[x].dur = 1;
			score_future_treble[x].key = key[0];
			score_future_treble[x].channel = chan;
			MppTrans(key, nk, 1);
		}
	}

	head.syncLast();
	head.stepLine(&start, &stop);

	mainWindow->cursorUpdate = 1;

	mainWindow->dlg_bpm->handle_beat_event_locked(unit);
}

/* must be called locked */
uint8_t
MppScoreMain :: handleKeyRemovePast(MppScoreEntry *pn, int vel, uint32_t key_delay)
{
	uint8_t retval = 0;
	unsigned x;

	for (x = 0; x != MPP_MAX_CHORD_MAP; x++) {
		if (score_past[x].dur != 0 &&
		    score_past[x].key == pn->key &&
		    score_past[x].channel == pn->channel) {

			mainWindow->output_key(score_past[x].track,
			    score_past[x].channel, score_past[x].key,
			    -vel, key_delay, 0);

			/* check for secondary event */
			if (score_past[x].channelSec != 0) {
				mainWindow->output_key(score_past[x].trackSec,
				    score_past[x].channelSec - 1, score_past[x].key,
				    -vel, key_delay, 0);
			}

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
MppScoreMain :: handleKeyPressChord(int in_key, int vel, uint32_t key_delay)
{
	int bk = baseKey / MPP_BAND_STEP_12;
	int ck = in_key / MPP_BAND_STEP_12;
	MppScoreEntry mse;
	uint16_t map;
	int off;

	off = ck - bk;
	if (off < 0 || off >= MPP_MAX_CHORD_MAP)
		return;

	if (keyMode == MM_PASS_NONE_CHORD_ALL)
		map = mpp_piano_chord_all_map[off];
	else if (keyMode == MM_PASS_NONE_CHORD_PIANO)
		map = mpp_piano_chord_map[off];
	else
		map = mpp_guitar_chord_map[off];

	if (map & MPP_CHORD_MAP_RELOAD) {
		handleChordsLoad();
		pressed_future = 2;
		return;
	} else if (map & MPP_CHORD_MAP_CUR) {
		if (head.isFirst()) {
			handleChordsLoad();
			pressed_future = 2;
		}
	} else if (map & MPP_CHORD_MAP_A) {
		if (pressed_future == 0 || head.isFirst())
			handleChordsLoad();
		if (pressed_future != 1)
			pressed_future = 1;
	} else if (map & MPP_CHORD_MAP_B) {
		if (pressed_future == 1 || head.isFirst())
			handleChordsLoad();
		if (pressed_future != 0)
			pressed_future = 0;
	} else {
		return;	/* dead keys */
	}

	if (map & MPP_CHORD_MAP_BASE) {
		mse = score_future_base[map & MPP_CHORD_MAP_KEY];
	} else {
		mse = score_future_treble[map & MPP_CHORD_MAP_KEY];
	}

	/* check for nonexisting key */
	if (mse.dur == 0)
		return;

	/* update channel and device */
	mse.channel = (mse.channel + synthChannel) & 0xF;
	mse.track = MPP_DEFAULT_TRACK(unit);

	/* remove key if already pressed */
	if (handleKeyRemovePast(&mse, vel, key_delay))
		key_delay++;

	if (map & MPP_CHORD_MAP_BASE) {
		if (synthChannelBase != (int)mse.channel ||
		    synthDeviceBase != synthDevice) {
			mse.channelSec = synthChannelBase + 1;
			mse.trackSec = MPP_BASS_TRACK(unit);
		}
	} else {
		if (synthChannelTreb != (int)mse.channel ||
		    synthDeviceTreb != synthDevice) {
			mse.channelSec = synthChannelTreb + 1;
			mse.trackSec = MPP_TREBLE_TRACK(unit);
		}
	}

	/* store information for key release command */
	score_past[off] = mse;

	mainWindow->output_key(mse.track, mse.channel, mse.key, vel, key_delay, 0);

	/* check for secondary event */
	if (mse.channelSec != 0) {
		mainWindow->output_key(mse.trackSec, mse.channelSec - 1,
		    mse.key, vel, key_delay, 0);
	}
	mainWindow->cursorUpdate = 1;
}

/* must be called locked */
void
MppScoreMain :: handleKeyPressureChord(int in_key, int vel, uint32_t key_delay)
{
	int bk = baseKey / MPP_BAND_STEP_12;
	int ck = in_key / MPP_BAND_STEP_12;
	MppScoreEntry *pn;
	int off;

	off = ck - bk;
	if (off < 0 || off >= MPP_MAX_CHORD_MAP)
		return;

	pn = &score_past[off];

	if (pn->dur != 0) {
		mainWindow->output_key_pressure(pn->track, pn->channel, pn->key, vel, key_delay);

		/* check for secondary event */
		if (pn->channelSec != 0) {
			mainWindow->output_key_pressure(
			    pn->trackSec, pn->channelSec - 1, pn->key, vel, key_delay);
		}
	}
}

/* must be called locked */
void
MppScoreMain :: handleKeyReleaseChord(int in_key, int vel, uint32_t key_delay)
{
	int bk = baseKey / MPP_BAND_STEP_12;
	int ck = in_key / MPP_BAND_STEP_12;
	MppScoreEntry *pn;
	int off;

	off = ck - bk;
	if (off < 0 || off >= MPP_MAX_CHORD_MAP)
		return;

	pn = &score_past[off];

	/* release key once, if any */
	if (pn->dur != 0) {
		mainWindow->output_key(pn->track, pn->channel,
		    pn->key, -vel, key_delay, 0);

		/* check for secondary event */
		if (pn->channelSec != 0) {
			mainWindow->output_key(pn->trackSec, pn->channelSec - 1,
			    pn->key, -vel, key_delay, 0);
		}
		pn->dur = 0;
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
	int vel_other;
	int transpose;

	head.currLine(&start, &stop);
	head.state.did_jump = 0;

	while (head.state.did_jump == 0) {
		t_pre = 0;
		t_post = 0;
		channel = 0;
		duration = 1;
		nscore = 0;
		transpose = key_trans;

		decrementDuration(vel, 0);

		for (ptr = start; ptr != stop;
		    ptr = TAILQ_NEXT(ptr, entry)) {
			switch (ptr->type) {
			case MPP_T_SCORE_SUBDIV:
				if (duration <= 0)
					break;
				nscore++;
				break;
			case MPP_T_DURATION:
				duration = ptr->value[0];
				break;
			default:
				break;
			}
		}

		if (nscore == 0) {
			vel_other = 0;
		} else {
			if (chordNormalize == 0) {
				vel_other = vel;
			} else {
				vel_other = vel +
				  ((vel * (nscore - 1) * (128 - chordContrast)) /
				    (nscore * 128));
				if (vel_other > 127)
					vel_other = 127;
				else if (vel_other < 0)
					vel_other = 0;
			}
		}

		duration = 1;

		for (ptr = start; ptr != stop; ptr = TAILQ_NEXT(ptr, entry)) {
			switch (ptr->type) {
			int ch;
			int out_key;
			int out_vel;
			int delay;
			int temp;

			case MPP_T_TRANSPOSE:
				if (transpose == MPP_KEY_MIN)
					break;

				transpose = ptr->value[0] + key_trans;

				switch (ptr->value[1]) {
				case 1:
					temp = mainWindow->getCurrTransposeScore();
					if (temp >= 0)
						transpose += temp;
					else
						transpose = MPP_KEY_MIN;
					break;
				case 2:
					temp = mainWindow->getCurrTransposeScore();
					if (temp >= 0)
						transpose += temp % MPP_MAX_BANDS;
					else
						transpose = MPP_KEY_MIN;
					break;
				default:
					break;
				}
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

			case MPP_T_SCORE_SUBDIV:
				if (duration <= 0)
					break;

				if (--nscore < 2)
					out_vel = vel;
				else
					out_vel = vel_other;

				out_key = ptr->value[0] + in_key + transpose;

				ch = (synthChannel + channel) & 0xF;

				if (delayNoise != 0)
					delay = mainWindow->noise8(delayNoise);
				else
					delay = 0;

				if (setPressedKey(ch, out_key, duration, delay))
					break;

				mainWindow->output_key(MPP_DEFAULT_TRACK(unit),
				    ch, out_key, out_vel, key_delay + delay, 0);
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
		decrementDuration(vel, key_delay);
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

	/* play sheet */

	handleKeyPressSub(in_key, vel, key_delay, -(int)baseKey, 1);

	/* update cursor, if any */

	mainWindow->cursorUpdate = 1;

	/* update bpm, if any */

	mainWindow->dlg_bpm->handle_beat_event_locked(unit);
}

/* must be called locked */
void
MppScoreMain :: decrementDuration(int vel, uint32_t timeout)
{
	int out_key;
	uint8_t chan;
	uint8_t delay;
	uint8_t x;

	for (x = 0; x != MPP_PRESSED_MAX; x++) {
		if (frozenKeys[x] != 0)
			continue;
		if ((pressedKeys[x] & 0xFF) == 1) {

			out_key = (pressedKeys[x] >> 32) & -1U;
			chan = (pressedKeys[x] >> 16) & 0xFF;
			delay = (pressedKeys[x] >> 24) & 0xFF;

			/* clear entry */
			pressedKeys[x] = 0;

			mainWindow->output_key(MPP_DEFAULT_TRACK(unit), chan,
			    out_key, -vel, timeout + delay, 0);
		}

		if (pressedKeys[x] != 0)
			pressedKeys[x] --;
	}
}

/* must be called locked */
void
MppScoreMain :: handleKeyRelease(int in_key, int vel, uint32_t key_delay)
{
	if (head.state.key_lock > 0) {
		if (in_key != (int)whatPlayKeyLocked) {
			return;
		}
	}

	decrementDuration(vel, key_delay);

	head.syncLast();
}

void
MppScoreMain :: handleScorePrint(void)
{
#ifdef HAVE_PRINTER
	QPrinter printer(QPrinter::HighResolution);
	QPrintDialog *dlg;
	QPoint orig;
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
		printer.setOutputFileName(Mpp.HomeDirTxt + QString("/NewSong.pdf"));
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

		handlePrintSub(&printer, orig);
	}

	delete dlg;
#endif
}

int
MppScoreMain :: setPressedKey(int chan, int out_key, int dur, int delay)
{
	uint64_t temp;
	uint8_t y;

	dur &= 0xFF;
	chan &= 0xFF;
	delay &= 0xFF;

	temp = dur | ((uint64_t)out_key << 32) | (chan << 16) | (delay << 24);

	if (dur == 0) {
		/* release key */
		for (y = 0; y != MPP_PRESSED_MAX; y++) {
			if (frozenKeys[y] != 0)
				continue;
			if ((pressedKeys[y] & 0xFFFFFFFF00FF0000ULL) == (temp & 0xFFFFFFFF00FF0000ULL)) {
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
			if ((pressedKeys[y] & 0xFFFFFFFF00FF0000ULL) == (temp & 0xFFFFFFFF00FF0000ULL)) {
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

		mainWindow->atomic_lock();
		handleParse(editText);
		mainWindow->atomic_unlock();

		return (1);
	}
	return (0);
}

void
MppScoreMain :: watchdog()
{
	MppElement *curr;
	int off;
	int y_blocks;

	QTextCursor cursor(editWidget->textCursor());

	QTextEdit::ExtraSelection format;

	/* Highlight the next line to be played */

	cursor.movePosition(QTextCursor::EndOfLine, QTextCursor::KeepAnchor, 1);

	format.cursor = cursor;
	format.format.setForeground(Mpp.ColorBlack);
	format.format.setBackground(Mpp.ColorGrey);

	QList<QTextEdit::ExtraSelection> extras;
	extras << format;

	editWidget->setExtraSelections(extras);

	/* Compute scrollbar */

	mainWindow->atomic_lock();
	curr = head.state.curr_start;
	mainWindow->atomic_unlock();

	/* Compute alignment factor */

	y_blocks = (viewWidgetSub->height() / visual_y_max);
	if (y_blocks == 0)
		y_blocks = 1;

	/* locate current play position */
	locateVisual(curr, &off, 0, 0);

	if (off < visual_max) {
		viewScroll->setValue(off - (off % y_blocks));
	} else {
		viewScroll->setValue(0);
	}

	/* check sheet view too */
	sheet->watchdog();
}

void
MppScoreMain :: handleScrollChanged(int value)
{
	mainWindow->atomic_lock();
	picScroll = value;
	mainWindow->atomic_unlock();

	viewWidgetSub->update();
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
		temp.optimise();
		break;
	case 1:
		temp.transposeScore(parm, flag);
		temp.optimise();
		break;
	case 2:
		temp.scaleTime(parm);
		break;
	case 4:
		temp.bassOffset(parm);
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
MppScoreMain :: handleScoreFileBassOffset(void)
{
	int which = spnScoreFileBassOffset->value();

	which = MPP_SUBDIV_REM_BITREV(which);

	handleScoreFileEffect(4, which, 0);
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
MppScoreMain :: handleScoreFileStepUpHalf(void)
{
	handleScoreFileEffect(1,MPP_BAND_STEP_12,0);
}

void
MppScoreMain :: handleScoreFileStepDownHalf(void)
{
	handleScoreFileEffect(1,-MPP_BAND_STEP_12,0);
}

void
MppScoreMain :: handleScoreFileStepUpSingle(void)
{
	handleScoreFileEffect(1, MPP_BAND_STEP_192, 0);
}

void
MppScoreMain :: handleScoreFileStepDownSingle(void)
{
	handleScoreFileEffect(1, -MPP_BAND_STEP_192, 0);
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
	cursor.insertText(temp.toLyrics(0));
	cursor.endEditBlock();

	mainWindow->handle_make_tab_visible(mainWindow->tab_import->editWidget);
}

void
MppScoreMain :: handleScoreFileExportNoChords(void)
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
	cursor.insertText(temp.toLyrics(1));
	cursor.endEditBlock();

	mainWindow->handle_make_tab_visible(mainWindow->tab_import->editWidget);
}

void
MppScoreMain :: handleEditLine(void)
{
	MppHead temp;
	MppChordElement info;
	QTextCursor cursor(editWidget->textCursor());
	int row;

	cursor.movePosition(QTextCursor::EndOfLine, QTextCursor::MoveAnchor, 1);
	cursor.movePosition(QTextCursor::StartOfLine, QTextCursor::KeepAnchor, 1);

	/* check if the line is empty */
	if (cursor.selectedText().simplified().size() == 0)
		return;

	row = cursor.blockNumber();

	temp += editWidget->toPlainText();
	temp.flush();

	/* check if the chord is valid */
	if (temp.getChord(row, &info) != 0) {
		MppMainWindow *mw = mainWindow;
		if (mw->tab_chord_gl->parseScoreChord(&info) == 0) {
			mw->main_tb->makeWidgetVisible(mw->tab_chord_gl, this->editWidget);
		}
	}
	editWidget->setTextCursor(cursor);
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
uint16_t
MppScoreMain :: outputChannelMaskGet(void)
{
	uint32_t mask;

	switch (keyMode) {
	case MM_PASS_ALL:
		mask = 1U;
		break;
	case MM_PASS_NONE_CHORD_ALL:
	case MM_PASS_NONE_CHORD_PIANO:
	case MM_PASS_NONE_CHORD_GUITAR:
		mask = 1U;
		if (synthChannelBase > -1) {
			const int y = (synthChannelBase - synthChannel) & 0xF;
			mask |= 1U << y;
		}
		if (synthChannelTreb > -1) {
			const int y = (synthChannelTreb - synthChannel) & 0xF;
			mask |= 1U << y;
		}
		/* check for wrap around */
		mask |= (mask >> 16);
		mask &= 0xFFFF;
		break;
	default:
		mask = active_channels;
		break;
	}
	return (mask);
}


uint8_t
MppScoreMain :: outputTrackMirror(uint8_t which)
{
	/* check for broadcasters */
	if (synthDevice == -1)
		return (which != MPP_DEFAULT_TRACK(0));
	if (synthChannelTreb > -1 && synthDeviceTreb == -1)
		return (which != MPP_TREBLE_TRACK(0));
	if (synthChannelBase > -1 && synthDeviceBase == -1)
		return (which != MPP_BASS_TRACK(0));
	return (0);
}

/* must be called locked */
void
MppScoreMain :: outputControl(uint8_t ctrl, uint8_t val)
{
	MppMainWindow *mw = mainWindow;
	struct mid_data *d = &mw->mid_data;
	const unsigned int off = unit * MPP_TRACKS_PER_VIEW;
	uint16_t ChannelMask;
	uint8_t chan;

	chan = synthChannel;
	ChannelMask = outputChannelMaskGet();

	/* the control event is distributed to all active channels */
	while (ChannelMask) {
		if (ChannelMask & 1) {
			for (unsigned int x = 0; x != MPP_TRACKS_PER_VIEW; x++) {
				if (outputTrackMirror(x))
					continue;
				if (mw->check_play(off + x, chan, 0))
					mid_control(d, ctrl, val);
				if (mw->check_record(off + x, chan, 0))
					mid_control(d, ctrl, val);
			}
		}
		ChannelMask /= 2;
		chan++;
		chan &= 0xF;
	}

	if (ctrl == 0x40) {
		mw->tab_loop->add_pedal(val);
		lastPedalValue = val;
	}
}

/* must be called locked */
void
MppScoreMain :: outputChanPressure(uint8_t pressure)
{
	MppMainWindow *mw = mainWindow;
	struct mid_data *d = &mw->mid_data;
	const unsigned int off = unit * MPP_TRACKS_PER_VIEW;
	uint16_t ChannelMask;
	uint8_t chan;
	uint8_t buf[4];

	chan = synthChannel;
	ChannelMask = outputChannelMaskGet();

	buf[0] = 0xD0;
	buf[1] = pressure & 0x7F;
	buf[2] = 0;
	buf[3] = 0;

	/* the pressure event is distributed to all active channels */
	while (ChannelMask) {
		if (ChannelMask & 1) {
			for (unsigned int x = 0; x != MPP_TRACKS_PER_VIEW; x++) {
				if (outputTrackMirror(x))
					continue;
				if (mw->check_play(off + x, chan, 0))
					mid_add_raw(d, buf, 2, 0);
				if (mw->check_record(off + x, chan, 0))
					mid_add_raw(d, buf, 2, 0);
			}
		}
		ChannelMask /= 2;
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
	const unsigned int off = unit * MPP_TRACKS_PER_VIEW;
	uint16_t ChannelMask;
	uint8_t chan;

	chan = synthChannel;
	ChannelMask = outputChannelMaskGet();

	/* the pitch event is distributed to all active channels */
	while (ChannelMask) {
		if (ChannelMask & 1) {
			/* the control event is distributed to all active devices */
			for (unsigned int x = 0; x != MPP_TRACKS_PER_VIEW; x++) {
				if (outputTrackMirror(x))
					continue;
				if (mw->check_play(off + x, chan, 0))
					mid_pitch_bend(d, val);
				if (mw->check_record(off + x, chan, 0))
					mid_pitch_bend(d, val);
			}
		}
		ChannelMask /= 2;
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

	mainWindow->atomic_lock();
	if (head.state.curr_start != 0)
		seq = head.state.curr_start->sequence;
	else
		seq = 0;
	mainWindow->atomic_unlock();

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
MppScoreMain :: handleMidiKeyPressLocked(int key, int vel)
{
	uint8_t chan = synthChannel;

	switch (keyMode) {
	case MM_PASS_NONE_FIXED:
		handleKeyPress(baseKey, vel, 0);
		break;
	case MM_PASS_NONE_TRANS:
		handleKeyPress(key, vel, 0);
		break;
	case MM_PASS_NONE_CHORD_ALL:
	case MM_PASS_NONE_CHORD_PIANO:
	case MM_PASS_NONE_CHORD_GUITAR:
		handleKeyPressChord(key, vel, 0);
		break;
	case MM_PASS_ALL:
		if (setPressedKey(chan, key, 255, 0) == 0)
			mainWindow->output_key(MPP_DEFAULT_TRACK(unit), chan, key, vel, 0, 0);
		break;
	default:
		break;
	}
}

void
MppScoreMain :: handleMidiKeyReleaseLocked(int key, int vel)
{
	uint8_t chan = synthChannel;

	switch (keyMode) {
	case MM_PASS_NONE_FIXED:
		handleKeyRelease(baseKey, vel, 0);
		break;
	case MM_PASS_NONE_TRANS:
		handleKeyRelease(key, vel, 0);
		break;
	case MM_PASS_NONE_CHORD_ALL:
	case MM_PASS_NONE_CHORD_PIANO:
	case MM_PASS_NONE_CHORD_GUITAR:
		handleKeyReleaseChord(key, vel, 0);
		break;
	case MM_PASS_ALL:
		if (setPressedKey(chan, key, 0, 0) == 0)
			mainWindow->output_key(MPP_DEFAULT_TRACK(unit), chan, key, -vel, 0, 0);
		break;
	default:
		break;
	}
}
