/*-
 * Copyright (c) 2009 Hans Petter Selasky. All rights reserved.
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

#include <sys/stdint.h>
#include <sys/param.h>

#include <QApplication>

#include <midipp.h>

#include <err.h>

static char *
MppQStringToAscii(QString s)
{
        QChar *ch;
	char *ptr;
	int len;
	int c;

	ch = s.data();
	if (ch == NULL)
		return (NULL);

	len = 1;

	while (1) {
		c = ch->toAscii();
		if (c == 0)
			break;
		len++;
		ch++;
	}

	ptr = (char *)malloc(len);
	if (ptr == NULL)
		return (NULL);

	ch = s.data();
	len = 0;

	while (1) {
		c = ch->toAscii();
		ptr[len] = c;
		if (c == 0)
			break;
		len++;
		ch++;
	}
	return (ptr);
}

/* XXX TODO: Error handling */

static QString
MppReadFile(QString fname)
{
	QFile file(fname);
	QString retval;

	if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
		return (NULL);

	retval = file.readAll();

	file.close();

	return (retval);
}

static void
MppWriteFile(QString fname, QString text)
{
	QFile file(fname);

	if (!file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate))
		return;

	file.write(text.toAscii());

	file.close();

	return;
}

static void
MppParse(struct MppSoftc *sc, const QString &ps)
{
	int c;
	int d;
	int x;
	int y;
	int line;
	int label;
	int index;
	int channel;
	int base_key;
	int duration;

	x = -1;
	line = 0;
	index = 0;
	memset(sc->ScNotes, 0, sizeof(sc->ScNotes));
	memset(sc->ScJumpNext, 0, sizeof(sc->ScJumpNext));
	memset(sc->ScJumpTable, 0, sizeof(sc->ScJumpTable));

	if (ps.isNull() || ps.isEmpty())
		goto done;

next_line:
	if (index != 0) {
		line++;
		index = 0;
	}
	y = -1;
	channel = 0;
	duration = 1;

next_char:
	x++;
	y++;

	c = ps[x].toUpper().toAscii();

	switch (c) {
	case 'C':
		if (y == 0) {
			base_key = C0;
			goto parse_note;
		}
		goto next_char;
	case 'D':
		if (y == 0) {
			base_key = D0;
			goto parse_note;
		}
		goto next_char;
	case 'E':
		if (y == 0) {
			base_key = E0;
			goto parse_note;
		}
		goto next_char;
	case 'F':
		if (y == 0) {
			base_key = F0;
			goto parse_note;
		}
		goto next_char;
	case 'G':
		if (y == 0) {
			base_key = G0;
			goto parse_note;
		}
		goto next_char;
	case 'A':
		if (y == 0) {
			base_key = A0;
			goto parse_note;
		}
		goto next_char;
	case 'B':
	case 'H':
		if (y == 0) {
			base_key = H0;
			goto parse_note;
		}
		goto next_char;
	case 'T':
		if (y == 0) {
			goto parse_channel;
		}
		goto next_char;
	case 'L':
		if (y == 0) {
			goto parse_label;
		}
		goto next_char;
	case 'J':
		if (y == 0) {
			goto parse_jump;
		}
		goto next_char;
	case 'U':
		if (y == 0) {
			goto parse_duration;
		}
		goto next_char;
	case 0:
		goto done;
	case '\n':
		goto next_line;
	case '/':
		if (y != 0)
			goto next_char;

		/* check for comment */
		c = ps[x+1].toUpper().toAscii();
		if (c == '*') {
			while (1) {
				c = ps[x+2].toUpper().toAscii();
				if (c == '*') {
					c = ps[x+3].toUpper().toAscii();
					if (c == '/') {
						/* end of comment */
						x += 3;
						break;
					}
				}
				if (c == 0)
					goto done;
				x++;
			}
			y = -1;
		}
		goto next_char;

	case ' ':
	case '\t':
		y = -1;
	default:
		goto next_char;
	}

parse_note:
	c = ps[x+1].toAscii();
	if (c >= '0' && c <= '9') {
		d = ps[x+2].toAscii();
		if (d >= '0' && d <= '9') {
			base_key += 120 * (c - '0');
			base_key += 12 * (d - '0');
			x += 2;
		} else {
			base_key += 12 * (c - '0');
			x += 1;
		}
		c = ps[x+1].toUpper().toAscii();
		if (c == 'B') {
			base_key -= 1;
			x += 1;
		}
		if ((line < MPP_MAX_LINES) && (index < MPP_MAX_NOTES)) {
			sc->ScNotes[line][index].key = base_key & 127;
			sc->ScNotes[line][index].dur = duration & 255;
			sc->ScNotes[line][index].channel = channel & 15;
			index++;
		}

	}
	goto next_char;

parse_duration:
	c = ps[x+1].toAscii();
	if (c >= '0' && c <= '9') {
		d = ps[x+2].toAscii();
		if (d >= '0' && d <= '9') {
			duration = (10 * (c - '0')) + (d - '0');
			x += 2;
		} else {
			duration = (c - '0');
			x += 1;
		}
	} else {
		duration = 0;
	}
	goto next_char;

parse_channel:
	c = ps[x+1].toAscii();
	if (c >= '0' && c <= '9') {
		d = ps[x+2].toAscii();
		if (d >= '0' && d <= '9') {
			channel = (10 * (c - '0')) + (d - '0');
			x += 2;
		} else {
			channel = (c - '0');
			x += 1;
		}
	} else {
		channel = 0;
	}
	goto next_char;

parse_label:
	c = ps[x+1].toAscii();
	if (c >= '0' && c <= '9') {
		d = ps[x+2].toAscii();
		if (d >= '0' && d <= '9') {
			label = (10 * (c - '0')) + (d - '0');
			x += 2;
		} else {
			label = (c - '0');
			x += 1;
		}
	} else {
		label = 0;
	}
	if ((label >= 0) && (label < MPP_MAX_LABELS)) {
		sc->ScJumpTable[label] = line + 1;
	}
	goto next_char;

parse_jump:
	c = ps[x+1].toAscii();
	if (c >= '0' && c <= '9') {
		d = ps[x+2].toAscii();
		if (d >= '0' && d <= '9') {
			label = (10 * (c - '0')) + (d - '0');
			x += 2;
		} else {
			label = (c - '0');
			x += 1;
		}
	} else {
		label = 0;
	}

	if (index != 0) {
		line++;
		index = 0;
	}

	if ((label >= 0) && (label < MPP_MAX_LABELS) && (line < MPP_MAX_LINES)) {
		sc->ScJumpNext[line] = label + 1;
		line++;
	}
	goto next_char;

done:
	if (index != 0)
		line++;

	/* resolve all jumps */
	for (x = 0; x != line; x++) {
		if (sc->ScJumpNext[x] != 0)
			sc->ScJumpNext[x] = sc->ScJumpTable[sc->ScJumpNext[x] - 1];
	}

	sc->ScLinesMax = line;
	sc->ScCurrPos = 0;
}

