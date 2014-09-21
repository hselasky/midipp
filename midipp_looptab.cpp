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

#include "midipp.h"

#include "midipp_buttonmap.h"
#include "midipp_chansel.h"
#include "midipp_mainwindow.h"
#include "midipp_scores.h"
#include "midipp_looptab.h"
#include "midipp_spinbox.h"
#include "midipp_button.h"
#include "midipp_midi.h"

MppLoopTab :: MppLoopTab(QWidget *parent, MppMainWindow *_mw)
  : QWidget(parent)
{
	char buf[32];
	int n;
	int x;

	/* set memory default */

	memset(auto_zero_start, 0, auto_zero_end - auto_zero_start);

	mw = _mw;

	gl = new QGridLayout(this);

	for (n = 0; n != MPP_LOOP_MAX; n++) {
		spn_chan[n] = new MppChanSel(0, 0);

		spn_key[n] = new MppSpinBox();
		spn_key[n]->setRange(0, 127);
		spn_key[n]->setValue(12 * n);

		snprintf(buf, sizeof(buf), "Loop%X", n);

		lbl_dur[n] = new QLabel(tr("00.00"));
		lbl_loop[n] = new QLabel(tr(buf));
		lbl_state[n] = new QLabel();

		but_clear[n] = new MppButton(tr("Clear"), n);
		but_trig[n] = new MppButton(tr("Trigger"), n);

		for (x = 0; x != MPP_MAX_VIEWS; x++)
			but_import[n][x] = new MppButton(QString("To %1-Scores").arg(QChar('A' + x)), n + (MPP_LOOP_MAX * x));
	}

	for (n = 0; n != MPP_LOOP_MAX; n++) {

		gl->addWidget(lbl_state[n], (MPP_LOOP_MAX-n-1) + 2, 6 + MPP_MAX_VIEWS, 1, 1);
		gl->addWidget(spn_chan[n], (MPP_LOOP_MAX-n-1) + 2, 5 + MPP_MAX_VIEWS, 1, 1);
		gl->addWidget(lbl_dur[n], (MPP_LOOP_MAX-n-1) + 2, 4 + MPP_MAX_VIEWS, 1, 1);
		gl->addWidget(but_clear[n], (MPP_LOOP_MAX-n-1) + 2, 2 + MPP_MAX_VIEWS, 1, 1);
		gl->addWidget(but_trig[n], (MPP_LOOP_MAX-n-1) + 2, 1 + MPP_MAX_VIEWS, 1, 1);
		gl->addWidget(lbl_loop[n], (MPP_LOOP_MAX-n-1) + 2, 0, 1, 1);
		gl->addWidget(spn_key[n], (MPP_LOOP_MAX-n-1) + 2, 3 + MPP_MAX_VIEWS, 1, 1);

		for (x = 0; x != MPP_MAX_VIEWS; x++) {
			gl->addWidget(but_import[n][x], (MPP_LOOP_MAX-n-1) + 2, 1 + x, 1, 1);
			connect(but_import[n][x], SIGNAL(released(int)), this, SLOT(handle_import(int)));
		}

		connect(spn_chan[n], SIGNAL(valueChanged(int)), this, SLOT(handle_value_changed(int)));
		connect(spn_key[n], SIGNAL(valueChanged(int)), this, SLOT(handle_value_changed(int)));
		connect(but_clear[n], SIGNAL(released(int)), this, SLOT(handle_clear(int)));
		connect(but_trig[n], SIGNAL(pressed(int)), this, SLOT(handle_trig(int)));
	}

	gl->setRowStretch(2 + MPP_LOOP_MAX, 1);
	gl->setColumnStretch(7 + MPP_MAX_VIEWS, 1);

	lbl_chn_title = new QLabel(tr("Channel"));
	lbl_dur_title = new QLabel(tr("Dur-s"));
	lbl_state_title = new QLabel(tr("State"));
	lbl_mkey_title = new QLabel(tr("Key-m"));

	gl->addWidget(lbl_state_title, 1, 6 + MPP_MAX_VIEWS, 1, 1);
	gl->addWidget(lbl_chn_title, 1, 5 + MPP_MAX_VIEWS, 1, 1);
	gl->addWidget(lbl_dur_title, 1, 4 + MPP_MAX_VIEWS, 1, 1);
	gl->addWidget(lbl_mkey_title, 1, 3 + MPP_MAX_VIEWS, 1, 1);

	but_reset = new QPushButton(tr("Reset"));
	connect(but_reset, SIGNAL(released()), this, SLOT(handle_reset()));
	gl->addWidget(but_reset, 0, 7, 1, 2);

	mbm_loop = new MppButtonMap("Looping feature\0" "OFF\0" "ON\0", 2, 2);
	connect(mbm_loop, SIGNAL(selectionChanged(int)), this, SLOT(handle_loop(int)));
	gl->addWidget(mbm_loop, 0, 0, 1, 2);

	mbm_multi = new MppButtonMap("Multiple keys feature\0" "OFF\0" "ON\0", 2, 2);
	connect(mbm_multi, SIGNAL(selectionChanged(int)), this, SLOT(handle_multi(int)));
	gl->addWidget(mbm_multi, 0, 2, 1, 2);

	mbm_pedal_rec = new MppButtonMap("Record pedal\0" "OFF\0" "ON\0", 2, 2);
	connect(mbm_pedal_rec, SIGNAL(selectionChanged(int)), this, SLOT(handle_pedal_rec(int)));
	gl->addWidget(mbm_pedal_rec, 0, 4, 1, 2);

	pthread_mutex_lock(&mw->mtx);

	needs_update = 1;

	for (n = 0; n != MPP_LOOP_MAX; n++)
		track[n] = umidi20_track_alloc();

	pthread_mutex_unlock(&mw->mtx);

	handle_value_changed(0);
}

