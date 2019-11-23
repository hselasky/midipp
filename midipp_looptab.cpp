/*-
 * Copyright (c) 2010-2019 Hans Petter Selasky. All rights reserved.
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
#include "midipp_instrument.h"
#include "midipp_groupbox.h"

static void
mid_add_event(struct mid_data *d, struct umidi20_event *event)
{
	event->position += d->position[0];

	if (d->cc_enabled) {
		/*
		 * Need to lock the root device before adding
		 * entries to the play queue:
		 */
		pthread_mutex_lock(&(root_dev.mutex));
		umidi20_event_queue_insert(&root_dev.play[d->cc_device_no].queue,
		    event, UMIDI20_CACHE_INPUT);
		pthread_mutex_unlock(&(root_dev.mutex));
	} else {
		umidi20_event_queue_insert(&d->track->queue,
		    event, UMIDI20_CACHE_INPUT);
	}
}

static void
MppLoopTabTimerCallback(void *arg)
{
	MppLoopTab *plt = (MppLoopTab *)arg;
	uint32_t period = 0;
	struct umidi20_event *event;
	struct umidi20_event *event_copy;

	plt->mw->atomic_lock();
	for (int x = 0; x != MPP_LOOP_MAX; x++) {
		if (plt->loop[x].state != MppLoopTab::ST_PLAYING)
			continue;
		if (period < plt->loop[x].period)
			period = plt->loop[x].period;
	}

	plt->pos_align = plt->mw->get_time_offset();
	if (plt->pos_align == 0)
		plt->pos_align = 1;
	
	if (period == 0 || plt->mw->midiTriggered == 0) {
	  	umidi20_update_timer(&MppLoopTabTimerCallback, plt, 1, 0);
	  	plt->mw->atomic_unlock();		
		return;
	}

	for (int x = 0; x != MPP_LOOP_MAX; x++) {
		if (plt->loop[x].state != MppLoopTab::ST_PLAYING)
			continue;
		plt->loop[x].repeat_factor = qRound((qreal)period / (qreal)plt->loop[x].period);
		plt->loop[x].scale_factor = (qreal)period / (plt->loop[x].repeat_factor * (qreal)plt->loop[x].period);

		for (qreal n = 0; n != plt->loop[x].repeat_factor; n++) {
			uint32_t off = 2 * (n * plt->loop[x].period * plt->loop[x].scale_factor);

			for (int z = 0; z != MPP_MAX_TRACKS; z++) {
				UMIDI20_QUEUE_FOREACH(event, &plt->loop[x].track[z]->queue) {
					if (~umidi20_event_get_what(event) & UMIDI20_WHAT_CHANNEL)
						continue;
					if (umidi20_event_get_control_address(event) == 0x40 &&
					    plt->pedal_rec == 0)
						continue;
					if (plt->mw->check_play(z, 0, off)) {
						event_copy = umidi20_event_copy(event, 0);
						if (event_copy != 0)
							mid_add_event(&plt->mw->mid_data, event_copy);
					}
					if (plt->mw->check_record(z, 0, off)) {
						event_copy = umidi20_event_copy(event, 0);
						if (event_copy != 0)
							mid_add_event(&plt->mw->mid_data, event_copy);
					}
				}
			}
		}
	}
	plt->mw->atomic_unlock();

	umidi20_update_timer(&MppLoopTabTimerCallback, plt, 2 * period, 0);
}

void
MppLoopTab :: handle_timer_sync()
{
	mw->atomic_lock();
	umidi20_update_timer(&MppLoopTabTimerCallback, this, 1, 1);
	mw->atomic_unlock();
}

