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

#ifndef _MIDIPP_SHOW_H_
#define	_MIDIPP_SHOW_H_

#include "midipp.h"

#define	MPP_TRAN_MAX 4

enum {
	MPP_SHOW_ST_BLANK,
	MPP_SHOW_ST_BACKGROUND,
	MPP_SHOW_ST_LYRICS,
	MPP_SHOW_ST_MAX,
};

class MppShowWidget : public QWidget
{
public:
	MppShowWidget(MppShowControl *);
	~MppShowWidget();

	MppShowControl *parent;

	void paintText(QPainter &, QString &, int, int);
	void paintEvent(QPaintEvent *);
	void keyPressEvent(QKeyEvent *);
	void mouseDoubleClickEvent(QMouseEvent *e);
};

class MppShowControl : public QObject
{
	Q_OBJECT;

public:
	MppShowControl(MppMainWindow *);
	~MppShowControl();

	MppMainWindow *mw;
	MppGridLayout *gl_main;

	void handle_label_change(int);

	QString labelTxt[MPP_MAX_VIEWS][MPP_MAX_LABELS];

	QPixmap background;

	QColor fontFgColor;
	QColor fontBgColor;

	int last_label;
	int last_st;
	int curr_label;
	int curr_st;
	int transition;
	int trackview;

	MppShowWidget *wg_show;

	MppButtonMap *butMode;
	MppButtonMap *butTrack;
	QPushButton *butShow;
	QPushButton *butFullScreen;
	QPushButton *butBackground;
	QPushButton *butFontFgColor;
	QPushButton *butFontBgColor;
	QTimer *watchdog;

public slots:
	void handle_mode_change(int);
	void handle_track_change(int);
	void handle_show();
	void handle_fullscreen();
	void handle_watchdog();
	void handle_background();
	void handle_change_font_fg_color();
	void handle_change_font_bg_color();
};

#endif		/* _MIDIPP_SHOW_H_ */
