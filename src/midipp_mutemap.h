/*-
 * Copyright (c) 2010-2022 Hans Petter Selasky
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

#include "midipp_dialog.h"

class MppMuteMapChBase : public MppDialog
{
	Q_OBJECT
public:
	MppMuteMapChBase(MppMainWindow *_mw, const QString &_title) : MppDialog(_mw, _title) {};

	int devno;

	MppCheckBox *cbx_mute[16];

	QPushButton *but_reset_all;
	QPushButton *but_close_all;

public slots:
	void handle_reset_all();
	void handle_apply_all();
	void handle_revert_all();
};

class MppMuteMapCh : public MppMuteMapChBase
{
public:
	MppMuteMapCh(MppMainWindow *, int);
};

class MppMuteMapOtherBase : public MppDialog
{
	Q_OBJECT
public:
	MppMuteMapOtherBase(MppMainWindow *_mw, const QString &_title) : MppDialog(_mw, _title) {};

	int devno;

	MppButtonMap *cbx_mute_program;
	MppButtonMap *cbx_mute_pedal;
	MppButtonMap *cbx_mute_local_keys;
	MppButtonMap *cbx_mute_control;
	MppButtonMap *cbx_mute_non_channel;

	QPushButton *but_reset_all;
	QPushButton *but_close_all;

public slots:
	void handle_reset_all();
	void handle_apply_all();
  	void handle_revert_all();
};

class MppMuteMapOther : public MppMuteMapOtherBase
{
public:
	MppMuteMapOther(MppMainWindow *, int);
};

#endif		/* _MIDIPP_MUTEMAP_H_ */
