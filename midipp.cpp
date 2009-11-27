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
	main_gl = new QGridLayout(this);
	main_tw = new QTabWidget();

	/* Editor */

	main_edit = new QTextEdit();

	main_gl->addWidget(main_edit,0,0,1,3);

	/* Tabs */

	main_gl->addWidget(main_tw,0,3,1,1);

	tab_file_wg = new QWidget();
	tab_edit_wg = new QWidget();

	tab_file_gl = new QGridLayout(tab_file_wg);
	tab_edit_gl = new QGridLayout(tab_edit_wg);

	main_tw->addTab(tab_file_wg, tr("File"));
	main_tw->addTab(tab_edit_wg, tr("Edit"));


	/* File Tab */

	but_new_file = new QPushButton(tr("New File"));
	but_open_file = new QPushButton(tr("Open File"));
	but_save_file = new QPushButton(tr("Save File"));
	but_save_file_as = new QPushButton(tr("Save File As"));
	but_print_file = new QPushButton(tr("Print"));
	but_configure = new QPushButton(tr("Configure"));
	but_quit = new QPushButton(tr("Quit"));

	tab_file_gl->addWidget(but_new_file, 0, 0, 1, 4);
	tab_file_gl->addWidget(but_open_file, 1, 0, 1, 4);
	tab_file_gl->addWidget(but_save_file, 2, 0, 1, 4);
	tab_file_gl->addWidget(but_save_file_as, 3, 0, 1, 4);
	tab_file_gl->addWidget(but_print_file, 4, 0, 1, 4);
	tab_file_gl->addWidget(but_configure, 5, 0, 1, 4);
	tab_file_gl->addWidget(but_quit, 6, 0, 1, 4);

	/* Edit Tab */

	but_pg_up = new QPushButton(tr("Page Up"));
	but_up = new QPushButton(tr("Up"));
	but_down = new QPushButton(tr("Down"));
	but_pg_down = new QPushButton(tr("Page Down"));
	but_compile = new QPushButton(tr("Compile"));
	but_record = new QPushButton(tr("Record"));
	but_play = new QPushButton(tr("Play"));
	but_undo = new QPushButton(tr("Undo"));

	lbl_volume = new QLabel(tr("Volume"));
	spn_volume = new QSpinBox();
	spn_volume->setMaximum(127);
	spn_volume->setMinimum(0);
	spn_volume->setValue(80);

	lbl_key = new QLabel(tr("Key"));
	spn_key = new QSpinBox();
	spn_key->setMaximum(127);
	spn_key->setMinimum(0);
	spn_key->setValue(C4);

	tab_edit_gl->addWidget(lbl_volume, 0, 0, 1, 3);
	tab_edit_gl->addWidget(spn_volume, 0, 3, 1, 1);
	tab_edit_gl->addWidget(lbl_key, 1, 0, 1, 3);
	tab_edit_gl->addWidget(spn_key, 1, 3, 1, 1);
	tab_edit_gl->addWidget(but_pg_up, 2, 0, 1, 4);
	tab_edit_gl->addWidget(but_up, 3, 0, 1, 4);
	tab_edit_gl->addWidget(but_down, 4, 0, 1, 4);
	tab_edit_gl->addWidget(but_pg_down, 5, 0, 1, 4);
	tab_edit_gl->addWidget(but_compile, 6, 0, 1, 4);
	tab_edit_gl->addWidget(but_record, 7, 0, 1, 4);
	tab_edit_gl->addWidget(but_play, 8, 0, 1, 4);
	tab_edit_gl->addWidget(but_undo, 9, 0, 1, 4);

	connect(but_quit, SIGNAL(pressed()), this, SLOT(handle_quit()));
	connect(but_pg_up, SIGNAL(pressed()), this, SLOT(handle_pg_up()));
	connect(but_up, SIGNAL(pressed()), this, SLOT(handle_up()));
	connect(but_down, SIGNAL(pressed()), this, SLOT(handle_down()));
	connect(but_pg_down, SIGNAL(pressed()), this, SLOT(handle_pg_down()));
	connect(but_compile, SIGNAL(pressed()), this, SLOT(handle_compile()));
	connect(but_record, SIGNAL(pressed()), this, SLOT(handle_record()));
	connect(but_play, SIGNAL(pressed()), this, SLOT(handle_play_press()));
	connect(but_play, SIGNAL(released()), this, SLOT(handle_play_release()));
	connect(but_undo, SIGNAL(pressed()), this, SLOT(handle_undo()));

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
MppMainWindow :: handle_pg_up()
{
}

void
MppMainWindow :: handle_up()
{
}

void
MppMainWindow :: handle_down()
{
}

void
MppMainWindow :: handle_pg_down()
{
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
MppMainWindow :: handle_record()
{
}

void
MppMainWindow :: handle_play_press()
{
	int vel = spn_volume->value();
	int key = spn_key->value();

	pthread_mutex_lock(&mtx);
	MidiEventHandleKeyPress(this, key, vel);
	pthread_mutex_unlock(&mtx);
}

void
MppMainWindow :: handle_play_release()
{
	int key = spn_key->value();

	pthread_mutex_lock(&mtx);
	MidiEventHandleKeyRelease(this, key);
	pthread_mutex_unlock(&mtx);
}

void
MppMainWindow :: handle_undo()
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

#if 0
	memset(notes+index, 0, sizeof(notes[0]));
	notes[index].keys[0] = umidi20_event_get_key(event);
	notes[index].duration[0] = 1;
#endif

	if (umidi20_event_get_control_address(event) == 0x40) {

		mid_set_position(d, umidi20_get_curr_position() + 1);
		mid_pedal(d, umidi20_event_get_control_value(event));

	} else if (umidi20_event_is_key_start(event)) {

		pos = umidi20_event_get_key(event) - C3;

		if (MidiEventHandleJump(mw, pos) == 0) {
			MidiEventHandleKeyPress(mw,
				umidi20_event_get_key(event),
				umidi20_event_get_velocity(event));
		}

	} else if (umidi20_event_is_key_end(event)) {
		MidiEventHandleKeyRelease(mw,
			umidi20_event_get_key(event));
	}
}

void
MppMainWindow :: MidiInit(void)
{
	struct umidi20_config cfg;

	memset(&main_sc, 0, sizeof(main_sc));

	memset(&mid_data, 0, sizeof(mid_data));

	umidi20_mutex_init(&mtx);

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

int
main(int argc, char **argv)
{
	QApplication app(argc, argv);
	MppMainWindow main;

	main.show();

	return (app.exec());
}
