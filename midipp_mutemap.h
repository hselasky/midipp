/*-
 * Copyright (c) 2010 Hans Petter Selasky. All rights reserved.
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

#ifndef _MIDIPP_MUTEMAP_H_
#define	_MIDIPP_MUTEMAP_H_

#include "midipp.h"

class MppMuteMap : public QDialog
{
	Q_OBJECT;

public:
	MppMuteMap(QWidget *parent, MppMainWindow *, int);
	~MppMuteMap();

	MppMainWindow *mw;
	int devno;

	QGridLayout *gl;

	QLabel *lbl_title[3];
	QLabel *lbl_chan[16];
	QLabel *lbl_mute_program;
	QLabel *lbl_mute_pedal;
	QLabel *lbl_mute_local_keys_enable;
	QLabel *lbl_mute_local_keys_disable;
	QLabel *lbl_mute_all_control;
	QCheckBox *cbx_mute[16];
	QCheckBox *cbx_mute_program;
	QCheckBox *cbx_mute_pedal;
	QCheckBox *cbx_mute_local_keys_enable;
	QCheckBox *cbx_mute_local_keys_disable;
	QCheckBox *cbx_mute_all_control;

	QPushButton *but_set_all;
	QPushButton *but_clear_all;
	QPushButton *but_revert_all;
	QPushButton *but_apply_all;
	QPushButton *but_cancel_all;

public slots:

	void handle_set_all();
	void handle_clear_all();
	void handle_revert_all();
	void handle_apply_all();
	void handle_cancel_all();
};

#endif		/* _MIDIPP_MUTEMAP_H_ */
