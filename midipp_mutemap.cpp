/*-
 * Copyright (c) 2010-2012 Hans Petter Selasky. All rights reserved.
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
	char buf[64];
	int n;

	mw = _mw;
	devno = _devno;

	gl = new QGridLayout(this);

	gb_mute = new MppGroupBox("MIDI Output Channel Mute Map");
	gb_other = new MppGroupBox("MIDI Output Mute");

	snprintf(buf, sizeof(buf), "- MIDI Output Mute "
	    "Map For Device %d -", _devno);

	setWindowTitle(tr(buf));
	setWindowIcon(QIcon(QString(MPP_ICON_FILE)));

	gb_mute->addWidget(new QLabel(tr("Mute enable")),
	    0, 1, 1, 1, Qt::AlignHCenter|Qt::AlignVCenter);
	gb_mute->addWidget(new QLabel(tr("Mute enable")),
	    0, 3, 1, 1, Qt::AlignHCenter|Qt::AlignVCenter);

	gl->addWidget(gb_mute, 0, 0, 1, 5);
	gl->addWidget(gb_other, 1, 0, 1, 5);

	but_set_all = new QPushButton(tr("Set All"));
	but_clear_all = new QPushButton(tr("Clear All"));
	but_revert_all = new QPushButton(tr("Revert"));
	but_apply_all = new QPushButton(tr("Apply"));
	but_cancel_all = new QPushButton(tr("Cancel"));

	gl->setRowStretch(2, 1);
	gl->setColumnStretch(5, 1);

	gl->addWidget(but_apply_all, 3, 3, 1, 1, Qt::AlignHCenter|Qt::AlignVCenter);
	gl->addWidget(but_cancel_all, 3, 4, 1, 1, Qt::AlignHCenter|Qt::AlignVCenter);
	gl->addWidget(but_set_all, 3, 0, 1, 1, Qt::AlignHCenter|Qt::AlignVCenter);
	gl->addWidget(but_clear_all, 3, 1, 1, 1, Qt::AlignHCenter|Qt::AlignVCenter);
	gl->addWidget(but_revert_all, 3, 2, 1, 1, Qt::AlignHCenter|Qt::AlignVCenter);

	connect(but_set_all, SIGNAL(released()), this, SLOT(handle_set_all()));
	connect(but_clear_all, SIGNAL(released()), this, SLOT(handle_clear_all()));
	connect(but_revert_all, SIGNAL(released()), this, SLOT(handle_revert_all()));
	connect(but_apply_all, SIGNAL(released()), this, SLOT(handle_apply_all()));
	connect(but_cancel_all, SIGNAL(released()), this, SLOT(handle_cancel_all()));

	for (n = 0; n != 16; n++) {
		int x_off;
		int y_off;

		cbx_mute[n] = new MppCheckBox();

		x_off = (n & 8) ? 2: 0;
		y_off = (n & 7) + 1;

		gb_mute->addWidget(new QLabel(MppChanName(n)),
		    y_off, x_off + 0, 1, 1, Qt::AlignHCenter|Qt::AlignVCenter);
		gb_mute->addWidget(cbx_mute[n],
		    y_off, x_off + 1, 1, 1, Qt::AlignHCenter|Qt::AlignVCenter);
	}

	cbx_mute_program = new MppButtonMap("Mute MIDI program events\0" "NO\0" "YES\0", 2, 2);
	cbx_mute_pedal = new MppButtonMap("Mute MIDI pedal events\0" "NO\0" "YES\0", 2, 2);
	cbx_mute_non_channel = new MppButtonMap("Mute non-channel MIDI events\0" "NO\0" "YES\0", 2, 2);
	cbx_mute_local_keys = new MppButtonMap("Local keys MIDI command sent\0" "NONE\0" "ENABLE\0" "DISABLE\0", 3, 3);
	cbx_mute_control = new MppButtonMap("Mute MIDI control events\0" "NO\0" "YES\0", 2, 2);

	gb_other->addWidget(cbx_mute_program, 0, 0, 1, 1);
	gb_other->addWidget(cbx_mute_pedal, 0, 1, 1, 1);
	gb_other->addWidget(cbx_mute_local_keys, 1, 0, 1, 2);
	gb_other->addWidget(cbx_mute_non_channel, 2, 0, 1, 1);
	gb_other->addWidget(cbx_mute_control, 2, 1, 1, 1);

	handle_revert_all();
}

MppMuteMap :: ~MppMuteMap()
{

}

void
MppMuteMap :: handle_set_all()
{
	int n;

	for (n = 0; n != 16; n++)
		cbx_mute[n]->setChecked(1);
}

void
MppMuteMap :: handle_clear_all()
{
	int n;

	for (n = 0; n != 16; n++)
		cbx_mute[n]->setChecked(0);
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
	int n;

	pthread_mutex_lock(&mw->mtx);
	for (n = 0; n != 16; n++)
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
	pthread_mutex_unlock(&mw->mtx);

	for (n = 0; n != 16; n++)
		cbx_mute[n]->setChecked(mute_copy[n]);

	cbx_mute_program->setSelection(mute_prog_copy);
	cbx_mute_pedal->setSelection(mute_pedal_copy);
	cbx_mute_local_keys->setSelection(mute_local_keys_copy);
	cbx_mute_non_channel->setSelection(mute_midi_non_channel_copy);
	cbx_mute_control->setSelection(mute_control_copy);
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
	int n;

	for (n = 0; n != 16; n++)
		mute_copy[n] = (cbx_mute[n]->checkState() == Qt::Checked);

	mute_prog_copy = (cbx_mute_program->currSelection != 0);
	mute_pedal_copy = (cbx_mute_pedal->currSelection != 0);
	mute_local_enable_copy = (cbx_mute_local_keys->currSelection == 1);
	mute_local_disable_copy = (cbx_mute_local_keys->currSelection == 2);
	mute_midi_non_channel_copy = (cbx_mute_non_channel->currSelection != 0);
	mute_control_copy = (cbx_mute_control->currSelection != 0);

	pthread_mutex_lock(&mw->mtx);
	for (n = 0; n != 16; n++)
		mw->muteMap[devno][n] = mute_copy[n];

	mw->muteProgram[devno] = mute_prog_copy;
	mw->mutePedal[devno] = mute_pedal_copy;
	mw->enableLocalKeys[devno] = mute_local_enable_copy;
	mw->disableLocalKeys[devno] = mute_local_disable_copy;
	mw->muteAllNonChannel[devno] = mute_midi_non_channel_copy;
	mw->muteAllControl[devno] = mute_control_copy;
	pthread_mutex_unlock(&mw->mtx);

	this->accept();
}

void
MppMuteMap :: handle_cancel_all()
{
	this->reject();
}