MppMainWindow :: MppMainWindow(QWidget *parent)
  : QWidget(parent)
{
	int n;
	int x;

	/* set memory default */

	memset(&main_sc, 0, sizeof(main_sc));

	memset(&mid_data, 0, sizeof(mid_data));

	CurrNoteFileName = NULL;
	CurrMidiFileName = NULL;
	song = NULL;
	track = NULL;

	umidi20_mutex_init(&mtx);

	/* Setup GUI */

	main_gl = new QGridLayout(this);
	main_tw = new QTabWidget();

	/* Watchdog */

	watchdog = new QTimer(this);
	connect(watchdog, SIGNAL(timeout()), this, SLOT(handle_watchdog()));

	/* Editor */

	main_edit = new QTextEdit();
	main_edit->setText(tr("/* Copyright (c) 2009 Hans Petter Selasky. All rights reserved. */\n\nC3\n"));

	main_gl->addWidget(main_edit,0,0,1,2);

	/* Tabs */

	main_gl->addWidget(main_tw,0,2,1,1);

	tab_file_wg = new QWidget();
	tab_play_wg = new QWidget();
	tab_config_wg = new QWidget();

	tab_file_gl = new QGridLayout(tab_file_wg);
	tab_play_gl = new QGridLayout(tab_play_wg);
	tab_config_gl = new QGridLayout(tab_config_wg);

	main_tw->addTab(tab_file_wg, tr("File"));
	main_tw->addTab(tab_play_wg, tr("Play"));
	main_tw->addTab(tab_config_wg, tr("Config"));

	/* Note File Tab */

	lbl_note_file = new QLabel(tr("- Note file -"));
	lbl_note_file->setAlignment(Qt::AlignHCenter|Qt::AlignVCenter);

	lbl_midi_file = new QLabel(tr("- MIDI file -"));
	lbl_midi_file->setAlignment(Qt::AlignHCenter|Qt::AlignVCenter);

	but_note_file_new = new QPushButton(tr("New"));
	but_note_file_open = new QPushButton(tr("Open"));
	but_note_file_save = new QPushButton(tr("Save"));
	but_note_file_save_as = new QPushButton(tr("Save As"));
	but_note_file_print = new QPushButton(tr("Print"));
	but_quit = new QPushButton(tr("Quit"));

	but_midi_file_new = new QPushButton(tr("New"));
	but_midi_file_open = new QPushButton(tr("Open"));
	but_midi_file_save = new QPushButton(tr("Save"));
	but_midi_file_save_as = new QPushButton(tr("Save As"));

	n = 0;

	tab_file_gl->addWidget(lbl_note_file, n++, 0, 1, 4);
	tab_file_gl->addWidget(but_note_file_new, n++, 0, 1, 4);
	tab_file_gl->addWidget(but_note_file_open, n++, 0, 1, 4);
	tab_file_gl->addWidget(but_note_file_save, n++, 0, 1, 4);
	tab_file_gl->addWidget(but_note_file_save_as, n++, 0, 1, 4);
	tab_file_gl->addWidget(but_note_file_print, n++, 0, 1, 4);
	tab_file_gl->setRowStretch(n++, 4);
	tab_file_gl->addWidget(but_quit, n++, 0, 1, 8);

	n = 0;

	tab_file_gl->addWidget(lbl_midi_file, n++, 4, 1, 4);
	tab_file_gl->addWidget(but_midi_file_new, n++, 4, 1, 4);
	tab_file_gl->addWidget(but_midi_file_open, n++, 4, 1, 4);
	tab_file_gl->addWidget(but_midi_file_save, n++, 4, 1, 4);
	tab_file_gl->addWidget(but_midi_file_save_as, n++, 4, 1, 4);

	/* Play Tab */

	lbl_prog_title = new QLabel(tr("- Synth Prog -"));
	lbl_prog_title->setAlignment(Qt::AlignHCenter|Qt::AlignVCenter);

	lbl_channel = new QLabel(tr("Chan:"));
	lbl_bank = new QLabel(tr("Bank:"));
	lbl_prog = new QLabel(tr("Prog:"));

	spn_channel = new QSpinBox();
	spn_channel->setMaximum(15);
	spn_channel->setMinimum(0);
	spn_channel->setValue(0);

	spn_bank = new QSpinBox();
	spn_bank->setMaximum(127);
	spn_bank->setMinimum(0);
	spn_bank->setValue(0);

	spn_prog = new QSpinBox();
	spn_prog->setMaximum(127);
	spn_prog->setMinimum(0);
	spn_prog->setValue(0);

	but_synth_program = new QPushButton(tr("Program"));

	lbl_note_record = new QLabel(QString());
	lbl_midi_record = new QLabel(QString());
	lbl_midi_pass_thru = new QLabel(QString());
	lbl_midi_play = new QLabel(QString());

	but_jump[0] = new QPushButton(tr("J0"));
	but_jump[1] = new QPushButton(tr("J1"));
	but_jump[2] = new QPushButton(tr("J2"));
	but_jump[3] = new QPushButton(tr("J3"));

	but_track[0] = new QPushButton(tr("T0"));
	but_track[1] = new QPushButton(tr("T1"));
	but_track[2] = new QPushButton(tr("T2"));
	but_track[3] = new QPushButton(tr("T3"));

	lbl_track[0] = new QLabel(QString());
	lbl_track[1] = new QLabel(QString());
	lbl_track[2] = new QLabel(QString());
	lbl_track[3] = new QLabel(QString());

	but_midi_pass_thru = new QPushButton(tr("Pass Thru"));
	but_compile = new QPushButton(tr("Compile"));
	but_note_record = new QPushButton(tr("Notes"));
	but_midi_record = new QPushButton(tr("MIDI"));

	but_midi_play = new QPushButton(tr("MIDI"));
	but_midi_trigger = new QPushButton(tr("Trigger"));
	but_midi_rewind = new QPushButton(tr("Rewind"));

	but_play = new QPushButton(tr(" \nPlay\n "));

	lbl_volume = new QLabel(tr("Volume (0-127)"));
	spn_volume = new QSpinBox();
	spn_volume->setMaximum(127);
	spn_volume->setMinimum(0);
	spn_volume->setValue(80);

	lbl_play_key = new QLabel(QString());
	spn_play_key = new QSpinBox();
	connect(spn_play_key, SIGNAL(valueChanged(int)), this, SLOT(handle_play_key_changed(int)));
	spn_play_key->setMaximum(127);
	spn_play_key->setMinimum(0);
	spn_play_key->setValue(C4);

	lbl_cmd_key = new QLabel(QString());
	spn_cmd_key = new QSpinBox();
	connect(spn_cmd_key, SIGNAL(valueChanged(int)), this, SLOT(handle_cmd_key_changed(int)));
	spn_cmd_key->setMaximum(127);
	spn_cmd_key->setMinimum(0);
	spn_cmd_key->setValue(C3);

	lbl_synth = new QLabel(tr("- Synth Play -"));
	lbl_synth->setAlignment(Qt::AlignHCenter|Qt::AlignVCenter);

	lbl_playback = new QLabel(tr("- Playback -"));
	lbl_playback->setAlignment(Qt::AlignHCenter|Qt::AlignVCenter);

	lbl_recording = new QLabel(tr("- Recording -"));
	lbl_recording->setAlignment(Qt::AlignHCenter|Qt::AlignVCenter);

	n = 0;

	tab_play_gl->addWidget(lbl_prog_title, n++, 0, 1, 4);
	tab_play_gl->addWidget(lbl_channel, n, 0, 1, 2);
	tab_play_gl->addWidget(spn_channel, n++, 2, 1, 2);
	tab_play_gl->addWidget(lbl_bank, n, 0, 1, 2);
	tab_play_gl->addWidget(spn_bank, n++, 2, 1, 2);
	tab_play_gl->addWidget(lbl_prog, n, 0, 1, 2);
	tab_play_gl->addWidget(spn_prog, n++, 2, 1, 2);
	tab_play_gl->addWidget(but_synth_program, n++, 0, 1, 4);

	tab_play_gl->addWidget(lbl_playback, n++, 0, 1, 4);

	tab_play_gl->addWidget(lbl_midi_play, n, 3, 1, 1);
	tab_play_gl->addWidget(but_midi_play, n++, 0, 1, 3);
	tab_play_gl->addWidget(but_midi_trigger, n++, 0, 1, 4);
	tab_play_gl->addWidget(but_midi_rewind, n++, 0, 1, 4);

	tab_play_gl->addWidget(lbl_recording, n++, 0, 1, 4);
	tab_play_gl->addWidget(lbl_midi_pass_thru, n, 3, 1, 1);
	tab_play_gl->addWidget(but_midi_pass_thru, n++, 0, 1, 3);

	tab_play_gl->addWidget(lbl_note_record, n, 3, 1, 1);
	tab_play_gl->addWidget(but_note_record, n++, 0, 1, 3);

	tab_play_gl->addWidget(lbl_midi_record, n, 3, 1, 1);
	tab_play_gl->addWidget(but_midi_record, n++, 0, 1, 3);

	n = 0;

	tab_play_gl->addWidget(lbl_synth, n++, 4, 1, 4);
	tab_play_gl->addWidget(lbl_volume, n, 4, 1, 3);
	tab_play_gl->addWidget(spn_volume, n++, 7, 1, 1);
	tab_play_gl->addWidget(lbl_play_key, n, 4, 1, 3);
	tab_play_gl->addWidget(spn_play_key, n++, 7, 1, 1);
	tab_play_gl->addWidget(lbl_cmd_key, n, 4, 1, 3);
	tab_play_gl->addWidget(spn_cmd_key, n++, 7, 1, 1);

	tab_play_gl->addWidget(lbl_track[0], n, 7, 1, 1);
	tab_play_gl->addWidget(but_track[0], n, 6, 1, 1);
	tab_play_gl->addWidget(but_jump[0], n++, 4, 1, 2);

	tab_play_gl->addWidget(lbl_track[1], n, 7, 1, 1);
	tab_play_gl->addWidget(but_track[1], n, 6, 1, 1);
	tab_play_gl->addWidget(but_jump[1], n++, 4, 1, 2);

	tab_play_gl->addWidget(lbl_track[2], n, 7, 1, 1);
	tab_play_gl->addWidget(but_track[2], n, 6, 1, 1);
	tab_play_gl->addWidget(but_jump[2], n++, 4, 1, 2);

	tab_play_gl->addWidget(lbl_track[3], n, 7, 1, 1);
	tab_play_gl->addWidget(but_track[3], n, 6, 1, 1);
	tab_play_gl->addWidget(but_jump[3], n++, 4, 1, 2);

	tab_play_gl->addWidget(but_compile, n++, 4, 1, 4);
	tab_play_gl->setRowStretch(n++, 4);
	tab_play_gl->addWidget(but_play, n++, 4, 3, 4);

	/* Configuration */

	lbl_config_title = new QLabel(tr("- Device configuration -"));
	lbl_config_title->setAlignment(Qt::AlignHCenter|Qt::AlignVCenter);
	lbl_config_play = new QLabel(tr("Play"));
	lbl_config_play->setAlignment(Qt::AlignHCenter|Qt::AlignVCenter);
	lbl_config_rec = new QLabel(tr("Rec."));
	lbl_config_rec->setAlignment(Qt::AlignHCenter|Qt::AlignVCenter);
	lbl_config_synth = new QLabel(tr("Synth"));
	lbl_config_synth->setAlignment(Qt::AlignHCenter|Qt::AlignVCenter);

	but_config_apply = new QPushButton(tr("Apply"));
	but_config_load = new QPushButton(tr("Revert"));

	x = 0;

	tab_config_gl->addWidget(lbl_config_title, x, 0, 1, 5);
	tab_config_gl->addWidget(lbl_config_play, x, 5, 1, 1);
	tab_config_gl->addWidget(lbl_config_rec, x, 6, 1, 1);
	tab_config_gl->addWidget(lbl_config_synth, x, 7, 1, 1);

	x++;

	for (n = 0; n != MPP_MAX_DEVS; n++) {
		char buf[16];

		led_config_dev[n] = new QLineEdit(QString());
		cbx_config_dev[(3*n)+0] = new QCheckBox();
		cbx_config_dev[(3*n)+1] = new QCheckBox();
		cbx_config_dev[(3*n)+2] = new QCheckBox();


		snprintf(buf, sizeof(buf), "Dev%d:", n);

		lbl_config_dev[n] = new QLabel(tr(buf));

		tab_config_gl->addWidget(lbl_config_dev[n], x, 0, 1, 1, Qt::AlignHCenter|Qt::AlignLeft);
		tab_config_gl->addWidget(led_config_dev[n], x, 1, 1, 4, Qt::AlignHCenter|Qt::AlignLeft);
		tab_config_gl->addWidget(cbx_config_dev[(3*n)+0], x, 5, 1, 1, Qt::AlignHCenter|Qt::AlignVCenter);
		tab_config_gl->addWidget(cbx_config_dev[(3*n)+1], x, 6, 1, 1, Qt::AlignHCenter|Qt::AlignVCenter);
		tab_config_gl->addWidget(cbx_config_dev[(3*n)+2], x, 7, 1, 1, Qt::AlignHCenter|Qt::AlignVCenter);

		x++;
	}

	tab_config_gl->setRowStretch(x, 4);

	x++;

	tab_config_gl->addWidget(but_config_apply, x, 4, 1, 4);
	tab_config_gl->addWidget(but_config_load, x, 0, 1, 4);

	connect(but_jump[0], SIGNAL(pressed()), this, SLOT(handle_jump_0()));
	connect(but_jump[1], SIGNAL(pressed()), this, SLOT(handle_jump_1()));
	connect(but_jump[2], SIGNAL(pressed()), this, SLOT(handle_jump_2()));
	connect(but_jump[3], SIGNAL(pressed()), this, SLOT(handle_jump_3()));

	connect(but_track[0], SIGNAL(pressed()), this, SLOT(handle_track_0()));
	connect(but_track[1], SIGNAL(pressed()), this, SLOT(handle_track_1()));
	connect(but_track[2], SIGNAL(pressed()), this, SLOT(handle_track_2()));
	connect(but_track[3], SIGNAL(pressed()), this, SLOT(handle_track_3()));

	connect(but_midi_pass_thru, SIGNAL(pressed()), this, SLOT(handle_pass_thru()));
	connect(but_compile, SIGNAL(pressed()), this, SLOT(handle_compile()));
	connect(but_note_record, SIGNAL(pressed()), this, SLOT(handle_note_record()));
	connect(but_midi_record, SIGNAL(pressed()), this, SLOT(handle_midi_record()));
	connect(but_midi_play, SIGNAL(pressed()), this, SLOT(handle_midi_play()));
	connect(but_play, SIGNAL(pressed()), this, SLOT(handle_play_press()));
	connect(but_play, SIGNAL(released()), this, SLOT(handle_play_release()));
	connect(but_quit, SIGNAL(pressed()), this, SLOT(handle_quit()));

	connect(but_note_file_new, SIGNAL(pressed()), this, SLOT(handle_note_file_new()));
	connect(but_note_file_open, SIGNAL(pressed()), this, SLOT(handle_note_file_open()));
	connect(but_note_file_save, SIGNAL(pressed()), this, SLOT(handle_note_file_save()));
	connect(but_note_file_save_as, SIGNAL(pressed()), this, SLOT(handle_note_file_save_as()));

	connect(but_midi_file_new, SIGNAL(pressed()), this, SLOT(handle_midi_file_new()));
	connect(but_midi_file_open, SIGNAL(pressed()), this, SLOT(handle_midi_file_open()));
	connect(but_midi_file_save, SIGNAL(pressed()), this, SLOT(handle_midi_file_save()));
	connect(but_midi_file_save_as, SIGNAL(pressed()), this, SLOT(handle_midi_file_save_as()));

	connect(but_midi_trigger, SIGNAL(pressed()), this, SLOT(handle_play_trigger()));
	connect(but_midi_rewind, SIGNAL(pressed()), this, SLOT(handle_rewind()));
	connect(but_config_apply, SIGNAL(pressed()), this, SLOT(handle_config_apply()));
	connect(but_config_load, SIGNAL(pressed()), this, SLOT(handle_config_load()));
	connect(but_synth_program, SIGNAL(pressed()), this, SLOT(handle_synth_program()));

	MidiInit();

	setWindowTitle(tr("MIDI Player Plus v1.0"));

	watchdog->start(250);
}

