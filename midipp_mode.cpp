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

#include <midipp_mode.h>
#include <midipp_spinbox.h>

MppMode :: MppMode(uint8_t _vi)
{
	uint32_t x;
	char buf[64];

	gl = new QGridLayout(this);

	input_mask = 0;
	base_key = 0;
	cmd_key = 0;
	key_delay = 0;
	view_index = _vi;
	key_mode = 0;
	channel = 0;

	setWindowTitle(tr("View ") + QChar('A' + _vi) + tr(" mode"));
	setWindowIcon(QIcon(QString(MPP_ICON_FILE)));

	lbl_input = new QLabel(tr(" - Input devices - "));
	lbl_base = new QLabel(tr("Base play key"));
	lbl_cmd = new QLabel(tr("Base command key"));
	lbl_delay = new QLabel(tr("Random key delay (0..255)"));
	lbl_mode = new QLabel(tr("ALL"));
	lbl_chan = new QLabel(tr("Synth channel"));

	for (x = 0; x != MPP_MAX_DEVS; x++) {

		snprintf(buf, sizeof(buf), "Dev%d", x);

		cbx_dev[x] = new QCheckBox();
		cbx_dev[x]->setChecked(view_index == 0);

		lbl_dev[x] = new QLabel(tr(buf));
	}

	but_mode = new QPushButton(tr("Key Press Mode"));
	but_done = new QPushButton(tr("Close"));
	but_set_all = new QPushButton(tr("All devs"));
	but_clear_all = new QPushButton(tr("No devs"));

	connect(but_mode, SIGNAL(released()), this, SLOT(handle_mode()));
	connect(but_done, SIGNAL(released()), this, SLOT(handle_done()));
	connect(but_set_all, SIGNAL(released()), this, SLOT(handle_set_all_devs()));
	connect(but_clear_all, SIGNAL(released()), this, SLOT(handle_clear_all_devs()));

	spn_cmd = new MppSpinBox();
	spn_cmd->setRange(0, 127);
	spn_cmd->setValue(C3);

	spn_base = new MppSpinBox();
	spn_base->setRange(0, 127);
	spn_base->setValue(C4);

	spn_delay = new QSpinBox();
	spn_delay->setRange(0, 255);
	spn_delay->setValue(25);
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

	gl->addWidget(but_mode, 4, 2, 1, 1);
	gl->addWidget(lbl_mode, 4, 3, 1, 1);

	gl->addWidget(but_set_all, MPP_MAX_DEVS + 1, 0, 1, 1);
	gl->addWidget(but_clear_all, MPP_MAX_DEVS + 1, 1, 1, 1);

	gl->addWidget(but_done, MPP_MAX_DEVS + 1, 3, 1, 1);
}

MppMode :: ~MppMode()
{

}

void
MppMode :: handle_mode()
{
	key_mode++;
	if (key_mode >= MM_PASS_MAX)
		key_mode = 0;

	switch (key_mode) {
	case MM_PASS_ALL:
		lbl_mode->setText(tr("ALL"));
		break;
	case MM_PASS_NONE_FIXED:
		lbl_mode->setText(tr("FIXED"));
		break;
	case MM_PASS_ONE_MIXED:
		lbl_mode->setText(tr("MIXED"));
		break;
	case MM_PASS_NONE_TRANS:
		lbl_mode->setText(tr("TRANSP"));
		break;
	case MM_PASS_NONE_CHORD:
		lbl_mode->setText(tr("CHORD"));
		break;
	default:
		lbl_mode->setText(tr("UNKNOWN"));
		break;
	}
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
	uint32_t x;

	input_mask = 0;

	for (x = 0; x != MPP_MAX_DEVS; x++) {
		if (cbx_dev[x]->isChecked())
			input_mask |= (1U << x);
	}

	base_key = spn_base->value();
	cmd_key = spn_cmd->value();
	key_delay = spn_delay->value();
	channel = spn_chan->value();

	accept();
}
