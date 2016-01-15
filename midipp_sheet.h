/*-
 * Copyright (c) 2016 Hans Petter Selasky. All rights reserved.
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

#ifndef _MIDIPP_SHEET_H_
#define	_MIDIPP_SHEET_H_

#include "midipp.h"

struct MppSheetRow {
	int	type;
	int	col;
	short	label;
	short	playing;
	union {
		struct {
			short	chan;
			short	trans;
			int	dur;
			int	num;
		}	score;
		struct {
			int	chan;
			int	trans;
			int	num;
		}	macro;
		struct {
			int	pre;
			int	post;
		}	timer;
	}	u;
};

struct MppSheetCol {
	int label;
	int line;
	int pre_timer;
	int post_timer;
};

class MppSheet : public QWidget
{
	Q_OBJECT;
public:
	MppSheet(MppMainWindow *, int);
	~MppSheet();
	void	compile(MppHead &);
	MppGridLayout *gl_sheet;
	MppMainWindow *mw;
	QScrollBar *vs_vert;
	QScrollBar *vs_horiz;
	MppButtonMap *mode_map;
	int	unit;
	int	xoff;
	int	yoff;
	int	boxs;
	int	mode;
	ssize_t	num_rows;
	ssize_t	num_cols;
	MppSheetRow *entries_rows;
	MppSheetCol *entries_cols;
	int	*entries_ptr;
	void	sizeInit();
	void	paintEvent(QPaintEvent *);	
	void	mousePressEvent(QMouseEvent *);
	void	mouseReleaseEvent(QMouseEvent *);
	const QString outputColumn(ssize_t);
	void	wheelEvent(QWheelEvent *);
	void	watchdog();

public slots:
	void	handleScrollChanged(int);
	void	handleModeChanged(int);
};

#endif					/* _MIDIPP_SHEET_H_ */
