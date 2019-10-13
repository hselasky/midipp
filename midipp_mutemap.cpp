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

#include "midipp_mainwindow.h"
#include "midipp_mutemap.h"
#include "midipp_groupbox.h"
#include "midipp_checkbox.h"
#include "midipp_buttonmap.h"

MppMuteMap :: MppMuteMap(QWidget *parent, MppMainWindow *_mw, int _devno)
  : QDialog(parent)
{

	mw = _mw;
	devno = _devno;

	gl = new QGridLayout(this);

	gb_mute = new MppGroupBox("MIDI Output Channel Mute Map");
	gb_other = new MppGroupBox("MIDI Output Mute");

	setWindowTitle(tr("MIDI Output Mute Map"));
	setWindowIcon(QIcon(MppIconFile));

	gb_mute->addWidget(new QLabel(tr("Mute enable")),
	    0, 1, 1, 1, Qt::AlignHCenter|Qt::AlignVCenter);
	gb_mute->addWidget(new QLabel(tr("Mute enable")),
	    0, 3, 1, 1, Qt::AlignHCenter|Qt::AlignVCenter);

	gl->addWidget(gb_mute, 0, 0, 1, 4);
	gl->addWidget(gb_other, 1, 0, 1, 4);

	but_reset_all = new QPushButton(tr("Reset"));
	but_close_all = new QPushButton(tr("Close"));

	gl->setRowStretch(2, 1);
	gl->setColumnStretch(4, 1);

	gl->addWidget(but_close_all, 3, 2, 1, 2);
	gl->addWidget(but_reset_all, 3, 0, 1, 2);

	connect(but_reset_all, SIGNAL(released()), this, SLOT(handle_reset_all()));
	connect(but_close_all, SIGNAL(released()), this, SLOT(handle_close_all()));

	for (uint8_t n = 0; n != 16; n++) {
		int x_off;
		int y_off;

		cbx_mute[n] = new MppCheckBox();
		connect(cbx_mute[n], SIGNAL(stateChanged(int,int)), this, SLOT(handle_apply_all()));

		x_off = (n & 8) ? 2: 0;
		y_off = (n & 7) + 1;

		gb_mute->addWidget(new QLabel(MppChanName(n)),
		    y_off, x_off + 0, 1, 1, Qt::AlignHCenter|Qt::AlignVCenter);
		gb_mute->addWidget(cbx_mute[n],
		    y_off, x_off + 1, 1, 1, Qt::AlignHCenter|Qt::AlignVCenter);
	}

	cbx_mute_program = new MppButtonMap("Mute MIDI program events\0" "NO\0" "YES\0", 2, 2);
	connect(cbx_mute_program, SIGNAL(selectionChanged(int)), this, SLOT(handle_apply_all()));

	cbx_mute_pedal = new MppButtonMap("Mute MIDI pedal events\0" "NO\0" "YES\0", 2, 2);
	connect(cbx_mute_pedal, SIGNAL(selectionChanged(int)), this, SLOT(handle_apply_all()));

	cbx_mute_non_channel = new MppButtonMap("Mute non-channel MIDI events\0" "NO\0" "YES\0", 2, 2);
	connect(cbx_mute_non_channel, SIGNAL(selectionChanged(int)), this, SLOT(handle_apply_all()));

	cbx_mute_local_keys = new MppButtonMap("Send MIDI local keys command\0" "NONE\0" "ENABLE\0" "DISABLE\0", 3, 3);
	connect(cbx_mute_local_keys, SIGNAL(selectionChanged(int)), this, SLOT(handle_apply_all()));

	cbx_mute_control = new MppButtonMap("Mute MIDI control events\0" "NO\0" "YES\0", 2, 2);
	connect(cbx_mute_control, SIGNAL(selectionChanged(int)), this, SLOT(handle_apply_all()));

	gb_other->addWidget(cbx_mute_program, 0, 0, 1, 1);
	gb_other->addWidget(cbx_mute_pedal, 0, 1, 1, 1);
	gb_other->addWidget(cbx_mute_local_keys, 1, 0, 1, 2);
	gb_other->addWidget(cbx_mute_non_channel, 2, 0, 1, 1);
	gb_other->addWidget(cbx_mute_control, 2, 1, 1, 1);

	handle_revert_all();
}