MppLoopTab :: MppLoopTab(QWidget *parent, MppMainWindow *_mw)
  : QWidget(parent)
{
	int n;
	int x;
	int y;
	int z;

	/* set memory default */

	memset(auto_zero_start, 0, auto_zero_end - auto_zero_start);

	mw = _mw;

	gl = new QGridLayout(this);

	mbm_pedal_rec = new MppButtonMap("Record pedal\0" "OFF\0" "ON\0", 2, 2);
	connect(mbm_pedal_rec, SIGNAL(selectionChanged(int)), this, SLOT(handle_pedal_rec(int)));
	gl->addWidget(mbm_pedal_rec, 0, 0, 1, 1);

	mbm_arm_import = new MppButtonMap("Import to\0" "OFF\0" "View-A\0" "View-B\0", 3, 3);
	gl->addWidget(mbm_arm_import, 0, 1, 1, 1);

	mbm_arm_reset = new MppButtonMap("Reset one\0" "OFF\0" "ON\0", 2, 2);
	gl->addWidget(mbm_arm_reset, 0, 2, 1, 1);

	gb_control = new MppGroupBox("Control area");
	gl->addWidget(gb_control, 1, 0, 1, 3);

	but_reset = new QPushButton(tr("Reset all"));
	connect(but_reset, SIGNAL(released()), this, SLOT(handle_reset()));
	gl->addWidget(but_reset, 2, 0, 1, 1);
	
#if MPP_LOOP_MAX != 16
#error "This code needs an update"
#endif
	for (n = x = 0; x != 4; x++) {
		for (y = 0; y != 4; y++, n++) {
			for (z = 0; z != MPP_MAX_TRACKS; z++)
				loop[n].track[z] = umidi20_track_alloc();
			loop[n].but_trig = new MppButton(
			    tr("Loop %1\n"
			       "IDLE :: 00.00\n").arg(n), n);
			connect(loop[n].but_trig, SIGNAL(released(int)),
			    this, SLOT(handle_trigger(int)));
			gb_control->addWidget(loop[n].but_trig, x, y, 1, 1);
		}
	}
	
	gl->setRowStretch(1, 1);
	gl->setColumnStretch(3, 1);

	mw->atomic_lock();
	needs_update = 1;
	mw->atomic_unlock();

	handle_value_changed(0);

	umidi20_set_timer(&MppLoopTabTimerCallback, this, 1);
}

MppLoopTab :: ~MppLoopTab()
{
	uint8_t n;
	uint8_t z;

	umidi20_unset_timer(&MppLoopTabTimerCallback, this);

	mw->atomic_lock();
	for (n = 0; n != MPP_LOOP_MAX; n++) {
		for (z = 0; z != MPP_MAX_TRACKS; z++)
			umidi20_track_free(loop[n].track[z]);
	}
	mw->atomic_unlock();
}

/* This function must be called locked */
bool
MppLoopTab :: check_record(uint8_t index, uint8_t chan, uint8_t n)
{
	struct mid_data *d = &mw->mid_data;
	uint32_t pos;

	if (index >= MPP_MAX_TRACKS || chan >= 0x10 || n >= MPP_LOOP_MAX)
		return (false);
	if (loop[n].state != ST_REC)
		return (false);

	pos = mw->get_time_offset();
	if (pos == 0)
		pos = 1;

	if (loop[n].first == 0)
		loop[n].first = pos_align;

	loop[n].last = pos;

	needs_update = 1;

	d->track = loop[n].track[index];
	mid_set_channel(d, chan);
	mid_set_position(d, pos - loop[n].first);
	mid_set_device_no(d, 0xFF);
	return (true);
}

bool
MppLoopTab :: handle_import(int n)
{
	struct umidi20_track *track_merged;
	struct umidi20_event *event;
	struct umidi20_event *event_copy;
	int arm_state = mbm_arm_import->currSelection;
	int z;

	if (arm_state == 0)
		return (false);

	mbm_arm_import->setSelection(0);

	track_merged = umidi20_track_alloc();

	mw->atomic_lock();
	for (z = 0; z != MPP_MAX_TRACKS; z++) {
		UMIDI20_QUEUE_FOREACH(event, &loop[n].track[z]->queue) {
			event_copy = umidi20_event_copy(event, 0);
			umidi20_event_queue_insert(&track_merged->queue,
			    event_copy, UMIDI20_CACHE_INPUT);
		}
	}
	mw->atomic_unlock();

    	mw->import_midi_track(track_merged, MIDI_FLAG_DURATION | MIDI_FLAG_DIALOG, n, arm_state - 1);

	umidi20_track_free(track_merged);

	return (true);
}

