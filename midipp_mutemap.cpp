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

#include <midipp_mainwindow.h>
#include <midipp_mutemap.h>

MppMuteMap :: MppMuteMap(QWidget *parent, MppMainWindow *_mw, int _devno)
  : QDialog(parent)
{
	char buf[64];
	int n;
	int x_off;
	int y_off;

	mw = _mw;
	devno = _devno;

	gl = new QGridLayout(this);

	snprintf(buf, sizeof(buf), "- Mute Map For Output Device %d -", _devno);

	setWindowTitle(tr(buf));
	setWindowIcon(QIcon(QString(MPP_ICON_FILE)));

	lbl_title[0] = new QLabel(tr("- Channel Mute -"));
	lbl_title[1] = new QLabel(tr("- Channel Mute -"));
	lbl_title[2] = new QLabel(tr("- - - - Other - - - -"));

	gl->addWidget(lbl_title[0], 1, 0, 1, 3, Qt::AlignHCenter|Qt::AlignVCenter);
	gl->addWidget(lbl_title[1], 1, 3, 1, 3, Qt::AlignHCenter|Qt::AlignVCenter);

	but_set_all = new QPushButton(tr("Set All"));
	but_clear_all = new QPushButton(tr("Clear All"));
	but_revert_all = new QPushButton(tr("Revert"));
	but_apply_all = new QPushButton(tr("Apply"));
	but_cancel_all = new QPushButton(tr("Cancel"));

	gl->addWidget(but_set_all, 16 + 2, 0, 1, 1, Qt::AlignHCenter|Qt::AlignVCenter);
	gl->addWidget(but_clear_all, 16 + 2, 1, 1, 1, Qt::AlignHCenter|Qt::AlignVCenter);

	gl->addWidget(but_revert_all, 16 + 2, 3, 1, 1, Qt::AlignHCenter|Qt::AlignVCenter);
	gl->addWidget(but_apply_all, 16 + 2, 4, 1, 1, Qt::AlignHCenter|Qt::AlignVCenter);
	gl->addWidget(but_cancel_all, 16 + 2, 5, 1, 1, Qt::AlignHCenter|Qt::AlignVCenter);

	connect(but_set_all, SIGNAL(pressed()), this, SLOT(handle_set_all()));
	connect(but_clear_all, SIGNAL(pressed()), this, SLOT(handle_clear_all()));
	connect(but_revert_all, SIGNAL(pressed()), this, SLOT(handle_revert_all()));
	connect(but_apply_all, SIGNAL(released()), this, SLOT(handle_apply_all()));
	connect(but_cancel_all, SIGNAL(released()), this, SLOT(handle_cancel_all()));

	for (n = 0; n != 16; n++) {

		snprintf(buf, sizeof(buf), "Ch%X", n);

		lbl_chan[n] = new QLabel(tr(buf));
		cbx_mute[n] = new QCheckBox();

		x_off = (n & 8) ? 3: 0;
		y_off = (n & 7) + 2;

		gl->addWidget(lbl_chan[n], y_off, x_off + 0, 1, 1, Qt::AlignHCenter|Qt::AlignVCenter);
		gl->addWidget(cbx_mute[n], y_off, x_off + 1, 1, 2, Qt::AlignHCenter|Qt::AlignVCenter);
	}

	cbx_mute_program = new QCheckBox();
	cbx_mute_pedal = new QCheckBox();
	cbx_mute_local_keys_enable = new QCheckBox();
	cbx_mute_local_keys_disable = new QCheckBox();
	cbx_mute_all_control = new QCheckBox();

	lbl_mute_program = new QLabel(tr("Mute all bank and program change events"));
	lbl_mute_pedal = new QLabel(tr("Mute all pedal events"));
	lbl_mute_local_keys_enable = new QLabel(tr("Local keys enabled on non-muted channels"));
	lbl_mute_local_keys_disable = new QLabel(tr("Local keys disabled on non-muted channels"));
	lbl_mute_all_control = new QLabel(tr("Mute all control events"));

	y_off = 10;

	gl->addWidget(lbl_title[2], y_off, 0, 1, 4, Qt::AlignHCenter|Qt::AlignVCenter);

	y_off++;

	gl->addWidget(lbl_mute_program, y_off, 0, 1, 3, Qt::AlignLeft|Qt::AlignVCenter);
	gl->addWidget(cbx_mute_program, y_off, 3, 1, 1, Qt::AlignHCenter|Qt::AlignVCenter);

	y_off++;

	gl->addWidget(lbl_mute_pedal, y_off, 0, 1, 3, Qt::AlignLeft|Qt::AlignVCenter);
	gl->addWidget(cbx_mute_pedal, y_off, 3, 1, 1, Qt::AlignHCenter|Qt::AlignVCenter);

	y_off++;

	gl->addWidget(lbl_mute_local_keys_enable, y_off, 0, 1, 3, Qt::AlignLeft|Qt::AlignVCenter);
	gl->addWidget(cbx_mute_local_keys_enable, y_off, 3, 1, 1, Qt::AlignHCenter|Qt::AlignVCenter);

	y_off++;

	gl->addWidget(lbl_mute_local_keys_disable, y_off, 0, 1, 3, Qt::AlignLeft|Qt::AlignVCenter);
	gl->addWidget(cbx_mute_local_keys_disable, y_off, 3, 1, 1, Qt::AlignHCenter|Qt::AlignVCenter);

	y_off++;

	gl->addWidget(lbl_mute_all_control, y_off, 0, 1, 3, Qt::AlignLeft|Qt::AlignVCenter);
	gl->addWidget(cbx_mute_all_control, y_off, 3, 1, 1, Qt::AlignHCenter|Qt::AlignVCenter);


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
