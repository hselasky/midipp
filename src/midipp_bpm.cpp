/*-
 * Copyright (c) 2011-2021 Hans Petter Selasky. All rights reserved.
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

#include "midipp_buttonmap.h"
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

	mw->atomic_lock();
	mb->handle_callback_locked(0);
	mw->atomic_unlock();
}

void
MppBpm :: handle_callback_locked(int force)
{
	MppScoreMain *sm;

	if (force != 0 || (mw->midiTriggered != 0 && (toggle != 0 || enabled != 0))) {
		toggle = !toggle;

		for (unsigned n = 0; n != MPP_MAX_VIEWS; n++) {
			if (view_out[n] == 0)
				continue;

			sm = mw->scores_main[n];

			if (toggle != 0)
				sm->handleKeyPress(key, amp, 0);
			else
				sm->handleKeyRelease(key, amp, 0);
		}

		/* check if we should send a beat event */
		if (beat != 0 && toggle != 0)
			mw->send_byte_event_locked(0xF8);
	}
}

MppBpm :: MppBpm(MppMainWindow *parent)
  : QDialog(parent)
{
	QLabel *lbl;
	char buf[64];
	int n;

	mw = parent;

	mbm_generator = new MppButtonMap("BPM generator\0" "OFF\0" "ON\0", 2, 2);
	connect(mbm_generator, SIGNAL(selectionChanged(int)), this, SLOT(handle_bpm_enable(int)));

	gl = new QGridLayout(this);

	tim_config = new QTimer(this);
	tim_config->setSingleShot(1);
	connect(tim_config, SIGNAL(timeout()), this, SLOT(handle_config_apply()));

	spn_bpm_value = new QSpinBox();
	spn_bpm_value->setRange(1, 6000);
	spn_bpm_value->setSuffix(tr(" bpm"));
	connect(spn_bpm_value, SIGNAL(valueChanged(int)), this, SLOT(handle_config_change()));

	spn_bpm_amp = new QSpinBox();
	spn_bpm_amp->setRange(1, 127);
	connect(spn_bpm_amp, SIGNAL(valueChanged(int)), this, SLOT(handle_config_change()));

	spn_bpm_key = new MppSpinBox(0,0);
	connect(spn_bpm_key, SIGNAL(valueChanged(int)), this, SLOT(handle_config_change()));

	spn_bpm_period_ref = new QSpinBox();
	spn_bpm_period_ref->setRange(1, 6000);
	spn_bpm_period_ref->setSuffix(tr(" bpm"));
	connect(spn_bpm_period_ref, SIGNAL(valueChanged(int)), this, SLOT(handle_config_change()));

	spn_bpm_period_cur = new QSpinBox();
	spn_bpm_period_cur->setRange(0, 60000);
	spn_bpm_period_cur->setSuffix(tr(" ms"));
	connect(spn_bpm_period_cur, SIGNAL(valueChanged(int)), this, SLOT(handle_config_change()));

	for (n = 0; n != MPP_MAX_VIEWS; n++) {
		cbx_out_view[n] = new MppCheckBox();
		connect(cbx_out_view[n], SIGNAL(stateChanged(int,int)), this, SLOT(handle_config_change()));
		cbx_sync_view[n] = new MppCheckBox();
		connect(cbx_sync_view[n], SIGNAL(stateChanged(int,int)), this, SLOT(handle_config_change()));
	}

	cbx_midi_beat = new MppCheckBox();
	connect(cbx_midi_beat, SIGNAL(stateChanged(int,int)), this, SLOT(handle_config_change()));

	but_reset_all = new QPushButton(tr("Reset all"));
	but_done_all = new QPushButton(tr("Close"));

	connect(but_reset_all, SIGNAL(released()), this, SLOT(handle_reset_all()));
	connect(but_done_all, SIGNAL(released()), this, SLOT(handle_done_all()));

	setWindowTitle(tr("BPM generator settings"));
	setWindowIcon(QIcon(MppIconFile));

	gb_ctrl = new MppGroupBox(tr("BPM control"));

	gb_ctrl->addWidget(new QLabel(tr("Value (1..6000)")), 0, 0, 1, 1);
	gb_ctrl->addWidget(spn_bpm_value, 0, 1, 1, 1);

	gb_ctrl->addWidget(new QLabel(tr("Amplitude (1..127)")), 1, 0, 1, 1);
	gb_ctrl->addWidget(spn_bpm_amp, 1, 1, 1, 1);

	gb_ctrl->addWidget(new QLabel(tr("Play key")), 2, 0, 1, 1);
	gb_ctrl->addWidget(spn_bpm_key, 2, 1, 1, 1);

	gb_ctrl->addWidget(new QLabel(tr("Output to MIDI beat event")), 3, 0, 1, 1);
	gb_ctrl->addWidget(cbx_midi_beat, 3, 1, 1, 1);

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

	gl->addWidget(but_reset_all, 2, 1, 1, 1);
	gl->addWidget(but_done_all, 2, 3, 1, 1);

	gl->setRowStretch(3, 1);
	gl->setColumnStretch(4, 1);

	toggle = 0;

	handle_reset_all();

	umidi20_set_timer(&MppTimerCallback, this, 1000);
}

