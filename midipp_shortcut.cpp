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

#include "midipp_shortcut.h"
#include "midipp_button.h"
#include "midipp_groupbox.h"
#include "midipp_gridlayout.h"
#include "midipp_scores.h"
#include "midipp_mode.h"
#include "midipp_mainwindow.h"
#include "midipp_bpm.h"

MppShortcutTab :: MppShortcutTab(MppMainWindow *_mw)
{
	uint32_t x;
	uint32_t xr = 0;
	uint32_t y = 0;

	mw = _mw;

	gb_jump = new MppGroupBox(tr("Label Jump MIDI shortcuts"));
	gb_mode = new MppGroupBox(tr("View Mode MIDI shortcuts"));
	gb_ops = new MppGroupBox(tr("Operation MIDI shortcuts"));

	gl = new MppGridLayout();
	gl->addWidget(gb_jump, 0,0,2,4);
	gl->addWidget(gb_mode, 0,4,1,4);
	gl->addWidget(gb_ops, 1,4,1,4);
	gl->setRowStretch(3, 1);
	gl->setColumnStretch(8, 1);

	watchdog = new QTimer(this);
	watchdog->setSingleShot(1);
	connect(watchdog, SIGNAL(timeout()), this, SLOT(handle_watchdog()));

	memset(filter, 0, sizeof(filter));

	memset(shortcut_desc, 0, sizeof(shortcut_desc));

	shortcut_desc[MPP_SHORTCUT_J0] = "J0";
	shortcut_desc[MPP_SHORTCUT_J1] = "J1";
	shortcut_desc[MPP_SHORTCUT_J2] = "J2";
	shortcut_desc[MPP_SHORTCUT_J3] = "J3";
	shortcut_desc[MPP_SHORTCUT_J4] = "J4";
	shortcut_desc[MPP_SHORTCUT_J5] = "J5";
	shortcut_desc[MPP_SHORTCUT_J6] = "J6";
	shortcut_desc[MPP_SHORTCUT_J7] = "J7";
	shortcut_desc[MPP_SHORTCUT_J8] = "J8";
	shortcut_desc[MPP_SHORTCUT_J9] = "J9";
	shortcut_desc[MPP_SHORTCUT_J10] = "J10";
	shortcut_desc[MPP_SHORTCUT_J11] = "J11";
	shortcut_desc[MPP_SHORTCUT_J12] = "J12";
	shortcut_desc[MPP_SHORTCUT_J13] = "J13";
	shortcut_desc[MPP_SHORTCUT_J14] = "J14";
	shortcut_desc[MPP_SHORTCUT_J15] = "J15";

	shortcut_desc[MPP_SHORTCUT_ALL] = "ALL";
	shortcut_desc[MPP_SHORTCUT_TRANS] = "TRANS";
	shortcut_desc[MPP_SHORTCUT_FIXED] = "FIXED";
	shortcut_desc[MPP_SHORTCUT_MIXED] = "MIXED";
	shortcut_desc[MPP_SHORTCUT_CHORD_PIANO] = "CHORD-PIANO";
	shortcut_desc[MPP_SHORTCUT_CHORD_GUITAR] = "CHORD-GUITAR";
	shortcut_desc[MPP_SHORTCUT_TRIGGER] = "TRIGGER";
	shortcut_desc[MPP_SHORTCUT_PAUSE] = "PAUSE";
	shortcut_desc[MPP_SHORTCUT_REWIND] = "REWIND";
	shortcut_desc[MPP_SHORTCUT_BPM_TOGGLE] = "BPMTOG";

	but_set_all = new QPushButton("Record All");
	connect(but_set_all, SIGNAL(released()), this, SLOT(handle_set_all_flat()));

	but_clr_all = new QPushButton("Record None");
	connect(but_clr_all, SIGNAL(released()), this, SLOT(handle_clr_all_flat()));

	but_reset = new QPushButton("Clear Selected");
	connect(but_reset, SIGNAL(released()), this, SLOT(handle_reset_all_flat()));

	but_default = new QPushButton("Set Defaults");
	connect(but_default, SIGNAL(released()), this, SLOT(handle_default()));

	gl->addWidget(but_set_all, 2, 0, 1, 1);
	gl->addWidget(but_clr_all, 2, 1, 1, 1);
	gl->addWidget(but_reset, 2, 2, 1, 1);
	gl->addWidget(but_default, 2, 3, 1, 1);

	for (x = 0; x != MPP_SHORTCUT_MAX; x++) {
		MppGroupBox *gb;

		if (x == MPP_SHORTCUT_LABEL_MAX ||
		    x == MPP_SHORTCUT_MODE_MAX) {
			xr = 0;
			y = 0;
		}

		but_rec[x] = new MppButton(tr("REC"), x);
		connect(but_rec[x], SIGNAL(released(int)), this, SLOT(handle_record(int)));

		led_cmd[x] = new QLineEdit();
		led_cmd[x]->setMaxLength(32);

		connect(led_cmd[x], SIGNAL(textChanged(const QString &)), this, SLOT(handle_text_changed(const QString &)));

		if (x < MPP_SHORTCUT_LABEL_MAX) {
			gb = gb_jump;
		} else if (x < MPP_SHORTCUT_MODE_MAX) {
			gb = gb_mode;
		} else {
			gb = gb_ops;
		}

		gb->addWidget(new QLabel(tr(shortcut_desc[x])), xr, y, 1, 1);
		gb->addWidget(led_cmd[x], xr, y + 1, 1, 1);
		gb->addWidget(but_rec[x], xr, y + 2, 1, 1);

		xr++;
		if (xr == 8) {
			xr = 0;
			y += 3;
		}
	}
	handle_default();
}