void
MppMuteMap :: handle_reset_all()
{

	for (uint8_t n = 0; n != 16; n++)
		MPP_BLOCKED(cbx_mute[n],setChecked(0));

	MPP_BLOCKED(cbx_mute_program,setSelection(0));
	MPP_BLOCKED(cbx_mute_pedal,setSelection(0));
	MPP_BLOCKED(cbx_mute_non_channel,setSelection(0));
	MPP_BLOCKED(cbx_mute_local_keys,setSelection(0));
	MPP_BLOCKED(cbx_mute_control,setSelection(0));

	handle_apply_all();
}

void
MppMuteMap :: handle_revert_all()
{
	uint8_t mute_copy[16];
	uint8_t mute_prog_copy;
	uint8_t mute_pedal_copy;
	uint8_t mute_local_keys_copy;
	uint8_t mute_midi_non_channel_copy;
	uint8_t mute_control_copy;

	mw->atomic_lock();
	for (uint8_t n = 0; n != 16; n++)
		mute_copy[n] = mw->muteMap[devno][n] ? 1 : 0;

	mute_prog_copy = mw->muteProgram[devno];
	mute_pedal_copy = mw->mutePedal[devno];
	if (mw->enableLocalKeys[devno])
		mute_local_keys_copy = 1;
	else if (mw->disableLocalKeys[devno])
		mute_local_keys_copy = 2;
	else
		mute_local_keys_copy = 0;
	mute_midi_non_channel_copy = mw->muteAllNonChannel[devno];
	mute_control_copy = mw->muteAllControl[devno];
	mw->atomic_unlock();

	for (uint8_t n = 0; n != 16; n++)
		MPP_BLOCKED(cbx_mute[n],setChecked(mute_copy[n]));

	MPP_BLOCKED(cbx_mute_program,setSelection(mute_prog_copy));
	MPP_BLOCKED(cbx_mute_pedal,setSelection(mute_pedal_copy));
	MPP_BLOCKED(cbx_mute_local_keys,setSelection(mute_local_keys_copy));
	MPP_BLOCKED(cbx_mute_non_channel,setSelection(mute_midi_non_channel_copy));
	MPP_BLOCKED(cbx_mute_control,setSelection(mute_control_copy));
}

void
MppMuteMap :: handle_apply_all()
{
	uint8_t mute_copy[16];
	uint8_t mute_prog_copy;
	uint8_t mute_pedal_copy;
	uint8_t mute_local_enable_copy;
	uint8_t mute_local_disable_copy;
	uint8_t mute_midi_non_channel_copy;
	uint8_t mute_control_copy;
	bool apply = false;

	for (uint8_t n = 0; n != 16; n++)
		mute_copy[n] = (cbx_mute[n]->checkState() == Qt::Checked);

	mute_prog_copy = (cbx_mute_program->currSelection != 0);
	mute_pedal_copy = (cbx_mute_pedal->currSelection != 0);
	mute_local_enable_copy = (cbx_mute_local_keys->currSelection == 1);
	mute_local_disable_copy = (cbx_mute_local_keys->currSelection == 2);
	mute_midi_non_channel_copy = (cbx_mute_non_channel->currSelection != 0);
	mute_control_copy = (cbx_mute_control->currSelection != 0);

	mw->atomic_lock();
	for (uint8_t n = 0; n != 16; n++)
		mw->muteMap[devno][n] = mute_copy[n];

	mw->muteProgram[devno] = mute_prog_copy;
	mw->mutePedal[devno] = mute_pedal_copy;
	apply |= (mw->enableLocalKeys[devno] != mute_local_enable_copy);
	mw->enableLocalKeys[devno] = mute_local_enable_copy;
	apply |= (mw->disableLocalKeys[devno] != mute_local_disable_copy);
	mw->disableLocalKeys[devno] = mute_local_disable_copy;
	mw->muteAllNonChannel[devno] = mute_midi_non_channel_copy;
	mw->muteAllControl[devno] = mute_control_copy;
	mw->atomic_unlock();

	if (apply)
		mw->handle_config_local_keys();
}

void
MppMuteMap :: handle_close_all()
{
	this->accept();
}
