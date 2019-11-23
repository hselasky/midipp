/*-
 * Copyright (c) 2009-2019 Hans Petter Selasky. All rights reserved.
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

#include <unistd.h>

#include "midipp_chansel.h"
#include "midipp_mainwindow.h"
#include "midipp_scores.h"
#include "midipp_mutemap.h"
#include "midipp_looptab.h"
#include "midipp_database.h"
#include "midipp_import.h"
#include "midipp_devices.h"
#include "midipp_spinbox.h"
#include "midipp_bpm.h"
#include "midipp_button.h"
#include "midipp_buttonmap.h"
#include "midipp_gpro.h"
#include "midipp_groupbox.h"
#include "midipp_gridlayout.h"
#include "midipp_midi.h"
#include "midipp_mode.h"
#include "midipp_settings.h"
#include "midipp_checkbox.h"
#include "midipp_show.h"
#include "midipp_custom.h"
#include "midipp_tabbar.h"
#include "midipp_shortcut.h"
#include "midipp_pianotab.h"
#include "midipp_decode.h"
#include "midipp_replace.h"
#include "midipp_replay.h"
#include "midipp_metronome.h"
#include "midipp_sheet.h"
#include "midipp_musicxml.h"
#include "midipp_instrument.h"
#include "midipp_volume.h"
#include "midipp_devsel.h"

uint8_t
MppMainWindow :: noise8(uint8_t factor)
{
	uint32_t temp;
	const uint32_t prime = 0xFFFF1D;

	if (factor == 0)
		return (0);

	if (noiseRem & 1)
		noiseRem += prime;

	noiseRem /= 2;

	temp = noiseRem * factor;

	return (temp >> 24);
}

MppMainWindow :: MppMainWindow(QWidget *parent)
  : QWidget(parent)
{
	QLabel *pl;
	int n;
	int x;

	/* set memory default */

	memset(auto_zero_start, 0, auto_zero_end - auto_zero_start);

	CurrMidiFileName = NULL;
	song = NULL;
	memset(track, 0, sizeof(track));

	umidi20_mutex_init(&mtx);

	noiseRem = 1;

	defaultFont.fromString(QString("Sans Serif,-1,20,5,75,0,0,0,0,0"));

	editFont.fromString(QString(
#ifdef __APPLE__
		    "Courier New,-1,14,5,50,0,0,0,0,0"
#else
		    "Monospace,-1,14,5,50,0,0,0,0,0"
#endif
		    ));
	editFont.setStyleHint(QFont::TypeWriter);

	/* Main GUI */

	mwRewind = new QPushButton();
	mwPlay = new QPushButton();
	mwReload = new QPushButton();
	mwPaste = new QPushButton();
	mwCopy = new QPushButton();
	mwUndo = new QPushButton();
	mwRedo = new QPushButton();
	mwEdit = new QPushButton();
	mwUpDown = new QPushButton();

	mwRewind->setToolTip(tr("Rewind"));
	mwPlay->setToolTip(tr("Trigger"));
	mwReload->setToolTip(tr("Recompile"));
	mwPaste->setToolTip(tr("Paste"));
	mwCopy->setToolTip(tr("Copy"));
	mwUndo->setToolTip(tr("Undo"));
	mwRedo->setToolTip(tr("Redo"));
	mwEdit->setToolTip(tr("Edit or Insert a Chord"));
	mwUpDown->setToolTip(tr("Move menu Up or Down"));

	mwRewind->setIcon(QIcon(QString(":/stop.png")));
	mwPlay->setIcon(QIcon(QString(":/play.png")));
	mwReload->setIcon(QIcon(QString(":/reload.png")));
	mwPaste->setIcon(QIcon(QString(":/paste.png")));
	mwCopy->setIcon(QIcon(QString(":/copy.png")));
	mwUndo->setIcon(QIcon(QString(":/undo.png")));
	mwRedo->setIcon(QIcon(QString(":/redo.png")));
	mwEdit->setIcon(QIcon(QString(":/edit.png")));
	mwUpDown->setIcon(QIcon(QString(":/up_down.png")));

	mwRewind->setFocusPolicy(Qt::NoFocus);
	mwPlay->setFocusPolicy(Qt::NoFocus);
	mwReload->setFocusPolicy(Qt::NoFocus);
	mwPaste->setFocusPolicy(Qt::NoFocus);
	mwCopy->setFocusPolicy(Qt::NoFocus);
	mwUndo->setFocusPolicy(Qt::NoFocus);
	mwRedo->setFocusPolicy(Qt::NoFocus);
	mwEdit->setFocusPolicy(Qt::NoFocus);
	mwUpDown->setFocusPolicy(Qt::NoFocus);

	connect(mwRewind, SIGNAL(released()), this, SLOT(handle_rewind()));
	connect(mwPlay, SIGNAL(released()), this, SLOT(handle_midi_trigger()));
	connect(mwReload, SIGNAL(released()), this, SLOT(handle_compile()));
	connect(mwPaste, SIGNAL(released()), this, SLOT(handle_paste()));
	connect(mwCopy, SIGNAL(released()), this, SLOT(handle_copy()));
	connect(mwUndo, SIGNAL(released()), this, SLOT(handle_undo()));
	connect(mwRedo, SIGNAL(released()), this, SLOT(handle_redo()));
	connect(mwEdit, SIGNAL(released()), this, SLOT(handle_edit()));
	connect(mwUpDown, SIGNAL(released()), this, SLOT(handle_up_down()));

	/* Setup GUI */

	main_gl = new QGridLayout(this);

	main_tb = new MppTabBar();
	main_tb_state = 0;

	main_gl->addWidget(main_tb,0,0,1,2);
	main_gl->addWidget(main_tb->split,1,0,1,2);

	/* Watchdog */

	watchdog = new QTimer(this);
	connect(watchdog, SIGNAL(timeout()), this, SLOT(handle_watchdog()));

	/* Buttons */

	main_tb->addWidget(0);
	main_tb->addWidget(mwPlay);
	main_tb->addWidget(mwRewind);
	main_tb->addWidget(0);
	main_tb->addWidget(mwCopy);
	main_tb->addWidget(mwPaste);
	main_tb->addWidget(0);
	main_tb->addWidget(mwEdit);
	main_tb->addWidget(0);
	main_tb->addWidget(mwUndo);
	main_tb->addWidget(mwRedo);
	main_tb->addWidget(0);
	main_tb->addWidget(mwReload);
	main_tb->addWidget(0);
	main_tb->addWidget(mwUpDown);

#ifndef HAVE_NO_SHOW
	tab_show_control = new MppShowControl(this);
#endif
	for (x = 0; x != MPP_MAX_VIEWS; x++)
		scores_main[x] = new MppScoreMain(this, x);

	tab_import = new MppImportTab(this);

	tab_instrument = new MppInstrumentTab(this);

	tab_loop = new MppLoopTab(this, this);

	tab_replay = new MppReplayTab(this);

	tab_database = new MppDataBase(this);

	tab_custom = new MppCustomTab(this, this);

	tab_shortcut = 	new MppShortcutTab(this);

	tab_help = new QPlainTextEdit();
	tab_help->setFont(editFont);
	tab_help->setLineWrapMode(QPlainTextEdit::NoWrap);
	tab_help->setPlainText(tr(
	    "/*\n"
	    " * Copyright (c) 2009-2019 Hans Petter Selasky. All rights reserved.\n"
	    " */\n"

	    "\n"
	    "/*\n"
	    " * Command syntax:\n"
	    " * ===============\n"
	    " *\n"
	    " * U<number>[.] - specifies the duration of the following scores [0..255].\n"
	    " * T<number> - specifies the channel offset for the following scores [0..15].\n"
	    " * K<number> - defines a command [0..99].\n"
	    " * W<number>.<number> - defines an autoplay timeout [1..9999ms].\n"
	    " * K0 - no operation.\n"
	    " * K1 - lock play key until next label jump.\n"
	    " * K2 - unlock play key.\n"
	    " * K3.<bpm>.<period_ms> - set reference BPM and period in ms.\n"
	    " * K4 - no operation.\n"
	    " * K5.0 - Disable micro tuning. (default)\n"
	    " * K5.1 - Enable micro tuning.\n"
	    " * K6 - no operation.\n"
	    " * K6.0 - select ALL key mode. (default)\n"
	    " * K6.1 - no operation.\n"
	    " * K6.2 - select FIXED key mode.\n"
	    " * K6.3 - select TRANS key mode.\n"
	    " * K6.4 - select CHORD-PIANO key mode.\n"
	    " * K6.5 - select CHORD-AUX key mode.\n"
	    " * K6.6 - select CHORD-TRANS key mode.\n"
	    " * K7.<number>.<how>.<align> - select show background.\n"
	    " * K7.<number>.0.<align> - select zoomed background. (default)\n"
	    " * K7.<number>.1.<align> - select best fit background.\n"
	    " * K7.<number>.<how>.0 - select center adjusted background. (default)\n"
	    " * K7.<number>.<how>.1 - select right adjusted background.\n"
	    " * K7.<number>.<how>.2 - select left adjusted background.\n"
	    " * K8.<red>.<green>.<blue> - select show background color, 0..255.\n"
	    " * K9.<red>.<green>.<blue> - select show foreground color, 0..255.\n"
	    " * K10.<align>.<space>.<shadow> - select text show properties.\n"
	    " * K10.0.<space>.<shadow> - select horizontally centered text. (default)\n"
	    " * K10.1.<space>.<shadow> - select right adjusted text.\n"
	    " * K10.2.<space>.<shadow> - select left adjusted text.\n"
	    " * K10.<align>.0.<shadow> - select no horizontal spacing. (default)\n"
	    " * K10.<align>.50.<shadow> - select 50% horizontal spacing.\n"
	    " * K10.<align>.<space>.0 - select no shadow. (default)\n"
	    " * K10.<align>.<space>.50 - select 50% solid shadow.\n"
	    " * K10.<align>.<space>.150 - select 50% shadow offset.\n"
	    " * K11.<red>.<green>.<blue> - select show text shadow color, 0..255.\n"
	    " * K12.<red>.<green>.<blue> - select show text color, 0..255.\n"
	    " * M<number> - macro inline the given label.\n"
	    " * L<number> - defines a label [0..31].\n"
	    " * J<P><number> - jumps to the given label [0..31] \n"
	    " *     and optionally starts a new page(P) in printouts.\n"
	    " * S\"<string .(chord) .(chord)>\" - creates a visual string.\n"
	    " * CDEFGABH<number><B>[.<subdivision>]"
	    " - defines a score in the given octave [0..10] and subdivision [0..15].\n"
	    " * X[+/-]<number>[.<subdivision>][.<mode>] - transpose the subsequent scores by the given\n"
	    " *  number of steps. There are 12 steps in an octave.\n"
	    " * X[+/-]<number>.<subdivision>.<mode>\n"
	    " * X[+/-]<number>.<subdivision>.0 - fixed transposition. (default)\n"
	    " * X[+/-]<number>.<subdivision>.1 - dynamic transposition using full\n"
	    " *   bass score in chord mode.\n"
	    " * X[+/-]<number>.<subdivision>.2 - dynamic transposition using remainder of\n"
	    " *   bass score in chord mode.\n"
	    " */\n"
	    "\n"
	    "/*\n"
	    " * Editor shortcuts:\n"
	    " * =================\n"
	    " *\n"
	    " * By double clicking on a score line in the editor window\n"
	    " * or by pressing the chord edit icon in the middle bar,\n"
	    " * a chord editor will pop up.\n"
	    " *\n"
	    " * CTRL+C: Copy text\n"
	    " * CTRL+V: Paste text\n"
	    " * CTRL+Z: Undo\n"
	    " * CTRL+SHIFT+Z: Redo\n"
	    " */\n"
	    "\n"
	    "/*\n"
	    " * List of supported chords:\n"
	    " * =========================\n"
	    " */\n") + Mpp.VariantList + tr("\n"));

	tab_file_gl = new MppGridLayout();
	tab_play_gl = new MppGridLayout();
	tab_chord_gl = new MppDecodeTab(this);
	tab_pianotab = new MppPianoTab(this);
	tab_config_gl = new MppGridLayout();

	gl_ctrl = new MppGroupBox(tr("Main controls"));
	gl_time = new MppGroupBox(tr("Time counter"));
	gl_bpm = new MppGroupBox(tr("Average Beats Per Minute, BPM, for generator"));
	gl_synth_play = new MppGroupBox(tr("Synth and Play controls"));
	gl_tuning = new MppGroupBox(tr("Master SysEx note tuning"));

	/* Fill up tabbar */
	for (x = 0; x != MPP_MAX_VIEWS; x++) {
		main_tb->addTab(scores_main[x]->gl_view,
		    tr("View %1").arg(QChar('A' + x)));
		main_tb->addTab(scores_main[x]->sheet->gl_sheet,
		    tr("Sheet %1").arg(QChar('A' + x)));
		main_tb->addTab(scores_main[x]->editWidget,
		    tr("Edit %1").arg(QChar('A' + x)));
	}
	main_tb->addTab(tab_import->editWidget, tr("Lyrics"));