MppShortcutTab :: ~MppShortcutTab()
{

}

/* this function is called locked */
uint8_t
MppShortcutTab :: handle_event_received_locked(MppScoreMain *sm,
    struct umidi20_event *event)
{
	uint8_t match[3] = {event->cmd[1],event->cmd[2],event->cmd[3]};
	uint8_t mask[3] = {0xF0,0xFF,0xFF};
	uint32_t x;
	uint8_t found = 0;

	/* check for start of MIDI command */
	if ((match[0] & 0x80) == 0)
		return (0);

	/* key end is not a valid event */
	if (umidi20_event_is_key_end(event)) {
		match[0] = 0x90;
		mask[2] = 0;	/* ignore velocity */

		/* if passing all keys, magic commands don't work */
		switch (sm->keyMode) {
		case MM_PASS_ALL:
			return (0);
		default:
			break;
		}
		for (x = 0; x != MPP_SHORTCUT_MAX; x++) {
			if (((filter[x][0] ^ match[0]) & mask[0]) ||
			    ((filter[x][1] ^ match[1]) & mask[1]) ||
			    ((filter[x][2] ^ match[2]) & mask[2]))
				continue;
			/* mask due to key-press match */
			return (1);
		}
		return (0);
	} else if (umidi20_event_is_key_start(event)) {
		match[0] = 0x90;
		mask[2] = 0;	/* ignore velocity */

		/* if passing all keys, magic commands don't work */
		switch (sm->keyMode) {
		case MM_PASS_ALL:
			return (0);
		default:
			break;
		}
	}
	for (x = 0; x != MPP_SHORTCUT_MAX; x++) {
		if (((filter[x][0] ^ match[0]) & mask[0]) ||
		    ((filter[x][1] ^ match[1]) & mask[1]) ||
		    ((filter[x][2] ^ match[2]) & mask[2]))
			continue;
		found = 1;
		switch (x) {
		case MPP_SHORTCUT_J0 ... MPP_SHORTCUT_J15:
			mw->handle_jump_locked(x - MPP_SHORTCUT_J0);
			break;
		case MPP_SHORTCUT_ALL:
			sm->keyMode = MM_PASS_ALL;
			mw->keyModeUpdated = 1;
			break;
		case MPP_SHORTCUT_TRANS:
			sm->keyMode = MM_PASS_NONE_TRANS;
			mw->keyModeUpdated = 1;
			break;
		case MPP_SHORTCUT_FIXED:
			sm->keyMode = MM_PASS_NONE_FIXED;
			mw->keyModeUpdated = 1;
			break;
		case MPP_SHORTCUT_MIXED:
			sm->keyMode = MM_PASS_ONE_MIXED;
			mw->keyModeUpdated = 1;
			break;
		case MPP_SHORTCUT_CHORD_PIANO:
			sm->keyMode = MM_PASS_NONE_CHORD_PIANO;
			mw->keyModeUpdated = 1;
			break;
		case MPP_SHORTCUT_CHORD_GUITAR:
			sm->keyMode = MM_PASS_NONE_CHORD_GUITAR;
			mw->keyModeUpdated = 1;
			break;
		case MPP_SHORTCUT_TRIGGER:
			mw->handle_midi_trigger();
			mw->doOperation &=
			    ~(MPP_OPERATION_PAUSE|MPP_OPERATION_REWIND);
			break;
		case MPP_SHORTCUT_PAUSE:
			mw->doOperation |= MPP_OPERATION_PAUSE;
			break;
		case MPP_SHORTCUT_REWIND:
			mw->doOperation &= ~MPP_OPERATION_PAUSE;
			mw->doOperation |= MPP_OPERATION_REWIND;
			break;
		case MPP_SHORTCUT_BPM_TOGGLE:
			mw->dlg_bpm->enabled ^= 1;
			mw->dlg_bpm->handle_update(mw->dlg_bpm->enabled);
			mw->doOperation |= MPP_OPERATION_BPM;
			break;
		default:
			break;
		}
	}
	return (found);
}

