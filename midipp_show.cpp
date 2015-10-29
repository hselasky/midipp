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
#include "midipp_groupbox.h"

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
MppShowWidget :: paintEvent(QPaintEvent *event)
{
	QPainter paint(this);
	int w = width();
	int h = height();
	int x;

	paint.setRenderHints(QPainter::Antialiasing, 1);
	paint.fillRect(QRectF(0,0,w,h), parent->aobj[2].props.color.bg());

	if (parent->aobj[2].isVisible()) {
		MppShowAnimObject &aobj = parent->aobj[2];
		int bg_w = parent->background.width();
		int bg_h = parent->background.height();
		int xo;
		qreal ratio;

		if (bg_w != 0 && bg_h != 0) {
			qreal ratio_w = (qreal)w / (qreal)bg_w;
			qreal ratio_h = (qreal)h / (qreal)bg_h;
			if (aobj.props.how == 0) {
				if (ratio_w > ratio_h)
					ratio = ratio_w;
				else
					ratio = ratio_h;
			} else {
				if (ratio_w < ratio_h)
					ratio = ratio_w;
				else
					ratio = ratio_h;
			}
		} else {
			ratio = 1.0;
		}
		switch (aobj.props.align) {
		case 1:
			xo = (w - bg_w * ratio);
			break;
		case 2:
			xo = 0;
			break;
		default:
			xo = (w - bg_w * ratio) / 2.0;
			break;
		}

		QRect bg_rect(xo, (h - bg_h * ratio) / 2.0, bg_w * ratio, bg_h * ratio);

		paint.setOpacity(aobj.opacity_curr);
		paint.drawPixmap(bg_rect, parent->background);
	}
	for (x = 0; x != 2; x++) {
		MppShowAnimObject &aobj = parent->aobj[x];

		if (parent->aobj[x].isVisible() == 0)
			continue;

		paint.setFont(parent->showFont);

		qreal wf = parent->showFont.pixelSize();
		qreal wa = (wf * (aobj.props.shadow % 100)) / 100.0;
		qreal ws = (w * (aobj.props.space % 100)) / 100.0;
		qreal wm = w - ws - wf;
		qreal xo;
		int flags;

		flags = Qt::TextWordWrap | Qt::TextDontClip | Qt::AlignTop;

		QRectF txtBound;
		QRectF txtMax(0,0,wm,h);
		paint.drawText(txtMax, Qt::AlignLeft |
		    Qt::TextDontPrint | flags, aobj.str, &txtBound);
		txtMax.setHeight(txtBound.height() + wf);

		switch (aobj.props.align) {
		case 0:
			flags |= Qt::AlignHCenter;
			xo = (w - wm) / 2.0;
			break;
		case 1:
			if (ws != 0.0) {
				flags |= Qt::AlignHCenter;
				xo = ws + ((w - ws - wm - wf) / 2.0);
			} else {
				flags |= Qt::AlignRight;
				xo = (w - wm - wf);
			}
			break;
		default:
			if (ws != 0.0) {
				flags |= Qt::AlignHCenter;
				xo = ((w - ws - wm + wf) / 2.0);
			} else {
				flags |= Qt::AlignLeft;
				xo = wf;
			}
			break;
		}

		/* offset bounding box */
		txtMax.adjust(
		    xo + aobj.xpos_curr,
		    aobj.ypos_curr + wf / 2.0,
		    xo + aobj.xpos_curr,
		    aobj.ypos_curr + wf / 2.0);

		/* draw background, if any */
		if (aobj.props.shadow < 100) {
			paint.setPen(Qt::NoPen);
			paint.setBrush(aobj.props.color.bg());
			paint.setOpacity(aobj.opacity_curr *
			    (aobj.props.shadow % 100) / 100.0);
			txtMax.adjust(-wf / 2.0, 0, wf / 2.0, -wf / 2.0);
			paint.drawRoundedRect(txtMax, wf, wf);
			txtMax.adjust(wf / 2.0, 0, -wf / 2.0, wf / 2.0);
		} else if (aobj.props.shadow < 200) {
			paint.setPen(aobj.props.color.bg());
			paint.setBrush(aobj.props.color.bg());
			paint.setOpacity(aobj.opacity_curr);
			txtMax.adjust(wa,wa,wa,wa);
			paint.drawText(txtMax, flags, aobj.str);
			txtMax.adjust(-wa,-wa,-wa,-wa);
		}

		/* draw text */
		paint.setPen(aobj.props.color.fg());
		paint.setBrush(aobj.props.color.fg());
		paint.setOpacity(aobj.opacity_curr);
		paint.drawText(txtMax, flags, aobj.str);

		/* store height and width */
		aobj.height = txtMax.height();
		aobj.width = txtMax.width();
	}
}