MppMainWindow :: ~MppMainWindow()
{
}

void
MppMainWindow :: handle_quit()
{
	exit(0);
}

void
MppMainWindow :: handle_jump_N(int index)
{
	pthread_mutex_lock(&mtx);
	handle_jump(index);
	pthread_mutex_unlock(&mtx);
}

void
MppMainWindow :: handle_jump_0()
{
	handle_jump_N(0);
}

void
MppMainWindow :: handle_jump_1()
{
	handle_jump_N(1);
}

void
MppMainWindow :: handle_jump_2()
{
	handle_jump_N(2);
}

void
MppMainWindow :: handle_jump_3()
{
	handle_jump_N(3);
}

void
MppMainWindow :: handle_track_N(int index)
{
	uint32_t mask = (1UL << index);
	uint32_t val;

	pthread_mutex_lock(&mtx);
	main_sc.ScTrackMask ^= mask;
	val = main_sc.ScTrackMask & mask;
	pthread_mutex_unlock(&mtx);

	if (val)
		lbl_track[index]->setText(tr("OFF"));
	else
		lbl_track[index]->setText(tr("ON"));
}

void
MppMainWindow :: handle_track_0()
{
	handle_track_N(0);
}

void
MppMainWindow :: handle_track_1()
{
	handle_track_N(1);
}

