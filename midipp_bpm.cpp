/*-
 * Copyright (c) 2011 Hans Petter Selasky. All rights reserved.
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
#include "midipp_pattern.h"
#include "midipp_scores.h"
#include "midipp_spinbox.h"

static void
MppTimerCallback(void *arg)
{
	MppBpm *mb = (MppBpm *)arg;
	MppMainWindow *mw = mb->mw;
	MppScoreMain *sm;
	int n;

	pthread_mutex_lock(&mw->mtx);
	if (mb->enabled &&
	    mb->duty_ticks &&
	    mb->amp &&
	    mw->midiTriggered &&
	    mb->led_bpm_pattern->matchPattern(mb->time)) {

		if (mb->skip_bpm) {
			mb->skip_bpm = 0;
			pthread_mutex_unlock(&mw->mtx);
			return;
		}

		for (n = 0; n != MPP_MAX_VIEWS; n++) {
			if (mb->view[n] == 0)
				continue;

			sm = mw->scores_main[n];
			sm->handleKeyPress(mb->key, mb->amp, 0);
			sm->handleKeyRelease(mb->key, mb->duty_ticks);
		}
	}
	mb->time++;
	pthread_mutex_unlock(&mw->mtx);
}

MppBpm :: MppBpm(MppMainWindow *parent)
  : QDialog(parent)
{
	char buf[64];
	int n;

	mw = parent;

	gl = new QGridLayout(this);

	lbl_bpm_pattern = new QLabel(tr("BPM pattern: 1+a+bc+d"));
	lbl_bpm_value = new QLabel(tr("BPM value (1..6000)"));
	lbl_bpm_duty = new QLabel(tr("BPM duty (1..199)"));
	lbl_bpm_amp = new QLabel(tr("BPM amplitude (1..127)"));
	lbl_bpm_key = new QLabel(tr("BPM play key"));
	lbl_bpm_ref = new QLabel(tr("BPM period reference (1..6000)"));
	lbl_bpm_period = new QLabel(tr("BPM period value (0..60000)"));

	for (n = 0; n != MPP_MAX_VIEWS; n++) {
		snprintf(buf, sizeof(buf), "BPM to view %c", 'A' + n);
		lbl_view[n] = new QLabel(tr(buf));
	}

	led_bpm_pattern = new MppPattern();

	spn_bpm_value = new QSpinBox();
	spn_bpm_value->setRange(1, 6000);
	spn_bpm_value->setSuffix(tr(" bpm"));
	connect(spn_bpm_value, SIGNAL(valueChanged(int)), this, SLOT(handle_bpm_value(int)));

	spn_bpm_duty = new QSpinBox();
	spn_bpm_duty->setRange(1, 199);
	spn_bpm_duty->setSuffix(tr(" %"));
	connect(spn_bpm_duty, SIGNAL(valueChanged(int)), this, SLOT(handle_bpm_duty(int)));

	spn_bpm_amp = new QSpinBox();
	spn_bpm_amp->setRange(1, 127);
	connect(spn_bpm_amp, SIGNAL(valueChanged(int)), this, SLOT(handle_bpm_amp(int)));

	spn_bpm_key = new MppSpinBox();
	spn_bpm_key->setRange(0, 127);
	connect(spn_bpm_key, SIGNAL(valueChanged(int)), this, SLOT(handle_bpm_key(int)));

	spn_bpm_ref = new QSpinBox();
	spn_bpm_ref->setRange(1, 6000);
	spn_bpm_ref->setSuffix(tr(" bpm"));
	connect(spn_bpm_ref, SIGNAL(valueChanged(int)), this, SLOT(handle_bpm_ref(int)));

	spn_bpm_period = new QSpinBox();
	spn_bpm_period->setRange(0, 60000);
	spn_bpm_period->setSuffix(tr(" ms"));
	connect(spn_bpm_period, SIGNAL(valueChanged(int)), this, SLOT(handle_bpm_period(int)));

	for (n = 0; n != MPP_MAX_VIEWS; n++) {
		cbx_view[n] = new QCheckBox();
		connect(cbx_view[n], SIGNAL(stateChanged(int)), this, SLOT(handle_view_all(int)));
	}

	but_bpm_enable = new QPushButton(tr("Enable"));
	but_reset_all = new QPushButton(tr("Reset all"));
	but_done_all = new QPushButton(tr("Close"));

	connect(but_bpm_enable, SIGNAL(pressed()), this, SLOT(handle_bpm_enable()));
	connect(but_reset_all, SIGNAL(pressed()), this, SLOT(handle_reset_all()));
	connect(but_done_all, SIGNAL(released()), this, SLOT(handle_done_all()));

	setWindowTitle(tr("BPM settings"));
	setWindowIcon(QIcon(QString(MPP_ICON_FILE)));

	gl->addWidget(lbl_bpm_pattern, 0, 0, 1, 2);
	gl->addWidget(led_bpm_pattern, 0, 2, 1, 1);

	gl->addWidget(lbl_bpm_value, 1, 0, 1, 2);
	gl->addWidget(spn_bpm_value, 1, 2, 1, 1);

	gl->addWidget(lbl_bpm_duty, 2, 0, 1, 2);
	gl->addWidget(spn_bpm_duty, 2, 2, 1, 1);

	gl->addWidget(lbl_bpm_amp, 3, 0, 1, 2);
	gl->addWidget(spn_bpm_amp, 3, 2, 1, 1);

	gl->addWidget(lbl_bpm_key, 4, 0, 1, 2);
	gl->addWidget(spn_bpm_key, 4, 2, 1, 1);

	gl->addWidget(lbl_bpm_ref, 5, 0, 1, 2);
	gl->addWidget(spn_bpm_ref, 5, 2, 1, 1);

	gl->addWidget(lbl_bpm_period, 6, 0, 1, 2);
	gl->addWidget(spn_bpm_period, 6, 2, 1, 1);

	for (n = 0; n != MPP_MAX_VIEWS; n++) {
		gl->addWidget(lbl_view[n], 7 + n, 0, 1, 2);
		gl->addWidget(cbx_view[n], 7 + n, 2, 1, 1);
	}

	n = 7 + MPP_MAX_VIEWS;

	gl->addWidget(but_bpm_enable, n, 0, 1, 1);
	gl->addWidget(but_reset_all, n, 1, 1, 1);
	gl->addWidget(but_done_all, n, 2, 1, 1);

	handle_reset_all();
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
	value[0] = bpm;
	value[1] = duty;
	value[2] = amp;
	value[3] = key;
	value[4] = ref;
	value[5] = period;
	pthread_mutex_unlock(&mw->mtx);

	spn_bpm_value->setValue(value[0]);
	spn_bpm_duty->setValue(value[1]);
	spn_bpm_amp->setValue(value[2]);
	spn_bpm_key->setValue(value[3]);
	spn_bpm_ref->setValue(value[4]);
	spn_bpm_period->setValue(value[5]);
}

void
MppBpm :: handle_reset_all()
{
	int n;

	but_bpm_enable->setText(tr("Enable"));

	for (n = 0; n != MPP_MAX_VIEWS; n++)
		cbx_view[n]->setCheckState(Qt::Unchecked);

	pthread_mutex_lock(&mw->mtx);

	for (n = 0; n != MPP_MAX_VIEWS; n++)
		view[n] = 0;

	skip_bpm = 0;
	time = 0;
	enabled = 0;
	bpm = 120;
	duty = 50;
	duty_ticks = 0;
	amp = 96;
	key = C4;
	ref = 120;
	period = 0;

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
	if (enabled) {
		pthread_mutex_lock(&mw->mtx);
		enabled = 0;
		skip_bpm = 0;
		handle_update();
		pthread_mutex_unlock(&mw->mtx);

		but_bpm_enable->setText(tr("Enable"));
	} else {
		pthread_mutex_lock(&mw->mtx);
		enabled = 1;
		skip_bpm = 0;
		handle_update();
		pthread_mutex_unlock(&mw->mtx);

		but_bpm_enable->setText(tr("Disable"));
	}
}

void
MppBpm :: handle_view_all(int dummy)
{
	int n;
	uint8_t temp[MPP_MAX_VIEWS];

	for (n = 0; n != MPP_MAX_VIEWS; n++)
		temp[n] = (cbx_view[n]->checkState() != Qt::Unchecked);

	pthread_mutex_lock(&mw->mtx);
	for (n = 0; n != MPP_MAX_VIEWS; n++)
		view[n] = temp[n];
	pthread_mutex_unlock(&mw->mtx);
}



/* must be called locked */
void
MppBpm :: handle_update()
{
	int temp = bpm;
	int i;

	if (temp > 0 && enabled) {
		if (period != 0 && ref != 0) {
			i = (period * ref) / temp;
		} else {
			i = 60000 / temp;
		}

		if (i < 1)
			i = 1;

		duty_ticks = (i * duty) / (2 * 100);
	} else {
		duty_ticks = 0;
		i = 0;
	}

	time = 0;

	umidi20_set_timer(&MppTimerCallback, this, i);
}

