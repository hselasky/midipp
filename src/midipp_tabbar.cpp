/*-
 * Copyright (c) 2013-2022 Hans Petter Selasky
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

#include "midipp_tabbar.h"

void
MppTabButton :: mouseDoubleClickEvent(QMouseEvent *event)
{
	MppTabBar *ptab = (MppTabBar *)parent();
	ptab->handleMouseDoubleClickEvent(this);
	QPushButton::mouseDoubleClickEvent(event);
}

MppTabBar :: MppTabBar(QWidget *parent) : QWidget(parent)
{
	right_sw = new QStackedWidget();
	left_sw = new QStackedWidget();

	nwidgets = 0;
	ntabs = 0;

	hideButtons = false;
	hideIcons = false;

	QFontMetrics fm(font());

	basic_size = fm.height();

	left_sw->setMinimumWidth(12 * basic_size);
	right_sw->setMinimumWidth(12 * basic_size);

	setFixedHeight(2 * basic_size);
	setMouseTracking(true);

	split = new QSplitter();

#if defined(Q_OS_IOS)
	split->setHandleWidth(32);
#else
	split->setHandleWidth(16);
#endif
	split->addWidget(left_sw);
	split->addWidget(right_sw);

	left_sw->setVisible(0);
	right_sw->setVisible(1);

	connect(this, SIGNAL(doRepaintEnqueue()), this, SLOT(doRepaintCb()));
}

void
MppTabBar :: addWidget(QWidget *pWidget)
{
	if (nwidgets < MPP_MAX_WIDGETS) {
		widgets[nwidgets].pWidget = pWidget;
		nwidgets++;
		if (pWidget != 0) {
			pWidget->setParent(this);
			connect(pWidget, SIGNAL(released()), this, SLOT(handleMouseReleaseEvent()));
		}
	}
}

void
MppTabBar :: addTab(QWidget *pw, const QString &name)
{
	if (ntabs < MPP_MAX_TABS) {
		tabs[ntabs].w = pw;
		tabs[ntabs].name = name;
		tabs[ntabs].flags = FLAG_RIGHT;
		tabs[ntabs].button.setText(name);
		tabs[ntabs].button.setParent(this);
		connect(&tabs[ntabs].button, SIGNAL(released()), this, SLOT(handleMouseReleaseEvent()));
		if (ntabs == 0)
			tabs[0].button.setFocus(Qt::TabFocusReason);
		right_sw->addWidget(pw);
		ntabs++;
	}
}

void
MppTabBar :: handleMouseReleaseEvent()
{
	for (int x = 0; x != ntabs; x++) {
		if (&tabs[x].button == sender()) {
			makeWidgetVisible(tabs[x].w);
			break;
		}
	}
}

void
MppTabBar :: handleMouseDoubleClickEvent(QWidget *orig)
{
	for (int x = 0; x != ntabs; x++) {
		if (&tabs[x].button == orig) {
			changeTab(x);
			if (tabs[x].flags == FLAG_LEFT)
				moveCurrWidgetRight();
			else
				moveCurrWidgetLeft();
			break;
		}
	}
}

void
MppTabBar :: moveCurrWidgetLeft()
{
	QWidget *pw;
	int index;

	index = right_sw->currentIndex();
	if (index < 0)
		return;

	pw = right_sw->widget(index);
	right_sw->removeWidget(pw);

	if (right_sw->widget(0) == NULL)
		right_sw->setVisible(0);

	left_sw->addWidget(pw);
	left_sw->setCurrentWidget(pw);
	left_sw->setVisible(1);

	for (index = 0; index != ntabs; index++) {
		if (tabs[index].w == pw) {
			tabs[index].flags = FLAG_LEFT;
			break;
		}
	}
	update();
}

void
MppTabBar :: moveCurrWidgetRight()
{
	QWidget *pw;
	int index;

	index = left_sw->currentIndex();
	if (index < 0)
		return;

	pw = left_sw->widget(index);
	left_sw->removeWidget(pw);

	if (left_sw->widget(0) == NULL)
		left_sw->setVisible(0);

	right_sw->addWidget(pw);
	right_sw->setCurrentWidget(pw);
	right_sw->setVisible(1);

	for (index = 0; index != ntabs; index++) {
		if (tabs[index].w == pw) {
			tabs[index].flags = FLAG_RIGHT;
			break;
		}
	}
	update();
}

void
MppTabBar :: makeWidgetVisible(QWidget *widget, QWidget *except)
{
	int x;

	for (x = 0; x != ntabs; x++) {
		if (tabs[x].w == widget)
			break;
	}

	for (x = 0; ; x++) {
		QWidget *tab = left_sw->widget(x);
		if (tab == NULL)
			break;
		if (tab == widget) {
			if (except != 0 && 
			    left_sw->currentWidget() == except)
				moveCurrWidgetRight();
			if (left_sw->currentWidget() != widget)
				left_sw->setCurrentWidget(widget);
			break;
		}
	}
	for (x = 0; ; x++) {
		QWidget *tab = right_sw->widget(x);
		if (tab == NULL)
			break;
		if (tab == widget) {
			if (except != 0 && 
			    right_sw->currentWidget() == except)
				moveCurrWidgetLeft();
			if (right_sw->currentWidget() != widget)
				right_sw->setCurrentWidget(widget);
			break;
		}
	}
	update();
}

void
MppTabBar :: changeTab(int index)
{
	QWidget *pw;
	QWidget *pm;
	int x;

	pm = tabs[index].w;

	for (x = 0; (pw = right_sw->widget(x)) != NULL; x++) {
		if (pw == pm) {
			right_sw->setCurrentIndex(x);
			break;
		}
	}
	for (x = 0; (pw = left_sw->widget(x)) != NULL; x++) {
		if (pw == pm) {
			left_sw->setCurrentIndex(x);
			break;
		}
	}
}

int
MppTabBar :: isVisible(QWidget *pw)
{
	return (left_sw->currentWidget() == pw ||
	    right_sw->currentWidget() == pw);
}

int
MppTabBar :: computeWidth(int n) const
{
	QFontMetrics fm(font());
	return ((3 * basic_size) +
	    fm.boundingRect(QRect(0,0,0,0),
	    Qt::TextSingleLine | Qt::AlignLeft, tabs[n].name).width());
}

void
MppTabBar :: paintEvent(QPaintEvent *event)
{
	int ht = computeHeight(width());

	if (ht != height())
		emit doRepaintEnqueue();

	QPainter paint(this);

	int w = width();
	int h = height();
	int x_off = 0;
	int y_off;
	int n;
	int r = 0;

	paint.setRenderHints(QPainter::Antialiasing, 1);
	paint.setFont(font());

	paint.setPen(QPen(Mpp.ColorGrey, 0));
	paint.setBrush(Mpp.ColorGrey);
	paint.drawRoundedRect(QRect(0,0,w,h), 4, 4);

	if (!hideButtons) {
	    for (n = 0; n != ntabs; n++) {
		int dw = computeWidth(n);
		if (x_off != 0 && x_off + dw >= w) {
			x_off = 0;
			r++;
		}
		y_off = r * basic_size * 2;

		if (isVisible(tabs[n].w)) {
			paint.setPen(QPen(Mpp.ColorBlack, 0));
			paint.setBrush(Mpp.ColorBlack);

			if (tabs[n].flags & FLAG_LEFT) {
				QPoint temp[3] = {
					QPoint(x_off, y_off + basic_size),
					QPoint(x_off + basic_size / 2, y_off + (basic_size / 4)),
					QPoint(x_off + basic_size / 2, y_off + (basic_size * 2) - (basic_size / 4) - 1)
				};
				paint.drawPolygon(temp, 3);

			} else if (tabs[n].flags & FLAG_RIGHT) {
				QPoint temp[3] = {
					QPoint(x_off + dw - basic_size / 2, y_off + (basic_size / 4)),
					QPoint(x_off + dw, y_off + basic_size),
					QPoint(x_off + dw - basic_size / 2, y_off + (basic_size * 2) - (basic_size / 4) - 1)
				};
				paint.drawPolygon(temp, 3);
			}
		}

		tabs[n].area = QRect(x_off + (basic_size / 2), y_off + (basic_size / 4),
		    dw - basic_size, (basic_size * 2) - (basic_size / 2));

		tabs[n].button.setGeometry(tabs[n].area);

		x_off += dw;
	    }
	}
#if 0
	if (x_off != 0) {
		x_off = 0;
		r++;
	}
#endif
	if (!hideIcons) {
	    for (n = 0; n != nwidgets; n++) {
		int dw = (widgets[n].pWidget ? 4 : 3) * basic_size;
		if (x_off != 0 && x_off + dw >= w) {
			x_off = 0;
			r++;
		}
		y_off = r * basic_size * 2;
		x_off += dw;
		if (widgets[n].pWidget == 0)
			continue;
		widgets[n].area = QRect(x_off - dw, y_off,
		    4 * basic_size, 2 * basic_size);
		widgets[n].pWidget->setGeometry(widgets[n].area);
	    }
	}
	y_off = (r + 1) * basic_size * 2;
}

int
MppTabBar :: computeHeight(int w) const
{
	int x_off = 0;
	int r = 0;
	int n;

	if (!hideButtons) {
	    for (n = 0; n != ntabs; n++) {
		int dw = computeWidth(n);
		if (x_off != 0 && x_off + dw >= w) {
			x_off = 0;
			r++;
		}
		x_off += dw;
	    }
	}
#if 0
	if (x_off != 0) {
		x_off = 0;
		r++;
	}
#endif
	if (!hideIcons) {
	    for (n = 0; n != nwidgets; n++) {
		int dw = (widgets[n].pWidget ? 4 : 3) * basic_size;
		if (x_off != 0 && x_off + dw >= w) {
			x_off = 0;
			r++;
		}
		x_off += dw;
	    }
	}
	return ((r + 1) * basic_size * 2);
}

void
MppTabBar :: doRepaintCb()
{
	const int ht = computeHeight(width());

	setFixedHeight(ht);
	update();
}