void
MppMainWindow :: handle_track_2()
{
	handle_track_N(2);
}

void
MppMainWindow :: handle_track_3()
{
	handle_track_N(3);
}

void
MppMainWindow :: handle_play_key_changed(int key)
{
	key &= 0x7F;

	lbl_play_key->setText(tr("Play Key ") + QString(mid_key_str[key]));
}

void
MppMainWindow :: handle_cmd_key_changed(int key)
{
	key &= 0x7F;

	pthread_mutex_lock(&mtx);
	main_sc.ScCmdKey = key;
	pthread_mutex_unlock(&mtx);

	lbl_cmd_key->setText(tr("Cmd Key ") + QString(mid_key_str[key]));
}

void
MppMainWindow :: handle_pass_thru()
{
	pthread_mutex_lock(&mtx);
	main_sc.is_midi_pass_thru_off = !main_sc.is_midi_pass_thru_off;
	pthread_mutex_unlock(&mtx);

	if (main_sc.is_midi_pass_thru_off == 0)
		lbl_midi_pass_thru->setText(tr("ON"));
	else
		lbl_midi_pass_thru->setText(tr("OFF"));
}

void
MppMainWindow :: handle_compile()
{
	pthread_mutex_lock(&mtx);
	handle_stop();
	MppParse(&main_sc, main_edit->toPlainText());
	pthread_mutex_unlock(&mtx);
}

