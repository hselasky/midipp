/*-
 * Copyright (c) 2011-2014 Hans Petter Selasky. All rights reserved.
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

#include "midipp_bpm.h"
#include "midipp_mainwindow.h"
#include "midipp_scores.h"
#include "midipp_spinbox.h"
#include "midipp_checkbox.h"
#include "midipp_groupbox.h"

static void
MppTimerCallback(void *arg)
{
	MppBpm *mb = (MppBpm *)arg;
	MppMainWindow *mw = mb->mw;
	MppScoreMain *sm;
	uint8_t temp;
	int n;

	pthread_mutex_lock(&mw->mtx);

	mb->last_timeout = umidi20_get_curr_position();

	mb->handle_update();

	if (mb->enabled != 0 && mw->midiTriggered != 0) {
		if (mb->duty_ticks != 0 && mb->amp != 0) {

			for (n = 0; n != MPP_MAX_VIEWS; n++) {
				if (mb->view_out[n] == 0)
					continue;

				/* avoid feedback */
				temp = mb->view_sync[n];
				mb->view_sync[n] = 0;

				sm = mw->scores_main[n];
				sm->handleKeyPress(mb->key, mb->amp, 0);
				sm->handleKeyRelease(mb->key, mb->duty_ticks);

				/* restore sync */
				mb->view_sync[n] = temp;
			}
		}
		if (mb->beat != 0)
			mw->send_byte_event_locked(0xF8);
	}
	pthread_mutex_unlock(&mw->mtx);
}