void
MppShowWidget :: keyPressEvent(QKeyEvent *key)
{
	if (key->key() == Qt::Key_Escape) {
		if (windowState() & Qt::WindowFullScreen)
			parent->handle_fullscreen();
	}
}

void
MppShowWidget :: mouseDoubleClickEvent(QMouseEvent *e)
{
	parent->handle_fullscreen();
}

MppShowControl :: MppShowControl(MppMainWindow *_mw)
{
	mw = _mw;

	current_mode = MPP_SHOW_ST_BLANK;
	trackview = 0;
	toggle = 0;

	showFont.fromString(QString("Sans Serif,-1,24,5,75,0,0,0,0,0"));

	butMode = new MppButtonMap("Current mode\0" "BLANK\0" "BACKGROUND\0" "LYRICS\0", 3, 3);
	connect(butMode, SIGNAL(selectionChanged(int)), this, SLOT(handle_mode_change(int)));
	
#if MPP_MAX_VIEWS <= 3
	butTrack = new MppButtonMap("Track\0" "View-A\0" "View-B\0" "View-C\0",
				    MPP_MAX_VIEWS, MPP_MAX_VIEWS);
#else
	butTrack = new MppButtonMap("Track\0" "View-A\0" "View-B\0" "View-C\0", 3, 3);
#endif
	connect(butTrack, SIGNAL(selectionChanged(int)), this, SLOT(handle_track_change(int)));

	butShowWindow = new QPushButton(tr("Show\nWindow"));
	connect(butShowWindow, SIGNAL(released()), this, SLOT(handle_show_window()));

	butFullScreen = new QPushButton(tr("Toggle\nFullscreen"));
	connect(butFullScreen, SIGNAL(released()), this, SLOT(handle_fullscreen()));

	butFontSelect = new QPushButton(tr("Change"));
	connect(butFontSelect, SIGNAL(released()), this, SLOT(handle_fontselect()));

	butFontFgColor = new QPushButton(tr("Select color"));
	connect(butFontFgColor, SIGNAL(released()), this, SLOT(handle_fontfgcolor()));

	butFontBgColor = new QPushButton(tr("Select shadow"));
	connect(butFontBgColor, SIGNAL(released()), this, SLOT(handle_fontbgcolor()));

	butFontCenter = new QPushButton(tr("Center adjust"));
	connect(butFontCenter, SIGNAL(released()), this, SLOT(handle_fontcenter()));

	butFontRight = new QPushButton(tr("Right adjust"));
	connect(butFontRight, SIGNAL(released()), this, SLOT(handle_fontright()));

	butFontLeft = new QPushButton(tr("Left adjust"));
	connect(butFontLeft, SIGNAL(released()), this, SLOT(handle_fontleft()));

	butFontMoreSpace = new QPushButton(tr("More image\nspace"));
	connect(butFontMoreSpace, SIGNAL(released()), this, SLOT(handle_fontmorespace()));

	butFontLessSpace = new QPushButton(tr("Less image\nspace"));
	connect(butFontLessSpace, SIGNAL(released()), this, SLOT(handle_fontlessspace()));

	butImageSelect = new QPushButton(tr("Select image(s)"));
	connect(butImageSelect, SIGNAL(released()), this, SLOT(handle_imageselect()));

	butImageBgColor = new QPushButton(tr("Select color"));
	connect(butImageBgColor, SIGNAL(released()), this, SLOT(handle_imagebgcolor()));

	butImageZoom = new QPushButton(tr("Select zoom"));
	connect(butImageZoom, SIGNAL(released()), this, SLOT(handle_imagezoom()));

	butImageFit = new QPushButton(tr("Select fit"));
	connect(butImageFit, SIGNAL(released()), this, SLOT(handle_imagefit()));

	butImageNext = new QPushButton(tr("Select next"));
	connect(butImageNext, SIGNAL(released()), this, SLOT(handle_imagenext()));

	butImagePrev = new QPushButton(tr("Select previous"));
	connect(butImagePrev, SIGNAL(released()), this, SLOT(handle_imageprev()));

	butImageCenter = new QPushButton(tr("Center adjust"));
	connect(butImageCenter, SIGNAL(released()), this, SLOT(handle_imagecenter()));

	butImageRight = new QPushButton(tr("Right adjust"));
	connect(butImageRight, SIGNAL(released()), this, SLOT(handle_imageright()));

	butImageLeft = new QPushButton(tr("Left adjust"));
	connect(butImageLeft, SIGNAL(released()), this, SLOT(handle_imageleft()));

	butCopySettings = new QPushButton(tr("Copy current settings\nto clipboard"));
	connect(butCopySettings, SIGNAL(released()), this, SLOT(handle_copysettings()));

	gl_main = new MppGridLayout();
	gb_font = new MppGroupBox(tr("Font"));
	gb_image = new MppGroupBox(tr("Background"));

	gl_main->addWidget(butTrack, 0, 0, 1, 2);
	gl_main->addWidget(butMode, 0, 2, 1, 2);

	gl_main->addWidget(butShowWindow, 4, 0, 1, 1);
	gl_main->addWidget(butFullScreen, 4, 1, 1, 1);
	gl_main->addWidget(butCopySettings, 4, 2, 1, 1);

	gb_font->addWidget(butFontSelect, 0, 0, 1, 1);
	gb_font->addWidget(butFontFgColor, 1, 0, 1, 1);
	gb_font->addWidget(butFontBgColor, 2, 0, 1, 1);
	gb_font->addWidget(butFontCenter, 3, 0, 1, 1);
	gb_font->addWidget(butFontRight, 4, 0, 1, 1);
	gb_font->addWidget(butFontLeft, 5, 0, 1, 1);
	gb_font->addWidget(butFontMoreSpace, 6, 0, 1, 1);
	gb_font->addWidget(butFontLessSpace, 7, 0, 1, 1);

	gb_image->addWidget(butImageSelect, 0, 0, 1, 1);
	gb_image->addWidget(butImageBgColor, 1, 0, 1, 1);
	gb_image->addWidget(butImageZoom, 2, 0, 1, 1);
	gb_image->addWidget(butImageFit, 3, 0, 1, 1);
	gb_image->addWidget(butImageNext, 4, 0, 1, 1);
	gb_image->addWidget(butImagePrev, 5, 0, 1, 1);
	gb_image->addWidget(butImageCenter, 6, 0, 1, 1);
	gb_image->addWidget(butImageRight, 7, 0, 1, 1);
	gb_image->addWidget(butImageLeft, 8, 0, 1, 1);

	gl_main->addWidget(gb_font, 2, 0, 1, 1);
	gl_main->addWidget(gb_image, 2, 1, 1, 1);

	gl_main->setRowStretch(3, 1);
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
MppShowControl :: handle_track_change(int n)
{
	trackview = n;
	handle_text_change();
	handle_pict_change();
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
	wg_show->setWindowState(wg_show->windowState() ^ Qt::WindowFullScreen);
}

void
MppShowControl :: handle_text_watchdog()
{
	MppScoreMain &sm = *mw->scores_main[trackview];
	MppElement *last;
	MppElement *curr;
	MppObjectProps text;
	int visual_next_index;
	int visual_curr_index;
	int visual_last_index;

	/* wait for transitions complete */
	if (aobj[0].isAnimating() ||
	    aobj[1].isAnimating())
		return;

	pthread_mutex_lock(&mw->mtx);
	last = sm.head.state.last_start;
	curr = sm.head.state.curr_start;
	text = sm.head.state.text_curr;
	pthread_mutex_unlock(&mw->mtx);

	/* locate last and current play position */
	sm.locateVisual(last, &visual_last_index, 0, 0);
	sm.locateVisual(curr, &visual_curr_index, &visual_next_index, 0);

	/* try to load next text */
	if (visual_last_index == visual_curr_index)
		visual_curr_index = visual_next_index;

	/* check validity of indexes */
	if (current_mode < MPP_SHOW_ST_LYRICS ||
	    visual_last_index < 0 ||
	    visual_last_index >= sm.visual_max ||
	    visual_curr_index < 0 ||
	    visual_curr_index >= sm.visual_max ||
	    sm.pVisual[visual_last_index].str == 0 ||
	    sm.pVisual[visual_curr_index].str == 0) {
		if (aobj[0].isVisible() || aobj[1].isVisible()) {
			aobj[0].fadeOut();
			aobj[1].fadeOut();
		}
		return;
	}

	/* avoid labels */
	if (MppIsLabel(*sm.pVisual[visual_curr_index].str))
		visual_curr_index = visual_last_index;

	int state = 0;
	if (aobj[toggle].isVisible())
		state |= 1;
	if (aobj[toggle ^ 1].isVisible())
		state |= 2;

	switch (state) {
	case 0:
		aobj[0].reset();
		aobj[0].props = text;
		aobj[0].str = *sm.pVisual[visual_last_index].str;
		aobj[0].fadeIn();
		toggle = 0;
		break;
	case 2:
		toggle ^= 1;
	case 1:
		/* update first object */
		aobj[toggle].moveUp(aobj[toggle].ypos_curr);
		if (aobj[toggle].str != *sm.pVisual[visual_last_index].str ||
		    aobj[toggle].props != text)
			aobj[toggle].fadeOut();
		else if (visual_last_index != visual_curr_index) {
			/* update second object */
			aobj[toggle ^ 1].reset();
			aobj[toggle ^ 1].props = text;
			aobj[toggle ^ 1].str = *sm.pVisual[visual_curr_index].str;
			aobj[toggle ^ 1].ypos_curr = aobj[toggle].height;
			aobj[toggle ^ 1].fadeIn();
		}
		break;
	default:
		if (visual_last_index != visual_curr_index) {
			if (aobj[toggle].str == *sm.pVisual[visual_last_index].str &&
			    aobj[toggle ^ 1].str == *sm.pVisual[visual_curr_index].str &&
			    aobj[toggle ^ 1].props == text)
				break;
		}
		/* update both objects */
		aobj[toggle].moveUp(aobj[toggle ^ 1].ypos_curr);
		aobj[toggle].fadeOut();
		toggle ^= 1;
		aobj[toggle].moveUp(aobj[toggle].ypos_curr);
		if (aobj[toggle].str != *sm.pVisual[visual_last_index].str ||
		    aobj[toggle].props != text)
			aobj[toggle].fadeOut();
		break;
	}
}

void
MppShowControl :: handle_pict_watchdog()
{
	MppScoreMain &sm = *mw->scores_main[trackview];
	MppObjectProps image;

	/* wait for transitions complete */
	if (aobj[2].isAnimating())
		return;
	
	pthread_mutex_lock(&mw->mtx);
	image = sm.head.state.image_curr;
	pthread_mutex_unlock(&mw->mtx);

	/* check if background should not be shown */
	if (current_mode < MPP_SHOW_ST_BACKGROUND) {
		if (aobj[2].isVisible())
			aobj[2].fadeOut();
		return;
	}

	/* check need for new transition */
	if (aobj[2].isVisible()) {
		if (aobj[2].props != image)
			aobj[2].fadeOut();
	} else {
		/* reset pixmap */
		background = QPixmap();
		/* check for valid number */
		if ((int)image.num < files.size())
			background.load(files[image.num]);
		aobj[2].props = image;
		aobj[2].fadeIn();
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
MppShowControl :: handle_fontselect()
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
MppShowControl :: handle_fontfgcolor()
{
  	if (trackview < 0 || trackview >= MPP_MAX_VIEWS)
		return;

	MppScoreMain &sm = *mw->scores_main[trackview];	

	pthread_mutex_lock(&mw->mtx);
	QColor color = sm.head.state.text_curr.color.fg();
	pthread_mutex_unlock(&mw->mtx);

	QColorDialog dlg(color);

	if (dlg.exec() == QDialog::Accepted) {
		pthread_mutex_lock(&mw->mtx);
		sm.head.state.text_curr.color.setFg(dlg.currentColor());
		pthread_mutex_unlock(&mw->mtx);
	}
}

void
MppShowControl :: handle_fontbgcolor()
{
  	if (trackview < 0 || trackview >= MPP_MAX_VIEWS)
		return;

	MppScoreMain &sm = *mw->scores_main[trackview];	

	pthread_mutex_lock(&mw->mtx);
	QColor color = sm.head.state.text_curr.color.bg();
	pthread_mutex_unlock(&mw->mtx);

	QColorDialog dlg(color);

	if (dlg.exec() == QDialog::Accepted) {
		pthread_mutex_lock(&mw->mtx);
		sm.head.state.text_curr.color.setBg(dlg.currentColor());
		pthread_mutex_unlock(&mw->mtx);
	}
}

void
MppShowControl :: handle_fontcenter()
{
  	if (trackview < 0 || trackview >= MPP_MAX_VIEWS)
		return;

	MppScoreMain &sm = *mw->scores_main[trackview];	

	pthread_mutex_lock(&mw->mtx);
	sm.head.state.text_curr.align = 0;
	pthread_mutex_unlock(&mw->mtx);
}

void
MppShowControl :: handle_fontright()
{
  	if (trackview < 0 || trackview >= MPP_MAX_VIEWS)
		return;

	MppScoreMain &sm = *mw->scores_main[trackview];	

	pthread_mutex_lock(&mw->mtx);
	sm.head.state.text_curr.align = 1;
	pthread_mutex_unlock(&mw->mtx);
}

void
MppShowControl :: handle_fontleft()
{
  	if (trackview < 0 || trackview >= MPP_MAX_VIEWS)
		return;

	MppScoreMain &sm = *mw->scores_main[trackview];	

	pthread_mutex_lock(&mw->mtx);
	sm.head.state.text_curr.align = 2;
	pthread_mutex_unlock(&mw->mtx);
}

void
MppShowControl :: handle_fontmorespace()
{
  	if (trackview < 0 || trackview >= MPP_MAX_VIEWS)
		return;

	MppScoreMain &sm = *mw->scores_main[trackview];	

	pthread_mutex_lock(&mw->mtx);
	if (sm.head.state.text_curr.space + 9 > 99)
		sm.head.state.text_curr.space = 99;
	else
		sm.head.state.text_curr.space += 9;
	pthread_mutex_unlock(&mw->mtx);
}

void
MppShowControl :: handle_fontlessspace()
{
  	if (trackview < 0 || trackview >= MPP_MAX_VIEWS)
		return;

	MppScoreMain &sm = *mw->scores_main[trackview];	

	pthread_mutex_lock(&mw->mtx);
	if (sm.head.state.text_curr.space > 9)
		sm.head.state.text_curr.space -= 9;
	else
		sm.head.state.text_curr.space = 0;
	pthread_mutex_unlock(&mw->mtx);
}

void
MppShowControl :: handle_imageselect()
{
	QFileDialog *diag = 
	  new QFileDialog(mw, tr("Select background image(s)"), 
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
MppShowControl :: handle_imagebgcolor()
{
	if (trackview < 0 || trackview >= MPP_MAX_VIEWS)
		return;

	MppScoreMain &sm = *mw->scores_main[trackview];	

	pthread_mutex_lock(&mw->mtx);
	QColor color = sm.head.state.image_curr.color.bg();
	pthread_mutex_unlock(&mw->mtx);

	QColorDialog dlg(color);

	if (dlg.exec() == QDialog::Accepted) {
		pthread_mutex_lock(&mw->mtx);
		sm.head.state.image_curr.color.setBg(dlg.currentColor());
		pthread_mutex_unlock(&mw->mtx);
	}
}

void
MppShowControl :: handle_imagezoom()
{
  	if (trackview < 0 || trackview >= MPP_MAX_VIEWS)
		return;

	MppScoreMain &sm = *mw->scores_main[trackview];	

	pthread_mutex_lock(&mw->mtx);
	sm.head.state.image_curr.how = 0;
	pthread_mutex_unlock(&mw->mtx);
}

void
MppShowControl :: handle_imagefit()
{
  	if (trackview < 0 || trackview >= MPP_MAX_VIEWS)
		return;

	MppScoreMain &sm = *mw->scores_main[trackview];	

	pthread_mutex_lock(&mw->mtx);
	sm.head.state.image_curr.how = 1;
	pthread_mutex_unlock(&mw->mtx);
}

void
MppShowControl :: handle_imagenext()
{
  	if (trackview < 0 || trackview >= MPP_MAX_VIEWS)
		return;

	MppScoreMain &sm = *mw->scores_main[trackview];	

	pthread_mutex_lock(&mw->mtx);
	if ((int)sm.head.state.image_curr.num < files.size())
		sm.head.state.image_curr.num++;
	/* range check */
	if ((int)sm.head.state.image_curr.num > files.size())
		sm.head.state.image_curr.num = 0;
	pthread_mutex_unlock(&mw->mtx);
}

void
MppShowControl :: handle_imageprev()
{
  	if (trackview < 0 || trackview >= MPP_MAX_VIEWS)
		return;

	MppScoreMain &sm = *mw->scores_main[trackview];	

	pthread_mutex_lock(&mw->mtx);
	if (sm.head.state.image_curr.num != 0)
		sm.head.state.image_curr.num--;
	/* range check */
	if ((int)sm.head.state.image_curr.num > files.size())
		sm.head.state.image_curr.num = 0;
	pthread_mutex_unlock(&mw->mtx);
}

void
MppShowControl :: handle_imagecenter()
{
  	if (trackview < 0 || trackview >= MPP_MAX_VIEWS)
		return;

	MppScoreMain &sm = *mw->scores_main[trackview];	

	pthread_mutex_lock(&mw->mtx);
	sm.head.state.image_curr.align = 0;
	pthread_mutex_unlock(&mw->mtx);
}

void
MppShowControl :: handle_imageright()
{
  	if (trackview < 0 || trackview >= MPP_MAX_VIEWS)
		return;

	MppScoreMain &sm = *mw->scores_main[trackview];	

	pthread_mutex_lock(&mw->mtx);
	sm.head.state.image_curr.align = 1;
	pthread_mutex_unlock(&mw->mtx);
}

void
MppShowControl :: handle_imageleft()
{
  	if (trackview < 0 || trackview >= MPP_MAX_VIEWS)
		return;

	MppScoreMain &sm = *mw->scores_main[trackview];	

	pthread_mutex_lock(&mw->mtx);
	sm.head.state.image_curr.align = 2;
	pthread_mutex_unlock(&mw->mtx);
}

void
MppShowControl :: handle_copysettings()
{
	if (trackview < 0 || trackview >= MPP_MAX_VIEWS)
		return;

	MppScoreMain &sm = *mw->scores_main[trackview];

	pthread_mutex_lock(&mw->mtx);
	QString str =
	  QString("K7.%1.%2.%3 /* image */\n"
		  "K8.%4.%5.%6 /* bg color */\n"
		  "K10.%7.%8.%9 /* font props */\n"
		  "K11.%10.%11.%12 /* shadow color */\n"
		  "K12.%13.%14.%15 /* text color */\n")
	  .arg(sm.head.state.image_curr.num)
	  .arg(sm.head.state.image_curr.how)
	  .arg(sm.head.state.image_curr.align)
	  .arg(sm.head.state.image_curr.color.bg_red)
	  .arg(sm.head.state.image_curr.color.bg_green)
	  .arg(sm.head.state.image_curr.color.bg_blue)
	  .arg(sm.head.state.text_curr.align)
	  .arg(sm.head.state.text_curr.space)
	  .arg(sm.head.state.text_curr.shadow)
	  .arg(sm.head.state.text_curr.color.bg_red)
	  .arg(sm.head.state.text_curr.color.bg_green)
	  .arg(sm.head.state.text_curr.color.bg_blue)
	  .arg(sm.head.state.text_curr.color.fg_red)
	  .arg(sm.head.state.text_curr.color.fg_green)
	  .arg(sm.head.state.text_curr.color.fg_blue);
	pthread_mutex_unlock(&mw->mtx);

	QApplication::clipboard()->setText(str);
}

void
MppShowControl :: handle_text_change()
{
	if (aobj[0].isVisible() || aobj[1].isVisible()) {
		aobj[0].fadeOut();
		aobj[1].fadeOut();
	}
}

void
MppShowControl :: handle_pict_change()
{
	if (aobj[2].isVisible())
		aobj[2].fadeOut();
}
