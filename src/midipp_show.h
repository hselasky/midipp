/*-
 * Copyright (c) 2013-2022 Hans Petter Selasky.
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

#include <QUdpSocket>
#include <QHostAddress>

#include "midipp.h"
#include "midipp_element.h"

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
	MppShowWidget(MppShowControl *, bool);

	MppShowControl *parent;
	bool showChords;

	void paintEvent(QPaintEvent *);
	void keyPressEvent(QKeyEvent *);
	void mouseDoubleClickEvent(QMouseEvent *e);
};

class MppShowAnimObject {
public:
	QString str;
	QString str_chord;
	uint8_t zero_start[0];
	qreal opacity_curr;
	qreal opacity_step;
	qreal xpos_curr;
	qreal xpos_step;
	qreal ypos_curr;
	qreal ypos_step;
	qreal width;
	qreal height;
	MppObjectProps props;
	uint8_t currStep;
	uint8_t zero_end[0];

	void reset()
	{
		memset(zero_start, 0, zero_end - zero_start);
		str = QString();
		str_chord = QString();
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
			currStep++;
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
	bool isVisible()
	{
		return (opacity_curr >= (1.0 / 1024.0));
	}
	bool isAnimating()
	{
		return (currStep <= MPP_TRAN_MAX);
	}
};

class MppShowControl : public QObject
{
	Q_OBJECT

public:
	MppShowControl(MppMainWindow *);
	~MppShowControl();

	void handle_text_watchdog();
	void handle_pict_watchdog();
	void handle_text_change();
	void handle_pict_change();
	void hpsjam_send_text(const QString &, const QString &);

	MppMainWindow *mw;
	MppGridLayout *gl_main;
	MppGroupBox *gb_font;
	MppGroupBox *gb_image;
	MppGroupBox *gb_hpsjam;

	enum {
		MPP_SHOW_AOBJ_TEXT_0,
		MPP_SHOW_AOBJ_TEXT_1,
		MPP_SHOW_AOBJ_PICTURE,
		MPP_SHOW_AOBJ_MAX,
	};

	MppShowAnimObject aobj[MPP_SHOW_AOBJ_MAX];

	QPixmap background;

	QStringList files;

	QFont showFont;

	/* for the whole window */
	int8_t current_mode;
	int8_t trackview;
	int8_t toggle;

	MppShowWidget *wg_lyrics_show;
	MppShowWidget *wg_chords_show;

	MppButtonMap *butMode;
	MppButtonMap *butTrack;

	QPushButton *butShowLyricsWindow;
	QPushButton *butShowChordsWindow;
	QPushButton *butFullScreen;

	QPushButton *butFontSelect;
	QPushButton *butFontFgColor;
	QPushButton *butFontBgColor;
	QPushButton *butFontCenter;
	QPushButton *butFontRight;
	QPushButton *butFontLeft;
	QPushButton *butFontMoreSpace;
	QPushButton *butFontLessSpace;
	QPushButton *butImageSelect;
	QPushButton *butImageFgColor;
	QPushButton *butImageBgColor;
	QPushButton *butImageZoom;
	QPushButton *butImageFit;
	QPushButton *butImageFirst;
	QPushButton *butImageNext;
	QPushButton *butImagePrev;
	QPushButton *butImageCenter;
	QPushButton *butImageRight;
	QPushButton *butImageLeft;

	QPushButton *butCopySettings;

	MppButtonMap *butHpsJamOnOff;
	QLineEdit *editHpsJamServer;
	QUdpSocket sockHpsJam;
	QHostAddress addrHpsJam;
	uint16_t portHpsJam;

	QTimer *watchdog;

public slots:
	void handle_watchdog();
	void handle_mode_change(int);
	void handle_track_change(int);
	void handle_hpsjam_change(int);
	void handle_show_lyrics_window();
	void handle_show_chords_window();
	void handle_fullscreen();
	void handle_fontselect();
	void handle_fontfgcolor();
	void handle_fontbgcolor();
	void handle_fontcenter();
	void handle_fontright();
	void handle_fontleft();
	void handle_fontmorespace();
	void handle_fontlessspace();
	void handle_imageselect();
	void handle_imagebgcolor();
	void handle_imagezoom();
	void handle_imagefit();
	void handle_imagefirst();
	void handle_imagenext();
	void handle_imageprev();
	void handle_imagecenter();
	void handle_imageright();
	void handle_imageleft();
	void handle_copysettings();
};

#endif		/* _MIDIPP_SHOW_H_ */