MppBpm :: MppBpm(MppMainWindow *parent)
  : QDialog(parent)
{
	QLabel *lbl;
	char buf[64];
	int n;

	mw = parent;

	gl = new QGridLayout(this);

	tim_config = new QTimer(this);
	tim_config->setSingleShot(1);
	connect(tim_config, SIGNAL(timeout()), this, SLOT(handle_config_apply()));

	spn_bpm_value = new QSpinBox();
	spn_bpm_value->setRange(1, 6000);
	spn_bpm_value->setSuffix(tr(" bpm"));
	connect(spn_bpm_value, SIGNAL(valueChanged(int)), this, SLOT(handle_config_change()));

	spn_bpm_duty = new QSpinBox();
	spn_bpm_duty->setRange(1, 199);
	spn_bpm_duty->setSuffix(tr(" %"));
	connect(spn_bpm_duty, SIGNAL(valueChanged(int)), this, SLOT(handle_config_change()));

	spn_bpm_amp = new QSpinBox();
	spn_bpm_amp->setRange(1, 127);
	connect(spn_bpm_amp, SIGNAL(valueChanged(int)), this, SLOT(handle_config_change()));

	spn_bpm_key = new MppSpinBox();
	spn_bpm_key->setRange(0, 127);
	connect(spn_bpm_key, SIGNAL(valueChanged(int)), this, SLOT(handle_config_change()));

	spn_bpm_period_ref = new QSpinBox();
	spn_bpm_period_ref->setRange(1, 6000);
	spn_bpm_period_ref->setSuffix(tr(" bpm"));
	connect(spn_bpm_period_ref, SIGNAL(valueChanged(int)), this, SLOT(handle_config_change()));

	spn_bpm_period_cur = new QSpinBox();
	spn_bpm_period_cur->setRange(0, 60000);
	spn_bpm_period_cur->setSuffix(tr(" ms"));
	connect(spn_bpm_period_cur, SIGNAL(valueChanged(int)), this, SLOT(handle_config_change()));

	spn_sync_max = new QSpinBox();
	spn_sync_max->setRange(0, 60000);
	spn_sync_max->setSuffix(tr(" ms"));
	connect(spn_sync_max, SIGNAL(valueChanged(int)), this, SLOT(handle_config_change()));

	for (n = 0; n != MPP_MAX_VIEWS; n++) {
		cbx_out_view[n] = new MppCheckBox();
		connect(cbx_out_view[n], SIGNAL(stateChanged(int,int)), this, SLOT(handle_config_change()));
		cbx_sync_view[n] = new MppCheckBox();
		connect(cbx_sync_view[n], SIGNAL(stateChanged(int,int)), this, SLOT(handle_config_change()));
	}

	cbx_midi_beat = new MppCheckBox();
	connect(cbx_midi_beat, SIGNAL(stateChanged(int,int)), this, SLOT(handle_config_change()));

	but_bpm_enable = new QPushButton(tr("Enable"));
	but_reset_all = new QPushButton(tr("Reset all"));
	but_done_all = new QPushButton(tr("Close"));

	connect(but_bpm_enable, SIGNAL(released()), this, SLOT(handle_bpm_enable()));
	connect(but_reset_all, SIGNAL(released()), this, SLOT(handle_reset_all()));
	connect(but_done_all, SIGNAL(released()), this, SLOT(handle_done_all()));

	setWindowTitle(tr("BPM generator settings"));
	setWindowIcon(QIcon(QString(MPP_ICON_FILE)));

	gb_ctrl = new MppGroupBox(tr("BPM control"));

	gb_ctrl->addWidget(new QLabel(tr("Value (1..6000)")), 0, 0, 1, 1);
	gb_ctrl->addWidget(spn_bpm_value, 0, 1, 1, 1);

	gb_ctrl->addWidget(new QLabel(tr("Max sync offset (0..60000)")), 1, 0, 1, 1);
	gb_ctrl->addWidget(spn_sync_max, 1, 1, 1, 1);

	gb_ctrl->addWidget(new QLabel(tr("Duty cycle (1..199)")), 2, 0, 1, 1);
	gb_ctrl->addWidget(spn_bpm_duty, 2, 1, 1, 1);

	gb_ctrl->addWidget(new QLabel(tr("Amplitude (1..127)")), 3, 0, 1, 1);
	gb_ctrl->addWidget(spn_bpm_amp, 3, 1, 1, 1);

	gb_ctrl->addWidget(new QLabel(tr("Play key")), 4, 0, 1, 1);
	gb_ctrl->addWidget(spn_bpm_key, 4, 1, 1, 1);

	gb_ctrl->addWidget(new QLabel(tr("Output to MIDI beat event")), 7, 0, 1, 1);
	gb_ctrl->addWidget(cbx_midi_beat, 7, 1, 1, 1);

	gb_scale = new MppGroupBox(tr("BPM time scaling"));

	gb_scale->addWidget(new QLabel(tr("Reference value (1..6000)")), 0, 0, 1, 1);
	gb_scale->addWidget(spn_bpm_period_ref, 0, 1, 1, 1);

	gb_scale->addWidget(new QLabel(tr("Reference time (0..60000)")), 1, 0, 1, 1);
	gb_scale->addWidget(spn_bpm_period_cur, 1, 1, 1, 1);

	gb_io = new MppGroupBox(tr("BPM I/O"));

	lbl = new QLabel(tr("Output\n" "enable"));
	lbl->setAlignment(Qt::AlignCenter);
	gb_io->addWidget(lbl, 0, 1, 1, 1, Qt::AlignCenter);

	lbl = new QLabel(tr("Sync\n" "source"));
	lbl->setAlignment(Qt::AlignCenter);
	gb_io->addWidget(lbl, 0, 2, 1, 1, Qt::AlignCenter);

	for (n = 0; n != MPP_MAX_VIEWS; n++) {
		snprintf(buf, sizeof(buf), "View %c", 'A' + n);
		lbl = new QLabel(tr(buf));
		lbl->setAlignment(Qt::AlignCenter);
		gb_io->addWidget(lbl, n + 1, 0, 1, 1, Qt::AlignCenter);
		gb_io->addWidget(cbx_out_view[n], n + 1, 1, 1, 1, Qt::AlignCenter);
		gb_io->addWidget(cbx_sync_view[n], n + 1, 2, 1, 1, Qt::AlignCenter);
	}

	gb_io->setColumnStretch(1,1);
	gb_io->setColumnStretch(2,1);

	gl->addWidget(gb_ctrl, 0, 0, 2, 1);
	gl->addWidget(gb_scale, 0, 1, 1, 3);
	gl->addWidget(gb_io, 1, 1, 1, 3);

	gl->addWidget(but_bpm_enable, 2, 1, 1, 1);
	gl->addWidget(but_reset_all, 2, 2, 1, 1);
	gl->addWidget(but_done_all, 2, 3, 1, 1);

	gl->setRowStretch(3, 1);
	gl->setColumnStretch(4, 1);

	handle_reset_all();

	umidi20_set_timer(&MppTimerCallback, this, 1000);
}

