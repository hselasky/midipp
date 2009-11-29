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

static void MidiEventHandleKeyPress(MppMainWindow *mw, int in_key, int vel);
static void MidiEventHandleKeyRelease(MppMainWindow *mw, int in_key);
static void MidiEventHandleStop(MppMainWindow *mw);
static uint8_t MidiEventHandleJump(MppMainWindow *mw, int);

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

	/* set memory default */

	memset(&main_sc, 0, sizeof(main_sc));

	memset(&mid_data, 0, sizeof(mid_data));

	umidi20_mutex_init(&mtx);

	/* Setup GUI */

	main_gl = new QGridLayout(this);
	main_tw = new QTabWidget();

	/* Watchdog */

	watchdog = new QTimer(this);
	connect(watchdog, SIGNAL(timeout()), this, SLOT(handle_watchdog()));
	watchdog->start(250);

	/* Editor */

	main_edit = new QTextEdit();
	main_edit->setText(tr("/* Copyright (c) 2009 Hans Petter Selasky. All rights reserved. */"));

	main_gl->addWidget(main_edit,0,0,1,2);

	/* Tabs */

	main_gl->addWidget(main_tw,0,2,1,1);

	tab_file_wg = new QWidget();
	tab_play_wg = new QWidget();

	tab_file_gl = new QGridLayout(tab_file_wg);
	tab_play_gl = new QGridLayout(tab_play_wg);

	main_tw->addTab(tab_file_wg, tr("File"));
	main_tw->addTab(tab_play_wg, tr("Play"));

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
	but_configure = new QPushButton(tr("Configure"));
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
	tab_file_gl->addWidget(but_configure, n++, 0, 1, 8);
	tab_file_gl->addWidget(but_quit, n++, 0, 1, 8);

	n = 0;

	tab_file_gl->addWidget(lbl_midi_file, n++, 4, 1, 4);
	tab_file_gl->addWidget(but_midi_file_new, n++, 4, 1, 4);
	tab_file_gl->addWidget(but_midi_file_open, n++, 4, 1, 4);
	tab_file_gl->addWidget(but_midi_file_save, n++, 4, 1, 4);
	tab_file_gl->addWidget(but_midi_file_save_as, n++, 4, 1, 4);

	/* Play Tab */

	lbl_note_record = new QLabel(QString());
	lbl_midi_record = new QLabel(QString());
	lbl_midi_pass_thru = new QLabel(QString());

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
	but_midi_play = new QPushButton(tr("Start"));
	but_midi_trigger = new QPushButton(tr("Trigger"));
	but_midi_rewind = new QPushButton(tr("Rewind"));

	but_play = new QPushButton(tr(" \nPlay\n "));

	lbl_volume = new QLabel(tr("Volume"));
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

	lbl_synth = new QLabel(tr("- Synth -"));
	lbl_synth->setAlignment(Qt::AlignHCenter|Qt::AlignVCenter);

	lbl_playback = new QLabel(tr("- Playback -"));
	lbl_playback->setAlignment(Qt::AlignHCenter|Qt::AlignVCenter);

	lbl_recording = new QLabel(tr("- Recording -"));
	lbl_recording->setAlignment(Qt::AlignHCenter|Qt::AlignVCenter);

	n = 0;

	tab_play_gl->addWidget(lbl_playback, n++, 0, 1, 4);
	tab_play_gl->addWidget(but_midi_play, n++, 0, 1, 4);
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
	tab_play_gl->addWidget(but_play, n, 4, 3, 4);
	n += 3;

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

	MidiInit();

	setWindowTitle(tr("MIDI Piano Player"));
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
	MidiEventHandleJump(this, index);
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
	main_sc.ScTrackInvMask ^= mask;
	val = main_sc.ScTrackInvMask & mask;
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
	MidiEventHandleStop(this);
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
MppMainWindow :: handle_midi_record()
{
	pthread_mutex_lock(&mtx);
	main_sc.is_midi_record_off = !main_sc.is_midi_record_off;
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
	MidiEventHandleKeyPress(this, key, vel);
	pthread_mutex_unlock(&mtx);
}

void
MppMainWindow :: handle_play_release()
{
	int key = spn_play_key->value();

	pthread_mutex_lock(&mtx);
	MidiEventHandleKeyRelease(this, key);
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
}

void
MppMainWindow :: handle_midi_file_open()
{
}

void
MppMainWindow :: handle_midi_file_save()
{
}

void
MppMainWindow :: handle_midi_file_save_as()
{
}

static void
MidiEventHandleStop(MppMainWindow *mw)
{
	struct mid_data *d = &mw->mid_data;
	uint8_t x;

	mid_set_position(d, umidi20_get_curr_position() + 1);

	for (x = 0; x != 128; x++) {
		if (mw->main_sc.ScPressed[x] != 0) {
			mw->main_sc.ScPressed[x] = 0;
			mid_key_press(d, x, 0, 0);
		}
	}
}

static uint8_t
MidiEventHandleJump(MppMainWindow *mw, int pos)
{
	if ((pos < 0) || (pos > 3) || (mw->main_sc.ScJumpTable[pos] == 0))
		return (0);

	mw->main_sc.ScCurrPos = mw->main_sc.ScJumpTable[pos] - 1;

	MidiEventHandleStop(mw);

	return (1);
}

