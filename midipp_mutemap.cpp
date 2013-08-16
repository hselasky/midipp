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

MppMuteMap :: MppMuteMap(QWidget *parent, MppMainWindow *_mw, int _devno)
  : QDialog(parent)
{
	char buf[64];
	int n;

	mw = _mw;
	devno = _devno;

	gl = new QGridLayout(this);

	gb_mute = new MppGroupBox("Mute Map");
	gb_other = new MppGroupBox("Other controls");

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

	gl->addWidget(but_set_all, 2, 0, 1, 1, Qt::AlignHCenter|Qt::AlignVCenter);
	gl->addWidget(but_clear_all, 2, 1, 1, 1, Qt::AlignHCenter|Qt::AlignVCenter);
	gl->addWidget(but_revert_all, 2, 2, 1, 1, Qt::AlignHCenter|Qt::AlignVCenter);
	gl->addWidget(but_apply_all, 2, 3, 1, 1, Qt::AlignHCenter|Qt::AlignVCenter);
	gl->addWidget(but_cancel_all, 2, 4, 1, 1, Qt::AlignHCenter|Qt::AlignVCenter);

	connect(but_set_all, SIGNAL(released()), this, SLOT(handle_set_all()));
	connect(but_clear_all, SIGNAL(released()), this, SLOT(handle_clear_all()));
	connect(but_revert_all, SIGNAL(released()), this, SLOT(handle_revert_all()));
	connect(but_apply_all, SIGNAL(released()), this, SLOT(handle_apply_all()));
	connect(but_cancel_all, SIGNAL(released()), this, SLOT(handle_cancel_all()));

	for (n = 0; n != 16; n++) {
		int x_off;
		int y_off;

		snprintf(buf, sizeof(buf), "Ch%X", n);

		cbx_mute[n] = new MppCheckBox();

		x_off = (n & 8) ? 2: 0;
		y_off = (n & 7) + 1;

		gb_mute->addWidget(new QLabel(tr(buf)),
		    y_off, x_off + 0, 1, 1, Qt::AlignHCenter|Qt::AlignVCenter);
		gb_mute->addWidget(cbx_mute[n],
		    y_off, x_off + 1, 1, 1, Qt::AlignHCenter|Qt::AlignVCenter);
	}

	cbx_mute_program = new MppCheckBox();
	cbx_mute_pedal = new MppCheckBox();
	cbx_mute_local_keys_enable = new MppCheckBox();
	cbx_mute_local_keys_disable = new MppCheckBox();
	cbx_mute_all_control = new MppCheckBox();

	gb_other->addWidget(new QLabel(tr("Mute all bank and program change events")),
	    0, 0, 1, 1, Qt::AlignLeft|Qt::AlignVCenter);
	gb_other->addWidget(cbx_mute_program,
	    0, 1, 1, 1, Qt::AlignHCenter|Qt::AlignVCenter);

	gb_other->addWidget(new QLabel(tr("Mute all pedal events")),
	    1, 0, 1, 1, Qt::AlignLeft|Qt::AlignVCenter);
	gb_other->addWidget(cbx_mute_pedal,
	    1, 1, 1, 1, Qt::AlignHCenter|Qt::AlignVCenter);

	gb_other->addWidget(new QLabel(tr("Local keys enabled on non-muted channels")),
	    2, 0, 1, 1, Qt::AlignLeft|Qt::AlignVCenter);
	gb_other->addWidget(cbx_mute_local_keys_enable,
	    2, 1, 1, 1, Qt::AlignHCenter|Qt::AlignVCenter);

	gb_other->addWidget(new QLabel(tr("Local keys disabled on non-muted channels")),
	    3, 0, 1, 1, Qt::AlignLeft|Qt::AlignVCenter);
	gb_other->addWidget(cbx_mute_local_keys_disable,
	    3, 1, 1, 1, Qt::AlignHCenter|Qt::AlignVCenter);

	gb_other->addWidget(new QLabel(tr("Mute all control events")),
	    4, 0, 1, 1, Qt::AlignLeft|Qt::AlignVCenter);
	gb_other->addWidget(cbx_mute_all_control,
	    4, 1, 1, 1, Qt::AlignHCenter|Qt::AlignVCenter);

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
	uint8_t mute_local_enable_copy;
	uint8_t mute_local_disable_copy;
	uint8_t mute_all_control_copy;
	int n;

	pthread_mutex_lock(&mw->mtx);
	for (n = 0; n != 16; n++)
		mute_copy[n] = mw->muteMap[devno][n] ? 1 : 0;

	mute_prog_copy = mw->muteProgram[devno];
	mute_pedal_copy = mw->mutePedal[devno];
	mute_local_enable_copy = mw->enableLocalKeys[devno];
	mute_local_disable_copy = mw->disableLocalKeys[devno];
	mute_all_control_copy = mw->muteAllControl[devno];
	pthread_mutex_unlock(&mw->mtx);

	for (n = 0; n != 16; n++)
		cbx_mute[n]->setChecked(mute_copy[n]);

	cbx_mute_program->setChecked(mute_prog_copy);
	cbx_mute_pedal->setChecked(mute_pedal_copy);
	cbx_mute_local_keys_enable->setChecked(mute_local_enable_copy);
	cbx_mute_local_keys_disable->setChecked(mute_local_disable_copy);
	cbx_mute_all_control->setChecked(mute_all_control_copy);
}

void
MppMuteMap :: handle_apply_all()
{
	uint8_t mute_copy[16];
	uint8_t mute_prog_copy;
	uint8_t mute_pedal_copy;
	uint8_t mute_local_enable_copy;
	uint8_t mute_local_disable_copy;
	uint8_t mute_all_control_copy;
	int n;

	for (n = 0; n != 16; n++)
		mute_copy[n] = (cbx_mute[n]->checkState() == Qt::Checked);

	mute_prog_copy = (cbx_mute_program->checkState() == Qt::Checked);
	mute_pedal_copy = (cbx_mute_pedal->checkState() == Qt::Checked);
	mute_local_enable_copy = (cbx_mute_local_keys_enable->checkState() == Qt::Checked);
	mute_local_disable_copy = (cbx_mute_local_keys_disable->checkState() == Qt::Checked);
	mute_all_control_copy = (cbx_mute_all_control->checkState() == Qt::Checked);

	if (mute_local_enable_copy && mute_local_disable_copy)
		mute_local_disable_copy = 0;

	pthread_mutex_lock(&mw->mtx);
	for (n = 0; n != 16; n++)
		mw->muteMap[devno][n] = mute_copy[n];

	mw->muteProgram[devno] = mute_prog_copy;
	mw->mutePedal[devno] = mute_pedal_copy;
	mw->enableLocalKeys[devno] = mute_local_enable_copy;
	mw->disableLocalKeys[devno] = mute_local_disable_copy;
	mw->muteAllControl[devno] = mute_all_control_copy;
	pthread_mutex_unlock(&mw->mtx);

	this->accept();
}

void
MppMuteMap :: handle_cancel_all()
{
	this->reject();
}
