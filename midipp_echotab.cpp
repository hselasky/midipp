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

#include <midipp.h>

#include <midipp_mainwindow.h>
#include <midipp_scores.h>
#include <midipp_echotab.h>

static void
MppEchoTabCallback(void *arg)
{
	MppEchoTab *met = (MppEchoTab *)arg;
	MppMainWindow *mw = met->mw;
	MppScoreMain *sm = mw->scores_main[met->echo_val.view];
	uint32_t val;
	int found_key;
	int found_chan;
	int n;
	int lowest;

	pthread_mutex_lock(&mw->mtx);
	if (met->echo_enabled == 0)
		goto done;
	if (met->echo_dirty) {
		met->echo_dirty = 0;
		goto done;
	}
	if (met->echo_st.seq >= met->echo_val.num_echo)
		goto done;
	met->echo_st.seq++;

	lowest = 255;

	for (n = 0; n != MPP_PRESSED_MAX; n++) {

		val = sm->pressedKeys[n];

		if (val != 0) {
			found_chan = (val >> 16) & 0xFF;
			if (met->echo_val.in_channel == found_chan) {
				found_key = (val >> 8) & 0xFF;
				if (found_key < lowest)
					lowest = found_key;
			}
		}
	}

	if (lowest == 255)
		goto done;

	switch (met->echo_val.mode) {
	case ME_MODE_SLIDE:
		break;
	default:
		met->echo_st.pressed = 0;
		break;
	}

	found_key = -1;
	found_chan = -1;

	for (n = 0; n != MPP_PRESSED_MAX; n++) {

		val = sm->pressedKeys[met->echo_st.pressed];

		met->echo_st.pressed++;
		if (met->echo_st.pressed >= MPP_PRESSED_MAX)
			met->echo_st.pressed = 0;

		if (val == 0)
			continue;

		found_chan = (val >> 16) & 0xFF;
		if (met->echo_val.in_channel != found_chan)
			continue;

		found_key = (val >> 8) & 0x7F;

		switch (met->echo_val.mode) {
		case ME_MODE_DEFAULT:
			met->echoKey(found_key);
			break;
		case ME_MODE_BASE_ONLY:
			if ((found_key != lowest) &&
			    (((found_key - lowest) % 12) == 0)) {
				met->echoKey(lowest);
				met->echoKey(found_key);
				goto done;
			}
			break;
		case ME_MODE_SLIDE:
			met->echoKey(found_key);
			goto done;
		default:
			break;
		}
	}
done:
	met->echo_st.amp = ((met->echo_st.amp * met->echo_val.amp_fact) / 128) +
	    (127 * mw->noise8(met->echo_val.amp_rand));

	if (met->echo_st.amp > (127 * 128))
		met->echo_st.amp = 127 * 128;

	pthread_mutex_unlock(&mw->mtx);
}

