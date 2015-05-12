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

	setWindowTitle(QString("MidiPlayerPro"));
	setWindowIcon(QIcon(QString(MPP_ICON_FILE)));
}

MppShowWidget :: ~MppShowWidget()
{
}

void
MppShowWidget :: paintText(QPainter &paint, int w, int h)
{
}

void
MppShowWidget :: paintEvent(QPaintEvent *event)
{
	QPainter paint(this);
	int w = width();
	int h = height();

	paint.setRenderHints(QPainter::Antialiasing, 1);

	QColor black(0,0,0);

	paint.fillRect(QRectF(0,0,w,h), black);

	if (parent->aobj[2].opacity_curr >= (1.0 / 256.0)) {
		MppShowAnimObject &aobj = parent->aobj[2];
		int bg_w = parent->background.width();
		int bg_h = parent->background.height();
		qreal ratio_w;
		qreal ratio_h;
		qreal ratio;

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

		QRect bg_rect((w - bg_w * ratio) / 2.0,
		    (h - bg_h * ratio) / 2.0, bg_w * ratio, bg_h * ratio);

		paint.setOpacity(aobj.opacity_curr);
		paint.drawPixmap(bg_rect, parent->background);
	}

	if (parent->aobj[0].opacity_curr >= (1.0 / 256.0) ||
	    parent->aobj[1].opacity_curr >= (1.0 / 256.0)) {
	  	qreal wm = w / 16.0;
		qreal hm = h / 16.0;
		int x;

		paint.setFont(parent->showFont);

		for (x = 0; x != 2; x++) {
			MppShowAnimObject &aobj = parent->aobj[x];

			if (aobj.text.isEmpty())
				continue;

			QRectF txtBound = paint.boundingRect(
			    QRectF(wm, hm, w - 2.0*wm, h - 2.0*hm),
			    Qt::AlignHCenter | Qt::AlignTop | Qt::TextWordWrap,
			    aobj.text + QChar('\n'));

			paint.setOpacity(aobj.opacity_curr);
			paint.setPen(Qt::NoPen);
			paint.setBrush(parent->fontBgColor);
			paint.drawRoundedRect(txtBound.adjusted(
			    -wm/2.0 + aobj.xpos_curr,
			    -hm/2.0 + aobj.ypos_curr,
			    wm/2.0 + aobj.xpos_curr,
			    hm/2.0 + aobj.ypos_curr), 16, 16);
			paint.setPen(parent->fontFgColor);
			paint.drawText(txtBound.adjusted(
			    aobj.xpos_curr, aobj.ypos_curr,
			    aobj.xpos_curr, aobj.ypos_curr),
			    Qt::AlignCenter | Qt::TextWordWrap,
			    aobj.text);

			aobj.height = txtBound.height() + hm;
			aobj.width = txtBound.width();
		}
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

void
MppShowWidget :: mouseDoubleClickEvent(QMouseEvent *e)
{
	/* toggle fullscreen */
	setWindowState(windowState() ^ Qt::WindowFullScreen);
}

MppShowControl :: MppShowControl(MppMainWindow *_mw)
{
	mw = _mw;

	anim_text_state = 0;
	anim_pict_state = 0;

	cached_last_num = -1;
	cached_last_index = -1;
	cached_curr_index = -1;

	current_mode = MPP_SHOW_ST_BLANK;

	trackview = 0;

	fontFgColor = QColor(0,0,0);
	fontBgColor = QColor(255,255,255);

	showFont.fromString(QString("Sans Serif,-1,24,5,75,0,0,0,0,0"));

	butShowFont = new QPushButton(tr("Select Font"));
	connect(butShowFont, SIGNAL(released()), this, SLOT(handle_show_fontsel()));

	butTrack = new MppButtonMap("Track\0" "View-A\0" "View-B\0" "View-C\0",
#if MPP_MAX_VIEWS <= 3
				    MPP_MAX_VIEWS, MPP_MAX_VIEWS
#else
				    3, 3
#endif
				    );
	connect(butTrack, SIGNAL(selectionChanged(int)), this, SLOT(handle_track_change(int)));

	butMode = new MppButtonMap("Current mode\0" "BLANK\0" "BACKGROUND\0" "LYRICS\0", 3, 3);
	connect(butMode, SIGNAL(selectionChanged(int)), this, SLOT(handle_mode_change(int)));

	butShowWindow = new QPushButton(tr("Show\nWindow"));
	connect(butShowWindow, SIGNAL(released()), this, SLOT(handle_show_window()));

	butFullScreen = new QPushButton(tr("Toggle\nFullscreen"));
	connect(butFullScreen, SIGNAL(released()), this, SLOT(handle_fullscreen()));

	butBackground = new QPushButton(tr("Set Background"));
	connect(butBackground, SIGNAL(released()), this, SLOT(handle_change_background()));

	butFontFgColor = new QPushButton(tr("Set Font Fg Color"));
	connect(butFontFgColor, SIGNAL(released()), this, SLOT(handle_change_font_fg_color()));

	butFontBgColor = new QPushButton(tr("Set Font Bg Color"));
	connect(butFontBgColor, SIGNAL(released()), this, SLOT(handle_change_font_bg_color()));

	gl_main = new MppGridLayout();

	gl_main->addWidget(butTrack, 0, 0, 1, 2);
	gl_main->addWidget(butMode, 0, 2, 1, 2);
	gl_main->addWidget(butShowFont, 3, 0, 1, 1);
	gl_main->addWidget(butBackground, 4, 0, 1, 1);
	gl_main->addWidget(butFontFgColor, 3, 1, 1, 1);
	gl_main->addWidget(butFontBgColor, 4, 1, 1, 1);
	gl_main->addWidget(butShowWindow, 3, 2, 2, 1);
	gl_main->addWidget(butFullScreen, 3, 3, 2, 1);
	gl_main->setRowStretch(2, 1);
	gl_main->setColumnStretch(4, 1);

	watchdog = new QTimer(this);
	connect(watchdog, SIGNAL(timeout()), this, SLOT(handle_watchdog()));

	wg_show = new MppShowWidget(this);

	watchdog->start(1000 / (3 * MPP_TRAN_MAX));
}

MppShowControl :: ~MppShowControl()
{
	watchdog->stop();
}

void
MppShowControl :: handle_mode_change(int mode)
{
	/* check if text is changed */
	if ((mode >= MPP_SHOW_ST_LYRICS) !=
	    (current_mode >= MPP_SHOW_ST_LYRICS)) {
		handle_text_change();
	}

	/* check if picture is changed */
	if ((mode >= MPP_SHOW_ST_BACKGROUND) !=
	    (current_mode >= MPP_SHOW_ST_BACKGROUND)) {
		handle_pict_change();
	}

	/* set current mode */
	current_mode = mode;
}

void
MppShowControl :: handle_show_window()
{
	wg_show->setWindowState(wg_show->windowState() & ~Qt::WindowFullScreen);
	wg_show->show();
}

void
MppShowControl :: handle_fullscreen()
{
	wg_show->setWindowState(wg_show->windowState() | Qt::WindowFullScreen);
}

void
MppShowControl :: handle_text_watchdog()
{
	MppScoreMain &sm = *mw->scores_main[trackview];
	MppElement *last;
	MppElement *curr;
	MppVisualDot *pcdot_curr = 0;
	MppVisualDot *pcdot_last = 0;
	int visual_curr_index = 0;
	int visual_last_index = 0;

	pthread_mutex_lock(&mw->mtx);
	last = sm.head.state.last_start;
	curr = sm.head.state.curr_start;
	pthread_mutex_unlock(&mw->mtx);

	/* locate last and current play position */
	sm.locateVisual(last, &visual_last_index, &pcdot_last);
	sm.locateVisual(curr, &visual_curr_index, &pcdot_curr);

	/* check validity of indexes */
	if (visual_last_index < 0 ||
	    visual_last_index >= sm.visual_max ||
	    visual_curr_index < 0 ||
	    visual_curr_index >= sm.visual_max)
		return;

	/* check need for new transition */
	if (anim_text_state != 0) {
		if (visual_last_index == cached_last_index &&
		    visual_curr_index == cached_curr_index &&
		    anim_text_state != 3)
			return;
		/* wait for transitions complete */
		if (aobj[0].currStep < MPP_TRAN_MAX ||
		    aobj[1].currStep < MPP_TRAN_MAX)
			return;
	}

	/* check if lyrics should not be shown */
	if (current_mode < MPP_SHOW_ST_LYRICS) {
		if (anim_text_state != 0) {
			aobj[0].fadeOut();
			aobj[1].fadeOut();
			anim_text_state = 0;
		}
		return;
	}

	/* get next state based on current state */
	switch (anim_text_state) {
	case 0:
		/* dim in first */
		anim_text_state = 1;

		/* reset state */
		aobj[0].reset();

		/* copy string */
		if (sm.pVisual[visual_last_index].str != 0)
			aobj[0].text = *sm.pVisual[visual_last_index].str;
		else
			aobj[0].text = QString();

		aobj[0].fadeIn();

		/* reset state */
		aobj[1].reset();
		aobj[1].text = QString();
		break;
	case 3:
	case 1:
		if (visual_curr_index != visual_last_index) {
			/* dim in second */
			anim_text_state = 2;

			/* reset state */
			aobj[1].reset();

			/* copy string */
			if (sm.pVisual[visual_curr_index].str != 0)
				aobj[1].text = *sm.pVisual[visual_curr_index].str;
			else
				aobj[1].text = QString();

			aobj[1].fadeIn();
			aobj[1].ypos_curr = aobj[0].height;
		} else {
			/* dim in second and dim out first */
			anim_text_state = 1;

			/* move text around */
			aobj[1] = aobj[0];
			aobj[1].fadeOut();

			/* reset state */
			aobj[0].reset();

			/* copy string */
			if (sm.pVisual[visual_curr_index].str != 0)
				aobj[0].text = *sm.pVisual[visual_curr_index].str;
			else
				aobj[0].text = QString();
			aobj[0].fadeIn();
		}
		break;
	default:
		if (visual_curr_index != visual_last_index ||
		    visual_last_index != cached_curr_index)
			anim_text_state = 3;
		else
			anim_text_state = 1;

		/* swap animation objects */
		MppShowAnimObject tmp = aobj[1];
		aobj[1] = aobj[0];
		aobj[0] = tmp;

		/* dim out first and move second up */
		aobj[0].moveUp(aobj[0].ypos_curr);
		aobj[1].moveUp(aobj[0].ypos_curr);
		aobj[1].fadeOut();
		break;
	}
	cached_last_index = visual_last_index;
	cached_curr_index = visual_curr_index;
}

void
MppShowControl :: handle_pict_watchdog()
{
	MppScoreMain &sm = *mw->scores_main[trackview];
	int last_num;

	pthread_mutex_lock(&mw->mtx);
	last_num = sm.head.state.image_num;
	pthread_mutex_unlock(&mw->mtx);

	/* check need for new transition */
	if (anim_pict_state != 0) {
		if (last_num == cached_last_num &&
		    anim_pict_state != 2)
			return;
		/* wait for transitions complete */
		if (aobj[2].currStep < MPP_TRAN_MAX)
			return;
	}

	/* check if background should not be shown */
	if (current_mode < MPP_SHOW_ST_BACKGROUND) {
		if (anim_pict_state != 0) {
			aobj[2].fadeOut();
			anim_pict_state = 0;
		}
		return;
	}

	/* get next state based on current state */
	switch (anim_pict_state) {
	case 1:
		anim_pict_state = 2;
		aobj[2].fadeOut();
		break;
	default:
		/* refresh pixmap */
		if (last_num != cached_last_num) {
			/* reset pixmap */
			background = QPixmap();
			/* check for valid number */
			if (last_num >= 0 && last_num < files.size())
				background.load(files[last_num]);
			cached_last_num = last_num;
		}

		/* fade in first */
		anim_pict_state = 1;

		/* reset state */
		aobj[2].reset();
		aobj[2].fadeIn();
		break;
	}
}

void
MppShowControl :: handle_watchdog()
{
	uint8_t x;

	if (trackview < 0 || trackview >= MPP_MAX_VIEWS)
		return;

	handle_text_watchdog();
	handle_pict_watchdog();

	for (x = 0; x != MPP_SHOW_AOBJ_MAX; x++) {
		if (aobj[x].step())
			wg_show->update();
	}
}

void
MppShowControl :: handle_change_background()
{
	QFileDialog *diag = 
	  new QFileDialog(mw, tr("Select Background File(s)"), 
		Mpp.HomeDirBackground,
		QString("Image Files (*.BMP *.bmp *.GIF *.gif *.JPG *.jpg *.JPEG "
		    "*.jpeg *.PNG *.png *.PBM *.pbm *.PGM *.pgm *.PPM *.ppm "
		    "*.XBM *.xbm *.XPM *.xpm)"));

	diag->setFileMode(QFileDialog::ExistingFiles);

	if (diag->exec() == QFileDialog::Accepted) {
		Mpp.HomeDirBackground = diag->directory().path();
		files = diag->selectedFiles();
		handle_pict_change();
	}
	delete diag;
}

void
MppShowControl :: handle_track_change(int n)
{
	trackview = n;
	handle_text_change();
	handle_pict_change();
}

void
MppShowControl :: handle_change_font_fg_color()
{
	QColorDialog dlg(fontFgColor);

	if (dlg.exec() == QDialog::Accepted) {
		fontFgColor = dlg.currentColor();
		handle_text_change();
	}
}

void
MppShowControl :: handle_change_font_bg_color()
{
	QColorDialog dlg(fontBgColor);

	if (dlg.exec() == QDialog::Accepted) {
		fontBgColor = dlg.currentColor();
		handle_text_change();
	}
}

void
MppShowControl :: handle_show_fontsel()
{
	bool success;

	QFont font = QFontDialog::getFont(&success, showFont, mw);

	if (success) {
		font.setPixelSize(QFontInfo(font).pixelSize());
		showFont = font;
		handle_text_change();
	}
}

void
MppShowControl :: handle_text_change()
{
	cached_last_index = -1;
	cached_curr_index = -1;
}

void
MppShowControl :: handle_pict_change()
{
	cached_last_num = -1;
}