void
MppMainWindow :: handle_note_record()
{
	pthread_mutex_lock(&mtx);
	main_sc.is_note_record_off = !main_sc.is_note_record_off;
	pthread_mutex_unlock(&mtx);

	if (main_sc.is_note_record_off == 0)
		lbl_note_record->setText(tr("ON"));
	else
		lbl_note_record->setText(tr("OFF"));
}

void
MppMainWindow :: handle_midi_play()
{
	pthread_mutex_lock(&mtx);
	main_sc.is_midi_play_off = !main_sc.is_midi_play_off;
	main_sc.is_midi_triggered = 0;
	update_play_device_no();
	pthread_mutex_unlock(&mtx);

	if (main_sc.is_midi_play_off == 0)
		lbl_midi_play->setText(tr("ON"));
	else {
		handle_rewind();
		lbl_midi_play->setText(tr("OFF"));
	}
}

void
MppMainWindow :: handle_midi_record()
{
	pthread_mutex_lock(&mtx);
	main_sc.is_midi_record_off = !main_sc.is_midi_record_off;
	main_sc.is_midi_triggered = 0;
	update_play_device_no();
	pthread_mutex_unlock(&mtx);

	if (main_sc.is_midi_record_off == 0)
		lbl_midi_record->setText(tr("ON"));
	else
		lbl_midi_record->setText(tr("OFF"));
}


void
MppMainWindow :: handle_play_press()
{
	int vel = spn_volume->value();
	int key = spn_play_key->value();

	pthread_mutex_lock(&mtx);
	handle_key_press(key, vel);
	pthread_mutex_unlock(&mtx);
}

void
MppMainWindow :: handle_play_release()
{
	int key = spn_play_key->value();

	pthread_mutex_lock(&mtx);
	handle_key_release(key);
	pthread_mutex_unlock(&mtx);
}

void
MppMainWindow :: handle_watchdog()
{
	QTextCursor cursor(main_edit->textCursor());
	uint8_t events_copy[MPP_MAX_QUEUE];
	uint8_t num_events;
	uint8_t x;

	pthread_mutex_lock(&mtx);
	num_events = main_sc.ScNumInputEvents;
	main_sc.ScNumInputEvents = 0;
	memcpy(events_copy, main_sc.ScInputEvents, num_events);
	pthread_mutex_unlock(&mtx);

	for (x = 0; x != num_events; x++) {

		cursor.movePosition(QTextCursor::EndOfWord, QTextCursor::MoveAnchor, 1);
		cursor.beginEditBlock();
		cursor.insertText(QString(" "));
		cursor.insertText(QString(mid_key_str[events_copy[x] & 0x7F]));
		cursor.endEditBlock();
	}
}


void
MppMainWindow :: handle_note_file_new()
{
	main_edit->setText(QString());
	handle_compile();
	if (CurrNoteFileName != NULL) {
		delete (CurrNoteFileName);
		CurrNoteFileName = NULL;
	}
}

void
MppMainWindow :: handle_note_file_open()
{
	QFileDialog *diag = 
	  new QFileDialog(this, tr("Select Note File"), 
		QString(), QString("Note file (*.txt)"));
	QString notes;

	diag->setAcceptMode(QFileDialog::AcceptOpen);
	diag->setFileMode(QFileDialog::ExistingFile);

	handle_note_file_new();

	if (diag->exec()) {
		CurrNoteFileName = new QString(diag->selectedFiles()[0]);
		notes = MppReadFile(*CurrNoteFileName);
		if (notes != NULL) {
			main_edit->setText(notes);
			handle_compile();
		}
	}

	delete diag;
}

void
MppMainWindow :: handle_note_file_save()
{
	if (CurrNoteFileName != NULL) {
		MppWriteFile(*CurrNoteFileName, main_edit->toPlainText());
	} else {
		handle_note_file_save_as();
	}
}

void
MppMainWindow :: handle_note_file_save_as()
{
	QFileDialog *diag = 
	  new QFileDialog(this, tr("Select Note File"), 
		QString(), QString("Note file (*.txt)"));

	diag->setAcceptMode(QFileDialog::AcceptSave);
	diag->setFileMode(QFileDialog::AnyFile);

	if (diag->exec()) {
		if (CurrNoteFileName != NULL)
			delete (CurrNoteFileName);

		CurrNoteFileName = new QString(diag->selectedFiles()[0]);

		if (CurrNoteFileName != NULL)
			handle_note_file_save();
	}

	delete diag;
}

void
MppMainWindow :: handle_midi_file_new()
{
	if (CurrMidiFileName != NULL) {
		delete (CurrMidiFileName);
		CurrMidiFileName = NULL;
	}

	handle_rewind();

	if (track != NULL) {
		pthread_mutex_lock(&mtx);
		umidi20_event_queue_drain(&(track->queue));
		pthread_mutex_unlock(&mtx);
	}
}