/* Must be called locked */
void
MppLoopTab :: handle_recordN(int n)
{
	struct umidi20_event *event;

	loop[n].period = 0;

	for (int z = 0; z != MPP_MAX_TRACKS; z++) {
		struct umidi20_event *first = 0;
		size_t num_first = 0;
		uint32_t period = 0;

		UMIDI20_QUEUE_FOREACH(event, &loop[n].track[z]->queue) {
			if (umidi20_event_is_key_start(event)) {
				if (first == 0) {
					first = event;
					num_first++;
				} else if (umidi20_event_get_key(event) == umidi20_event_get_key(first) &&
					   umidi20_event_get_channel(event) == umidi20_event_get_channel(first)) {
					num_first++;
				}
			}
		}
		if (num_first & 1)
			continue;
		if (first == 0)
			continue;

		num_first /= 2;

		UMIDI20_QUEUE_FOREACH(event, &loop[n].track[z]->queue) {
			if (umidi20_event_is_key_start(event)) {
				if (umidi20_event_get_key(event) == umidi20_event_get_key(first) &&
				    umidi20_event_get_channel(event) == umidi20_event_get_channel(first)) {
					if (!num_first--) {
						period = event->position - first->position;
						break;
					}
				}
			}
		}
		/* collect largest period */
		if (period > loop[n].period)
			loop[n].period = period;
	}
	/* check if there are no events */
	if (loop[n].period == 0)
		handle_clearN(n);
}

void
MppLoopTab :: handle_trigger(int n)
{
	if (handle_import(n) || handle_clear(n))
		return;

	mw->atomic_lock();
	switch (loop[n].state) {
	case ST_IDLE:
		loop[n].state = ST_REC;
		break;
	case ST_REC:
		loop[n].state = ST_PLAYING;
		handle_recordN(n);
		break;
	case ST_PLAYING:
		loop[n].state = ST_STOPPED;
		break;
	case ST_STOPPED:
		loop[n].state = ST_PLAYING;
		break;
	default:
		break;
	}
	needs_update = 1;
	mw->atomic_unlock();
}

bool
MppLoopTab :: handle_clear(int n)
{
  	int arm_state = mbm_arm_reset->currSelection;

	if (arm_state == 0)
		return (false);

	mbm_arm_reset->setSelection(0);

	mw->atomic_lock();
	handle_clearN(n);
	mw->atomic_unlock();

	return (true);
}

/* Must be called locked */
void
MppLoopTab :: handle_clearN(int n)
{
	for (unsigned z = 0; z != MPP_MAX_TRACKS; z++)
		umidi20_event_queue_drain(&loop[n].track[z]->queue);

	loop[n].state = ST_IDLE;
	loop[n].first = 0;
	loop[n].last = 0;

	needs_update = 1;
}

void
MppLoopTab :: handle_reset()
{
	int n;

	mw->atomic_lock();
	for (n = 0; n != MPP_LOOP_MAX; n++)
		handle_clearN(n);
	mw->atomic_unlock();

	mbm_pedal_rec->setSelection(0);
	mbm_arm_import->setSelection(0);
	mbm_arm_reset->setSelection(0);
}

void
MppLoopTab :: handle_value_changed(int dummy)
{

}

void
MppLoopTab :: handle_pedal_rec(int value)
{
	mw->atomic_lock();
	pedal_rec = value;
	mw->atomic_unlock();
}

void
MppLoopTab :: watchdog()
{
	uint8_t n;
	uint32_t dur;
	char buf_dur[16];
	const char *pbuf;

	mw->atomic_lock();
	n = needs_update;
	needs_update = 0;
	mw->atomic_unlock();

	if (n == 0)
		return;

	for (n = 0; n != MPP_LOOP_MAX; n++) {

		mw->atomic_lock();

		dur = (loop[n].last - loop[n].first) / 10;

		snprintf(buf_dur, sizeof(buf_dur),
		    "%02u.%02u", (dur / 100) % 100, (dur % 100));

		switch(loop[n].state) {
		case ST_IDLE:
			pbuf = "IDLE";
			break;
		case ST_REC:
			pbuf = "REC";
			break;
		case ST_PLAYING:
			pbuf = "PLAY";
			break;
		default:
			pbuf = "STOP";
			break;
		}

		mw->atomic_unlock();

		loop[n].but_trig->setText(
		    tr("Loop %1\n" "%2 :: %3\n").arg(n).arg(pbuf).arg(buf_dur));
	}
}
