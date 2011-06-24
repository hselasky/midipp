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

#include <midipp_bpm.h>
#include <midipp_mainwindow.h>
#include <midipp_pattern.h>
#include <midipp_scores.h>

static void
MppTimerCallback(void *arg)
{
	MppBpm *mb = (MppBpm *)arg;
	MppMainWindow *mw = mb->mw;
	MppScoreMain *sm;
	int key;
	int n;

	pthread_mutex_lock(&mw->mtx);
	if (mb->enabled &&
	    mw->midiTriggered &&
	    mb->led_bpm_pattern->matchPattern(mb->time)) {

		key = mw->playKey;

		for (n = 0; n != MPP_MAX_VIEWS; n++) {
			if (mb->view[n] == 0)
				continue;

			sm = mw->scores_main[n];
			sm->handleKeyPress(key, 127, 0);
			sm->handleKeyRelease(key, mb->duty_ticks);
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

	lbl_bpm_pattern = new QLabel(tr("BPM pattern: a+bc+d"));
	lbl_bpm_value = new QLabel(tr("BPM value (1..6000)"));
	lbl_bpm_duty = new QLabel(tr("BPM duty (1..99)"));

	for (n = 0; n != MPP_MAX_VIEWS; n++) {
		snprintf(buf, sizeof(buf), "BPM to view %c", 'A' + n);
		lbl_view[n] = new QLabel(tr(buf));
	}

	led_bpm_pattern = new MppPattern();

	spn_bpm_value = new QSpinBox();
	spn_bpm_value->setRange(1, 6000);
	connect(spn_bpm_value, SIGNAL(valueChanged(int)), this, SLOT(handle_bpm_value(int)));

	spn_bpm_duty = new QSpinBox();
	spn_bpm_duty->setRange(1, 99);
	connect(spn_bpm_duty, SIGNAL(valueChanged(int)), this, SLOT(handle_bpm_duty(int)));

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

	for (n = 0; n != MPP_MAX_VIEWS; n++) {
		gl->addWidget(lbl_view[n], 3 + n, 0, 1, 2);
		gl->addWidget(cbx_view[n], 3 + n, 2, 1, 1);
	}

	n = 3 + MPP_MAX_VIEWS;

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
MppBpm :: handle_reset_all()
{
	int n;

	spn_bpm_value->setValue(60);
	spn_bpm_duty->setValue(50);
	but_bpm_enable->setText(tr("Enable"));

	for (n = 0; n != MPP_MAX_VIEWS; n++)
		cbx_view[n]->setCheckState(Qt::Unchecked);


	pthread_mutex_lock(&mw->mtx);

	for (n = 0; n != MPP_MAX_VIEWS; n++)
		view[n] = 0;

	time = 0;
	enabled = 0;
	bpm = 60;
	duty = 50;
	duty_ticks = 0;

	handle_update();

	pthread_mutex_unlock(&mw->mtx);
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
		handle_update();
		pthread_mutex_unlock(&mw->mtx);

		but_bpm_enable->setText(tr("Enable"));
	} else {
		pthread_mutex_lock(&mw->mtx);
		enabled = 1;
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
		i = 60000 / temp;
		if (i == 0)
			i = 1;
	} else {
		i = 0;
	}

	time = 0;
	duty_ticks = (i * duty) / (2 * 100);

	umidi20_set_timer(&MppTimerCallback, this, i);
}

void
MppBpm :: handle_bpm_value(int val)
{
	pthread_mutex_lock(&mw->mtx);
	if (bpm != (uint32_t)val) {
		bpm = (uint32_t)val;
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
		handle_update();
	}
	pthread_mutex_unlock(&mw->mtx);
}