void
MppMainWindow :: update_play_device_no()
{
	uint8_t device_no = main_sc.ScPlayDevice;
	struct umidi20_event *event;

	if (track == NULL)
		return;

	UMIDI20_QUEUE_FOREACH(event, &(track->queue)) {
		event->device_no = device_no;
	}
}

void
MppMainWindow :: handle_midi_file_open()
{
	QFileDialog *diag = 
	  new QFileDialog(this, tr("Select Midi File"), 
		QString(), QString("MIDI file (*.mid)"));
	struct umidi20_song *song_copy;
	struct umidi20_track *track_copy;
	struct umidi20_event *event;
	struct umidi20_event *event_copy;
	const char *filename;

	diag->setAcceptMode(QFileDialog::AcceptOpen);
	diag->setFileMode(QFileDialog::ExistingFile);

	handle_midi_file_new();

	if (diag->exec()) {
		CurrMidiFileName = new QString(diag->selectedFiles()[0]);

		filename = MppQStringToAscii(*CurrMidiFileName);

		if (filename != NULL) {
			pthread_mutex_lock(&mtx);
			song_copy = umidi20_load_file(&mtx, filename);
			pthread_mutex_unlock(&mtx);

			free((void *)filename);

			if (song_copy != NULL) {
				goto load_file;
			}
		}
	}

	goto done;

load_file:

	printf("format %d\n", song_copy->midi_file_format);
	printf("resolution %d\n", song_copy->midi_resolution);
	printf("division_type %d\n", song_copy->midi_division_type);

	pthread_mutex_lock(&mtx);

	UMIDI20_QUEUE_FOREACH(track_copy, &(song_copy->queue)) {

	    printf("track %p\n", track_copy);

	    UMIDI20_QUEUE_FOREACH(event, &(track_copy->queue)) {

	        if (umidi20_event_is_voice(event) ||
		    umidi20_event_is_sysex(event)) {

		    event_copy = umidi20_event_copy(event, 0);

		    if (event_copy != NULL) {
			umidi20_event_queue_insert(&(track->queue),
			    event_copy, UMIDI20_CACHE_INPUT);
		    }
		}
	    }
	}

	umidi20_song_free(song_copy);

	pthread_mutex_unlock(&mtx);

done:
	delete diag;
}

void
MppMainWindow :: handle_midi_file_save()
{
	const char *filename;

	if (CurrMidiFileName != NULL) {

		filename = MppQStringToAscii(*CurrMidiFileName);

		if (filename != NULL) {
			pthread_mutex_lock(&mtx);
			umidi20_save_file(song, filename);
			pthread_mutex_unlock(&mtx);
			free((void *)filename);
		}
	} else {
		handle_midi_file_save_as();
	}
}

void
MppMainWindow :: handle_midi_file_save_as()
{
	QFileDialog *diag = 
	  new QFileDialog(this, tr("Select MIDI File"), 
		QString(), QString("MIDI file (*.mid)"));

	diag->setAcceptMode(QFileDialog::AcceptSave);
	diag->setFileMode(QFileDialog::AnyFile);

	if (diag->exec()) {
		if (CurrMidiFileName != NULL)
			delete (CurrMidiFileName);

		CurrMidiFileName = new QString(diag->selectedFiles()[0]);

		if (CurrMidiFileName != NULL)
			handle_midi_file_save();
	}

	delete diag;
}

void
MppMainWindow :: handle_rewind()
{
	pthread_mutex_lock(&mtx);

	main_sc.is_midi_triggered = 0;
	update_play_device_no();

	if (song != NULL) {
		umidi20_song_stop(song, UMIDI20_FLAG_PLAY | UMIDI20_FLAG_RECORD);
		umidi20_song_start(song, 0x40000000, 0x80000000, UMIDI20_FLAG_PLAY | UMIDI20_FLAG_RECORD);
		main_sc.ScPosition = umidi20_get_curr_position();
	}

	pthread_mutex_unlock(&mtx);
}

void
MppMainWindow :: handle_play_trigger()
{
	pthread_mutex_lock(&mtx);

	if (main_sc.is_midi_triggered == 0) {
		if (main_sc.is_midi_play_off == 0) {
			umidi20_song_stop(song, UMIDI20_FLAG_PLAY | UMIDI20_FLAG_RECORD);
			umidi20_song_start(song, 0, 0x40000000, UMIDI20_FLAG_PLAY | UMIDI20_FLAG_RECORD);
			main_sc.ScPosition = umidi20_get_curr_position();

		} else {
			umidi20_song_stop(song, UMIDI20_FLAG_PLAY | UMIDI20_FLAG_RECORD);
			umidi20_song_start(song, 0x40000000, 0x80000000, UMIDI20_FLAG_PLAY | UMIDI20_FLAG_RECORD);
			main_sc.ScPosition = umidi20_get_curr_position();

		}
		main_sc.is_midi_triggered = 1;
	}

	pthread_mutex_unlock(&mtx);
}

void
MppMainWindow :: handle_synth_program()
{
	struct mid_data *d = &mid_data;
	int chan = spn_channel->value();
	int bank = spn_bank->value();
	int prog = spn_prog->value();
	uint8_t y;

	pthread_mutex_lock(&mtx);

	main_sc.ScChannel = chan;

	for (y = 0; y != MPP_MAX_DEVS; y++) {
		if (check_synth(y)) {
			mid_set_bank_program(d, chan, bank, prog);
		}
	}

	if (check_record()) {
		mid_set_bank_program(d, chan, bank, prog);
	}

	pthread_mutex_unlock(&mtx);
}