static void
MidiEventHandleKeyPress(MppMainWindow *mw, int in_key, int vel)
{
	struct mid_data *d = &mw->mid_data;
	struct MppNote *pn;
	uint16_t pos;
	uint8_t x;
	uint8_t out_key;

	mid_set_position(d, umidi20_get_curr_position() + 1);

	pos = mw->main_sc.ScJumpNext[mw->main_sc.ScCurrPos];
	if (pos != 0)
		mw->main_sc.ScCurrPos = pos - 1;

	pn = &mw->main_sc.ScNotes[mw->main_sc.ScCurrPos][0];

	for (x = 0; x != MPP_MAX_NOTES; x++) {

		if (pn->dur != 0) {
			out_key = pn->key + (in_key - C4);
			out_key &= 127;

			mid_set_channel(d, pn->channel);

			mid_key_press(d, out_key, vel, 0);

			mw->main_sc.ScPressed[out_key] = pn->dur;
		}
		pn++;
	}

	mw->main_sc.ScCurrPos++;

	if (mw->main_sc.ScCurrPos >= mw->main_sc.ScLinesMax)
		mw->main_sc.ScCurrPos = 0;
}

static void
MidiEventHandleKeyRelease(MppMainWindow *mw, int in_key)
{
	struct mid_data *d = &mw->mid_data;
	uint8_t x;

	mid_set_position(d, umidi20_get_curr_position() + 1);

	for (x = 0; x != 128; x++) {
		if (mw->main_sc.ScPressed[x] == 1) {
			mid_key_press(d, x, 0, 0);
		}

		if (mw->main_sc.ScPressed[x] != 0)
			mw->main_sc.ScPressed[x] --;
	}
}

static void
MidiEventCallback(uint8_t device_no, void *arg, struct umidi20_event *event)
{
	MppMainWindow *mw = (MppMainWindow *)arg;
	struct mid_data *d = &mw->mid_data;
	int pos;
	int vel;

	if (umidi20_event_get_control_address(event) == 0x40) {

		mid_set_position(d, umidi20_get_curr_position() + 1);
		mid_pedal(d, umidi20_event_get_control_value(event));

	} else if (umidi20_event_is_key_start(event)) {

		pos = umidi20_event_get_key(event) & 0x7F;
		vel = umidi20_event_get_velocity(event);

		if (mw->main_sc.is_note_record_off == 0) {
			if (mw->main_sc.ScNumInputEvents < MPP_MAX_QUEUE) {
				mw->main_sc.ScInputEvents[mw->main_sc.ScNumInputEvents] = pos;
				mw->main_sc.ScNumInputEvents++;
			}
		}

		if (mw->main_sc.is_midi_pass_thru_off != 0) {
			if (MidiEventHandleJump(mw, pos - mw->main_sc.ScCmdKey) == 0) {
				MidiEventHandleKeyPress(mw, pos, vel);
			}
		} else {
			mid_set_position(d, umidi20_get_curr_position() + 1);

			mid_set_channel(d, 0);

			mid_key_press(d, pos, vel, 0);

			mw->main_sc.ScPressed[pos] = 1;
		}
	} else if (umidi20_event_is_key_end(event)) {

		pos = umidi20_event_get_key(event) & 0x7F;

		if (mw->main_sc.is_midi_pass_thru_off != 0) {
			MidiEventHandleKeyRelease(mw, pos);
		} else {
			mid_set_position(d, umidi20_get_curr_position() + 1);

			mid_set_channel(d, 0);

			mid_key_press(d, pos, 0, 0);

			mw->main_sc.ScPressed[pos] = 0;
		}
	}
}

void
MppMainWindow :: MidiInit(void)
{
	struct umidi20_config cfg;

	handle_track_N(0);
	handle_track_N(1);
	handle_track_N(2);
	handle_track_N(3);
	handle_midi_record();
	handle_note_record();
	handle_pass_thru();

	umidi20_init();

	/* setup the I/O devices */

	umidi20_config_export(&cfg);

	strlcpy(cfg.cfg_dev[0].rec_fname, "/dev/umidi0.0",
	    sizeof(cfg.cfg_dev[0].rec_fname));

	strlcpy(cfg.cfg_dev[1].rec_fname, "/dev/umidi1.0",
	    sizeof(cfg.cfg_dev[1].rec_fname));

	cfg.cfg_dev[0].rec_enabled_cfg = 1;
	cfg.cfg_dev[1].rec_enabled_cfg = 1;

	strlcpy(cfg.cfg_dev[0].play_fname, "/midi",
	    sizeof(cfg.cfg_dev[0].play_fname));

	cfg.cfg_dev[0].play_enabled_cfg = 1;

	umidi20_config_import(&cfg);

	umidi20_set_record_event_callback(0, &MidiEventCallback, this);
	umidi20_set_record_event_callback(1, &MidiEventCallback, this);

	pthread_mutex_lock(&mtx);

	song = umidi20_song_alloc(&mtx, UMIDI20_FILE_FORMAT_TYPE_0, 500,
	    UMIDI20_FILE_DIVISION_TYPE_PPQ);

	track = umidi20_track_alloc();

	if (song == NULL) {
		pthread_mutex_unlock(&mtx);
		err(1, "Could not allocate new song\n");
	}
	umidi20_song_track_add(song, NULL, track, 0);

	umidi20_song_set_record_track(song, track);

	/* get the line up! */

	mid_init(&mid_data, track);

	mid_delay_all(&mid_data, 1);

	mid_set_device_no(&mid_data, 0);

	umidi20_song_start(song, 0, 0x80000000,
		UMIDI20_FLAG_PLAY | UMIDI20_FLAG_RECORD);

	pthread_mutex_unlock(&mtx);
}

void
MppMainWindow :: MidiUnInit(void)
{
	pthread_mutex_lock(&mtx);

	umidi20_song_stop(song, UMIDI20_FLAG_PLAY | UMIDI20_FLAG_RECORD);

	mid_dump(&mid_data);

	umidi20_song_free(song);

	pthread_mutex_unlock(&mtx);
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
