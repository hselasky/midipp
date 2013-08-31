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

#include "midipp_show.h"
#include "midipp_buttonmap.h"
#include "midipp_gridlayout.h"
#include "midipp_scores.h"
#include "midipp_mainwindow.h"

MppShowWidget :: MppShowWidget(MppShowControl *_parent)
{
	parent = _parent;
}

MppShowWidget :: ~MppShowWidget()
{
}

void
MppShowWidget :: paintEvent(QPaintEvent *event)
{
	QPainter paint(this);
	int w = width();
	int h = height();
	int p;

	paint.setRenderHints(QPainter::Antialiasing, 1);

	QColor black(0,0,0);
	QColor white(255,255,255);

	paint.fillRect(QRectF(0,0,w,h), black);

	p = parent->transition;

	if (p < MPP_TRAN_MAX) {
		paint.setOpacity((qreal)(MPP_TRAN_MAX - p) / (qreal)MPP_TRAN_MAX);
		switch(parent->last_st) {
		case MPP_SHOW_ST_BLANK:
			break;
		case MPP_SHOW_ST_LOGO:
			paint.drawPixmap(0,0,parent->background);
			break;
		case MPP_SHOW_ST_LIVE:
			paint.drawPixmap(0,0,parent->labelPix[parent->last_label]);
			break;
		default:
			break;
		}
		paint.setOpacity((qreal)p / (qreal)MPP_TRAN_MAX);
	}

	switch(parent->curr_st) {
	case MPP_SHOW_ST_BLANK:
		break;
	case MPP_SHOW_ST_LOGO:
		paint.drawPixmap(0,0,parent->background);
		break;
	case MPP_SHOW_ST_LIVE:
		paint.drawPixmap(0,0,parent->labelPix[parent->curr_label]);
		break;
	default:
		break;
	}
}

void
MppShowWidget :: keyPressEvent(QKeyEvent *key)
{
	if (key->key() == Qt::Key_Escape) {
		Qt::WindowStates state = windowState();
		if (state & Qt::WindowFullScreen)
			setWindowState(state ^ Qt::WindowFullScreen);
	}
}

MppShowControl :: MppShowControl(MppMainWindow *_mw)
{
	mw = _mw;

	last_label = 0;
	last_st = MPP_SHOW_ST_BLANK;

	curr_label = 0;
	curr_st = MPP_SHOW_ST_BLANK;

	transition = 0;

	butMode = new MppButtonMap("Current mode\0" "BLANK\0" "LOGO\0" "LIVE\0", 3, 3);
	connect(butMode, SIGNAL(selectionChanged(int)), this, SLOT(handle_mode_change(int)));

	butLabel = new MppButtonMap("Current label\0" "Title\0" "L1\0" "L2\0" "L3\0"
	    "L4\0" "L5\0" "L6\0" "L7\0"
	    "L8\0" "L9\0" "L10\0" "L11\0"
	    "L12\0" "L13\0" "L14\0" "L15\0", 16, 4);
	connect(butLabel, SIGNAL(selectionChanged(int)), this, SLOT(handle_label_change(int)));

	butLoadA = new QPushButton(tr("Load-A"));
	connect(butLoadA, SIGNAL(released()), this, SLOT(handle_load_a()));

	butLoadB = new QPushButton(tr("Load-B"));
	connect(butLoadB, SIGNAL(released()), this, SLOT(handle_load_b()));

	butShow = new QPushButton(tr("Show"));
	connect(butShow, SIGNAL(released()), this, SLOT(handle_show()));

	butFullScreen = new QPushButton(tr("FullScreen"));
	connect(butFullScreen, SIGNAL(released()), this, SLOT(handle_fullscreen()));

	gl_main = new MppGridLayout();
	gl_main->addWidget(butShow, 0, 0, 1, 1);
	gl_main->addWidget(butFullScreen, 0, 1, 1, 1);
	gl_main->addWidget(butMode, 0, 2, 1, 1);
	gl_main->addWidget(butLabel, 1, 0, 1, 3);
	gl_main->addWidget(butLoadA, 2, 0, 1, 1);
	gl_main->addWidget(butLoadB, 2, 1, 1, 1);

	watchdog = new QTimer(this);
	connect(watchdog, SIGNAL(timeout()), this, SLOT(handle_watchdog()));

	wg_show = new MppShowWidget(this);

	watchdog->start(1000 / (2 * MPP_TRAN_MAX));
}

MppShowControl :: ~MppShowControl()
{
	watchdog->stop();
}

void
MppShowControl :: handle_mode_change(int mode)
{
	last_st = curr_st;
	curr_st = mode;
	transition = 0;
}

void
MppShowControl :: handle_label_change(int lbl)
{
	last_label = curr_label;
	curr_label = lbl;
	transition = 0;
}

void
MppShowControl :: handle_select_bg()
{

}

void
MppShowControl :: handle_show()
{
	wg_show->setWindowState(wg_show->windowState() & ~Qt::WindowFullScreen);
	wg_show->show();
}

void
MppShowControl :: handle_load(QString str)
{

}

void
MppShowControl :: handle_load_a()
{
#if MPP_MAX_VIEWS > 0
	handle_load(mw->scores_main[0]->editWidget->toPlainText());
#endif
}

void
MppShowControl :: handle_load_b()
{
#if MPP_MAX_VIEWS > 1
	handle_load(mw->scores_main[1]->editWidget->toPlainText());
#endif
}

void
MppShowControl :: handle_fullscreen()
{
	wg_show->setWindowState(wg_show->windowState() | Qt::WindowFullScreen);
}

void
MppShowControl :: handle_watchdog()
{
	if (transition < MPP_TRAN_MAX) {
		transition++;
		wg_show->repaint();
	}
}
