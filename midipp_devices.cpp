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

#include "midipp_mainwindow.h"
#include "midipp_devices.h"
#include "midipp_groupbox.h"

MppDevices :: MppDevices(QWidget *parent)
  : QDialog(parent)
{
	int n;

	gl = new QGridLayout(this);

	setWindowTitle(tr("Select playback and recording device"));
	setWindowIcon(QIcon(QString(MPP_ICON_FILE)));

	lw_rec = new QListWidget();
	lw_play = new QListWidget();

	gb_rec = new MppGroupBox(tr("Recording devices"));
	gb_play = new MppGroupBox(tr("Playback devices"));

	gb_rec->addWidget(lw_rec, 0, 0, 1, 1);
	gb_play->addWidget(lw_play, 0, 0, 1, 1);

	but_ok = new QPushButton(tr("Ok"));
	but_cancel = new QPushButton(tr("Cancel"));

	connect(but_ok, SIGNAL(released()), this, SLOT(accept()));
	connect(but_cancel, SIGNAL(released()), this, SLOT(reject()));

	gl->addWidget(gb_play, 0,0,1,2);
	gl->addWidget(gb_rec, 0,2,1,2);

	gl->addWidget(but_ok, 1,2,1,1);
	gl->addWidget(but_cancel, 1,3,1,1);

	QDir dir("/dev");

	QStringList filters;
	filters << "umidi*" << "midi*";
	dir.setFilter(QDir::Files | QDir::System);
	dir.setNameFilters(filters);
	dir.setSorting(QDir::Name);
	QFileInfoList list = dir.entryInfoList();

	new QListWidgetItem(tr("No recording device"), lw_rec);

	for (n = 0; n != list.size(); n++) {
		QFileInfo fileInfo = list.at(n);
		if (fileInfo.fileName() == QString("midistat"))
			continue;
		new QListWidgetItem(QString("D:/dev/") + fileInfo.fileName(), lw_rec);
	}

	new QListWidgetItem(tr("No playback device"), lw_play);

	for (n = 0; n != list.size(); n++) {
		QFileInfo fileInfo = list.at(n);
		if (fileInfo.fileName() == QString("midistat"))
			continue;
		new QListWidgetItem(QString("D:/dev/") + fileInfo.fileName(), lw_play);
	}

	rec_jack_str = umidi20_jack_alloc_outputs();
	if (rec_jack_str != NULL) {
		for (n = 0; rec_jack_str[n] != NULL; n++) {
			if (rec_jack_str[n][0] != 0)
				new QListWidgetItem(QString("A:") + QString(rec_jack_str[n]), lw_rec);
		}
	}

	play_jack_str = umidi20_jack_alloc_inputs();
	if (play_jack_str != NULL) {
		for (n = 0; play_jack_str[n] != NULL; n++) {
			if (play_jack_str[n][0] != 0)
				new QListWidgetItem(QString("A:") + QString(play_jack_str[n]), lw_play);
		}
	}

	rec_coremidi_str = umidi20_coremidi_alloc_outputs();
	if (rec_coremidi_str != NULL) {
		for (n = 0; rec_coremidi_str[n] != NULL; n++) {
			if (rec_coremidi_str[n][0] != 0)
				new QListWidgetItem(QString("C:") + QString(rec_coremidi_str[n]), lw_rec);
		}
	}

	play_coremidi_str = umidi20_coremidi_alloc_inputs();
	if (play_coremidi_str != NULL) {
		for (n = 0; play_coremidi_str[n] != NULL; n++) {
			if (play_coremidi_str[n][0] != 0)
				new QListWidgetItem(QString("C:") + QString(play_coremidi_str[n]), lw_play);
		}
	}

	lw_rec->setCurrentRow(0);
	lw_play->setCurrentRow(0);
}

int
MppDevices :: autoSelect()
{
	/* check for zero or once choice only */
	if (lw_rec->count() == 1 && lw_play->count() == 1) {
		accept();
		return (-1);
	} else if (lw_rec->count() == 2 && lw_play->count() == 2) {
		lw_rec->setCurrentRow(1);
		lw_play->setCurrentRow(1);
		accept();
		return (1);
	}
	return (0);
}

MppDevices :: ~MppDevices()
{
	umidi20_jack_free_outputs(rec_jack_str);
	umidi20_jack_free_inputs(play_jack_str);

	umidi20_coremidi_free_outputs(rec_coremidi_str);
	umidi20_coremidi_free_inputs(play_coremidi_str);
}

void
MppDevices :: accept(void)
{
	QListWidgetItem *ptr;

	ptr = lw_rec->currentItem();
	if (ptr != NULL && lw_rec->currentRow() > 0)
		rec_dev = ptr->text();
	else
		rec_dev = QString("X:");

	ptr = lw_play->currentItem();
	if (ptr != NULL && lw_play->currentRow() > 0)
		play_dev = ptr->text();
	else
		play_dev = QString("X:");

	/* filter invalid characters */
	rec_dev = rec_dev.replace(QString("|"),QString(""));
	play_dev = play_dev.replace(QString("|"),QString(""));

	if (rec_dev == play_dev)
		result_dev = play_dev;
	else
		result_dev = play_dev + QString("|") + rec_dev;

	QDialog :: accept();
}