#ifndef HAVE_NO_SHOW
	main_tb->addTab(tab_show_control->gl_main, tr("Show"));
#endif
	main_tb->addTab(tab_loop, tr("Loop"));
	main_tb->addTab(tab_replay, tr("RePlay A"));
	main_tb->addTab(tab_file_gl, tr("File"));
	main_tb->addTab(tab_play_gl, tr("Play"));
	main_tb->addTab(tab_chord_gl, tr("Chord"));
	main_tb->addTab(tab_pianotab, tr("PianoTab"));
	main_tb->addTab(tab_config_gl, tr("Config"));
	main_tb->addTab(tab_custom, tr("Custom"));
	main_tb->addTab(tab_shortcut->gl, tr("Shortcut"));
	main_tb->addTab(tab_instrument->gl, tr("Instrument"));
	main_tb->addTab(tab_database, tr("Database"));
	main_tb->addTab(tab_help, tr("Help"));

	/* <File> Tab */

	but_quit = new QPushButton(tr("Quit"));

	but_midi_file_new = new QPushButton(tr("New"));
	but_midi_file_open_single = new QPushButton(tr("Open as\nsingle device"));
	but_midi_file_open_multi = new QPushButton(tr("Open as\nmulti device"));
	but_midi_file_merge_single = new QPushButton(tr("Merge as\nsingle device"));
	but_midi_file_merge_multi = new QPushButton(tr("Merge as\nmulti device"));
	but_midi_file_save = new QPushButton(tr("Save"));
	but_midi_file_save_as = new QPushButton(tr("Save As"));

	gb_midi_file = new MppGroupBox(tr("MIDI File"));
	gb_midi_file->addWidget(but_midi_file_new, 0, 0, 1, 1);
	gb_midi_file->addWidget(but_midi_file_open_single, 1, 0, 1, 1);
	gb_midi_file->addWidget(but_midi_file_open_multi, 2, 0, 1, 1);
	gb_midi_file->addWidget(but_midi_file_merge_single, 3, 0, 1, 1);
	gb_midi_file->addWidget(but_midi_file_merge_multi, 4, 0, 1, 1);
	gb_midi_file->addWidget(but_midi_file_save, 5, 0, 1, 1);
	gb_midi_file->addWidget(but_midi_file_save_as, 6, 0, 1, 1);

	for (x = 0; x != MPP_MAX_VIEWS; x++) {
		but_midi_file_import[x] = new MppButton(QString("To %1-Scores").arg(QChar('A' + x)), x);
		gb_midi_file->addWidget(but_midi_file_import[x], 7 + x, 0, 1, 1);
		connect(but_midi_file_import[x], SIGNAL(released(int)), this, SLOT(handle_midi_file_import(int)));
	}

	gb_gpro_file_import = new MppGroupBox(tr("GPro v3/4"));

	for (x = 0; x != MPP_MAX_VIEWS; x++) {
		but_gpro_file_import[x] = new MppButton(QString("Import and\n" "open in %1-Scores").arg(QChar('A' + x)), x);
		gb_gpro_file_import->addWidget(but_gpro_file_import[x], x, 0, 1, 1);
		connect(but_gpro_file_import[x], SIGNAL(released(int)), this, SLOT(handle_gpro_file_import(int)));
	}

	gb_mxml_file_import = new MppGroupBox(tr("MusicXML"));

	for (x = 0; x != MPP_MAX_VIEWS; x++) {
		but_mxml_file_import[x] = new MppButton(QString("Import and\n" "open in %1-Scores").arg(QChar('A' + x)), x);
		gb_mxml_file_import->addWidget(but_mxml_file_import[x], x, 0, 1, 1);
		connect(but_mxml_file_import[x], SIGNAL(released(int)), this, SLOT(handle_mxml_file_import(int)));
	}

	for (x = 0; x != MPP_MAX_VIEWS; x++)
		tab_file_gl->addWidget(scores_main[x]->gbScoreFile, 0, x, 3, 1);
	tab_file_gl->addWidget(tab_import->gbImport, 0, MPP_MAX_VIEWS, 1, 1);
	tab_file_gl->addWidget(gb_mxml_file_import, 1, MPP_MAX_VIEWS, 1, 1);
	tab_file_gl->addWidget(gb_gpro_file_import, 2, MPP_MAX_VIEWS, 1, 1);
	tab_file_gl->addWidget(gb_midi_file, 0, MPP_MAX_VIEWS + 1, 3, 1);

	tab_file_gl->setRowStretch(3, 1);
	tab_file_gl->setColumnStretch(4, 1);

	tab_file_gl->addWidget(but_quit, 4, 0, 1, 4);

	/* <Play> Tab */

	but_bpm = new QPushButton(tr("BP&M generator\nsettings"));
	connect(but_bpm, SIGNAL(released()), this, SLOT(handle_bpm()));

	spn_tuning = new MppVolume();
	spn_tuning->setRange(-4096,4096,0);
	spn_tuning->setValue(0);
	connect(spn_tuning, SIGNAL(valueChanged(int)), this, SLOT(handle_tuning()));

	gl_tuning->addWidget(spn_tuning,0,0,1,1);

	dlg_bpm = new MppBpm(this);

	lbl_curr_time_val = new QLCDNumber(12);
	lbl_curr_time_val->setMode(QLCDNumber::Dec);
	lbl_curr_time_val->setFrameShape(QLCDNumber::NoFrame);
	lbl_curr_time_val->setSegmentStyle(QLCDNumber::Flat);
	lbl_curr_time_val->setAutoFillBackground(1);

	lbl_bpm_avg_val = new QLCDNumber(4);
	lbl_bpm_avg_val->setMode(QLCDNumber::Dec);
	lbl_bpm_avg_val->setFrameShape(QLCDNumber::NoFrame);
	lbl_bpm_avg_val->setSegmentStyle(QLCDNumber::Flat);
	lbl_bpm_avg_val->setAutoFillBackground(1);

	for (n = 0; n != MPP_MAX_VIEWS; n++) {
		dlg_mode[n] = new MppMode(scores_main[n], n);
		but_mode[n] = new MppButton(tr("View &") + QChar('A' + n) + tr(" mode"), n);
		connect(but_mode[n], SIGNAL(released(int)), this, SLOT(handle_mode(int)));
	}

	for (n = 0; n != MPP_MAX_LBUTTON; n++)
		but_jump[n] = new MppButton(tr("J&%1").arg(n), n);

	but_compile = new QPushButton(tr("Com&pile"));

	but_midi_pause = new QPushButton(tr("Pau&se"));
	but_midi_trigger = new QPushButton(tr("Tri&gger"));
	but_midi_rewind = new QPushButton(tr("Re&wind"));

	mbm_midi_play = new MppButtonMap("MIDI playback\0" "OFF\0" "ON\0", 2, 2);
	connect(mbm_midi_play, SIGNAL(selectionChanged(int)), this, SLOT(handle_midi_play(int)));

	mbm_midi_record = new MppButtonMap("MIDI recording\0" "OFF\0" "ON\0", 2, 2);
	connect(mbm_midi_record, SIGNAL(selectionChanged(int)), this, SLOT(handle_midi_record(int)));

	mbm_score_record = new MppButtonMap("Score recording\0" "OFF\0" "ON\0" "ONE\0", 3, 3);
	connect(mbm_score_record, SIGNAL(selectionChanged(int)), this, SLOT(handle_score_record(int)));

	mbm_key_mode_a = new MppKeyModeButtonMap("Input key mode for view A");
	connect(mbm_key_mode_a, SIGNAL(selectionChanged(int)), this, SLOT(handle_key_mode_a(int)));

	mbm_key_mode_b = new MppKeyModeButtonMap("Input key mode for view B");
	connect(mbm_key_mode_b, SIGNAL(selectionChanged(int)), this, SLOT(handle_key_mode_b(int)));

	mbm_bpm_generator = new MppButtonMap("BPM generator\0" "OFF\0" "ON\0", 2, 2);
	connect(mbm_bpm_generator, SIGNAL(selectionChanged(int)), dlg_bpm, SLOT(handle_bpm_enable(int)));

	/* First column */

	tab_play_gl->addWidget(gl_time,0,0,1,2);
	tab_play_gl->addWidget(gl_ctrl,1,0,1,2);
	tab_play_gl->addWidget(mbm_midi_play, 2,0,1,1);
	tab_play_gl->addWidget(mbm_midi_record, 2,1,1,1);
	tab_play_gl->addWidget(mbm_key_mode_a, 3,0,1,2);
	tab_play_gl->addWidget(mbm_key_mode_b, 4,0,1,2);

	/* Second column */

	tab_play_gl->addWidget(gl_synth_play, 0,2,2,2);
	tab_play_gl->addWidget(mbm_score_record, 2,3,1,1);
	tab_play_gl->addWidget(mbm_bpm_generator, 2,2,1,1);
	tab_play_gl->addWidget(gl_bpm, 3,2,1,2);

	tab_play_gl->addWidget(gl_tuning, 4,3,1,1);

	tab_play_gl->setRowStretch(5, 1);
	tab_play_gl->setColumnStretch(5, 1);

	gl_bpm->addWidget(lbl_bpm_avg_val, 0, 0, 1, 1, Qt::AlignHCenter|Qt::AlignVCenter);

	gl_time->addWidget(lbl_curr_time_val, 0, 0, 1, 1, Qt::AlignHCenter|Qt::AlignVCenter);

	gl_ctrl->addWidget(but_midi_pause, 0, 0, 1, 1);
	gl_ctrl->addWidget(but_midi_trigger, 1, 0, 1, 1);
	gl_ctrl->addWidget(but_midi_rewind, 2, 0, 1, 1);
	gl_ctrl->addWidget(but_compile, 3, 0, 1, 1);

	gl_synth_play->addWidget(but_bpm, 0, 0, 2, 2);

	for (x = 0; x != MPP_MAX_VIEWS; x++)
		gl_synth_play->addWidget(but_mode[x], x, 2, 1, 2);

	for (x = 0; x != MPP_MAX_LBUTTON; x++)
		gl_synth_play->addWidget(but_jump[x], MPP_MAX_VIEWS + (x / 4), (x % 4), 1, 1);

	/* <Configuration> Tab */

	mpp_settings = new MppSettings(this, "midipp");

	tim_config_apply = new QTimer(this);
        tim_config_apply->setSingleShot(1);
        connect(tim_config_apply, SIGNAL(timeout()), this, SLOT(handle_config_apply()));

	but_config_view_fontsel = new QPushButton(tr("Change View Font"));
	but_config_edit_fontsel = new QPushButton(tr("Change Editor Font"));
	but_config_print_fontsel = new QPushButton(tr("Change Print Font"));

	gb_config_device = new MppGroupBox(tr("Device configuration"));

	gb_config_device->addWidget(new QLabel(tr("Group")), 0, 0, 1, 1, Qt::AlignHCenter|Qt::AlignVCenter);
	gb_config_device->addWidget(new QLabel(tr("Device name")), 0, 1, 1, 1, Qt::AlignHCenter|Qt::AlignVCenter);

	pl = new QLabel(tr("Output\nenable"));
	pl->setAlignment(Qt::AlignCenter);
	gb_config_device->addWidget(pl, 0, 2, 1, 1, Qt::AlignHCenter|Qt::AlignVCenter);
	for (x = 0; x != MPP_MAX_VIEWS; x++) {
		pl = new QLabel(tr("Input to\nview-%1").arg(QChar('A' + x)));
		pl->setAlignment(Qt::AlignCenter);
		gb_config_device->addWidget(pl, 0, 3 + x, 1, 1, Qt::AlignHCenter|Qt::AlignVCenter);
	}

	gb_config_device->addWidget(new QLabel(tr("Mute map")), 0, 3 + MPP_MAX_VIEWS, 1, 1, Qt::AlignHCenter|Qt::AlignVCenter);

	pl = new QLabel(tr("Device\nselection"));
	pl->setAlignment(Qt::AlignCenter);
	gb_config_device->addWidget(pl, 0, 4 + MPP_MAX_VIEWS, 1, 1, Qt::AlignHCenter|Qt::AlignVCenter);

	gb_config_device->setColumnStretch(1, 1);

	for (n = 0; n != MPP_MAX_DEVS; n++) {

	  	/* set default device selection map */
		devSelMap[n] = n;

		but_config_sel[n] = new MppDevSel(n, 0);
		connect(but_config_sel[n], SIGNAL(valueChanged(int)), this, SLOT(handle_config_changed()));
 
		but_config_mm[n] = new MppButton(tr("MM"), n);
		connect(but_config_mm[n], SIGNAL(released(int)), this, SLOT(handle_mute_map(int)));

		but_config_dev[n] = new MppButton(tr("DEV"), n);
		connect(but_config_dev[n], SIGNAL(released(int)), this, SLOT(handle_config_dev(int)));

		led_config_dev[n] = new QLineEdit();
		led_config_dev[n]->setMaxLength(256);
		connect(led_config_dev[n], SIGNAL(textChanged(const QString &)), this, SLOT(handle_config_changed()));

		for (x = 0; x != (1 + MPP_MAX_VIEWS); x++) {
			cbx_config_dev[n][x] = new MppCheckBox(n);
			connect(cbx_config_dev[n][x], SIGNAL(stateChanged(int,int)), this, SLOT(handle_config_changed()));
			gb_config_device->addWidget(cbx_config_dev[n][x], n + 1, 2 + x, 1, 1, Qt::AlignHCenter|Qt::AlignVCenter);
		}

		gb_config_device->addWidget(but_config_sel[n], n + 1, 0, 1, 1, Qt::AlignHCenter|Qt::AlignLeft);
		gb_config_device->addWidget(led_config_dev[n], n + 1, 1, 1, 1);
		gb_config_device->addWidget(but_config_mm[n], n + 1, 3 + MPP_MAX_VIEWS, 1, 1, Qt::AlignHCenter|Qt::AlignVCenter);
		gb_config_device->addWidget(but_config_dev[n], n + 1, 4 + MPP_MAX_VIEWS, 1, 1, Qt::AlignHCenter|Qt::AlignVCenter);
	}

	x = 0;

	tab_config_gl->addWidget(gb_config_device, x, 0, 1, 8);

	x++;

	tab_config_gl->setRowStretch(x, 1);

	x++;

	tab_config_gl->addWidget(mpp_settings->but_config_save, x, 0, 1, 1);
	tab_config_gl->addWidget(but_config_view_fontsel, x, 1, 1, 1);

	x++;

	tab_config_gl->addWidget(mpp_settings->but_config_what, x, 0, 1, 1);
	tab_config_gl->addWidget(but_config_edit_fontsel, x, 1, 1, 1);

	x++;

	tab_config_gl->addWidget(mpp_settings->but_config_load, x, 0, 1, 1);
	tab_config_gl->addWidget(but_config_print_fontsel, x, 1, 1, 1);
	tab_config_gl->addWidget(mpp_settings->but_config_clean, x, 3, 1, 1);
	tab_config_gl->setColumnStretch(8, 1);

	/* Connect all */

	for (n = 0; n != MPP_MAX_LBUTTON; n++)
		connect(but_jump[n], SIGNAL(released(int)), this, SLOT(handle_jump(int)));

	connect(but_compile, SIGNAL(released()), this, SLOT(handle_compile()));
	connect(but_quit, SIGNAL(released()), this, SLOT(handle_quit()));

	connect(but_midi_file_new, SIGNAL(released()), this, SLOT(handle_midi_file_new()));
	connect(but_midi_file_open_single, SIGNAL(released()), this, SLOT(handle_midi_file_new_single_open()));
	connect(but_midi_file_merge_single, SIGNAL(released()), this, SLOT(handle_midi_file_merge_single_open()));
	connect(but_midi_file_open_multi, SIGNAL(released()), this, SLOT(handle_midi_file_new_multi_open()));
	connect(but_midi_file_merge_multi, SIGNAL(released()), this, SLOT(handle_midi_file_merge_multi_open()));
	connect(but_midi_file_save, SIGNAL(released()), this, SLOT(handle_midi_file_save()));
	connect(but_midi_file_save_as, SIGNAL(released()), this, SLOT(handle_midi_file_save_as()));

	connect(but_midi_trigger, SIGNAL(pressed()), this, SLOT(handle_midi_trigger()));
	connect(but_midi_rewind, SIGNAL(pressed()), this, SLOT(handle_rewind()));
	connect(but_config_view_fontsel, SIGNAL(released()), this, SLOT(handle_config_view_fontsel()));
	connect(but_config_edit_fontsel, SIGNAL(released()), this, SLOT(handle_config_edit_fontsel()));
	connect(but_config_print_fontsel, SIGNAL(released()), this, SLOT(handle_config_print_fontsel()));

	connect(but_midi_pause, SIGNAL(pressed()), this, SLOT(handle_midi_pause()));

	MidiInit();

	setWindowTitle(MppVersion);
	setWindowIcon(QIcon(MppIconFile));

	handle_tab_changed(1);

	watchdog->start(250);

	mpp_settings->handle_load();
}

