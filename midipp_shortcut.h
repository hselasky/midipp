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

#ifndef _MIDIPP_SHORTCUT_H_
#define	_MIDIPP_SHORTCUT_H_

#include "midipp.h"

enum {
	MPP_SHORTCUT_J0 = 0,
	MPP_SHORTCUT_J1,
	MPP_SHORTCUT_J2,
	MPP_SHORTCUT_J3,
	MPP_SHORTCUT_J4,
	MPP_SHORTCUT_J5,
	MPP_SHORTCUT_J6,
	MPP_SHORTCUT_J7,
	MPP_SHORTCUT_J8,
	MPP_SHORTCUT_J9,
	MPP_SHORTCUT_J10,
	MPP_SHORTCUT_J11,
	MPP_SHORTCUT_J12,
	MPP_SHORTCUT_J13,
	MPP_SHORTCUT_J14,
	MPP_SHORTCUT_J15,
	MPP_SHORTCUT_LABEL_MAX,

	MPP_SHORTCUT_ALL = MPP_SHORTCUT_LABEL_MAX,
	MPP_SHORTCUT_TRANS,
	MPP_SHORTCUT_FIXED,
	MPP_SHORTCUT_MIXED,
	MPP_SHORTCUT_CHORD_PIANO,
	MPP_SHORTCUT_CHORD_GUITAR,
	MPP_SHORTCUT_MODE_MAX,

	MPP_SHORTCUT_TRIGGER = MPP_SHORTCUT_MODE_MAX,
	MPP_SHORTCUT_PAUSE,
	MPP_SHORTCUT_REWIND,
	MPP_SHORTCUT_BPM_TOGGLE,
	MPP_SHORTCUT_MAX,
};

class MppShortcutTab : public QObject
{
	Q_OBJECT;

public:
	MppShortcutTab(MppMainWindow *);
	~MppShortcutTab();

	MppMainWindow *mw;

	const char *shortcut_desc[MPP_SHORTCUT_MAX];

	uint8_t filter[MPP_SHORTCUT_MAX][4];

	MppGridLayout *gl;
	MppGroupBox *gb_jump;
	MppGroupBox *gb_mode;
	MppGroupBox *gb_ops;

	QPushButton *but_set_all;
	QPushButton *but_clr_all;
	QPushButton *but_reset;
	QPushButton *but_default;

	MppButton *but_rec[MPP_SHORTCUT_MAX];
	QLineEdit *led_cmd[MPP_SHORTCUT_MAX];
	QTimer *watchdog;

	uint8_t handle_event_received_locked(MppScoreMain *, struct umidi20_event *);
	void handle_record_event(const uint8_t *);
	void handle_update();

public slots:
	void handle_record(int);
	void handle_watchdog();
	void handle_text_changed(const QString &);
	void handle_clr_all_flat();
	void handle_set_all_flat();
	void handle_reset_all_flat();
	void handle_default();
};

#endif		/* _MIDIPP_SHORTCUT_H_ */
