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

#ifndef _MIDIPP_TABBAR_H_
#define	_MIDIPP_TABBAR_H_

#include "midipp.h"

class MppTabWidget {
public:
	MppTabWidget() {
		pWidget = 0;
	}
	~MppTabWidget() {
	}
	QRect area;
	QWidget *pWidget;
};

class MppTab {
public:
	MppTab() {
		w = 0;
		flags = 0;
	};
	~MppTab() {
	};

	QString name;
	QRect area;

	QWidget *w;
	int flags;
};

class MppTabBarRepaint : private QObject {
	Q_OBJECT;
public:
	MppTabBarRepaint(MppTabBar *);
	~MppTabBarRepaint();

	MppTabBar *parent;

public slots:
	void doRepaintCb();
signals:
	void doRepaintEnqueue();
};

class MppTabBar : public QWidget, public MppTabBarRepaint {
public:
	MppTabBar(QWidget * = 0);
	~MppTabBar();

	void addTab(QWidget *, const QString &);
	void addWidget(QWidget *);
	void mousePressEvent(QMouseEvent *);
	void mouseMoveEvent(QMouseEvent *);
	void mouseReleaseEvent(QMouseEvent *);
	void makeWidgetVisible(QWidget *, QWidget * = 0);
	void moveCurrWidgetLeft();
	void moveCurrWidgetRight();
	void changeTab(int);
	void paintEvent(QPaintEvent *);
	int isVisible(QWidget *);
	int computeWidth(int) const;
	int computeHeight(int) const;

	QSplitter *split;
	QStackedWidget *right_sw;
	QStackedWidget *left_sw;

	int nwidgets;
	int ntabs;
	int basic_size;

	enum {
		FLAG_LEFT = 0x01,
		FLAG_RIGHT = 0x02,
	};

	MppTabWidget widgets[MPP_MAX_WIDGETS];
	MppTab tabs[MPP_MAX_TABS];
};

#endif	/* _MIDIPP_TABBAR_H_ */