MppMainWindow :: ~MppMainWindow()
{
	watchdog->stop();
	tim_config_apply->stop();

	MidiUnInit();
}

void
MppMainWindow :: closeEvent(QCloseEvent *event)
{
	QCoreApplication::exit(0);
}

void
MppMainWindow :: handle_quit()
{
	QCoreApplication::exit(0);
}

void
MppMainWindow :: handle_jump_locked(int index)
{
	int x;

	for (x = 0; x != MPP_MAX_VIEWS; x++)
		scores_main[x]->handleLabelJump(index);
}

void
MppMainWindow :: handle_jump(int index)
{
	atomic_lock();
	handle_jump_locked(index);
	atomic_unlock();
}

void
MppMainWindow :: handle_compile(int force)
{
	int x;
	int y;

	for (y = x = 0; x != MPP_MAX_VIEWS; x++)
		y += scores_main[x]->handleCompile(force);

	if (y != 0) {
		atomic_lock();
		handle_stop();
		atomic_unlock();
	}
}

void
MppMainWindow :: handle_score_record(int value)
{
	atomic_lock();
	scoreRecordOn = value;
	atomic_unlock();
}

void
MppMainWindow :: handle_midi_pause()
{
	uint32_t pos;
	uint8_t triggered;
	uint8_t paused;

	atomic_lock();
	pos = (umidi20_get_curr_position() - startPosition) & 0x3FFFFFFFU;
	triggered = midiTriggered;
	paused = midiPaused;
	atomic_unlock();

	if (paused)
		return;		/* nothing to do */

	handle_rewind();

	atomic_lock();
	if (triggered != 0) {
		midiPaused = 1;
		pausePosition = pos;
	}
	atomic_unlock();
}

void
MppMainWindow :: handle_midi_play(int value)
{
	uint8_t triggered;

	atomic_lock();
	midiPlayOff = value ? 0 : 1;
	triggered = midiTriggered;
	update_play_device_no();
	atomic_unlock();

	handle_midi_pause();

	if (triggered)
		handle_midi_trigger();
}

void
MppMainWindow :: handle_midi_record(int value)
{
	uint8_t triggered;

	atomic_lock();
	midiRecordOff = value ? 0 : 1;
	triggered = midiTriggered;
	update_play_device_no();
	atomic_unlock();

	handle_midi_pause();

	if (triggered)
		handle_midi_trigger();
}

void
MppMainWindow :: handle_play_press(int key, int which)
{
	if (which < 0 || which >= MPP_MAX_VIEWS)
		which = 0;

	atomic_lock();
	if (tab_loop->handle_trigN(key, 90) != 0) {
		/* ignore */
	} else {
		MppScoreMain *sm = scores_main[which];
		sm->handleMidiKeyPressLocked(key, 90);
	}
	atomic_unlock();
}

void
MppMainWindow :: handle_play_release(int key, int which)
{
	if (which < 0 || which >= MPP_MAX_VIEWS)
		which = 0;

	MppScoreMain *sm = scores_main[which];

	atomic_lock();
	sm->handleMidiKeyReleaseLocked(key, 90);
	atomic_unlock();
}

void
MppMainWindow :: handle_sustain_press(int which)
{
	if (which < 0 || which >= MPP_MAX_VIEWS)
		which = 0;

	MppScoreMain *sm = scores_main[which];

	atomic_lock();
	sm->outputControl(0x40, 127);
	atomic_unlock();
}

void
MppMainWindow :: handle_sustain_release(int which)
{
	if (which < 0 || which >= MPP_MAX_VIEWS)
		which = 0;

	MppScoreMain *sm = scores_main[which];

	atomic_lock();
	sm->outputControl(0x40, 0);
	atomic_unlock();
}

void
MppMainWindow :: handle_watchdog_sub(MppScoreMain *sm, int update_cursor)
{
	QTextCursor cursor(sm->editWidget->textCursor());
	int play_line;

	atomic_lock();
	play_line = sm->head.getCurrLine();
	atomic_unlock();

	if (update_cursor) {
		cursor.movePosition(QTextCursor::Start, QTextCursor::MoveAnchor, 1);
		cursor.movePosition(QTextCursor::Down, QTextCursor::MoveAnchor, play_line);
		cursor.movePosition(QTextCursor::StartOfLine, QTextCursor::MoveAnchor, 1);
		sm->editWidget->setTextCursor(cursor);
		sm->watchdog();
	}

	sm->viewWidgetSub->update();
	sm->sheet->update();
}

void
MppMainWindow :: handle_watchdog()
{
	uint32_t delta;
	int bpm;
	char buf[32];
	uint8_t events_copy[MPP_MAX_QUEUE][4];
	uint8_t num_events;
	uint8_t x;
	uint8_t y;
	uint8_t z;
	uint8_t last_duration;
	uint8_t instr_update;
	uint8_t cursor_update;
	uint8_t key_mode_update;
	uint8_t ops;

	/* update focus if any */
	handle_tab_changed();

	atomic_lock();
	cursor_update = cursorUpdate;
	cursorUpdate = 0;
	instr_update = instrUpdated;
	instrUpdated = 0;
	num_events = numInputEvents;
	key_mode_update = keyModeUpdated;
	keyModeUpdated = 0;
	ops = doOperation;
	doOperation = 0;
	bpm = dlg_bpm->bpm_other;

	if (num_events != 0) {
		delta =  umidi20_get_curr_position() - lastInputEvent;
		if (delta >= ((UMIDI20_BPM + 60 - 1) / 60)) {
			numInputEvents = 0;
			memcpy(events_copy, inputEvents, num_events);
		} else {
			/* wait until 1 second has elapsed */
			num_events = 0;
		}
	}
	if (num_events != 0) {

		atomic_unlock();

		QPlainTextEdit *ped = currEditor();
		QTextCursor cursor;

		mid_sort(events_copy[0], num_events);

		last_duration = 0;

		if (ped != 0) {
			cursor = QTextCursor(ped->textCursor());
			cursor.movePosition(QTextCursor::StartOfLine, QTextCursor::MoveAnchor, 1);
			cursor.beginEditBlock();
		}

		for (x = 0; x != num_events; x++) {
			for (y = x; y != num_events; y++) {
				if (events_copy[0][x] != events_copy[0][y])
					break;
			}

			z = y - 1;
			y = y - x;

			if (y != last_duration) {
				last_duration = y;
				snprintf(buf, sizeof(buf), "U%d %s ",
				    y, mid_key_str[events_copy[0][x] & 0x7F]);
			} else {
				snprintf(buf, sizeof(buf), "%s ",
				    mid_key_str[events_copy[0][x] & 0x7F]);
			}

			if (ped != 0) {
				cursor.insertText(QString(buf));
			}
			x = z;
		}

		if (ped != 0) {
			cursor.insertText(QString("\n"));
			cursor.endEditBlock();
			ped->setTextCursor(cursor);
		}

		if (scoreRecordOn == 2)
			mbm_score_record->setSelection(0);

		atomic_lock();
	}
	num_events = numControlEvents;
	if (num_events != 0) {
		numControlEvents = 0;
		memcpy(events_copy, controlEvents, num_events * sizeof(controlEvents[0]));
	}
	atomic_unlock();

	for (x = 0; x != num_events; x++) {
		tab_shortcut->handle_record_event(events_copy[x]);
	}

	if (instr_update)
		tab_instrument->handle_instr_changed(0);

	if (bpm < 0)
		bpm = 0;
	else if (bpm > 9999)
		bpm = 9999;

	lbl_bpm_avg_val->display(bpm);

	do_clock_stats();

	for (x = 0; x != MPP_MAX_VIEWS; x++) {
		if (key_mode_update)
			handle_mode(x, 0);
		handle_watchdog_sub(scores_main[x], cursor_update);
	}

	tab_loop->watchdog();

	if (ops & MPP_OPERATION_PAUSE)
		handle_midi_pause();
	if (ops & MPP_OPERATION_REWIND)
		handle_rewind();
	if (ops & MPP_OPERATION_BPM) {
		int value;

	  	atomic_lock();
		value = dlg_bpm->enabled;
		atomic_unlock();

		MPP_BLOCKED(mbm_bpm_generator,setSelection(value));
	}
}

