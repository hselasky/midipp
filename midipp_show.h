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

#define	MPP_TRAN_MAX 5

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

	void paintEvent(QPaintEvent *);
	void keyPressEvent(QKeyEvent *);
	void mouseDoubleClickEvent(QMouseEvent *e);
};

class MppShowAnimObject {
public:
	QString text;
	uint8_t zero_start[0];
	qreal opacity_curr;
	qreal opacity_step;
	qreal xpos_curr;
	qreal xpos_step;
	qreal ypos_curr;
	qreal ypos_step;
	qreal width;
	qreal height;
	uint8_t currStep;
	uint8_t zero_end[0];

	void reset()
	{
		memset(zero_start, 0, zero_end - zero_start);
	}
	MppShowAnimObject()
	{
		reset();
		currStep = MPP_TRAN_MAX;
	}
	int step()
	{
		if (currStep >= MPP_TRAN_MAX) {
			/* animation finished */
			opacity_step = 0;
			xpos_step = 0;
			ypos_step = 0;
			return (0);
		}
		/* animation in progress */
		opacity_curr += opacity_step;
		xpos_curr += xpos_step;
		ypos_curr += ypos_step;
		currStep++;
		return (1);
	};
	void fadeOut()
	{
		opacity_step = -(opacity_curr / MPP_TRAN_MAX);
		currStep = 0;
	}
	void fadeIn()
	{
		opacity_step = ((1.0 - opacity_curr) / MPP_TRAN_MAX);
		currStep = 0;
	}
	void moveUp(qreal pix)
	{
		ypos_step = -(pix / MPP_TRAN_MAX);
		currStep = 0;
	}
};

class MppShowControl : public QObject
{
	Q_OBJECT;

public:
	MppShowControl(MppMainWindow *);
	~MppShowControl();

	void handle_text_watchdog();
	void handle_pict_watchdog();
	void handle_text_change();
	void handle_pict_change();

	MppMainWindow *mw;
	MppGridLayout *gl_main;

	enum {
		MPP_SHOW_AOBJ_TEXT_0,
		MPP_SHOW_AOBJ_TEXT_1,
		MPP_SHOW_AOBJ_PICTURE,
		MPP_SHOW_AOBJ_MAX,
	};

	MppShowAnimObject aobj[MPP_SHOW_AOBJ_MAX];
	
	QPixmap background;

	QStringList files;

	QColor fontFgColor;
	QColor fontBgColor;

	QFont showFont;

	/* for the whole window */
	int8_t current_mode;
	int8_t anim_text_state;
	int8_t anim_pict_state;
	int8_t trackview;

	int cached_last_num;
	int cached_last_index;
	int cached_curr_index;

	MppShowWidget *wg_show;
	MppButtonMap *butMode;
	MppButtonMap *butTrack;
	QPushButton *butShowWindow;
	QPushButton *butShowFont;
	QPushButton *butFullScreen;
	QPushButton *butBackground;
	QPushButton *butFontFgColor;
	QPushButton *butFontBgColor;
	QTimer *watchdog;

public slots:
	void handle_watchdog();
	void handle_mode_change(int);
	void handle_track_change(int);
	void handle_show_window();
	void handle_fullscreen();
	void handle_show_fontsel();
	void handle_change_background();
	void handle_change_font_fg_color();
	void handle_change_font_bg_color();
};

#endif		/* _MIDIPP_SHOW_H_ */