MppBpm :: ~MppBpm()
{
	umidi20_unset_timer(&MppTimerCallback, this);

	tim_config->stop();
}

void
MppBpm :: handle_reload_all()
{
	int value[5];

	mw->atomic_lock();
	value[0] = bpm_cur;
	value[1] = amp;
	value[2] = key;
	value[3] = period_ref;
	value[4] = period_cur;
	mw->atomic_unlock();

	spn_bpm_value->setValue(value[0]);
	spn_bpm_amp->setValue(value[1]);
	spn_bpm_key->setValue(value[2]);
	spn_bpm_period_ref->setValue(value[3]);
	spn_bpm_period_cur->setValue(value[4]);
}

void
MppBpm :: handle_reset_all()
{
	for (unsigned n = 0; n != MPP_MAX_VIEWS; n++) {
		MPP_BLOCKED(cbx_out_view[n], setCheckState(Qt::Unchecked));
		MPP_BLOCKED(cbx_sync_view[n], setCheckState(Qt::Unchecked));
	}

	MPP_BLOCKED(cbx_midi_beat, setCheckState(Qt::Unchecked));
	MPP_BLOCKED(mbm_generator, setSelection(0));

	mw->atomic_lock();

	for (unsigned n = 0; n != MPP_MAX_VIEWS; n++) {
		view_out[n] = 0;
		view_sync[n] = 0;
	}

	beat = 0;
	enabled = 0;
	bpm_cur = 120;
	bpm_other = 120;
	amp = 96;
	key = MPP_DEFAULT_BASE_KEY;
	period_ref = 120;
	period_cur = 0;

	handle_update();

	mw->atomic_unlock();

	handle_reload_all();
}

void
MppBpm :: handle_done_all()
{
	accept();
}

void
MppBpm :: handle_bpm_enable(int value)
{
	mw->atomic_lock();
	enabled = value;
	handle_update(enabled);
	mw->atomic_unlock();
}

/* must be called locked */
void
MppBpm :: handle_update(int restart)
{
	uint32_t time_ms;

	if (restart != 0)
		bpm_other = bpm_cur;

	/* compute timer period in milliseconds */
	time_ms = bpm_get() / (2 * bpm_other);
	if (time_ms == 0)
		time_ms = 1;

	umidi20_update_timer(&MppTimerCallback, this, time_ms, (restart != 0));
}

void
MppBpm :: handle_config_apply()
{
	uint32_t val[6];
	uint32_t tmp[MPP_MAX_VIEWS];
	uint32_t sync[MPP_MAX_VIEWS];
	uint32_t n;

	for (n = 0; n != MPP_MAX_VIEWS; n++) {
		tmp[n] = (cbx_out_view[n]->checkState() != Qt::Unchecked);
		sync[n] = (cbx_sync_view[n]->checkState() != Qt::Unchecked);
	}

	val[0] = spn_bpm_value->value();
	val[1] = spn_bpm_amp->value();
	val[2] = spn_bpm_key->value();
	val[3] = spn_bpm_period_ref->value();
	val[4] = spn_bpm_period_cur->value();
	val[5] = (cbx_midi_beat->checkState() != Qt::Unchecked);

	mw->atomic_lock();
	bpm_other = bpm_cur = val[0];
	amp = val[1];
	key = val[2];
	period_ref = val[3];
	period_cur = val[4];
	beat = val[5];
	for (n = 0; n != MPP_MAX_VIEWS; n++) {
		view_out[n] = tmp[n];
		view_sync[n] = sync[n];
	}
	handle_update();
	mw->atomic_unlock();

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
	if (view_index < 0 || view_index >= MPP_MAX_VIEWS ||
	    view_sync[view_index] == 0)
		return;

	if (toggle != 0)
		handle_callback_locked(1);
	handle_callback_locked(1);
}