MppEchoTab :: MppEchoTab(QWidget *parent, MppMainWindow *_mw)
  : QWidget(parent)
{
	uint32_t n;

	/* set memory default */

	memset(auto_zero_start, 0, auto_zero_end - auto_zero_start);

	mw = _mw;

	gl = new QGridLayout(this);

	spn_echo_ival_init = new QSpinBox();
	spn_echo_ival_init->setMaximum(8000);
	spn_echo_ival_init->setMinimum(0);

	spn_echo_ival_repeat = new QSpinBox();
	spn_echo_ival_repeat->setMaximum(8000);
	spn_echo_ival_repeat->setMinimum(0);

	spn_echo_ival_rand = new QSpinBox();
	spn_echo_ival_rand->setMaximum(8000);
	spn_echo_ival_rand->setMinimum(0);

	spn_echo_amp_init = new QSpinBox();
	spn_echo_amp_init->setMaximum(128);
	spn_echo_amp_init->setMinimum(0);

	spn_echo_amp_fact = new QSpinBox();
	spn_echo_amp_fact->setMaximum(128);
	spn_echo_amp_fact->setMinimum(0);

	spn_echo_amp_rand = new QSpinBox();
	spn_echo_amp_rand->setMaximum(128);
	spn_echo_amp_rand->setMinimum(0);

	spn_echo_num = new QSpinBox();
	spn_echo_num->setMaximum(1000);
	spn_echo_num->setMinimum(0);

	spn_echo_transpose = new QSpinBox();
	spn_echo_transpose->setMaximum(127);
	spn_echo_transpose->setMinimum(-127);

	spn_echo_in_channel = new QSpinBox();
	spn_echo_in_channel->setMaximum(15);
	spn_echo_in_channel->setMinimum(0);

	spn_echo_out_channel = new QSpinBox();
	spn_echo_out_channel->setMaximum(15);
	spn_echo_out_channel->setMinimum(0);

	spn_echo_view = new QSpinBox();
	spn_echo_view->setMaximum(MPP_MAX_VIEWS - 1);
	spn_echo_view->setMinimum(0);

	lbl_echo_mode = new QLabel();

	handle_echo_reset();

	connect(spn_echo_ival_init, SIGNAL(valueChanged(int)), this, SLOT(handle_echo_generate(int)));
	connect(spn_echo_ival_repeat, SIGNAL(valueChanged(int)), this, SLOT(handle_echo_generate(int)));
	connect(spn_echo_ival_rand, SIGNAL(valueChanged(int)), this, SLOT(handle_echo_generate(int)));
	connect(spn_echo_amp_init, SIGNAL(valueChanged(int)), this, SLOT(handle_echo_generate(int)));
	connect(spn_echo_amp_fact, SIGNAL(valueChanged(int)), this, SLOT(handle_echo_generate(int)));
	connect(spn_echo_amp_rand, SIGNAL(valueChanged(int)), this, SLOT(handle_echo_generate(int)));
	connect(spn_echo_num, SIGNAL(valueChanged(int)), this, SLOT(handle_echo_generate(int)));
	connect(spn_echo_transpose, SIGNAL(valueChanged(int)), this, SLOT(handle_echo_generate(int)));
	connect(spn_echo_in_channel, SIGNAL(valueChanged(int)), this, SLOT(handle_echo_generate(int)));
	connect(spn_echo_out_channel, SIGNAL(valueChanged(int)), this, SLOT(handle_echo_generate(int)));
	connect(spn_echo_view, SIGNAL(valueChanged(int)), this, SLOT(handle_echo_generate(int)));

	lbl_echo_title = new QLabel(tr("- Echo -"));
	lbl_echo_status = new QLabel(tr("OFF"));

	lbl_echo_ival_init = new QLabel(tr("Initial interval (0 .. 8000ms)"));
	lbl_echo_ival_repeat = new QLabel(tr("Interval repetition (0 .. 8000ms)"));
	lbl_echo_ival_rand = new QLabel(tr("Interval randomness (0 .. 8000ms)"));
	lbl_echo_amp_init = new QLabel(tr("Initial amplitude (0 .. 128)"));
	lbl_echo_amp_fact = new QLabel(tr("Amplitude factor (0 .. 128)"));
	lbl_echo_amp_rand = new QLabel(tr("Amplitude randomness (0 .. 128)"));
	lbl_echo_num = new QLabel(tr("Number of echos (0 .. 1000)"));
	lbl_echo_transpose = new QLabel(tr("Transpose echo (-127 .. 127)"));
	lbl_echo_in_channel = new QLabel(tr("Input channel number (0 .. 15)"));
	lbl_echo_out_channel = new QLabel(tr("Output channel number (0 .. 15)"));
	lbl_echo_view = new QLabel(tr("Input view number"));

	but_echo_mode = new QPushButton(tr("Mode"));
	connect(but_echo_mode, SIGNAL(pressed()), this, SLOT(handle_echo_mode()));

	but_echo_enable = new QPushButton(tr("Echo"));
	connect(but_echo_enable, SIGNAL(pressed()), this, SLOT(handle_echo_enable()));

	but_echo_reset = new QPushButton(tr("Reset"));
	connect(but_echo_reset, SIGNAL(pressed()), this, SLOT(handle_echo_reset()));

	gl->addWidget(lbl_echo_title, 0, 4, 1, 4, Qt::AlignHCenter|Qt::AlignVCenter);

	n = 1;

	gl->addWidget(lbl_echo_ival_init, n, 4, 1, 2);
	gl->addWidget(spn_echo_ival_init, n, 6, 1, 2);

	n++;
	gl->addWidget(lbl_echo_ival_repeat, n, 4, 1, 2);
	gl->addWidget(spn_echo_ival_repeat, n, 6, 1, 2);

	n++;
	gl->addWidget(lbl_echo_ival_rand, n, 4, 1, 2);
	gl->addWidget(spn_echo_ival_rand, n, 6, 1, 2);

	n++;
	gl->addWidget(lbl_echo_amp_init, n, 4, 1, 2);
	gl->addWidget(spn_echo_amp_init, n, 6, 1, 2);

	n++;
	gl->addWidget(lbl_echo_amp_fact, n, 4, 1, 2);
	gl->addWidget(spn_echo_amp_fact, n, 6, 1, 2);

	n++;
	gl->addWidget(lbl_echo_amp_rand, n, 4, 1, 2);
	gl->addWidget(spn_echo_amp_rand, n, 6, 1, 2);

	n++;
	gl->addWidget(lbl_echo_num, n, 4, 1, 2);
	gl->addWidget(spn_echo_num, n, 6, 1, 2);

	n++;
	gl->addWidget(lbl_echo_transpose, n, 4, 1, 2);
	gl->addWidget(spn_echo_transpose, n, 6, 1, 2);

	n++;
	gl->addWidget(lbl_echo_in_channel, n, 4, 1, 2);
	gl->addWidget(spn_echo_in_channel, n, 6, 1, 2);

	n++;
	gl->addWidget(lbl_echo_out_channel, n, 4, 1, 2);
	gl->addWidget(spn_echo_out_channel, n, 6, 1, 2);

	n++;
	gl->addWidget(lbl_echo_view, n, 4, 1, 2);
	gl->addWidget(spn_echo_view, n, 6, 1, 2);

	n++;
	gl->setRowStretch(n, 5);

	n+=5;
	gl->addWidget(but_echo_reset, n, 0, 1, 2);

	gl->addWidget(but_echo_enable, 1, 0, 1, 2);
	gl->addWidget(lbl_echo_status, 1, 2, 1, 2);
	gl->addWidget(but_echo_mode, 2, 0, 1, 2);
	gl->addWidget(lbl_echo_mode, 2, 2, 1, 2);

	handle_echo_generate(0);
}