MppLoopTab :: ~MppLoopTab()
{
	uint8_t n;

	pthread_mutex_lock(&mw->mtx);

	for (n = 0; n != MPP_LOOP_MAX; n++)
		umidi20_track_free(track[n]);

	pthread_mutex_unlock(&mw->mtx);
}

/* This function must be called locked */
void
MppLoopTab :: add_key(uint8_t key, uint8_t vel)
{
	struct mid_data *d = &mid_data;
	uint32_t pos;
	uint8_t n;

	pos = mw->get_time_offset();
	if (pos == 0)
		pos = 1;

	for (n = 0; n != MPP_LOOP_MAX; n++) {

		if (state[n] != ST_REC)
			continue;

		if (first_pos[n] == 0)
			first_pos[n] = pos;

		last_pos[n] = pos;

		needs_update = 1;

		d->track = track[n];
	
		mid_set_channel(d, chan_val[n]);
		mid_set_position(d, pos - first_pos[n]);
		mid_key_press(d, key, vel, 0);
	}
}

/* This function must be called locked */
void
MppLoopTab :: add_pedal(uint8_t val)
{
	struct mid_data *d = &mid_data;
	uint32_t pos;
	uint32_t off;
	uint8_t n;

	if (pedal_rec == 0)
		return;

	pos = mw->get_time_offset();
	if (pos == 0)
		pos = 1;

	for (n = 0; n != MPP_LOOP_MAX; n++) {

		if (state[n] != ST_REC)
			continue;

		if (first_pos[n] == 0) {
			off = 0;
		} else {
			off = pos - first_pos[n];
			last_pos[n] = pos;
		}

		needs_update = 1;

		d->track = track[n];
	
		mid_set_channel(d, chan_val[n]);
		mid_set_position(d, off);
		mid_pedal(d, val);
	}
}

void
MppLoopTab :: handle_trig(int n)
{
	pthread_mutex_lock(&mw->mtx);
	switch (state[n]) {
	case ST_IDLE:
		state[n] = ST_REC;
		break;
	case ST_REC:
		/* FALLTHROUGH */
	default:
		state[n] = ST_DONE;
		break;
	}
	last_loop = n;
	needs_update = 1;
	pthread_mutex_unlock(&mw->mtx);
}

void
MppLoopTab :: handle_import(int n)
{
	int which = n % MPP_LOOP_MAX;
	int view = n / MPP_LOOP_MAX;

	mw->import_midi_track(track[which], MIDI_FLAG_DURATION | MIDI_FLAG_DIALOG, which, view);
}

/* Must be called locked */
void
MppLoopTab :: fill_loop_data(int n, int vel, int key_off)
{
	struct umidi20_event *event;
	uint32_t pos;
	int key;
	uint8_t chan;
	uint8_t y;

	UMIDI20_QUEUE_FOREACH(event, &(track[n]->queue)) {

		key = umidi20_event_get_key(event) & 0x7F;

		key += key_off;

		if (key > 127)
			continue;
		if (key < 0)
			continue;
#if 0
		vel = umidi20_event_get_velocity(event);
#endif
		chan = chan_val[n];
		pos = event->position;

		if (umidi20_event_get_control_address(event) == 0x40) {

			uint8_t val;

			val = umidi20_event_get_control_value(event);

			for (y = 0; y != MPP_MAX_DEVS; y++) {
				if (mw->check_synth(y, chan, pos)) {
					mid_pedal(&mw->mid_data, val);
				}
			}

			if (mw->check_record(chan, pos)) {
				mid_pedal(&mw->mid_data, val);
			}

		} else if (umidi20_event_is_key_start(event)) {

			for (y = 0; y != MPP_MAX_DEVS; y++) {
				if (mw->check_synth(y, chan, pos)) {
					mw->do_key_press(key, vel, 0);
				}
			}

			if (mw->check_record(chan, pos)) {
				mw->do_key_press(key, vel, 0);
			}

		} else if (umidi20_event_is_key_end(event)) {

			for (y = 0; y != MPP_MAX_DEVS; y++) {
				if (mw->check_synth(y, chan, pos)) {
					mw->do_key_press(key, 0, 0);
				}
			}

			if (mw->check_record(chan, pos)) {
				mw->do_key_press(key, 0, 0);
			}
		}
	}
}

