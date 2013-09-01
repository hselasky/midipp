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
#include "midipp_button.h"
#include "midipp_buttonmap.h"
#include "midipp_gridlayout.h"
#include "midipp_scores.h"
#include "midipp_import.h"
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
	QRectF txtBound;
	int w = width();
	int h = height();
	int bg_w = parent->background.width();
	int bg_h = parent->background.height();
	qreal ratio_w;
	qreal ratio_h;
	qreal ratio;
	qreal factor;
	qreal ps;
	int p;

	paint.setRenderHints(QPainter::Antialiasing, 1);

	QColor black(0,0,0);
	QColor white(255,255,255);

	paint.fillRect(QRectF(0,0,w,h), black);

	p = parent->transition;

	if (bg_w != 0 && bg_h != 0) {
		ratio_w = (qreal)w / (qreal)bg_w;
		ratio_h = (qreal)h / (qreal)bg_h;
		if (ratio_w > ratio_h)
			ratio = ratio_w;
		else
			ratio = ratio_h;
	} else {
		ratio = 1.0;
	}

	QRect bg_rect((w - bg_w * ratio) / 2.0, (h - bg_h * ratio) / 2.0, bg_w * ratio, bg_h * ratio);

	QFont fnt = parent->mw->defaultFont;

	if (p < MPP_TRAN_MAX) {
		paint.setOpacity((qreal)(MPP_TRAN_MAX - p) / (qreal)MPP_TRAN_MAX);

		switch(parent->last_st) {
		case MPP_SHOW_ST_BLANK:
			break;
		case MPP_SHOW_ST_LOGO:
			paint.drawPixmap(bg_rect,parent->background);
			break;
		case MPP_SHOW_ST_LIVE:
			paint.drawPixmap(bg_rect,parent->background);

			if (parent->labelTxt[parent->last_label].size() == 0)
				break;
			for (ps = 1.0; ps < 128.0; ps *= 2.0) {
				fnt.setPixelSize(ps);
				paint.setFont(fnt);
				txtBound = paint.boundingRect(QRectF(0,0,w,h), Qt::AlignCenter, parent->labelTxt[parent->last_label]);

				if (txtBound.width() >= w || 
				    txtBound.height() >= h)
					break;
			}
			ps /= 2.0;

 			fnt.setPixelSize(ps);
			paint.setFont(fnt);
			txtBound = paint.boundingRect(QRectF(0,0,w,h), Qt::AlignCenter, parent->labelTxt[parent->last_label]);
			txtBound = QRectF(txtBound.x() - (2.0 * ps), txtBound.y() - (2.0 * ps),
			    txtBound.width() + (4.0 * ps), txtBound.height() + (4.0 * ps));
			paint.setPen(QPen(white, 16));
			paint.setBrush(white);
			factor = paint.opacity();
			paint.setOpacity(0.75 * factor);
			paint.drawRoundedRect(txtBound, 16, 16);
			paint.setOpacity(factor);
			paint.setPen(black);
			paint.drawText(QRectF(0,0,w,h), Qt::AlignCenter, parent->labelTxt[parent->last_label]);
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
		paint.drawPixmap(bg_rect,parent->background);
		break;
	case MPP_SHOW_ST_LIVE:
		paint.drawPixmap(bg_rect,parent->background);

		if (parent->labelTxt[parent->curr_label].size() == 0)
			break;

 		for (ps = 1.0; ps < 128.0; ps *= 2.0) {
 			fnt.setPixelSize(ps);
			paint.setFont(fnt);
			txtBound = paint.boundingRect(QRectF(0,0,w,h), Qt::AlignCenter, parent->labelTxt[parent->curr_label]);

			if (txtBound.width() >= w || 
			    txtBound.height() >= h)
				break;
		}
		ps /= 2.0;

		fnt.setPixelSize(ps);
		paint.setFont(fnt);
		txtBound = paint.boundingRect(QRectF(0,0,w,h), Qt::AlignCenter, parent->labelTxt[parent->curr_label]);
		txtBound = QRectF(txtBound.x() - (2.0 * ps), txtBound.y() - (2.0 * ps),
		    txtBound.width() + (4.0 * ps), txtBound.height() + (4.0 * ps));
		paint.setPen(QPen(white, 16));
		paint.setBrush(white);
		factor = paint.opacity();
		paint.setOpacity(0.75 * factor);
		paint.drawRoundedRect(txtBound, 16, 16);
		paint.setOpacity(factor);
		paint.setPen(black);
		paint.drawText(QRectF(0,0,w,h), Qt::AlignCenter, parent->labelTxt[parent->curr_label]);
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

	butLabel = new MppButtonMap("Current label\0" "L0\0" "L1\0" "L2\0" "L3\0"
	    "L4\0" "L5\0" "L6\0" "L7\0"
	    "L8\0" "L9\0" "L10\0" "L11\0"
	    "L12\0" "L13\0" "L14\0" "L15\0", 16, 4);
	connect(butLabel, SIGNAL(selectionChanged(int)), this, SLOT(handle_label_change(int)));

	butLoadLyrics = new QPushButton(tr("Load-Lyrics"));
	connect(butLoadLyrics, SIGNAL(released()), this, SLOT(handle_load_lyrics()));

	butShow = new QPushButton(tr("Show"));
	connect(butShow, SIGNAL(released()), this, SLOT(handle_show()));

	butFullScreen = new QPushButton(tr("FullScreen"));
	connect(butFullScreen, SIGNAL(released()), this, SLOT(handle_fullscreen()));

	butBackground = new QPushButton(tr("Background"));
	connect(butBackground, SIGNAL(released()), this, SLOT(handle_background()));

	gl_main = new MppGridLayout();
	gl_main->addWidget(butShow, 0, 0, 1, 1);
	gl_main->addWidget(butFullScreen, 0, 1, 1, 1);
	gl_main->addWidget(butMode, 0, 2, 1, 1);
	gl_main->addWidget(butLabel, 1, 0, 1, 3);
	gl_main->addWidget(butLoadLyrics, 2, 0, 1, 1);
	gl_main->addWidget(butBackground, 2, 1, 1, 1);

	watchdog = new QTimer(this);
	connect(watchdog, SIGNAL(timeout()), this, SLOT(handle_watchdog()));

	wg_show = new MppShowWidget(this);

	handle_load(QString());

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

	if (curr_st == MPP_SHOW_ST_LIVE)
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
	int x;
	int y;
	int last_nl = 0;

	for (x = 0; x != MPP_SHOW_MAX; x++)
		labelTxt[x] = QString();

	for (x = y = 0; x != str.size(); x++) {
		QChar ch = str[x];

		if (ch == '\r')
			continue;
		if (ch == '\n') {
			if (last_nl == 0) {
				if (y < MPP_SHOW_MAX)
					labelTxt[y] += ch;
			} else if (last_nl == 1) {
				if (y < MPP_SHOW_MAX)
					y++;
			}
			last_nl++;
			continue;
		}
		if (y < MPP_SHOW_MAX)
			labelTxt[y] += ch;
		last_nl = 0;
	}

	for (x = 0; x != MPP_SHOW_MAX; x++) {
		if (labelTxt[x].size() == 0) {
			butLabel->but[x]->setEnabled(0);
			butLabel->but[x]->setText(QString("L%1").arg(x));
		} else {
			QString tmpdesc;
			for (y = 0; y != labelTxt[x].size(); y++) {
				QChar ch = labelTxt[x][y];
				if (ch == '\r')
					continue;
				if (ch == '\n')
					ch = ' ';
				if (y >= 20)
					break;
				tmpdesc += ch;
			}

			butLabel->but[x]->setEnabled(1);
			butLabel->but[x]->setText(QString("L%1 ").arg(x) + tmpdesc);
		}
	}
}

void
MppShowControl :: handle_load_lyrics()
{
	QFileDialog *diag = 
	  new QFileDialog(mw, tr("Select Lyrics File"), 
		MppHomeDirTxt,
		QString("Lyrics File (*.txt; *.TXT)"));

	diag->setAcceptMode(QFileDialog::AcceptOpen);
	diag->setFileMode(QFileDialog::ExistingFile);

	if (curr_st == MPP_SHOW_ST_LIVE)
		butMode->setSelection(MPP_SHOW_ST_LOGO);

	if (curr_label != 0)
		butLabel->setSelection(0);

	if (diag->exec()) {

		MppHomeDirTxt = diag->directory().path();

		handle_load(MppReadFile(diag->selectedFiles()[0]));
	}
	delete diag;
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
	} else if (transition == MPP_TRAN_MAX) {
		transition++;
		last_label = curr_label;
		last_st = curr_st;
	}
}

void
MppShowControl :: handle_background()
{
        QFileDialog *diag = 
          new QFileDialog(mw, tr("Select Background File"), 
                MppHomeDirBackground,
                QString("Image Files (*.BMP *.bmp *.GIF *.gif *.JPG *.jpg *.JPEG "
		    "*.jpeg *.PNG *.png *.PBM *.pbm *.PGM *.pgm *.PPM *.ppm "
		    "*.XBM *.xbm *.XPM *.xpm)"));

	if (diag->exec()) {
                MppHomeDirBackground = diag->directory().path();
                background.load(diag->selectedFiles()[0]);
		transition = 0;
	}
}
