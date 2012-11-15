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

#include <midipp_buttonmap.h>
#include <midipp_mode.h>
#include <midipp_spinbox.h>
#include <midipp_scores.h>
#include <midipp_mainwindow.h>

MppMode :: MppMode(MppScoreMain *_parent, uint8_t _vi)
{
	uint32_t x;
	char buf[64];

	sm = _parent;
	view_index = _vi;

	gl = new QGridLayout(this);

	setWindowTitle(tr("View ") + QChar('A' + _vi) + tr(" mode"));
	setWindowIcon(QIcon(QString(MPP_ICON_FILE)));

	lbl_input = new QLabel(tr(" - Input devices - "));
	lbl_base = new QLabel(tr("Base play key"));
	lbl_cmd = new QLabel(tr("Base command key"));
	lbl_delay = new QLabel(tr("Random key delay (0..255)"));
	lbl_chan = new QLabel(tr("Synth channel"));

	for (x = 0; x != MPP_MAX_DEVS; x++) {

		snprintf(buf, sizeof(buf), "Dev%d", x);

		cbx_dev[x] = new QCheckBox();
		cbx_dev[x]->setChecked(view_index == 0);

		lbl_dev[x] = new QLabel(tr(buf));
	}

	but_mode = new MppButtonMap("Key mode\0" "ALL\0" "MIXED\0" "FIXED\0" "TRANSP\0" "CHORD\0", 5, 3);

	but_done = new QPushButton(tr("Close"));
	connect(but_done, SIGNAL(released()), this, SLOT(handle_done()));

	but_set_all = new QPushButton(tr("All devs"));
	connect(but_set_all, SIGNAL(released()), this, SLOT(handle_set_all_devs()));

	but_clear_all = new QPushButton(tr("No devs"));
	connect(but_clear_all, SIGNAL(released()), this, SLOT(handle_clear_all_devs()));

	spn_cmd = new MppSpinBox();
	spn_cmd->setRange(0, 127);
	spn_cmd->setValue(0);

	spn_base = new MppSpinBox();
	spn_base->setRange(0, 127);
	spn_base->setValue(0);

	spn_delay = new QSpinBox();
	spn_delay->setRange(0, 255);
	spn_delay->setValue(0);
	spn_delay->setSuffix(tr(" ms"));

	spn_chan = new QSpinBox();
	spn_chan->setRange(0, 15);
	spn_chan->setValue(0);

	for (x = 0; x != MPP_MAX_DEVS; x++) {
		gl->addWidget(lbl_dev[x], x + 1, 0, 1, 1);
		gl->addWidget(cbx_dev[x], x + 1, 1, 1, 1);
	}

	gl->addWidget(lbl_input, 0, 0, 1, 2);

	gl->addWidget(lbl_cmd, 0, 2, 1, 1);
	gl->addWidget(spn_cmd, 0, 3, 1, 1);

	gl->addWidget(lbl_base, 1, 2, 1, 1);
	gl->addWidget(spn_base, 1, 3, 1, 1);

	gl->addWidget(lbl_delay, 2, 2, 1, 1);
	gl->addWidget(spn_delay, 2, 3, 1, 1);

	gl->addWidget(lbl_chan, 3, 2, 1, 1);
	gl->addWidget(spn_chan, 3, 3, 1, 1);

	gl->addWidget(but_mode, 4, 2, 3, 2);
	gl->addWidget(but_set_all, MPP_MAX_DEVS + 1, 0, 1, 1);
	gl->addWidget(but_clear_all, MPP_MAX_DEVS + 1, 1, 1, 1);

	gl->addWidget(but_done, MPP_MAX_DEVS + 1, 3, 1, 1);
}

MppMode :: ~MppMode()
{

}

void
MppMode :: update_all(void)
{
	int base_key;
	int cmd_key;
	int key_delay;
	int channel;
	int key_mode;
	int input_mask;
	int x;

	pthread_mutex_lock(&sm->mainWindow->mtx);
	base_key = sm->baseKey;
	cmd_key = sm->cmdKey;
	key_delay = sm->delayNoise;
	channel = sm->synthChannel;
	key_mode = sm->keyMode;
	input_mask = sm->devInputMask;
	pthread_mutex_unlock(&sm->mainWindow->mtx);

	for (x = 0; x != MPP_MAX_DEVS; x++)
		cbx_dev[x]->setChecked((input_mask >> x) & 1);

	spn_base->setValue(base_key);
	spn_cmd->setValue(cmd_key);
	spn_delay->setValue(key_delay);
	spn_chan->setValue(channel);
	but_mode->handle_pressed(key_mode);
}

void
MppMode :: handle_set_all_devs()
{
	uint32_t x;

	for (x = 0; x != MPP_MAX_DEVS; x++)
		cbx_dev[x]->setChecked(1);
}

void
MppMode :: handle_clear_all_devs()
{
	uint32_t x;

	for (x = 0; x != MPP_MAX_DEVS; x++)
		cbx_dev[x]->setChecked(0);
}

void
MppMode :: handle_done()
{
	int base_key;
	int cmd_key;
	int key_delay;
	int channel;
	int key_mode;
	int input_mask;
	int x;

	input_mask = 0;

	for (x = 0; x != MPP_MAX_DEVS; x++) {
		if (cbx_dev[x]->isChecked())
			input_mask |= (1U << x);
	}

	base_key = spn_base->value();
	cmd_key = spn_cmd->value();
	key_delay = spn_delay->value();
	channel = spn_chan->value();
	key_mode = but_mode->currSelection;

	if (key_mode < 0 || key_mode >= MM_PASS_MAX)
		key_mode = 0;

	pthread_mutex_lock(&sm->mainWindow->mtx);
	sm->baseKey = base_key;
	sm->cmdKey = cmd_key;
	sm->delayNoise = key_delay;
	sm->synthChannel = channel;
	sm->keyMode = key_mode;
	sm->devInputMask = input_mask;
	pthread_mutex_unlock(&sm->mainWindow->mtx);

	accept();
}