/* Must be called locked */
int
MppLoopTab :: handle_trigN(int key, int vel)
{
	int n;
	int found = 0;

	if (loop_on == 0)
		return (0);
	for (n = 0; n != MPP_LOOP_MAX; n++) {
		if (state[n] == ST_REC)
			return (0);
	}

	if (is_multi == 0) {
		n = last_loop;
		key -= MPP_DEFAULT_BASE_KEY;

		switch (state[n]) {
		case ST_IDLE:
		case ST_REC:
			break;
		default:
			fill_loop_data(n, vel, key);
			found = 1;
			break;
		}
	} else {
		for (n = 0; n != MPP_LOOP_MAX; n++) {
			if (key_val[n] != key)
				continue;

			switch (state[n]) {
			case ST_IDLE:
			case ST_REC:
				break;
			default:
				fill_loop_data(n, vel, 0);
				found = 1;
				break;
			}
		}
	}
	return (found);
}

void
MppLoopTab :: handle_clear(int n)
{
	pthread_mutex_lock(&mw->mtx);
	handle_clearN(n);
	pthread_mutex_unlock(&mw->mtx);
}

/* Must be called locked */
void
MppLoopTab :: handle_clearN(int n)
{
	umidi20_event_queue_drain(&(track[n]->queue));
	state[n] = ST_IDLE;
	first_pos[n] = 0;
	last_pos[n] = 0;

	needs_update = 1;
}

void
MppLoopTab :: handle_reset()
{
	uint8_t n;

	pthread_mutex_lock(&mw->mtx);
	for (n = 0; n != MPP_LOOP_MAX; n++)
		handle_clearN(n);
	last_loop = 0;
	pthread_mutex_unlock(&mw->mtx);

	mbm_loop->setSelection(0);
	mbm_pedal_rec->setSelection(0);
	mbm_multi->setSelection(0);

	for (n = 0; n != MPP_LOOP_MAX; n++) {
		spn_chan[n]->setValue(0);
		spn_key[n]->setValue(12 * n);
	}
}

void
MppLoopTab :: handle_value_changed(int dummy)
{
	uint8_t x;
	uint8_t temp[MPP_LOOP_MAX];

	for (x = 0; x != MPP_LOOP_MAX; x++)
		temp[x] = spn_chan[x]->value();

	pthread_mutex_lock(&mw->mtx);
	for (x = 0; x != MPP_LOOP_MAX; x++)
		chan_val[x] = temp[x];
	pthread_mutex_unlock(&mw->mtx);

	for (x = 0; x != MPP_LOOP_MAX; x++)
		temp[x] = spn_key[x]->value();

	pthread_mutex_lock(&mw->mtx);
	for (x = 0; x != MPP_LOOP_MAX; x++)
		key_val[x] = temp[x];
	pthread_mutex_unlock(&mw->mtx);
}

void
MppLoopTab :: handle_pedal_rec(int value)
{
	pthread_mutex_lock(&mw->mtx);
	pedal_rec = value;
	pthread_mutex_unlock(&mw->mtx);
}

void
MppLoopTab :: handle_loop(int value)
{
	pthread_mutex_lock(&mw->mtx);
	loop_on = value;
	pthread_mutex_unlock(&mw->mtx);
}

void
MppLoopTab :: handle_multi(int value)
{
	pthread_mutex_lock(&mw->mtx);
	is_multi = value;
	pthread_mutex_unlock(&mw->mtx);
}

void
MppLoopTab :: watchdog()
{
	uint8_t n;
	uint32_t dur;
	char buf_dur[16];
	char buf_state[16];
	const char *pbuf;
	uint8_t new_chan;

	pthread_mutex_lock(&mw->mtx);
	n = needs_update;
	needs_update = 0;
	pthread_mutex_unlock(&mw->mtx);

	if (n == 0)
		return;

	new_chan = 255;

	for (n = 0; n != MPP_LOOP_MAX; n++) {

		pthread_mutex_lock(&mw->mtx);

		dur = (last_pos[n] - first_pos[n]) / 10;

		snprintf(buf_dur, sizeof(buf_dur),
		    "%02u.%02u", (dur / 100) % 100, (dur % 100));

		switch(state[n]) {
		case ST_IDLE:
			pbuf = "IDLE";
			break;
		case ST_REC:
			new_chan = chan_val[n];
			pbuf = "REC";
			break;
		default:
			pbuf = "READY";
			break;
		}

		if (n == last_loop)
			snprintf(buf_state, sizeof(buf_state), ">%s<", pbuf);
		else
			snprintf(buf_state, sizeof(buf_state), "<%s>", pbuf);

		pthread_mutex_unlock(&mw->mtx);

		lbl_dur[n]->setText(tr(buf_dur));
		lbl_state[n]->setText(tr(buf_state));
	}

	if (new_chan < 16)
		mw->handle_instr_channel_changed(new_chan);
}
