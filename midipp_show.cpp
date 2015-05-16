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

	if (parent->aobj[2].opacity_curr >= (1.0 / 256.0)) {
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

		if (parent->aobj[x].opacity_curr < (1.0 / 256.0))
			continue;

		paint.setFont(parent->showFont);

		qreal wf = parent->showFont.pixelSize();
		qreal wa = (wf * (aobj.props.shadow % 100)) / 100.0;
		qreal ws = (w * (aobj.props.space % 100)) / 100.0;
		qreal xo;

		QRectF txtBound = paint.boundingRect(
		    QRectF(0, 0, w - ws - (2 * wf), h),
		    Qt::AlignLeft | Qt::AlignTop | Qt::TextWordWrap,
		    aobj.str + QChar('\n'));

		switch (aobj.props.align) {
		case 0:
			xo = (w - txtBound.width()) / 2.0;
			break;
		case 1:
			xo = (w - txtBound.width() - wf);
			break;
		default:
			xo = wf;
			break;
		}

		/* offset bounding box */
		txtBound.adjust(
		    xo + aobj.xpos_curr,
		    aobj.ypos_curr,
		    xo + aobj.xpos_curr,
		    aobj.ypos_curr);

		/* draw background, if any */
		if (aobj.props.shadow < 100) {
			paint.setPen(Qt::NoPen);
			paint.setBrush(aobj.props.color.bg());
			paint.setOpacity(aobj.opacity_curr *
			    (aobj.props.shadow % 100) / 100.0);
			txtBound.adjust(-wf,0,wf,0);
			paint.drawRoundedRect(txtBound, wf, wf);
			txtBound.adjust(wf,0,-wf,0);
		} else if (aobj.props.shadow < 200) {
			paint.setPen(aobj.props.color.bg());
			paint.setBrush(aobj.props.color.bg());
			paint.setOpacity(aobj.opacity_curr);
			txtBound.adjust(wa,wa,wa,wa);
			paint.drawText(txtBound, Qt::AlignCenter |
			    Qt::TextWordWrap, aobj.str);
			txtBound.adjust(-wa,-wa,-wa,-wa);
		}

		/* draw text */
		paint.setPen(aobj.props.color.fg());
		paint.setBrush(aobj.props.color.fg());
		paint.setOpacity(aobj.opacity_curr);
		paint.drawText(txtBound, Qt::AlignCenter |
		    Qt::TextWordWrap, aobj.str);

		/* store height and width */
		txtBound.adjust(-wf, 0, wf + wa, wa);
		aobj.height = txtBound.height();
		aobj.width = txtBound.width();
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

	anim_text_state = 0;
	anim_pict_state = 0;

	last_image.reset();
	last_text.reset();
	
	cached_last_index = -1;
	cached_curr_index = -1;

	current_mode = MPP_SHOW_ST_BLANK;
	trackview = 0;

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
	int visual_curr_index;
	int visual_last_index;

	pthread_mutex_lock(&mw->mtx);
	last = sm.head.state.last_start;
	curr = sm.head.state.curr_start;
	text = sm.head.state.text_curr;
	pthread_mutex_unlock(&mw->mtx);

	/* locate last and current play position */
	sm.locateVisual(last, &visual_last_index,
	    &visual_curr_index, 0);

	/* check if next text is accross a jump */
	if (visual_last_index == visual_curr_index)
		sm.locateVisual(curr, &visual_curr_index, 0, 0);

	/* check validity of indexes */
	if (visual_last_index < 0 ||
	    visual_last_index >= sm.visual_max ||
	    visual_curr_index < 0 ||
	    visual_curr_index >= sm.visual_max) {
		if (anim_text_state != 0) {
			aobj[0].fadeOut();
			aobj[1].fadeOut();
			anim_text_state = 0;
		}
		return;
	}

	/* check need for new transition */
	if (anim_text_state != 0) {
		if (visual_last_index == cached_last_index &&
		    visual_curr_index == cached_curr_index &&
		    anim_text_state != 3 && (text != last_text) == 0)
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
		aobj[1].reset();

		/* copy string, if any */
		if (sm.pVisual[visual_last_index].str != 0) {
			aobj[0].props = text;
			aobj[0].str = *sm.pVisual[visual_last_index].str;
			aobj[0].fadeIn();
		}
		break;
	case 3:
	case 1:
		if (visual_curr_index != visual_last_index) {
			/* dim in second */
			anim_text_state = 2;

			/* reset state */
			aobj[1].reset();

			/* copy string */
			if (sm.pVisual[visual_curr_index].str != 0) {
				aobj[1].props = text;
				aobj[1].str = *sm.pVisual[visual_curr_index].str;
				aobj[1].fadeIn();
				aobj[1].ypos_curr = aobj[0].height;
			}
		} else {
			/* dim in second and dim out first */
			anim_text_state = 1;

			/* move text around */
			aobj[1] = aobj[0];
			aobj[1].fadeOut();

			/* reset state */
			aobj[0].reset();

			/* copy string */
			if (sm.pVisual[visual_curr_index].str != 0) {
				aobj[0].props = text;
				aobj[0].str = *sm.pVisual[visual_curr_index].str;
				aobj[0].fadeIn();
			}
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
	last_text = text;
}

void
MppShowControl :: handle_pict_watchdog()
{
	MppScoreMain &sm = *mw->scores_main[trackview];
	MppObjectProps image;

	pthread_mutex_lock(&mw->mtx);
	image = sm.head.state.image_curr;
	pthread_mutex_unlock(&mw->mtx);

	/* check need for new transition */
	if (anim_pict_state != 0) {
		if (anim_pict_state != 2 &&
		    (image != last_image) == 0)
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
		/* fade in first */
		anim_pict_state = 1;

		aobj[2].reset();

		/* refresh pixmap */
		if (image != last_image) {
			/* reset pixmap */
			background = QPixmap();
			/* check for valid number */
			if ((int)image.num < files.size())
				background.load(files[image.num]);
			last_image = image;
			aobj[2].props = image;
		}
		/* reset state */
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
	cached_last_index = -1;
	cached_curr_index = -1;
	last_text.reset();
}

void
MppShowControl :: handle_pict_change()
{
	last_image.reset();
}
