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

#include "midipp_tabbar.h"

MppTabBar :: MppTabBar(QWidget *parent)
    : QWidget(parent), MppTabBarRepaint(this)
{
	right_sw = new QStackedWidget(this);
	left_sw = new QStackedWidget(this);

	left_sw->setVisible(0);
	right_sw->setVisible(1);

	nwidgets = 0;
	ntabs = 0;

	QFontMetrics fm(font());

	basic_size = fm.height();

	left_sw->setMinimumWidth(12 * basic_size);
	right_sw->setMinimumWidth(12 * basic_size);

	setFixedHeight(2 * basic_size);
	setMouseTracking(1);

	split = new QSplitter();
#if defined(TARGET_OS_IPHONE) || defined(TARGET_IPHONE_SIMULATOR)
	split->setHandleWidth(32);
#else
	split->setHandleWidth(16);
#endif
	split->addWidget(left_sw);
	split->addWidget(right_sw);
}

MppTabBar :: ~MppTabBar()
{

}

void
MppTabBar :: addWidget(QWidget *pWidget)
{
	if (nwidgets < MPP_MAX_WIDGETS) {
		widgets[nwidgets].pWidget = pWidget;
		nwidgets++;
		if (pWidget != 0) {
			pWidget->setParent(this);
			pWidget->show();
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
		right_sw->addWidget(pw);
		ntabs++;
	}
}

void
MppTabBar :: mousePressEvent(QMouseEvent *event)
{
	int x;
	QPoint pos = event->pos();

	for (x = 0; x != ntabs; x++) {
		if (tabs[x].area.contains(pos)) {
			makeWidgetVisible(tabs[x].w);
			break;
		}
	}
	for (x = 0; x != nwidgets; x++) {
		if (widgets[x].pWidget == 0)
			continue;
		if (widgets[x].area.contains(pos))
			QWidget::eventFilter(widgets[x].pWidget, event);
	}
	event->accept();
}

void
MppTabBar :: mouseReleaseEvent(QMouseEvent *event)
{
	int x;
	QPoint pos = event->pos();
    
	for (x = 0; x != ntabs; x++) {
		if (tabs[x].area.contains(pos)) {
			makeWidgetVisible(tabs[x].w);
			break;
		}
	}
	for (x = 0; x != nwidgets; x++) {
		if (widgets[x].pWidget == 0)
			continue;
		if (widgets[x].area.contains(pos))
			QWidget::eventFilter(widgets[x].pWidget, event);
	}
	event->accept();
}


void
MppTabBar :: mouseMoveEvent(QMouseEvent *event)
{
	int x;
	QPoint pos = event->pos();
	for (x = 0; x != nwidgets; x++) {
		if (widgets[x].pWidget == 0)
			continue;
		if (widgets[x].area.contains(pos))
			QWidget::eventFilter(widgets[x].pWidget, event);
	}
	event->accept();
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
		doRepaintEnqueue();

	QPainter paint(this);

	int w = width();
	int h = height();
	int x_off = 0;
	int y_off;
	int n;
	int r;

	paint.setRenderHints(QPainter::Antialiasing, 1);
	paint.setFont(font());

	QColor grey(192,192,192);
	QColor light(128,128,128);
	QColor white(255,255,255);
	QColor black(0,0,0);

	paint.setPen(QPen(grey, 0));
	paint.setBrush(grey);
	paint.drawRoundedRect(QRect(0,0,w,h), 4, 4);

	for (r = n = 0; n != ntabs; n++) {
		int dw = computeWidth(n);
		if (x_off != 0 && x_off + dw >= w) {
			x_off = 0;
			r++;
		}
		y_off = r * basic_size * 2;

		if (isVisible(tabs[n].w)) {
			paint.setPen(QPen(black, 0));
			paint.setBrush(black);
		} else {
			paint.setPen(QPen(light, 0));
			paint.setBrush(light);
		}
		if (tabs[n].flags & FLAG_LEFT) {
			QPoint temp[3] = {
				QPoint(x_off, y_off + basic_size),
				QPoint(x_off + basic_size, y_off + (basic_size / 4)),
				QPoint(x_off + basic_size, y_off + (basic_size * 2) - (basic_size / 4) - 1)
			};
			paint.drawPolygon(temp, 3);

		} else if (tabs[n].flags & FLAG_RIGHT) {
			QPoint temp[3] = {
				QPoint(x_off + dw - basic_size, y_off + (basic_size / 4)),
				QPoint(x_off + dw, y_off + basic_size),
				QPoint(x_off + dw - basic_size, y_off + (basic_size * 2) - (basic_size / 4) - 1)
			};
			paint.drawPolygon(temp, 3);
		}

		tabs[n].area = QRect(x_off + (basic_size / 2), y_off + (basic_size / 4),
		    dw - basic_size, (basic_size * 2) - (basic_size / 2));

		if (isVisible(tabs[n].w)) {
			QRect area(x_off + basic_size, y_off + (basic_size / 4),
			    dw - (2*basic_size), (basic_size * 2) - (basic_size / 2));

			paint.setPen(QPen(white, 0));
			paint.setBrush(white);
			paint.drawRect(area);
		}

		paint.setPen(QPen(black, 0));
		paint.setBrush(black);
		paint.drawText(QRect(x_off + basic_size + (basic_size / 4),
		    y_off + (basic_size / 2),
		    dw - (2 * basic_size) - (basic_size / 2), basic_size),
		    Qt::TextSingleLine | Qt::AlignCenter, tabs[n].name);

		x_off += dw;
	}
	if (x_off != 0) {
		x_off = 0;
		r++;
	}
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
		QWidget::eventFilter(widgets[n].pWidget, event);
	}
	y_off = (r + 1) * basic_size * 2;
}

int
MppTabBar :: computeHeight(int w) const
{
	int x_off = 0;
	int r;
	int n;

	for (r = n = 0; n != ntabs; n++) {
		int dw = computeWidth(n);
		if (x_off != 0 && x_off + dw >= w) {
			x_off = 0;
			r++;
		}
		x_off += dw;
	}
	if (x_off != 0) {
		x_off = 0;
		r++;
	}
	for (n = 0; n != nwidgets; n++) {
		int dw = (widgets[n].pWidget ? 4 : 3) * basic_size;
		if (x_off != 0 && x_off + dw >= w) {
			x_off = 0;
			r++;
		}
		x_off += dw;
	}
	return ((r + 1) * basic_size * 2);
}

MppTabBarRepaint :: MppTabBarRepaint(MppTabBar *_parent)
{
	parent = _parent;
	connect(this, SIGNAL(doRepaintEnqueue()), this, SLOT(doRepaintCb()));
}

MppTabBarRepaint :: ~MppTabBarRepaint()
{

}

void
MppTabBarRepaint :: doRepaintCb()
{
	int ht = parent->computeHeight(parent->width());

	parent->setFixedHeight(ht);
	parent->update();
}