void
MppMainWindow :: handle_config_reload()
{
	struct umidi20_config cfg;
	int n;

	/* setup the I/O devices */

	umidi20_config_export(&cfg);

	main_sc.ScPlayDevice = 0;

	for (n = 0; n != MPP_MAX_DEVS; n++) {

		if ((main_sc.ScDeviceBits & (MPP_DEV0_RECORD << (3 * n))) && 
		    (main_sc.ScDeviceName[n] != NULL) && 
		    (main_sc.ScDeviceName[n][0] != 0))  {
			strlcpy(cfg.cfg_dev[n].rec_fname, main_sc.ScDeviceName[n],
			    sizeof(cfg.cfg_dev[n].rec_fname));
			cfg.cfg_dev[n].rec_enabled_cfg = 1;
		} else {
			cfg.cfg_dev[n].rec_enabled_cfg = 0;
		}

		if ((main_sc.ScDeviceBits & ((MPP_DEV0_SYNTH | MPP_DEV0_PLAY) << (3 * n))) && 
		    (main_sc.ScDeviceName[n] != NULL) && 
		    (main_sc.ScDeviceName[n][0] != 0))  {
			strlcpy(cfg.cfg_dev[n].play_fname, main_sc.ScDeviceName[n],
			    sizeof(cfg.cfg_dev[n].play_fname));
			cfg.cfg_dev[n].play_enabled_cfg = 1;
		} else {
			cfg.cfg_dev[n].play_enabled_cfg = 0;
		}

		if (main_sc.ScDeviceBits & (MPP_DEV0_PLAY << (3 * n))) {
			main_sc.ScPlayDevice = n;
		}
	}

	umidi20_config_import(&cfg);

	main_sc.ScDeviceBits &= ~(MPP_DEV0_PLAY|MPP_DEV1_PLAY|MPP_DEV2_PLAY);
	main_sc.ScDeviceBits |= MPP_DEV0_PLAY << (3 * main_sc.ScPlayDevice);

	handle_compile();

	handle_config_load();
}

void
MppMainWindow :: handle_config_load()
{
	int n;

	for (n = 0; n != MPP_MAX_DEVS; n++) {

		if (main_sc.ScDeviceName[n] != NULL)
			led_config_dev[n]->setText(QString(main_sc.ScDeviceName[n]));
		else
			led_config_dev[n]->setText(QString());

		cbx_config_dev[(3*n)+0]->setChecked(
		    (main_sc.ScDeviceBits & (1UL << ((3*n)+0))) ? 1 : 0);

		cbx_config_dev[(3*n)+1]->setChecked(
		    (main_sc.ScDeviceBits & (1UL << ((3*n)+1))) ? 1 : 0);

		cbx_config_dev[(3*n)+2]->setChecked(
		    (main_sc.ScDeviceBits & (1UL << ((3*n)+2))) ? 1 : 0);
	}
}

void
MppMainWindow :: handle_config_apply()
{
	int n;

	main_sc.ScDeviceBits = 0;

	for (n = 0; n != MPP_MAX_DEVS; n++) {

		if (main_sc.ScDeviceName[n] != NULL)
			free(main_sc.ScDeviceName[n]);

		main_sc.ScDeviceName[n] = MppQStringToAscii(led_config_dev[n]->text());

		if (cbx_config_dev[(3*n)+0]->isChecked())
			main_sc.ScDeviceBits |= 1UL << ((3*n)+0);
		if (cbx_config_dev[(3*n)+1]->isChecked())
			main_sc.ScDeviceBits |= 1UL << ((3*n)+1);
		if (cbx_config_dev[(3*n)+2]->isChecked())
			main_sc.ScDeviceBits |= 1UL << ((3*n)+2);
	}

	handle_config_reload();
}

uint8_t
MppMainWindow :: check_synth(uint8_t device_no)
{
	struct mid_data *d = &mid_data;

	if (device_no >= MPP_MAX_DEVS)
		return (0);

	if (main_sc.ScDeviceBits & (MPP_DEV0_SYNTH << (3 * device_no))) {

		handle_play_trigger();

		mid_set_channel(d, main_sc.ScChannel);
		mid_set_position(d, umidi20_get_curr_position() - main_sc.ScPosition + 1);
		mid_set_device_no(d, device_no);

		return (1);
	}
	return (0);
}

uint8_t
MppMainWindow :: check_record()
{
	struct mid_data *d = &mid_data;

	if (main_sc.is_midi_record_off)
		return (0);

	handle_play_trigger();

	mid_set_channel(d, main_sc.ScChannel);
	mid_set_position(d, umidi20_get_curr_position() - main_sc.ScPosition + 1);
	mid_set_device_no(d, 0xFF);

	return (1);
}

void
MppMainWindow :: handle_stop(void)
{
	struct mid_data *d = &mid_data;
	uint8_t x;
	uint8_t y;

	for (x = 0; x != 128; x++) {
		if (main_sc.ScPressed[x] != 0) {
			main_sc.ScPressed[x] = 0;

			for (y = 0; y != MPP_MAX_DEVS; y++) {
				if (check_synth(y)) {
					mid_key_press(d, x, 0, 0);
				}
			}

			if (check_record()) {
				mid_key_press(d, x, 0, 0);
			}
		}
	}
}

uint8_t
MppMainWindow :: handle_jump(int pos)
{
	if ((pos < 0) || (pos > 3) || (main_sc.ScJumpTable[pos] == 0))
		return (0);

	main_sc.ScCurrPos = main_sc.ScJumpTable[pos] - 1;

	handle_stop();

	return (1);
}

void
MppMainWindow :: handle_key_press(int in_key, int vel)
{
	struct mid_data *d = &mid_data;
	struct MppNote *pn;
	uint16_t pos;
	uint8_t x;
	uint8_t y;
	uint8_t out_key;

	pos = main_sc.ScJumpNext[main_sc.ScCurrPos];
	if (pos != 0)
		main_sc.ScCurrPos = pos - 1;

	pn = &main_sc.ScNotes[main_sc.ScCurrPos][0];

	for (x = 0; x != MPP_MAX_NOTES; x++) {

		if (pn->dur != 0) {
			out_key = pn->key + (in_key - C4);
			out_key &= 127;

			/* check if channel is masked */
			if (main_sc.ScTrackMask & (1UL << pn->channel))
				continue;

			for (y = 0; y != MPP_MAX_DEVS; y++) {
				if (check_synth(y)) {
					mid_key_press(d, out_key, vel, 0);
				}
			}

			if (check_record()) {
				mid_key_press(d, out_key, vel, 0);
			}

			main_sc.ScPressed[out_key] = pn->dur;
		}
		pn++;
	}

	main_sc.ScCurrPos++;

	if (main_sc.ScCurrPos >= main_sc.ScLinesMax)
		main_sc.ScCurrPos = 0;
}