MppEchoTab :: ~MppEchoTab()
{
	umidi20_unset_timer(&MppEchoTabCallback, this);
}

/* must be called locked */
void
MppEchoTab :: updateVel(int vel)
{
	if (vel < 0)
		vel = 0;
	if (vel > 127)
		vel = 127;

	echo_st.seq = 0;
	echo_st.amp = vel * echo_val.amp_init;
}

/* must be called locked */
void
MppEchoTab :: echoKey(int key)
{
	uint32_t delay;

	key += echo_val.transpose;

	/* range check */
	if (key < 0)
		return;
	if (key > 127)
		return;

	delay = echo_val.ival_init +
	    (echo_val.ival_rand * mw->noise8(128)) / 128;

	mw->output_key(echo_val.out_channel, key, echo_st.amp / 128, delay, 0);

	delay += echo_val.ival_repeat / 2;

	mw->output_key(echo_val.out_channel, key, 0, delay, 0);
}

void
MppEchoTab :: handle_echo_reset()
{
	spn_echo_ival_init->setValue(30);
	spn_echo_ival_repeat->setValue(100);
	spn_echo_ival_rand->setValue(2);
	spn_echo_amp_init->setValue(128);
	spn_echo_amp_fact->setValue(110);
	spn_echo_amp_rand->setValue(2);
	spn_echo_num->setValue(20);
	spn_echo_transpose->setValue(12);
	spn_echo_in_channel->setValue(0);
	spn_echo_out_channel->setValue(15);
	spn_echo_view->setValue(0);

	echo_mode = ME_MODE_MAX - 1;

	handle_echo_mode();
}

void
MppEchoTab :: handle_echo_enable()
{
	pthread_mutex_lock(&mw->mtx);
	echo_enabled ^= 1;
	handle_echo_update();
	pthread_mutex_unlock(&mw->mtx);

	lbl_echo_status->setText(tr(echo_enabled ? "ON" : "OFF"));
}

void
MppEchoTab :: handle_echo_generate(int)
{
	struct MppEchoValues temp;

	memset(&temp, 0, sizeof(temp));

	temp.ival_init = spn_echo_ival_init->value();
	temp.ival_repeat = spn_echo_ival_repeat->value();
	temp.ival_rand = spn_echo_ival_rand->value();
	temp.amp_init = spn_echo_amp_init->value();
	temp.amp_fact = spn_echo_amp_fact->value();
	temp.amp_rand = spn_echo_amp_rand->value();
	temp.num_echo = spn_echo_num->value();
	temp.transpose = spn_echo_transpose->value();
 	temp.in_channel = spn_echo_in_channel->value();
 	temp.out_channel = spn_echo_out_channel->value();	
 	temp.view = spn_echo_view->value();
	temp.mode = echo_mode;

	pthread_mutex_lock(&mw->mtx);
	echo_val = temp;
	handle_echo_update();
	pthread_mutex_unlock(&mw->mtx);
}

void
MppEchoTab :: watchdog()
{

}

/* must be called locked */
void
MppEchoTab :: handle_echo_update()
{
	echo_dirty = 1;

	if (echo_enabled)
		umidi20_set_timer(&MppEchoTabCallback, this, echo_val.ival_repeat);
	else
		umidi20_set_timer(&MppEchoTabCallback, this, 0);
}

void
MppEchoTab :: handle_echo_mode()
{
	echo_mode++;
	if (echo_mode >= ME_MODE_MAX)
		echo_mode = 0;

	switch (echo_mode) {
	case ME_MODE_DEFAULT:
		lbl_echo_mode->setText(tr("DEFAULT"));
		break;
	case ME_MODE_BASE_ONLY:
		lbl_echo_mode->setText(tr("BASE ONLY"));
		break;
	case ME_MODE_SLIDE:
		lbl_echo_mode->setText(tr("SLIDE"));
		break;
	default:
		lbl_echo_mode->setText(tr("UNKNOWN"));
		break;
	}

	handle_echo_generate(0);
}