MppBpm :: ~MppBpm()
{
	umidi20_unset_timer(&MppTimerCallback, this);
}

void
MppBpm :: handle_reload_all()
{
	int value[8];

	pthread_mutex_lock(&mw->mtx);
	value[0] = bpm_cur;
	value[1] = duty;
	value[2] = amp;
	value[3] = key;
	value[4] = period_ref;
	value[5] = period_cur;
	value[6] = sync_max;
	pthread_mutex_unlock(&mw->mtx);

	spn_bpm_value->setValue(value[0]);
	spn_bpm_duty->setValue(value[1]);
	spn_bpm_amp->setValue(value[2]);
	spn_bpm_key->setValue(value[3]);
	spn_bpm_period_ref->setValue(value[4]);
	spn_bpm_period_cur->setValue(value[5]);
	spn_sync_max->setValue(value[6]);
}

void
MppBpm :: handle_reset_all()
{
	int n;

	but_bpm_enable->setText(tr("Enable"));

	for (n = 0; n != MPP_MAX_VIEWS; n++) {
		cbx_out_view[n]->setCheckState(Qt::Unchecked);
		cbx_sync_view[n]->setCheckState((n == 0) ? Qt::Checked : Qt::Unchecked);
	}

	cbx_midi_beat->setCheckState(Qt::Unchecked);

	pthread_mutex_lock(&mw->mtx);

	for (n = 0; n != MPP_MAX_VIEWS; n++) {
		view_out[n] = 0;
		view_sync[n] = (n == 0);
	}

	beat = 0;
	enabled = 0;
	bpm_cur = 120;
	bpm_other = 120;
	sync_max = 250;
	duty = 50;
	duty_ticks = 0;
	amp = 96;
	key = MPP_DEFAULT_BASE_KEY;
	period_ref = 120;
	period_cur = 0;

	history_in = 0;
	history_out = 0;
	last_timeout = 0;

	handle_update();

	pthread_mutex_unlock(&mw->mtx);

	handle_reload_all();
}

void
MppBpm :: handle_done_all()
{
	accept();
}

void
MppBpm :: handle_bpm_enable()
{
	pthread_mutex_lock(&mw->mtx);
	enabled = !enabled;
	handle_update(enabled);
	pthread_mutex_unlock(&mw->mtx);
	sync();
}

void
MppBpm :: sync()
{
	int value;

	pthread_mutex_lock(&mw->mtx);
	value = enabled;
	pthread_mutex_unlock(&mw->mtx);

	if (value == 0)
		but_bpm_enable->setText(tr("Enable"));
	else
		but_bpm_enable->setText(tr("Disable"));
}

/* must be called locked */
void
MppBpm :: handle_update(int restart)
{
	uint32_t cur_duration;
	uint32_t new_duration;
	uint32_t tmp_duration;
	uint32_t max_offset;
	uint32_t min_offset;
	uint32_t tmp_offset;
	uint32_t time_ms;
	uint32_t bpm;
	uint32_t x;
	uint32_t y;

	if (restart != 0) {
		history_out = 0;
		history_in = 0;
		bpm_other = bpm_cur;
	}

	bpm = bpm_get();

	/* compute period in milliseconds */
	new_duration = 0;
	cur_duration = (bpm / bpm_cur);
	if (cur_duration == 0)
		cur_duration = 1;

	min_offset = -1U;
	max_offset = sync_max;
	if (max_offset > cur_duration)
		max_offset = cur_duration;

	for (x = history_out; x != history_in; x = (x + 1) % MPP_MAX_BPM) {
		for (y = (x + 1) % MPP_MAX_BPM; y != history_in;
		     y = (y + 1) % MPP_MAX_BPM) {

			tmp_duration = history_data[y] - history_data[x];

			if (tmp_duration > cur_duration)
				tmp_offset = tmp_duration - cur_duration;
			else
				tmp_offset = cur_duration - tmp_duration;

			if (tmp_offset < (cur_duration / 2)) {
				if (tmp_offset < min_offset) {
					min_offset = tmp_offset;
					new_duration = tmp_duration;
				}
			}
		}
	}

	/* check if we should adjust the BPM value */
	if (new_duration != 0) {
		if (new_duration < (cur_duration - max_offset))
			new_duration = cur_duration - max_offset;
		else if (new_duration > (cur_duration + max_offset))
			new_duration = cur_duration + max_offset;
		if (new_duration == 0)
			new_duration = 1;

		bpm_other = bpm / new_duration;

		if (bpm_other > 6000)
			bpm_other = 6000;
		else if (bpm_other == 0)
			bpm_other = 1;
	}

	/* compute timer period in milliseconds */
	time_ms = bpm_get() / bpm_other;
	if (time_ms == 0)
		time_ms = 1;

	duty_ticks = ((time_ms * duty) + ((2 * 100) - 1)) / (2 * 100);

	umidi20_update_timer(&MppTimerCallback, this, time_ms, (restart != 0));
}

