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
#include <midipp_echotab.h>

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
	gl->setRowStretch(n, 5);

	n+=5;
	gl->addWidget(but_echo_reset, n, 0, 1, 2);

	gl->addWidget(but_echo_enable, 1, 0, 1, 2);
	gl->addWidget(lbl_echo_status, 1, 2, 1, 2);

	handle_echo_generate(0);
}

MppEchoTab :: ~MppEchoTab()
{

}

void
MppEchoTab :: handle_echo_reset()
{
	spn_echo_ival_init->setValue(50);
	spn_echo_ival_repeat->setValue(50);
	spn_echo_ival_rand->setValue(2);
	spn_echo_amp_init->setValue(64);
	spn_echo_amp_fact->setValue(100);
	spn_echo_amp_rand->setValue(2);
	spn_echo_num->setValue(8);
	spn_echo_transpose->setValue(0);
	spn_echo_in_channel->setValue(0);
	spn_echo_out_channel->setValue(15);
}

void
MppEchoTab :: handle_echo_enable()
{
	pthread_mutex_lock(&mw->mtx);
	echo_enabled ^= 1;
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

	pthread_mutex_lock(&mw->mtx);
	echo_dirty = 1;
	echo_val = temp;
	pthread_mutex_unlock(&mw->mtx);
}

void
MppEchoTab :: watchdog()
{

}