void
MppBpm :: handle_bpm_value(int val)
{
	pthread_mutex_lock(&mw->mtx);
	if (bpm != (uint32_t)val) {
		bpm = (uint32_t)val;
		skip_bpm = enabled && mw->midiTriggered;
		handle_update();
	}
	pthread_mutex_unlock(&mw->mtx);

}

void
MppBpm :: handle_bpm_duty(int val)
{
	pthread_mutex_lock(&mw->mtx);
	if (duty != (uint32_t)val) {
		duty = (uint32_t)val;
		skip_bpm = enabled && mw->midiTriggered;
		handle_update();
	}
	pthread_mutex_unlock(&mw->mtx);
}

void
MppBpm :: handle_bpm_amp(int val)
{
	pthread_mutex_lock(&mw->mtx);
	if (amp != (uint8_t)val) {
		amp = (uint8_t)val;
		skip_bpm = enabled && mw->midiTriggered;
		handle_update();
	}
	pthread_mutex_unlock(&mw->mtx);
}

void
MppBpm :: handle_bpm_key(int val)
{
	pthread_mutex_lock(&mw->mtx);
	if (key != (uint8_t)val) {
		key = (uint8_t)val;
		skip_bpm = enabled && mw->midiTriggered;
		handle_update();
	}
	pthread_mutex_unlock(&mw->mtx);
}

void
MppBpm :: handle_bpm_ref(int val)
{
	pthread_mutex_lock(&mw->mtx);
	if (ref != (uint32_t)val) {
		ref = (uint32_t)val;
		skip_bpm = enabled && mw->midiTriggered;
		handle_update();
	}
	pthread_mutex_unlock(&mw->mtx);
}

void
MppBpm :: handle_bpm_period(int val)
{
	pthread_mutex_lock(&mw->mtx);
	if (period != (uint32_t)val) {
		period = (uint32_t)val;
		skip_bpm = enabled && mw->midiTriggered;
		handle_update();
	}
	pthread_mutex_unlock(&mw->mtx);
}