void
MppMainWindow :: handle_midi_file_clear_name()
{
	if (CurrMidiFileName != NULL) {
		delete (CurrMidiFileName);
		CurrMidiFileName = NULL;
	}
}

void
MppMainWindow :: handle_midi_file_new()
{
	int any = 0;

	handle_midi_file_clear_name();

	handle_rewind();

	atomic_lock();
	for (unsigned int x = 0; x != MPP_MAX_TRACKS; x++) {
		if (track[x] == 0)
			continue;
		umidi20_event_queue_drain(&track[x]->queue);
		any = 1;
	}
	atomic_unlock();

	if (any) {
		tab_instrument->handle_instr_reset();
		tab_instrument->handle_instr_channel_changed(0);
	}
}

void
MppMainWindow :: update_play_device_no()
{
	struct umidi20_event *event;

	for (unsigned int x = 0; x != MPP_MAX_TRACKS; x++) {
		if (track[x] == 0)
			continue;

		/* hint for "MidiEventTxCallback() */
		UMIDI20_QUEUE_FOREACH(event, &track[x]->queue)
			event->device_no = MPP_MAGIC_DEVNO + x;
	}
}

void
MppMainWindow :: handle_midi_file_merge_single_open()
{
	handle_midi_file_open(1);
}

void
MppMainWindow :: handle_midi_file_new_single_open()
{
	handle_midi_file_open(0);
}

void
MppMainWindow :: handle_midi_file_merge_multi_open()
{
	handle_midi_file_open(2|1);
}

void
MppMainWindow :: handle_midi_file_new_multi_open()
{
	handle_midi_file_open(2|0);
}

void
MppMainWindow :: handle_midi_file_open(int how)
{
	QFileDialog *diag = 
	  new QFileDialog(this, tr("Select MIDI File"), 
		Mpp.HomeDirMid,
		QString("MIDI File (*.mid *.MID)"));
	struct umidi20_song *song_copy;
	struct umidi20_track *track_copy;
	struct umidi20_event *event;
	struct umidi20_event *event_copy;
	QByteArray data;
	unsigned int x;

	diag->setAcceptMode(QFileDialog::AcceptOpen);
	diag->setFileMode(QFileDialog::ExistingFile);

	if (diag->exec()) {

		Mpp.HomeDirMid = diag->directory().path();

		if (how & 1) {
			handle_midi_file_clear_name();
			handle_rewind();
		} else {
			handle_midi_file_new();
		}

		CurrMidiFileName = new QString(diag->selectedFiles()[0]);

		if (MppReadRawFile(*CurrMidiFileName, &data) == 0) {
		
			atomic_lock();
			song_copy = umidi20_load_file(&mtx,
			    (const uint8_t *)data.data(), data.size());
			atomic_unlock();

			if (song_copy == NULL) {
				QMessageBox box;

				box.setText(tr("Invalid MIDI file!"));
				box.setStandardButtons(QMessageBox::Ok);
				box.setIcon(QMessageBox::Information);
				box.setWindowIcon(QIcon(MppIconFile));
				box.setWindowTitle(MppVersion);
				box.exec();
			} else {
				goto load_file;
			}
		} else {
			song_copy = NULL;
		}
	}

	goto done;

load_file:

	printf("format %d\n", song_copy->midi_file_format);
	printf("resolution %d\n", song_copy->midi_resolution);
	printf("division_type %d\n", song_copy->midi_division_type);

	atomic_lock();

	x = 0;
	UMIDI20_QUEUE_FOREACH(track_copy, &song_copy->queue) {

	    UMIDI20_QUEUE_FOREACH(event, &track_copy->queue) {

	        if (umidi20_event_is_voice(event) ||
		    umidi20_event_is_sysex(event)) {

		    if (do_instr_check(event, NULL) != 0) {
			event_copy = NULL;
		    } else {
			event_copy = umidi20_event_copy(event, 0);
		    }
		} else if (umidi20_event_get_what(event) &
		    (UMIDI20_WHAT_SONG_EVENT | UMIDI20_WHAT_BEAT_EVENT)) {
			event_copy = umidi20_event_copy(event, 0);
		} else {
			event_copy = NULL;
		}

		if (event_copy != NULL) {
			/* reserve low positions for channel program events */
			if (event_copy->position < MPP_MIN_POS)
				event_copy->position = MPP_MIN_POS;
			if (track[x] != 0) {
				umidi20_event_queue_insert(&track[x]->queue,
				    event_copy, UMIDI20_CACHE_INPUT);
			} else {
				umidi20_event_free(event_copy);
			}
		}
	    }
	    if ((how & 2) && ++x == MPP_MAX_TRACKS)
		break;
	}

	umidi20_song_free(song_copy);

	update_play_device_no();

	atomic_unlock();

	if (how & 4)
		handle_midi_file_import(0);
done:
	/* make sure we save into a new file */
	if (how & (4 | 1))
		handle_midi_file_clear_name();

	delete diag;
}

/* must be called locked */
void
MppMainWindow :: handle_midi_file_instr_prepend()
{
	struct mid_data *d = &mid_data;
	uint8_t x;
	uint8_t n;

	for (n = 0; n != MPP_MAX_TRACKS; n++) {
		for (x = 0; x != 16; x++) {
			d->track = track[n];
			mid_set_channel(d, x);
			mid_set_position(d, 0);
			mid_set_device_no(d, 0xFF);
			mid_set_bank_program(d, x,
			    instr[x].bank,
			    instr[x].prog);
		}
	}
}

/* must be called locked */
void
MppMainWindow :: handle_midi_file_instr_delete()
{
	for (unsigned int x = 0; x != MPP_MAX_TRACKS; x++) {
		if (track[x] == 0)
			continue;

		umidi20_event_queue_move(&track[x]->queue, NULL, 0,
		    MPP_MIN_POS, 0, 0-1, UMIDI20_CACHE_INPUT);
	}
}