void
MppShortcutTab :: handle_record(int x)
{
	if (but_rec[x]->isFlat()) {
		but_rec[x]->setFlat(0);
	} else {
		but_rec[x]->setFlat(1);
	}
	handle_update();
}

void
MppShortcutTab :: handle_watchdog()
{
	int n;

	for (n = 0; n != MPP_SHORTCUT_MAX; n++) {
		QString str = led_cmd[n]->text();
		uint8_t buf[str.size() | 4];
		int x;
		int y;

		/* parse MIDI string */

		memset(buf, 0, sizeof(buf));

		for (x = y = 0; x != str.size(); x++) {
			char ch = str[x].toLatin1();
			if (ch >= '0' && ch <= '9') {
				buf[y / 2] <<= 4;
				buf[y / 2] |= (ch - '0');
				y++;
			} else if (ch >= 'a' && ch <= 'f') {
				buf[y / 2] <<= 4;
				buf[y / 2] |= (ch - 'a' + 10);
				y++;
			} else if (ch >= 'A' && ch <= 'F') {
				buf[y / 2] <<= 4;
				buf[y / 2] |= (ch - 'A' + 10);
				y++;
			} else if (ch == ' ' || ch == '\t' || ch == '\n') {
				if (y & 1)
					y++;
			}
		}

		pthread_mutex_lock(&mw->mtx);
		filter[n][0] = buf[0];
		filter[n][1] = buf[1];
		filter[n][2] = buf[2];
		pthread_mutex_unlock(&mw->mtx);
	}
}

void
MppShortcutTab :: handle_text_changed(const QString &arg)
{
	watchdog->start(1000);
}

void
MppShortcutTab :: handle_clr_all_flat()
{
	uint32_t x;
	for (x = 0; x != MPP_SHORTCUT_MAX; x++)
		but_rec[x]->setFlat(0);
	handle_update();
}

void
MppShortcutTab :: handle_set_all_flat()
{
	uint32_t x;
	for (x = 0; x != MPP_SHORTCUT_MAX; x++)
		but_rec[x]->setFlat(1);
	handle_update();
}

void
MppShortcutTab :: handle_update()
{
	uint32_t x;
	for (x = 0; x != MPP_SHORTCUT_MAX; x++) {
		if (but_rec[x]->isFlat())
			break;
	}
	pthread_mutex_lock(&mw->mtx);
	mw->controlRecordOn = (x != MPP_SHORTCUT_MAX);
	pthread_mutex_unlock(&mw->mtx);
}

void
MppShortcutTab :: handle_record_event(const uint8_t *data)
{
	uint32_t x;
	uint8_t match[3] = {(uint8_t)(data[1] & 0xF0),data[2],data[3]};

	for (x = 0; x != MPP_SHORTCUT_MAX; x++) {
		if (but_rec[x]->isFlat())
			break;
	}
	if (x == MPP_SHORTCUT_MAX)
		return;

	/* remove amplitude from key-presses */
	if (match[0] == 0x90)
		match[2] = 0x7e;

	led_cmd[x]->setText(QString("%1 %2 %3")
		.arg(match[0], 2, 16, QChar('0'))
		.arg(match[1], 2, 16, QChar('0'))
		.arg(match[2], 2, 16, QChar('0')));

	but_rec[x]->setFlat(0);
	handle_update();
}

void
MppShortcutTab :: handle_reset_all_flat()
{
	uint32_t x;
	for (x = 0; x != MPP_SHORTCUT_MAX; x++) {
		if (but_rec[x]->isFlat()) {
			but_rec[x]->setFlat(0);
			led_cmd[x]->setText(QString());
		}
	}
}

void
MppShortcutTab :: handle_default()
{
	uint32_t x;
	for (x = 0; x != MPP_SHORTCUT_MAX; x++) {
		but_rec[x]->setFlat(0);
		if (x < MPP_SHORTCUT_J12) {
			led_cmd[x]->setText(QString("%1 %2 %3")
			    .arg(0x90, 2, 16, QChar('0'))
			    .arg(MPP_DEFAULT_CMD_KEY + x -
			        MPP_SHORTCUT_J0, 2, 16, QChar('0'))
			    .arg(0x7e, 2, 16, QChar('0')));
		} else {
			led_cmd[x]->setText(QString());
		}
	}
}