/* must be called locked */
void
MppMainWindow :: handle_key_release(int in_key)
{
	struct mid_data *d = &mid_data;
	uint8_t x;
	uint8_t y;

	for (x = 0; x != 128; x++) {
		if (main_sc.ScPressed[x] == 1) {

			for (y = 0; y != MPP_MAX_DEVS; y++) {
				if (check_synth(y)) {
					mid_key_press(d, x, 0, 0);
				}
			}

			if (check_record()) {
				mid_key_press(d, x, 0, 0);
			}
		}

		if (main_sc.ScPressed[x] != 0)
			main_sc.ScPressed[x] --;
	}
}

static void
MidiEventCallback(uint8_t device_no, void *arg, struct umidi20_event *event, uint8_t *drop)
{
	MppMainWindow *mw = (MppMainWindow *)arg;
	struct mid_data *d = &mw->mid_data;
	uint8_t y;
	int key;
	int vel;

	*drop = 1;

	if (umidi20_event_get_control_address(event) == 0x40) {

		for (y = 0; y != MPP_MAX_DEVS; y++) {
			if (mw->check_synth(y)) {
				mid_pedal(d, umidi20_event_get_control_value(event));
			}
		}

		if (mw->check_record()) {
			mid_pedal(d, umidi20_event_get_control_value(event));
		}

	} else if (umidi20_event_is_key_start(event)) {

		key = umidi20_event_get_key(event) & 0x7F;
		vel = umidi20_event_get_velocity(event);

		if (mw->main_sc.is_note_record_off == 0) {
			if (mw->main_sc.ScNumInputEvents < MPP_MAX_QUEUE) {
				mw->main_sc.ScInputEvents[mw->main_sc.ScNumInputEvents] = key;
				mw->main_sc.ScNumInputEvents++;
			}
		}

		if (mw->main_sc.is_midi_pass_thru_off != 0) {
			if (mw->handle_jump(key - mw->main_sc.ScCmdKey) == 0) {
				mw->handle_key_press(key, vel);
			}
		} else {

			for (y = 0; y != MPP_MAX_DEVS; y++) {
				if (mw->check_synth(y)) {
					mid_key_press(d, key, vel, 0);
				}
			}

			if (mw->check_record()) {
				mid_key_press(d, key, vel, 0);
			}

			mw->main_sc.ScPressed[key] = 1;
		}
	} else if (umidi20_event_is_key_end(event)) {

		key = umidi20_event_get_key(event) & 0x7F;

		if (mw->main_sc.is_midi_pass_thru_off != 0) {

			mw->handle_key_release(key);

		} else {

			for (y = 0; y != MPP_MAX_DEVS; y++) {
				if (mw->check_synth(y)) {
					mid_key_press(d, key, 0, 0);
				}
			}

			if (mw->check_record()) {
				mid_key_press(d, key, 0, 0);
			}

			mw->main_sc.ScPressed[key] = 0;
		}
	}
}

void
MppMainWindow :: MidiInit(void)
{
	int n;

	main_sc.ScTrackMask ^= 0x0F;
	main_sc.ScDeviceBits = MPP_DEV0_SYNTH | MPP_DEV0_PLAY | MPP_DEV1_RECORD | MPP_DEV2_RECORD;
	main_sc.ScDeviceName[0] = strdup("/midi");
	main_sc.ScDeviceName[1] = strdup("/dev/umidi0.0");
	main_sc.ScDeviceName[2] = strdup("/dev/umidi1.0");

	handle_track_N(0);
	handle_track_N(1);
	handle_track_N(2);
	handle_track_N(3);

	handle_midi_record();
	handle_midi_play();
	handle_note_record();
	handle_pass_thru();

	umidi20_init();

	handle_config_reload();

	for (n = 0; n != UMIDI20_N_DEVICES; n++)
		umidi20_set_record_event_callback(n, &MidiEventCallback, this);

	pthread_mutex_lock(&mtx);

	song = umidi20_song_alloc(&mtx, UMIDI20_FILE_FORMAT_TYPE_0, 500,
	    UMIDI20_FILE_DIVISION_TYPE_PPQ);

	track = umidi20_track_alloc();

	if (song == NULL || track == NULL) {
		pthread_mutex_unlock(&mtx);
		err(1, "Could not allocate new song or track\n");
	}
	umidi20_song_track_add(song, NULL, track, 0);

	umidi20_song_set_record_track(song, track);

	/* get the MIDI up! */

	mid_init(&mid_data, track);

	umidi20_song_start(song, 0x40000000, 0x80000000, UMIDI20_FLAG_PLAY | UMIDI20_FLAG_RECORD);
	main_sc.ScPosition = umidi20_get_curr_position();

	pthread_mutex_unlock(&mtx);
}

void
MppMainWindow :: MidiUnInit(void)
{
	int n;

	handle_rewind();

	pthread_mutex_lock(&mtx);

	umidi20_song_free(song);

	song = NULL;

	umidi20_song_stop(song, UMIDI20_FLAG_PLAY | UMIDI20_FLAG_RECORD);

	pthread_mutex_unlock(&mtx);

	for (n = 0; n != MPP_MAX_DEVS; n++) {
		if (main_sc.ScDeviceName[n] != NULL) {
			free (main_sc.ScDeviceName[n]);
			main_sc.ScDeviceName[n] = NULL;
		}
	}
}

void
MppMainWindow :: keyPressEvent(QKeyEvent *event)
{
#if 0
	if (event->isAutoRepeat())
		return;

	if (event->key() == Qt::Key_PageUp || 
	    event->key() == Qt::Key_PageDown) {
		handle_play_press();
	}
#endif
}

void
MppMainWindow :: keyReleaseEvent(QKeyEvent *event)
{
#if 0
	if (event->isAutoRepeat())
		return;

	if (event->key() == Qt::Key_PageUp || 
	    event->key() == Qt::Key_PageDown) {
		handle_play_release();
	}
#endif
}

int
main(int argc, char **argv)
{
	QApplication app(argc, argv);
	MppMainWindow main;

	main.show();

	return (app.exec());
}