void
MppMainWindow :: handle_midi_file_save()
{
	uint8_t *data;
	uint32_t len;
	uint8_t status;

	if (CurrMidiFileName != NULL) {

		atomic_lock();
		handle_midi_file_instr_prepend();
		status = umidi20_save_file(song, &data, &len);
		handle_midi_file_instr_delete();
		atomic_unlock();

		if (status == 0) {
			QByteArray qdata = QByteArray::
			    fromRawData((const char *)data, len);

			status = MppWriteRawFile(*CurrMidiFileName, &qdata);

			free(data);

			if (status) {
				QMessageBox box;

				box.setText(tr("Could not write MIDI file!"));
				box.setStandardButtons(QMessageBox::Ok);
				box.setIcon(QMessageBox::Information);
				box.setWindowIcon(QIcon(MppIconFile));
				box.setWindowTitle(MppVersion);
				box.exec();
			}
		} else {
			QMessageBox box;

			box.setText(tr("Could not get MIDI data!"));
			box.setStandardButtons(QMessageBox::Ok);
			box.setIcon(QMessageBox::Information);
			box.setWindowIcon(QIcon(MppIconFile));
			box.setWindowTitle(MppVersion);

			box.exec();
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
		Mpp.HomeDirMid,
		QString("MIDI File (*.mid *.MID)"));

	diag->setAcceptMode(QFileDialog::AcceptSave);
	diag->setFileMode(QFileDialog::AnyFile);
	diag->setDefaultSuffix(QString("mid"));

	if (diag->exec()) {

		Mpp.HomeDirMid = diag->directory().path();

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
	if (midiTriggered != 0) {
		atomic_lock();
		/* kill all leftover notes */
		handle_stop();
		/* send song stop event */
		send_song_stop_locked();
		atomic_unlock();

		/* wait for MIDI events to propagate */
		MppSleep::msleep(100 /* ms */);
	}

	atomic_lock();

	midiTriggered = 0;
	midiPaused = 0;
	pausePosition = 0;

	update_play_device_no();

	if (song != NULL) {
		umidi20_song_stop(song,
		    UMIDI20_FLAG_PLAY | UMIDI20_FLAG_RECORD);
		umidi20_song_start(song, 0x40000000, 0x80000000,
		    UMIDI20_FLAG_PLAY | UMIDI20_FLAG_RECORD);
		startPosition = umidi20_get_curr_position() - 0x40000000;
	}
	atomic_unlock();
}

void
MppMainWindow :: handle_midi_trigger()
{
	atomic_lock();

	if (midiTriggered == 0) {
		if (midiPlayOff == 0) {
			umidi20_song_stop(song,
			    UMIDI20_FLAG_PLAY | UMIDI20_FLAG_RECORD);
			umidi20_song_start(song, pausePosition, 0x40000000,
			    UMIDI20_FLAG_PLAY | UMIDI20_FLAG_RECORD);
			startPosition = umidi20_get_curr_position() - pausePosition;
		} else {
			umidi20_song_stop(song,
			    UMIDI20_FLAG_PLAY | UMIDI20_FLAG_RECORD);
			umidi20_song_start(song, 0x40000000 + pausePosition, 0x80000000,
			    UMIDI20_FLAG_PLAY | UMIDI20_FLAG_RECORD);
			startPosition = umidi20_get_curr_position() - 0x40000000 - pausePosition;
		}

		/* XXX first recursion point */
		send_song_trigger_locked();

		midiTriggered = 1;
		midiPaused = 0;
		pausePosition = 0;

		dlg_bpm->handle_update(1);
		tab_replay->metronome->handleUpdateLocked();
	}
	atomic_unlock();
}

void
MppMainWindow :: handle_config_reload()
{
	struct umidi20_config cfg;
	char *p_play;
	char *p_rec;
	int n;

	/* setup the I/O devices */

	umidi20_config_export(&cfg);

	for (n = 0; n != MPP_MAX_DEVS; n++) {

		if (deviceName[n] != NULL &&
		    deviceName[n][0] != 0) {
			char *pa;
			pa = strstr(deviceName[n], "|");
			if (pa != NULL)
				*pa = 0;
			p_play = strdup(deviceName[n]);   
			if (pa != NULL)
				p_rec = strdup(pa + 1);
			else
				p_rec = strdup(deviceName[n]);   
			if (pa != NULL)
				*pa = '|';
		} else {
			p_play = NULL;
			p_rec = NULL;
		}
		if ((deviceBits & (MPP_DEV0_RECORD << (2 * n))) && 
		    (p_rec != NULL) && (p_rec[0] != 0) &&
		    (p_rec[1] == ':') && (p_rec[2] != 0))  {
			switch (p_rec[0]) {
			case 'A':
				STRLCPY(cfg.cfg_dev[n].rec_fname, p_rec + 2,
				    sizeof(cfg.cfg_dev[n].rec_fname));
				cfg.cfg_dev[n].rec_enabled_cfg = UMIDI20_ENABLED_CFG_JACK;
				break;
			case 'C':
				STRLCPY(cfg.cfg_dev[n].rec_fname, p_rec + 2,
				    sizeof(cfg.cfg_dev[n].rec_fname));
				cfg.cfg_dev[n].rec_enabled_cfg = UMIDI20_ENABLED_CFG_COREMIDI;
				break;
			case 'D':
				STRLCPY(cfg.cfg_dev[n].rec_fname, p_rec + 2,
				    sizeof(cfg.cfg_dev[n].rec_fname));
				cfg.cfg_dev[n].rec_enabled_cfg = UMIDI20_ENABLED_CFG_DEV;
				break;
			case 'R':
				STRLCPY(cfg.cfg_dev[n].rec_fname, p_rec + 2,
				    sizeof(cfg.cfg_dev[n].rec_fname));
				cfg.cfg_dev[n].rec_enabled_cfg = UMIDI20_ENABLED_CFG_ANDROID;
				break;
			default:
				cfg.cfg_dev[n].rec_enabled_cfg = UMIDI20_DISABLE_CFG;
				break;
			}
		} else {
			cfg.cfg_dev[n].rec_enabled_cfg = UMIDI20_DISABLE_CFG;
		}

		if ((deviceBits & (MPP_DEV0_PLAY << (2 * n))) && 
		    (p_play != NULL) && (p_play[0] != 0) &&
		    (p_play[1] == ':') && (p_play[2] != 0))  {
			switch (p_play[0]) {
			case 'A':
				STRLCPY(cfg.cfg_dev[n].play_fname, p_play + 2,
				    sizeof(cfg.cfg_dev[n].play_fname));
				cfg.cfg_dev[n].play_enabled_cfg = UMIDI20_ENABLED_CFG_JACK;
				break;
			case 'C':
				STRLCPY(cfg.cfg_dev[n].play_fname, p_play + 2,
				    sizeof(cfg.cfg_dev[n].play_fname));
				cfg.cfg_dev[n].play_enabled_cfg = UMIDI20_ENABLED_CFG_COREMIDI;
				break;
			case 'D':
				STRLCPY(cfg.cfg_dev[n].play_fname, p_play + 2,
				    sizeof(cfg.cfg_dev[n].play_fname));
				cfg.cfg_dev[n].play_enabled_cfg = UMIDI20_ENABLED_CFG_DEV;
				break;
			case 'R':
				STRLCPY(cfg.cfg_dev[n].play_fname, p_play + 2,
				    sizeof(cfg.cfg_dev[n].play_fname));
				cfg.cfg_dev[n].play_enabled_cfg = UMIDI20_ENABLED_CFG_ANDROID;
				break;
			default:
				cfg.cfg_dev[n].play_enabled_cfg = UMIDI20_DISABLE_CFG;
				break;
			}
		} else {
			cfg.cfg_dev[n].play_enabled_cfg = UMIDI20_DISABLE_CFG;
		}

		free(p_play);
		free(p_rec);
	}

	/* enable magic device */
	for (n = MPP_MAGIC_DEVNO; n != UMIDI20_N_DEVICES; n++) {
		STRLCPY(cfg.cfg_dev[n].play_fname, "/dev/null",
		    sizeof(cfg.cfg_dev[n].play_fname));
		cfg.cfg_dev[n].play_enabled_cfg = 1;
	}

	umidi20_config_import(&cfg);

	handle_compile();

	/* wait for MIDI devices to be opened */
	MppSleep::msleep(100 /* ms */);

	/* apply local MIDI keys configuration */
	handle_config_local_keys();
}

void
MppMainWindow :: handle_config_apply()
{
	uint8_t deviceSelectionMap[MPP_MAX_DEVS];
	uint32_t devInputMaskCopy[MPP_MAX_DEVS];

	deviceBits = 0;
	memset(devInputMaskCopy, 0, sizeof(devInputMaskCopy));

	for (uint8_t n = 0; n != MPP_MAX_DEVS; n++) {

		free(deviceName[n]);
		deviceName[n] = MppQStringToAscii(led_config_dev[n]->text());

		if (cbx_config_dev[n][0]->isChecked())
			deviceBits |= (MPP_DEV0_PLAY << (2 * n));

		for (uint8_t x = 0; x != MPP_MAX_VIEWS; x++) {
			if (cbx_config_dev[n][1 + x]->isChecked() == 0)
				continue;
			devInputMaskCopy[n] |= (1U << x);
			deviceBits |= (MPP_DEV0_RECORD << (2 * n));
		}

		deviceSelectionMap[n] = but_config_sel[n]->value();
	}

	atomic_lock();
	memcpy(devSelMap, deviceSelectionMap, sizeof(devSelMap));
	memcpy(devInputMask, devInputMaskCopy, sizeof(devInputMask));
	atomic_unlock();
	
	handle_config_reload();
}

void
MppMainWindow :: handle_config_local_keys()
{
	struct mid_data *d = &mid_data;
	uint32_t pos;
	uint32_t off = 0;
	uint8_t midiTriggeredOld;

	atomic_lock();
	midiTriggeredOld = midiTriggered;
	midiTriggered = 1;

	handle_midi_trigger();

	/* compute relative time distance */
	pos = umidi20_get_curr_position() - startPosition;

	/* compensate for processing delay */
	if (pos != 0)
		pos--;

	/*
	 * Update local key enable/disable on all devices and
	 * channels:
	 */
	for (uint8_t n = 0; n != MPP_MAX_DEVS; n++) {
		for (uint8_t x = 0; x != 16; x++) {
			uint8_t buf[4];

			d->track = track[0];
			mid_set_channel(d, x);
			mid_set_position(d, pos);
			mid_set_device_no(d, n);

			if (enableLocalKeys[n]) {
				buf[0] = 0xB0 | x;
				buf[1] = 0x7A;
				buf[2] = 0x7F;
				mid_add_raw(d, buf, 3, off++);
			}

			if (disableLocalKeys[n]) {
				buf[0] = 0xB0 | x;
				buf[1] = 0x7A;
				buf[2] = 0x00;
				mid_add_raw(d, buf, 3, off++);
			}
		}
	}

	midiTriggered = midiTriggeredOld;
	atomic_unlock();
}

void
MppMainWindow :: handle_config_view_fontsel()
{
	bool success;
	int x;

	QFont font = QFontDialog::getFont(&success, defaultFont, this);

	if (success) {
		font.setPixelSize(QFontInfo(font).pixelSize());

		defaultFont = font;

		for (x = 0; x != MPP_MAX_VIEWS; x++)
			scores_main[x]->handleCompile(1);
	}
}

void
MppMainWindow :: handle_config_edit_fontsel()
{
	bool success;
	int x;

	QFont font = QFontDialog::getFont(&success, editFont, this);

	if (success) {
		font.setPixelSize(QFontInfo(font).pixelSize());

		editFont = font;

		for (x = 0; x != MPP_MAX_VIEWS; x++) {
			scores_main[x]->editWidget->setFont(font);
			scores_main[x]->sheet->update();
		}

		tab_help->setFont(font);
		tab_import->editWidget->setFont(font);
	}
}

void
MppMainWindow :: handle_config_print_fontsel()
{
	bool success;

	QFont font = QFontDialog::getFont(&success, printFont, this);

	if (success) {
		font.setPixelSize(QFontInfo(font).pixelSize());
		printFont = font;
	}
}

bool
MppMainWindow :: check_play(uint8_t index, uint8_t chan, uint32_t off)
{
	struct mid_data *d = &mid_data;
	uint32_t pos;

	if (index >= MPP_MAX_TRACKS || chan >= 0x10)
		return (false);

	handle_midi_trigger();

	/* compute relative time distance */
	pos = umidi20_get_curr_position() - startPosition + off;

	/* compensate for processing delay */
	if (pos != 0)
		pos--;

	d->track = track[index];
	noteMode = scores_main[index / MPP_TRACKS_PER_VIEW]->noteMode;
	mid_set_channel(d, chan);
	mid_set_position(d, pos);
	mid_set_device_no(d, MPP_MAGIC_DEVNO + index);

	return (true);
}

bool
MppMainWindow :: check_record(uint8_t index, uint8_t chan, uint32_t off)
{
	struct mid_data *d = &mid_data;
	uint32_t pos;

	if (midiRecordOff || index >= MPP_MAX_TRACKS || chan >= 0x10)
		return (false);

	handle_midi_trigger();

	pos = (umidi20_get_curr_position() - startPosition + off) & 0x3FFFFFFFU;
	if (pos < MPP_MIN_POS)
		pos = MPP_MIN_POS;

	d->track = track[index];
	noteMode = scores_main[index / MPP_TRACKS_PER_VIEW]->noteMode;
	mid_set_channel(d, chan);
	mid_set_position(d, pos);
	mid_set_device_no(d, 0xFF);

	return (true);
}

int
MppMainWindow :: do_extended_alloc(int key, int refcount)
{
	key++;	/* avoid zero default */

	/* use existing key, if possible */
	for (int x = 0; x != 128; x++) {
		if (extended_keys[x][0] == key) {
			extended_keys[x][1] += refcount;
			if (extended_keys[x][1] < 0) {
				/* already released */
				extended_keys[x][1] -= refcount;
				return (-1);
			} else {
				return (x);
			}
		}
	}

	if (refcount > 0) {
		/* try to allocate an empty key */
		for (int x = 0; x != 128; x++) {
			if (extended_keys[x][0] != 0)
				continue;
			extended_keys[x][0] = key;
			extended_keys[x][1] = refcount;
			return (x);
		}
		/* release all unused keys, shouldn't happen */
		for (int x = 0; x != 128; x++) {
			if (extended_keys[x][1] != 0)
				continue;
			extended_keys[x][0] = 0;
			extended_keys[x][1] = 0;
		}
		/* try to allocate an empty key */
		for (int x = 0; x != 128; x++) {
			if (extended_keys[x][0] != 0)
				continue;
			extended_keys[x][0] = key;
			extended_keys[x][1] = refcount;
			return (x);
		}
	}
	return (-1);
}

void
MppMainWindow :: do_key_press(int key, int vel, int dur)
{
	struct mid_data *d = &mid_data;
	int index;

	/* range check(s) */
	if (vel > 127)
		vel = 127;
	else if (vel < -127)
		vel = -127;

	/* master tuning */
	key += (int)masterPitchBend * (MPP_BAND_STEP_12 / 4096);

	/* range check(s) */
	if (key >= MPP_MAX_KEYS || key < 0 || dur < 0)
		return;

	switch (noteMode) {
	case MM_NOTEMODE_SYSEX:
		index = do_extended_alloc(key, (vel <= 0) ? -1 : 1);
		if (index < 0)
			return;
		mid_extended_key_press(d, index, key, vel, dur);
		break;
	default:
		key = (key + MPP_BAND_STEP_24) / MPP_BAND_STEP_12;
		/* range check(s) */
		if (key > 127 || key < 0)
			return;
		mid_key_press(d, key, vel, dur);
		break;
	}
}

void
MppMainWindow :: do_key_pressure(int key, int pressure)
{
	struct mid_data *d = &mid_data;
	uint8_t buf[4];

	if (pressure > 127)
		pressure = 127;
	else if (pressure < 0)
		pressure = 0;

	/* range check(s) */
	if (key >= MPP_MAX_KEYS || key < 0)
		return;

	switch (noteMode) {
	case MM_NOTEMODE_SYSEX:
		key = do_extended_alloc(key, 0);
		if (key < 0)
			return;
		break;
	default:
		key = (key + MPP_BAND_STEP_24) / MPP_BAND_STEP_12;
		if (key >= 128 || key < 0)
			return;
		break;
	}

	buf[0] = 0xA0;
	buf[1] = key & 0x7F;
	buf[2] = pressure & 0x7F;

	mid_add_raw(d, buf, 3, 0);
}

/* must be called locked */
void
MppMainWindow :: handle_stop(int flag)
{
	uint64_t *pkey;
	uint8_t ScMidiTriggered;
	uint8_t ScMidiRecordOff;
	int out_key;
	uint8_t chan;
	uint8_t x;
	uint8_t z;
	uint8_t delay;

	ScMidiTriggered = midiTriggered;
	ScMidiRecordOff = midiRecordOff;
	midiTriggered = 1;
	midiRecordOff = 1;

	for (z = 0; z != MPP_MAX_VIEWS; z++) {

	    /* TRANS mode cleanup */

	    for (x = 0; x != MPP_PRESSED_MAX; x++) {

		pkey = &scores_main[z]->pressedKeys[x];

		if (*pkey != 0) {

			out_key = (*pkey >> 32) & -1U;
			chan = (*pkey >> 16) & 0xFF;
			delay = (*pkey >> 24) & 0xFF;

			/* only release once */
			*pkey = 0;

			output_key(MPP_DEFAULT_TRACK(z),
			    chan, out_key, 0, delay, 0);
		}
	    }

	    /* CHORD mode cleanup */

	    for (x = 0; x != MPP_MAX_CHORD_MAP; x++) {
		struct MppScoreEntry *ps;

		ps = &scores_main[z]->score_past[x];

		if (ps->dur == 0)
			continue;

		/* only release once */
		ps->dur = 0;

		output_key(ps->track, ps->channel, ps->key, 0, 0, 0);

		/* check for secondary event */
		if (ps->channelSec != 0)
			output_key(ps->trackSec, ps->channelSec - 1, ps->key, 0, 0, 0);
	    }

	    /* SYSEX mode cleanup */
	    memset(extended_keys, 0, sizeof(extended_keys));

	    /* check if we should kill the pedal, modulation and pitch */
	    if (!(flag & 1)) {
		scores_main[z]->outputControl(0x40, 0);
		scores_main[z]->outputControl(0x01, 0);
		scores_main[z]->outputPitch(8192);
	    }
	}

	midiTriggered = ScMidiTriggered;
	midiRecordOff = ScMidiRecordOff;
}

uint32_t
MppMainWindow :: get_time_offset(void)
{
	uint32_t time_offset;

	if (midiTriggered == 0) {
		if (midiPaused != 0)
			time_offset = pausePosition;
		else
			time_offset = 0;
	} else {
		time_offset = (umidi20_get_curr_position() - startPosition) & 0x3FFFFFFFU;
	}

	time_offset %= 100000000UL;

	return (time_offset);
}

void
MppMainWindow :: do_clock_stats(void)
{
	uint32_t time_offset;
	char buf[32];

	atomic_lock();
	time_offset = get_time_offset();
	atomic_unlock();

	snprintf(buf, sizeof(buf), "%u.%03u", time_offset / 1000, time_offset % 1000);

	lbl_curr_time_val->display(QString(buf));
}

/* NOTE: Is called unlocked */
static void
MidiEventRxCallback(uint8_t device_no, void *arg, struct umidi20_event *event, uint8_t *drop)
{
	MppMainWindow *mw = (MppMainWindow *)arg;
	MppScoreMain *sm;
	uint32_t what;
	uint8_t chan;
	uint8_t ctrl;
	int key;
	int vel;
	int n;

	*drop = 1;

	mw->atomic_lock();

	what = umidi20_event_get_what(event);

	if (what & UMIDI20_WHAT_CHANNEL) {
		if (mw->controlRecordOn != 0 &&
		    umidi20_event_is_key_end(event) == 0) {
			if (mw->numControlEvents < MPP_MAX_QUEUE) {
				mw->controlEvents[mw->numControlEvents][0] = event->cmd[0];
				mw->controlEvents[mw->numControlEvents][1] = event->cmd[1];
				mw->controlEvents[mw->numControlEvents][2] = event->cmd[2];
				mw->controlEvents[mw->numControlEvents][3] = event->cmd[3];
				mw->numControlEvents++;
			}
		}
	}
	if (umidi20_event_is_key_start(event)) {

		key = (umidi20_event_get_key(event) & 0x7F) * MPP_BAND_STEP_12;
		vel = umidi20_event_get_velocity(event);

		switch (mw->scoreRecordOn) {
		case 1:
		case 2:
			if (mw->numInputEvents < MPP_MAX_QUEUE) {
				mw->inputEvents[mw->numInputEvents] = key / MPP_BAND_STEP_12;
				mw->numInputEvents++;
				mw->lastInputEvent = umidi20_get_curr_position();
			}
			break;
		default:
			break;
		}

		if (mw->tab_loop->handle_trigN(key, vel))
			goto done;
	}

	for (n = 0; n != MPP_MAX_VIEWS; n++) {
		sm = mw->scores_main[n];

		/* filter on device, if any */
		if (!(mw->devInputMask[device_no] & (1U << n)))
			continue;

		/* filter on channel, if any */
		if ((what & UMIDI20_WHAT_CHANNEL) && (sm->inputChannel > -1)) {
			if ((uint8_t)sm->inputChannel != umidi20_event_get_channel(event))
					continue;
		}

		chan = sm->synthChannel;

		ctrl = umidi20_event_get_control_address(event);

		if (mw->controlRecordOn == 0 &&
		    mw->tab_shortcut->handle_event_received_locked(sm, event) != 0) {
			/* command event */
		} else if (umidi20_event_is_pitch_bend(event)) {

			vel = umidi20_event_get_pitch_value(event);

			sm->outputPitch(vel);

		} else if (what & UMIDI20_WHAT_KEY_PRESSURE) {

			key = (umidi20_event_get_key(event) & 0x7F) * MPP_BAND_STEP_12;
			vel = umidi20_event_get_pressure(event) & 0x7F;

			switch (sm->keyMode) {
			case MM_PASS_ALL:
				mw->output_key_pressure(MPP_DEFAULT_TRACK(sm->unit),
				    sm->synthChannel, key, vel);
				break;
			case MM_PASS_NONE_CHORD_PIANO:
			case MM_PASS_NONE_CHORD_AUX:
			case MM_PASS_NONE_CHORD_TRANS:
				sm->handleKeyPressureChord(key, vel, 0);
				break;
			default:
				break;
			}

		} else if (what & UMIDI20_WHAT_CHANNEL_PRESSURE) {

			vel = umidi20_event_get_pressure(event) & 0x7F;

			switch (sm->keyMode) {
			case MM_PASS_ALL:
			case MM_PASS_NONE_CHORD_PIANO:
			case MM_PASS_NONE_CHORD_AUX:
			case MM_PASS_NONE_CHORD_TRANS:
				sm->outputChanPressure(vel);
				break;
			default:
				break;
			}

		} else if (umidi20_event_is_key_start(event)) {

			key = (umidi20_event_get_key(event) & 0x7F) * MPP_BAND_STEP_12;
			vel = umidi20_event_get_velocity(event);

			sm->handleMidiKeyPressLocked(key, vel);

		} else if (umidi20_event_is_key_end(event)) {

			key = (umidi20_event_get_key(event) & 0x7F) * MPP_BAND_STEP_12;
			vel = umidi20_event_get_velocity(event);

			sm->handleMidiKeyReleaseLocked(key, vel);

		} else if (mw->do_instr_check(event, &chan)) {

		} else if ((what & UMIDI20_WHAT_CONTROL_VALUE) &&
		    (ctrl < 120)) {

			/* Only pass non-channel-mode control messages */

			vel = umidi20_event_get_control_value(event);

			sm->outputControl(ctrl, vel);
		}
	}
done:
	mw->atomic_unlock();
}

/* NOTE: Is called unlocked */
static void
MidiEventTxCallback(uint8_t device_no, void *arg, struct umidi20_event *event, uint8_t *drop)
{
	MppMainWindow *mw = (MppMainWindow *)arg;
	struct umidi20_event *p_event;
	uint32_t what;
	int do_drop = 0;

	mw->atomic_lock();

	what = umidi20_event_get_what(event);

	if (what & UMIDI20_WHAT_CHANNEL) {
		uint8_t chan = umidi20_event_get_channel(event) & 0xF;
		int vel = umidi20_event_get_velocity(event);

		if (mw->instr[chan].muted) {
			do_drop = 1;
		} else if (device_no < MPP_MAX_DEVS) {
			/* check for pedal and control events mute */
			if (what & UMIDI20_WHAT_CONTROL_VALUE) {
				if (umidi20_event_get_control_address(event) == 0x40) {
					if (mw->mutePedal[device_no])
						do_drop = 1;
				} else {
					if (mw->muteAllControl[device_no])
						do_drop = 1;
				}
			}
			/* check for program mute */
			if (what & UMIDI20_WHAT_PROGRAM_VALUE) {
			  	if (mw->muteProgram[device_no])
					do_drop = 1;
			}
			/* check for channel mute */
			if (mw->muteMap[device_no][chan]) {
				do_drop = 1;
			}
		} else if (device_no >= MPP_MAGIC_DEVNO &&
		    device_no < UMIDI20_N_DEVICES) {
			int index = device_no - MPP_MAGIC_DEVNO;
			int devno = -2;	/* no device */

			if (vel != 0) {
				/* adjust volume, if any */
				vel = (vel * mw->trackVolume[index]) / MPP_VOLUME_UNIT;

				if (vel > 127)
					vel = 127;
				else if (vel < 1)
					vel = 1;

				umidi20_event_set_velocity(event, vel);
			}

			/* check if we should duplicate events for other devices */
			MppScoreMain *sm = mw->scores_main[index / MPP_TRACKS_PER_VIEW];

			switch (index % MPP_TRACKS_PER_VIEW) {
			case MPP_DEFAULT_TRACK(0):
				devno = sm->synthDevice;
				break;
			case MPP_TREBLE_TRACK(0):
				devno = sm->synthDeviceTreb;
				break;
			case MPP_BASS_TRACK(0):
				devno = sm->synthDeviceBase;
				break;
			default:
				break;
			}

			for (int x = 0; x != MPP_MAX_DEVS; x++) {
				if (devno != -1 && mw->devSelMap[x] != devno)
					continue;
				if (((mw->deviceBits >> (2 * x)) & MPP_DEV0_PLAY) == 0)
					continue;
				/* check for pedal and control events mute */
				if (what & UMIDI20_WHAT_CONTROL_VALUE) {
					if (umidi20_event_get_control_address(event) == 0x40) {
						if (mw->mutePedal[x])
							continue;
					} else {
						if (mw->muteAllControl[x])
							continue;
					}
				}

				/* check for program mute */
				if (what & UMIDI20_WHAT_PROGRAM_VALUE) {
					if (mw->muteProgram[x])
						continue;
				}

				/* check for channel mute */
				if (mw->muteMap[x][chan])
					continue;

				/* duplicate event */
				p_event = umidi20_event_copy(event, 1);
				if (p_event != NULL) {
					p_event->device_no = x;
					umidi20_event_queue_insert(&root_dev.play[x].queue,
					    p_event, UMIDI20_CACHE_INPUT);
				}
			}
			do_drop = 1;
		} else {
			do_drop = 1;
		}
	} else if (event->cmd[1] != 0xFF) {
		if (device_no < MPP_MAX_DEVS) {
			if (mw->muteAllNonChannel[device_no])
				do_drop = 1;
		} else if (device_no >= MPP_MAGIC_DEVNO &&
		    device_no < UMIDI20_N_DEVICES) {
			int index = device_no - MPP_MAGIC_DEVNO;
			int devno = -2;	/* no device */

			/* check if we should duplicate events for other devices */
			MppScoreMain *sm = mw->scores_main[index / MPP_TRACKS_PER_VIEW];

			switch (index % MPP_TRACKS_PER_VIEW) {
			case MPP_DEFAULT_TRACK(0):
				devno = sm->synthDevice;
				break;
			case MPP_TREBLE_TRACK(0):
				devno = sm->synthDeviceTreb;
				break;
			case MPP_BASS_TRACK(0):
				devno = sm->synthDeviceBase;
				break;
			default:
				break;
			}

			for (int x = 0; x != MPP_MAX_DEVS; x++) {
				if (devno != -1 && mw->devSelMap[x] != devno)
					continue;
				if (((mw->deviceBits >> (2 * x)) & MPP_DEV0_PLAY) == 0)
					continue;
				if (mw->muteAllNonChannel[x])
					continue;

				/* duplicate event */
				p_event = umidi20_event_copy(event, 1);
				if (p_event != NULL) {
					p_event->device_no = x;
					umidi20_event_queue_insert(&root_dev.play[x].queue,
					    p_event, UMIDI20_CACHE_INPUT);
				}
			}
			do_drop = 1;
		} else {
			do_drop = 1;
		}
	} else if (device_no >= MPP_MAGIC_DEVNO) {
		do_drop = 1;
	}
	*drop = do_drop;
	mw->atomic_unlock();
}

/* must be called locked */
uint8_t
MppMainWindow :: do_instr_check(struct umidi20_event *event, uint8_t *pchan)
{
	if (umidi20_event_get_what(event) & UMIDI20_WHAT_CONTROL_VALUE) {
		uint8_t addr;
		uint8_t val;
		uint8_t chan;

		addr = umidi20_event_get_control_address(event);
		val = umidi20_event_get_control_value(event);
		chan = umidi20_event_get_channel(event) & 0xF;

		if (addr == 0x00) {
			instr[chan].bank &= 0x007F;
			instr[chan].bank |= (val << 7);
			instr[chan].updated |= 2;
			instr[chan].muted = 0;
			instrUpdated = 1;
			if (pchan != NULL)
				*pchan = chan;
			return (1);
		} else if (addr == 0x20) {
			instr[chan].bank &= 0xFF80;
			instr[chan].bank |= (val & 0x7F);
			instr[chan].updated |= 2;
			instr[chan].muted = 0;
			instrUpdated = 1;
			if (pchan != NULL)
				*pchan = chan;
			return (1);
		}
	} else if (umidi20_event_get_what(event) & UMIDI20_WHAT_PROGRAM_VALUE) {
		uint8_t val;
		uint8_t chan;

		val = umidi20_event_get_program_number(event);
		chan = umidi20_event_get_channel(event) & 0xF;

		instr[chan].prog = val;
		instr[chan].updated |= 2;
		instr[chan].muted = 0;
		instrUpdated = 1;
		if (pchan != NULL)
			*pchan = chan;
		return (1);
	}
	return (0);
}

void
MppMainWindow :: handle_make_scores_visible(MppScoreMain *sm)
{
	if (sm->visual_max == 0)
		handle_make_tab_visible(sm->editWidget);
	else
		handle_make_tab_visible(sm->gl_view);
}

void
MppMainWindow :: handle_make_tab_visible(QWidget *widget)
{
	main_tb->makeWidgetVisible(widget);
}

void
MppMainWindow :: handle_tab_changed(int force)
{
	int x;

	for (x = 0; x != MPP_MAX_VIEWS; x++) {
		if (scores_main[x]->editWidget->hasFocus()) {
			x = (2 * x) + 0;
			goto found;
		}
	}
	for (x = 0; x != MPP_MAX_VIEWS; x++) {
		if (main_tb->isVisible(scores_main[x]->gl_view) ||
		    main_tb->isVisible(scores_main[x]->sheet->gl_sheet)) {
			x = (2 * x) + 1;
			goto found;
		}
		if (main_tb->isVisible(scores_main[x]->editWidget)) {
			x = (2 * x) + 0;
			goto found;
		}
	}

	/* set some defaults */
	x = 0;
found:
	if (x == lastViewIndex && force == 0)
		return;
	lastViewIndex = x;

	QString *ps = scores_main[x/2]->currScoreFileName;
	if (ps != NULL)
		setWindowTitle(MppVersion + " - " + MppBaseName(*ps));
	else
		setWindowTitle(MppVersion);

	if ((force != 0) || (x & 1))
		handle_compile();
}

int
MppMainWindow :: convert_midi_duration(struct umidi20_track *im_track, uint32_t thres, uint32_t chan_mask)
{
	struct umidi20_event *event;

	uint32_t last_pos;
	uint32_t delta;
	uint32_t index;
	uint32_t curr_pos;
	uint32_t duration;

	last_pos = -thres;	/* make sure we get the first score */
	index = 0;
	curr_pos = 0;
	duration = 0;

	memset(convLineStart, 0, sizeof(convLineStart));
	memset(convLineEnd, 0, sizeof(convLineEnd));

	UMIDI20_QUEUE_FOREACH(event, &im_track->queue) {

		curr_pos = (event->position & 0x3FFFFFFF);
		duration = event->duration;

		delta = (curr_pos - last_pos);

		if (!(umidi20_event_get_what(event) & UMIDI20_WHAT_CHANNEL))
			continue;
		if (!(chan_mask & (1 << (umidi20_event_get_channel(event) & 0xF))))
			continue;

		if (umidi20_event_is_key_start(event)) {
			if (delta >= thres) {
				last_pos = curr_pos;
				if (index < MPP_MAX_LINES) {
					convLineStart[index] = last_pos;
					index++;
				}
			}
		}

		if (umidi20_event_is_key_end(event)) {
			if (index > 0 && index < MPP_MAX_LINES)
				convLineEnd[index - 1] = curr_pos;
		}
	}
	if ((curr_pos + duration) > last_pos && index < MPP_MAX_LINES) {
		convLineStart[index] = curr_pos + duration;
		index++;
	}
	return (index);
}

QString
MppMainWindow :: get_midi_score_duration(uint32_t *psum)
{
	uint32_t retval;
	uint32_t lend;
	char buf[32];

	retval = convLineStart[convIndex] - 
	    convLineStart[convIndex - 1];

	lend = convLineEnd[convIndex - 1];

	if (lend <= convLineStart[convIndex] &&
	    lend >= convLineStart[convIndex - 1])
		lend = convLineStart[convIndex] - lend;
	else
		lend = retval / 2;

	snprintf(buf, sizeof(buf), "W%u.%u /* %u @ %u ms */",
	    retval - lend, lend, retval, *psum);

	*psum += retval;

	return (QString(buf));
}

int
MppMainWindow :: log_midi_score_duration(void)
{
	uint32_t retval;
	uint8_t j;

	retval = convLineStart[convIndex] - 
	    convLineStart[convIndex - 1];

	for (j = 0; j != 9; j++) {
		if (retval > (1000U >> (j+1)))
			break;
	}

	return (j);
}

void
MppMainWindow :: import_midi_track(struct umidi20_track *im_track, uint32_t flags, int label, int view)
{
	QString output;
	QString out_block;
	QString out_desc;
	QString out_prefix;

	struct umidi20_event *event;

	char buf[128];

	uint32_t end;
	uint32_t max_index;
	uint32_t x;
	uint32_t last_u = MPP_MAX_DURATION + 1;
	uint32_t chan_mask = 0;
	uint32_t thres = 25;
	uint32_t sumdur = 0;
	uint8_t last_chan = 0;
	uint8_t chan;
	uint8_t first_score;

	convIndex = 0;
	first_score = 0;

	atomic_lock();

	UMIDI20_QUEUE_FOREACH(event, &im_track->queue) {

		if (!(umidi20_event_get_what(event) & UMIDI20_WHAT_CHANNEL))
			continue;

		if (umidi20_event_is_key_start(event)) {
			chan = umidi20_event_get_channel(event) & 0xF;
			chan_mask |= (1 << chan);
		}
	}

	atomic_unlock();

	if (flags & MIDI_FLAG_DIALOG) {

		MppMidi *diag;

		diag = new MppMidi(chan_mask, flags, thres);

		flags = diag->flags;
		thres = diag->thres;
		chan_mask = diag->chan_mask;
		out_prefix = diag->prefix;

		delete diag;
	}

	if (label > -1) {
		snprintf(buf, sizeof(buf), "L%d:\n\n", label);
		output += buf;
	}

	/* if no channels, just return */
	if (chan_mask == 0)
		return;

	atomic_lock();

	umidi20_track_compute_max_min(im_track);

	max_index = convert_midi_duration(im_track, thres, chan_mask);

	UMIDI20_QUEUE_FOREACH(event, &im_track->queue) {

		if (!(umidi20_event_get_what(event) & UMIDI20_WHAT_CHANNEL))
			continue;
		chan = umidi20_event_get_channel(event) & 0xF;

		if (!(chan_mask & (1 << chan)))
			continue;

		while ((convIndex < max_index) &&
		       ((event->position & 0x3FFFFFFFU) >= (convLineStart[convIndex] + thres))) {

			uint8_t do_flush;
			uint8_t new_page;
			uint8_t duration;

			convIndex++;

			if (convIndex < max_index)
				duration = log_midi_score_duration();
			else
				duration = 0;

			do_flush = ((convIndex & 0xF) == 0);
			new_page = ((convIndex & 0xFF) == 0);

			if (duration != 0)
				snprintf(buf, sizeof(buf), ".[%u]   ", (int)duration);
			else
				snprintf(buf, sizeof(buf), ".   ");

			out_desc += buf;
			if (flags & MIDI_FLAG_DURATION) {
				if (convIndex < max_index)
					out_block += get_midi_score_duration(&sumdur);
			}
			out_block += "\n";
			first_score = 0;

			if (do_flush) {

				if (flags & MIDI_FLAG_STRING) {
					snprintf(buf, sizeof(buf), "%5u", convIndex / 16);

					output += "\nS\"";
					output += buf;
					output += out_desc;
					output += "\"\n";
				}
				output += out_block;
				if (flags & MIDI_FLAG_STRING) {
					if (new_page)
						output += "\nJP\n";
				}
				output += "\n";

				out_desc = "";
				out_block = "";
			}

			last_u = MPP_MAX_DURATION + 1;
			last_chan = 0;
		}

		end = (event->position & 0x3FFFFFFFU) + event->duration;

		if (umidi20_event_is_key_start(event)) {
			uint32_t ext_key;

			x = convIndex;
			while (x < max_index) {
				if (convLineStart[x] >= end)
					break;
				x++;
			}

			x = x - convIndex;

			if (x > MPP_MAX_DURATION)
				x = MPP_MAX_DURATION;
			else if (x == 0) {
				x = 1;
				if ((convIndex + 1) < max_index)
					end = convLineStart[convIndex + 1];
				else
					end = 0;	/* should not happen */

			}

			if (first_score == 0) {
				first_score = 1;
				if (out_prefix.size() > 0)
					out_block += out_prefix + " ";
			}

			if (chan != last_chan) {
				last_chan = chan;
				if (flags & MIDI_FLAG_MULTI_CHAN) {
					snprintf(buf, sizeof(buf), "T%u ", chan);
					out_block += buf;
				}
			}

			if (x != last_u) {
				last_u = x;
				snprintf(buf, sizeof(buf), "U%u ", x);
				out_block += buf;
			}

			ext_key = umidi20_event_get_extended_key(event);
			if (ext_key != -1U)
				out_block += MppKeyStr(ext_key);
			else
				out_block += mid_key_str[umidi20_event_get_key(event) & 0x7F];
			out_block += " ";
		}
	}

	atomic_unlock();

	if (flags & MIDI_FLAG_STRING) {
		snprintf(buf, sizeof(buf), "%5u", (convIndex + 15) / 16);

		output += "\nS\"";
		output += buf;
		output += out_desc;
		output += "\"\n";
	}
	output += out_block;
	output += "\n";

	if (flags & MIDI_FLAG_DURATION) {
		snprintf(buf, sizeof(buf), "/* W = %u ms */\n\n", sumdur);
		output += buf;
	}

	if (label > -1) {
		snprintf(buf, sizeof(buf), "J%d\n", label);
		output += buf;
	}

	if (flags & MIDI_FLAG_ERASE_DEST) {
		scores_main[view]->handleScoreFileNew();

		QTextCursor cursor(scores_main[view]->editWidget->textCursor());
		cursor.insertText(output);
	} else {
		QTextCursor cursor(scores_main[view]->editWidget->textCursor());
		cursor.beginEditBlock();
		cursor.insertText(output);
		cursor.endEditBlock();
	}
	handle_compile();
	handle_make_tab_visible(scores_main[view]->editWidget);
}

void
MppMainWindow :: handle_midi_file_import(int n)
{
	if (track[0] == 0)
		return;
	import_midi_track(track[0], MIDI_FLAG_DIALOG |
	    MIDI_FLAG_MULTI_CHAN | MIDI_FLAG_ERASE_DEST, 0, n);
}

void
MppMainWindow :: handle_gpro_file_import(int view)
{
	QFileDialog *diag = 
	  new QFileDialog(this, tr("Select GPro v3 or v4 File"), 
		Mpp.HomeDirGp3,
		QString("GPro File (*.gp *.gp3 *.gp4 *.GP *.GP3 *.GP4)"));
	QByteArray data;
	MppGPro *gpro;
	QTextCursor *cursor;

	diag->setAcceptMode(QFileDialog::AcceptOpen);
	diag->setFileMode(QFileDialog::ExistingFile);

	if (diag->exec()) {

		Mpp.HomeDirGp3 = diag->directory().path();

		QString fname(diag->selectedFiles()[0]);

		if (MppReadRawFile(fname, &data) != 0) {
			QMessageBox box;

			box.setText(tr("Could not read MIDI file!"));
			box.setStandardButtons(QMessageBox::Ok);
			box.setIcon(QMessageBox::Information);
			box.setWindowIcon(QIcon(MppIconFile));
			box.setWindowTitle(MppVersion);

			box.exec();
		} else {
			goto load_file;
		}
	}

	goto done;

load_file:

	gpro = new MppGPro((uint8_t *)data.data(), data.size());

	cursor = new QTextCursor(scores_main[view]->editWidget->textCursor());
	cursor->beginEditBlock();
	cursor->insertText(gpro->output);
	cursor->endEditBlock();

	delete gpro;
	delete cursor;

	handle_compile();
	handle_make_scores_visible(scores_main[view]);

done:
	delete diag;
}

void
MppMainWindow :: handle_mxml_file_import(int view)
{
	QFileDialog *diag = 
	  new QFileDialog(this, tr("Select MusicXML file"), 
		Mpp.HomeDirMXML,
		QString("MusicXML file (*.xml *.XML)"));
	QByteArray data;
	MppMusicXmlImport *mxml;

	diag->setAcceptMode(QFileDialog::AcceptOpen);
	diag->setFileMode(QFileDialog::ExistingFile);

	if (diag->exec()) {

		Mpp.HomeDirMXML = diag->directory().path();

		QString fname(diag->selectedFiles()[0]);

		if (MppReadRawFile(fname, &data) != 0) {
			QMessageBox box;

			box.setText(tr("Could not read MusicXML file!"));
			box.setStandardButtons(QMessageBox::Ok);
			box.setIcon(QMessageBox::Information);
			box.setWindowIcon(QIcon(MppIconFile));
			box.setWindowTitle(MppVersion);
			box.exec();
		} else {
			goto load_file;
		}
	}

	goto done;

load_file:

	mxml = new MppMusicXmlImport(data);

	if (mxml->output.isEmpty()) {
		QMessageBox box;
		box.setText(tr("No parts found in Music XML file!"));
		box.setStandardButtons(QMessageBox::Ok);
		box.setIcon(QMessageBox::Information);
		box.setWindowIcon(QIcon(MppIconFile));
		box.setWindowTitle(MppVersion);
		box.exec();
		goto done;
	}

	if (mxml->cbx_erase->isChecked()) {
		scores_main[view]->handleScoreFileNew();

		QTextCursor cursor(scores_main[view]->editWidget->textCursor());
		cursor.insertText(mxml->output);
	} else {
		QTextCursor cursor(scores_main[view]->editWidget->textCursor());
		cursor.beginEditBlock();
		cursor.insertText(mxml->output);
		cursor.endEditBlock();
	}

	delete mxml;

	handle_compile();
	handle_make_scores_visible(scores_main[view]);

done:
	delete diag;
}

void
MppMainWindow :: MidiInit(void)
{
	int n;

	/* enable all output and input */
	for (n = 0; n != MPP_MAX_DEVS; n++) {
	  	cbx_config_dev[n][0]->setChecked(1);	/* Output */
		cbx_config_dev[n][1]->setChecked(1);	/* Input view-A */
	}

	led_config_dev[0]->setText(QString("X:"));

	handle_midi_record(0);
	handle_midi_play(0);
	handle_score_record(0);
	tab_instrument->handle_instr_reset();

	for (n = 0; n != UMIDI20_N_DEVICES; n++) {
		umidi20_set_record_event_callback(n, &MidiEventRxCallback, this);
		umidi20_set_play_event_callback(n, &MidiEventTxCallback, this);
	}

	atomic_lock();
	song = umidi20_song_alloc(&mtx, UMIDI20_FILE_FORMAT_TYPE_0, 500,
	    UMIDI20_FILE_DIVISION_TYPE_PPQ);
	if (song == 0) {
		atomic_unlock();
		err(1, "Could not allocate new song\n");
	}

	for (n = 0; n != MPP_MAX_TRACKS; n++) {
		trackVolume[n] = MPP_VOLUME_UNIT;
		track[n] = umidi20_track_alloc();
		if (track[n] == 0) {
		  	atomic_unlock();
			err(1, "Could not allocate new song or track\n");
		}
		umidi20_song_track_add(song, NULL, track[n], 0);
	}

	/* disable recording track */
	umidi20_song_set_record_track(song, 0);

	/* get the MIDI up! */
	mid_init(&mid_data, 0);

	umidi20_song_start(song, 0x40000000, 0x80000000,
	    UMIDI20_FLAG_PLAY | UMIDI20_FLAG_RECORD);

	startPosition = umidi20_get_curr_position() - 0x40000000;

	atomic_unlock();

	handle_config_reload();
}

void
MppMainWindow :: MidiUnInit(void)
{
	int n;

	handle_rewind();

	atomic_lock();

	umidi20_song_free(song);

	song = NULL;

	umidi20_song_stop(song, UMIDI20_FLAG_PLAY | UMIDI20_FLAG_RECORD);

	atomic_unlock();

	for (n = 0; n != MPP_MAX_DEVS; n++) {
		free(deviceName[n]);
		deviceName[n] = NULL;
	}
}

void
MppMainWindow :: handle_mute_map(int n)
{
	MppMuteMap diag(this, this, n);

	diag.exec();
}

int
MppMainWindow :: handle_config_dev(int n, int automagic)
{
	int retval;

	MppDevices diag(this);

	if (automagic == 0) {
		retval = diag.exec();
	} else switch (diag.autoSelect()) {
	case 0:
		retval = diag.exec();
		break;
	case 1:
		retval = QDialog::Accepted;
		break;
	default:
		retval = QDialog::Rejected;
		break;
	}

	if (retval == QDialog::Accepted) {
		MPP_BLOCKED(led_config_dev[n], setText(diag.result_dev));
		handle_config_apply();
	}
	return (retval);
}

void
MppMainWindow :: output_key(int index, int chan, int key, int vel, int delay, int dur)
{
	struct mid_data *d = &mid_data;
	uint8_t n;

	/* check for time scaling */
	if (dlg_bpm->period_cur != 0 && dlg_bpm->bpm_other != 0)
		delay = (dlg_bpm->period_ref * delay) / dlg_bpm->bpm_other;

	/* output key to all playback device(s) */
	if (check_play(index, chan, 0)) {
		mid_delay(d, delay);
		do_key_press(key, vel, dur);
	}

	/* output key to recording device(s) */
	if (check_record(index, chan, 0)) {
		mid_delay(d, delay);
		do_key_press(key, vel, dur);
	}

	/* output key to loop recording, if any */
	for (n = 0; n != MPP_LOOP_MAX; n++) {
		if (tab_loop->check_record(n)) {
			mid_delay(d, delay);
			do_key_press(key, vel, dur);
		}
	}
}

/* must be called locked */
void
MppMainWindow :: output_key_pressure(int index, int chan, int key, int pressure, int delay)
{
	struct mid_data *d = &mid_data;

	/* output key to all playback device(s) */
	if (check_play(index, chan, 0)) {
		mid_delay(d, delay);
		do_key_pressure(key, pressure);
	}

	/* output key to recording device(s) */
	if (check_record(index, chan, 0)) {
		mid_delay(d, delay);
		do_key_pressure(key, pressure);
	}
}

void
MppMainWindow :: handle_bpm()
{
	dlg_bpm->handle_reload_all();
	dlg_bpm->exec();
}

void
MppMainWindow :: handle_mode(int index, int dialog)
{
	int value;

	dlg_mode[index]->update_all();

	if (dialog != 0)
		dlg_mode[index]->exec();

	if (index == 0) {
		atomic_lock();
		value = scores_main[0]->keyMode;
		atomic_unlock();
		mbm_key_mode_a->setSelection(value);
	} else 	if (index == 1) {
		atomic_lock();
		value = scores_main[1]->keyMode;
		atomic_unlock();
		mbm_key_mode_b->setSelection(value);
	}
}

void
MppMainWindow :: handle_key_mode_a(int value)
{
	if (value < 0 || value >= MM_PASS_MAX)
		value = 0;

	atomic_lock();
	scores_main[0]->keyMode = value;
	atomic_unlock();
}

void
MppMainWindow :: handle_key_mode_b(int value)
{
	if (value < 0 || value >= MM_PASS_MAX)
		value = 0;

	atomic_lock();
	scores_main[1]->keyMode = value;
	atomic_unlock();
}

void
MppMainWindow :: handle_move_right()
{
	main_tb->moveCurrWidgetRight();
}

void
MppMainWindow :: handle_move_left()
{
	main_tb->moveCurrWidgetLeft();
}

QPlainTextEdit *
MppMainWindow :: currEditor()
{
	QPlainTextEdit *pe;
	QPlainTextEdit *pev = 0;
	int x;
	int n = 0;

	for (x = 0; x != MPP_MAX_VIEWS; x++) {
		pe = scores_main[x]->editWidget;
		if (pe->hasFocus())
			return (pe);
		if (main_tb->isVisible(pe)) {
			pev = pe;
			n++;
		}
	}
	pe = tab_import->editWidget;
	if (pe->hasFocus())
		return (pe);
	if (main_tb->isVisible(pe)) {
		pev = pe;
		n++;
	}
	pe = tab_help;
	if (pe->hasFocus())
		return (pe);
	if (main_tb->isVisible(pe)) {
		pev = pe;
		n++;
	}
	/* if only one editor is visible, return that */
	if (n == 1)
		return (pev);
	return (0);
}

MppScoreMain *
MppMainWindow :: currScores()
{
	QPlainTextEdit *pe;
	int x;
	int y = 0;
	int n = 0;

	for (x = 0; x != MPP_MAX_VIEWS; x++) {
		pe = scores_main[x]->editWidget;
		if (pe->hasFocus())
			return (scores_main[x]);
		if (main_tb->isVisible(pe)) {
			y = x;
			n++;
		}
	}

	/* if only one score editor is visible, return that */
	if (n == 1)
		return (scores_main[y]);
	return (0);
}

void
MppMainWindow :: handle_copy()
{
	QPlainTextEdit *qedit = currEditor();
	if (qedit != 0) {
		/* if there is no selection, select all */
		if (!qedit->textCursor().hasSelection())
			qedit->selectAll();
		qedit->copy();
	}
}

void
MppMainWindow :: handle_paste()
{
	QPlainTextEdit *qedit = currEditor();
	if (qedit != 0)
		qedit->paste();
}

void
MppMainWindow :: handle_redo()
{
	QPlainTextEdit *qedit = currEditor();
	if (qedit != 0)
		qedit->redo();
}

void
MppMainWindow :: handle_undo()
{
	QPlainTextEdit *qedit = currEditor();
	if (qedit != 0)
		qedit->undo();
}

void
MppMainWindow :: handle_edit()
{
	MppScoreMain *sm = currScores();
	if (sm == 0)
		return;
	/* edit the line */
	sm->handleEditLine();
}

void
MppMainWindow :: send_song_stop_locked()
{
	int n;

	for (n = 0; n != MPP_MAX_VIEWS; n++) {
		if (scores_main[n]->songEventsOn != 0)
			break;
	}
	if (n != MPP_MAX_VIEWS)
		send_byte_event_locked(0xFC);
}

void
MppMainWindow :: send_song_trigger_locked()
{
	int n;

	for (n = 0; n != MPP_MAX_VIEWS; n++) {
		if (scores_main[n]->songEventsOn != 0)
			break;
	}
	if (n != MPP_MAX_VIEWS) {
		if (midiPaused != 0)
			send_byte_event_locked(0xFB);	/* continue */
		else if (midiTriggered == 0)
			send_byte_event_locked(0xFA);	/* start */
	}
}

void
MppMainWindow :: send_byte_event_locked(uint8_t which)
{
	uint8_t buf[1];
	uint8_t trig;
	uint8_t n;

	trig = midiTriggered;
	midiTriggered = 1;

	buf[0] = which;

	for (n = 0; n != MPP_MAX_TRACKS; n += MPP_TRACKS_PER_VIEW) {
		if (check_play(n, 0, 0))
			mid_add_raw(&mid_data, buf, 1, 0);
		if (check_record(n, 0, 0))
			mid_add_raw(&mid_data, buf, 1, 0);
	}

	midiTriggered = trig;
}

void
MppMainWindow :: send_song_select_locked(uint8_t which)
{
	uint8_t buf[2];
	uint8_t trig;
	uint8_t n;

	trig = midiTriggered;
	midiTriggered = 1;

	buf[0] = 0xF3;
	buf[1] = which & 0x7F;

	for (n = 0; n != MPP_MAX_TRACKS; n += MPP_TRACKS_PER_VIEW) {
		if (check_play(n, 0, 0))
			mid_add_raw(&mid_data, buf, 2, 0);
		if (check_record(n, 0, 0))
			mid_add_raw(&mid_data, buf, 2, 0);
	}

	midiTriggered = trig;
}

void
MppMainWindow :: handle_config_changed()
{
	tim_config_apply->start(500);
}

void
MppMainWindow :: handle_up_down()
{
    main_tb_state = !main_tb_state;
    main_gl->removeWidget(main_tb);
    if (main_tb_state)
        main_gl->addWidget(main_tb,2,0,1,2);
    else
        main_gl->addWidget(main_tb,0,0,1,2);
}

void
MppMainWindow :: atomic_lock(void)
{
	pthread_mutex_lock(&mtx);
}

void
MppMainWindow :: atomic_unlock(void)
{
	pthread_mutex_unlock(&mtx);
}

/* must be called locked */
int
MppMainWindow :: getCurrTransposeScore(void)
{
	for (int x = 0; x != MPP_MAX_VIEWS; x++) {
		MppScoreMain *sm = scores_main[x];
		unsigned int y;

		if (sm->keyMode != MM_PASS_NONE_CHORD_PIANO &&
		    sm->keyMode != MM_PASS_NONE_CHORD_AUX &&
		    sm->keyMode != MM_PASS_NONE_CHORD_TRANS)
			continue;
		for (y = 0; y != MPP_MAX_CHORD_MAP; y++) {
			if (sm->score_past[y].dur != 0)
				break;
		}
		if (y == MPP_MAX_CHORD_MAP && sm->lastPedalValue <= 0x40)
			continue;
		if (sm->score_future_base[0].dur == 0)
			continue;
		return (sm->score_future_base[0].key);
	}
	return (-1);
}

/* must be called locked */
void
MppMainWindow :: handle_tuning()
{
	atomic_lock();
	masterPitchBend = spn_tuning->value();
	atomic_unlock();
}

#ifdef HAVE_SCREENSHOT
void
MppMainWindow :: ScreenShot(QApplication &app)
{
	int x;

	/* dummy - refresh */
	main_tb->changeTab(1);

	for (x = 0; x != main_tb->ntabs; x++) {
		main_tb->changeTab(x);
		MppScreenShot(this, app);
	}

	main_tb->changeTab(0);
	handle_move_left();
	main_tb->changeTab(8);
	MppScreenShot(this, app);

	MppMuteMap diag0(this, this, 0);
	diag0.exec();
	MppScreenShot(&diag0, app);

	MppDevices diag1(this);
	diag1.exec();
	MppScreenShot(&diag1, app);

	dlg_bpm->exec();
	MppScreenShot(dlg_bpm, app);

	dlg_mode[0]->exec();
	MppScreenShot(dlg_mode[0], app);

	MppReplace diag3(this, scores_main[0], QString(), QString());
	diag3.exec();
	MppScreenShot(&diag3, app);

	MppChanSelDiag diag4(this, 0, MPP_CHAN_ANY);
	diag4.exec();
	MppScreenShot(&diag4, app);

	tab_show_control->handle_mode_change(MPP_SHOW_ST_LYRICS);
	tab_show_control->handle_show_window();
	for (x = 0; x != 32; x++)
		tab_show_control->handle_watchdog();
	MppScreenShot(tab_show_control->wg_show, app);
}
#endif