void
MppBpm :: handle_config_apply()
{
	uint32_t val[8];
	uint32_t tmp[MPP_MAX_VIEWS];
	uint32_t sync[MPP_MAX_VIEWS];
	uint32_t n;

	for (n = 0; n != MPP_MAX_VIEWS; n++) {
		tmp[n] = (cbx_out_view[n]->checkState() != Qt::Unchecked);
		sync[n] = (cbx_sync_view[n]->checkState() != Qt::Unchecked);
	}

	val[0] = spn_bpm_value->value();
	val[1] = spn_bpm_duty->value();
	val[2] = spn_bpm_amp->value();
	val[3] = spn_bpm_key->value();
	val[4] = spn_bpm_period_ref->value();
	val[5] = spn_bpm_period_cur->value();
	val[6] = spn_sync_max->value();
	val[7] = (cbx_midi_beat->checkState() != Qt::Unchecked);

	pthread_mutex_lock(&mw->mtx);
	bpm_other = bpm_cur = val[0];
	duty = val[1];
	amp = val[2];
	key = val[3];
	period_ref = val[4];
	period_cur = val[5];
	sync_max = val[6];
	beat = val[7];
	for (n = 0; n != MPP_MAX_VIEWS; n++) {
		view_out[n] = tmp[n];
		view_sync[n] = sync[n];
	}
	handle_update();
	pthread_mutex_unlock(&mw->mtx);

}

void
MppBpm :: handle_config_change(int val)
{
	tim_config->start(2000);
}

void
MppBpm :: handle_jump_event_locked(int view_index)
{
	if (view_index < 0 || view_index >= MPP_MAX_VIEWS ||
	    view_sync[view_index] == 0)
		return;

	history_out = 0;
	history_in = 0;
}

uint32_t
MppBpm :: bpm_get()
{
	uint32_t temp;

	temp = period_cur * period_ref;
	if (temp == 0)
		temp = 60000;

	return (temp);
}

void
MppBpm :: handle_beat_event_locked(int view_index)
{
	uint32_t limit_ms;
	uint32_t time_ms;
	uint32_t bpm;
	uint32_t pos;

	if (view_index < 0 || view_index >= MPP_MAX_VIEWS ||
	    view_sync[view_index] == 0)
		return;

	pos = umidi20_get_curr_position();

	/* get current bpm */
	bpm = bpm_get();

	/* compute period in milliseconds */
	time_ms = bpm / bpm_cur;
	if (time_ms == 0)
		time_ms = 1;

	/* remove old history */
	while (history_in != history_out) {
		uint32_t delta = pos -
		    history_data[history_out];
		if (delta < (2 * time_ms))
			break;

		history_out++;
		history_out %= MPP_MAX_BPM;
	}

	/* keep adding data */
	history_data[history_in] = pos;
	history_in++;
	history_in %= MPP_MAX_BPM;

	/* compute limit in milliseconds */
	if (time_ms > sync_max)
		limit_ms = time_ms - sync_max;
	else
		limit_ms = 0;

	/* check if beat is speeding up */
	if ((uint32_t)(pos - last_timeout) > limit_ms)
		umidi20_update_timer(&MppTimerCallback, this, time_ms, 1);
}
