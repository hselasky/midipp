/*-
 * Copyright (c) 2009-2015 Hans Petter Selasky. All rights reserved.
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
#include "midipp_volume.h"
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
	int n;
	int x;

	/* set memory default */

	memset(auto_zero_start, 0, auto_zero_end - auto_zero_start);

	CurrMidiFileName = NULL;
	song = NULL;
	track = NULL;

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

	mwRight = new QPushButton();
	mwLeft = new QPushButton();
	mwRewind = new QPushButton();
	mwPlay = new QPushButton();
	mwReload = new QPushButton();
	mwPaste = new QPushButton();
	mwCopy = new QPushButton();
	mwUndo = new QPushButton();
	mwRedo = new QPushButton();
	mwEdit = new QPushButton();
	mwUpDown = new QPushButton();

	mwRight->setToolTip(tr("Move Left to Right"));
	mwLeft->setToolTip(tr("Move Right to Left"));
	mwRewind->setToolTip(tr("Rewind"));
	mwPlay->setToolTip(tr("Trigger"));
	mwReload->setToolTip(tr("Recompile"));
	mwPaste->setToolTip(tr("Paste"));
	mwCopy->setToolTip(tr("Copy"));
	mwUndo->setToolTip(tr("Undo"));
	mwRedo->setToolTip(tr("Redo"));
	mwEdit->setToolTip(tr("Edit or Insert a Chord"));
	mwUpDown->setToolTip(tr("Move menu Up or Down"));

	mwRight->setIcon(QIcon(QString(":/right_arrow.png")));
	mwLeft->setIcon(QIcon(QString(":/left_arrow.png")));
	mwRewind->setIcon(QIcon(QString(":/stop.png")));
	mwPlay->setIcon(QIcon(QString(":/play.png")));
	mwReload->setIcon(QIcon(QString(":/reload.png")));
	mwPaste->setIcon(QIcon(QString(":/paste.png")));
	mwCopy->setIcon(QIcon(QString(":/copy.png")));
	mwUndo->setIcon(QIcon(QString(":/undo.png")));
	mwRedo->setIcon(QIcon(QString(":/redo.png")));
	mwEdit->setIcon(QIcon(QString(":/edit.png")));
	mwUpDown->setIcon(QIcon(QString(":/up_down.png")));

	mwRight->setFocusPolicy(Qt::NoFocus);
	mwLeft->setFocusPolicy(Qt::NoFocus);
	mwRewind->setFocusPolicy(Qt::NoFocus);
	mwPlay->setFocusPolicy(Qt::NoFocus);
	mwReload->setFocusPolicy(Qt::NoFocus);
	mwPaste->setFocusPolicy(Qt::NoFocus);
	mwCopy->setFocusPolicy(Qt::NoFocus);
	mwUndo->setFocusPolicy(Qt::NoFocus);
	mwRedo->setFocusPolicy(Qt::NoFocus);
	mwEdit->setFocusPolicy(Qt::NoFocus);
	mwUpDown->setFocusPolicy(Qt::NoFocus);

	connect(mwRight, SIGNAL(released()), this, SLOT(handle_move_right()));
	connect(mwLeft, SIGNAL(released()), this, SLOT(handle_move_left()));
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
	main_tb->addWidget(mwLeft);
	main_tb->addWidget(mwRight);
	main_tb->addWidget(0);
	main_tb->addWidget(mwUpDown);

#ifndef HAVE_NO_SHOW
	tab_show_control = new MppShowControl(this);
#endif
	for (x = 0; x != MPP_MAX_VIEWS; x++)
		scores_main[x] = new MppScoreMain(this, x);

	tab_import = new MppImportTab(this);

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
	    " * Copyright (c) 2009-2015 Hans Petter Selasky. All rights reserved.\n"
	    " */\n"

	    "\n"
	    "/*\n"
	    " * Command syntax:\n"
	    " * ===============\n"
	    " *\n"
	    " * U<number>[.] - specifies the duration of the following scores [0..255].\n"
	    " * T<number> - specifies the track number of the following scores [0..15].\n"
	    " * K<number> - defines a command [0..99].\n"
	    " * W<number>.<number> - defines an autoplay timeout [1..9999ms].\n"
	    " * K0 - no operation.\n"
	    " * K1 - lock play key until next label jump.\n"
	    " * K2 - unlock play key.\n"
	    " * K3.<bpm>.<period_ms> - set reference BPM and period in ms.\n"
	    " * K4.<number> - enable automatic melody effect on the N-th note, if non-zero.\n"
	    " * K5.<number> - set number of base scores for chord mode. Default value is 2.\n"
	    " * M<number> - macro inline the given label.\n"
	    " * L<number> - defines a label [0..31].\n"
	    " * J<P><number> - jumps to the given label [0..31] \n"
	    " *     and optionally starts a new page(P) in printouts.\n"
	    " * S\"<string .(chord) .(chord)>\" - creates a visual string.\n"
	    " * CDEFGAH<number><B> - defines a score in the given octave [0..10].\n"
	    " * X[+/-]<number> - defines the transpose level of the following scores in half-steps.\n"
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
	tab_play_gl = new MppPlayGridLayout(this);
	tab_chord_gl = new MppDecodeTab(this);
	tab_pianotab = new MppPianoTab(this);
	tab_config_gl = new MppGridLayout();
	tab_instr_gl = new MppGridLayout();
	tab_volume_gl = new MppGridLayout();

	gl_ctrl = new MppGroupBox(tr("Main controls"));
	gl_time = new MppGroupBox(tr("Time counter"));
	gl_bpm = new MppGroupBox(tr("Average Beats Per Minute, BPM, for generator"));
	gl_synth_play = new MppGroupBox(tr("Synth and Play controls"));

	/* Fill up tabbar */
#if MPP_MAX_VIEWS > 0
	main_tb->addTab(&scores_main[0]->viewWidget, tr("View A"));
	main_tb->addTab(scores_main[0]->editWidget, tr("Edit A"));
#endif
#if MPP_MAX_VIEWS > 1
	main_tb->addTab(&scores_main[1]->viewWidget, tr("View B"));
	main_tb->addTab(scores_main[1]->editWidget, tr("Edit B"));
#endif
#if MPP_MAX_VIEWS > 2
	main_tb->addTab(&scores_main[2]->viewWidget, tr("View C"));
	main_tb->addTab(scores_main[2]->editWidget, tr("Edit C"));
#endif
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
	main_tb->addTab(tab_instr_gl, tr("Instrument"));
	main_tb->addTab(tab_volume_gl, tr("Volume"));
	main_tb->addTab(tab_database, tr("Database"));
	main_tb->addTab(tab_help, tr("Help"));

	/* <File> Tab */

	but_quit = new QPushButton(tr("Quit"));

	but_midi_file_new = new QPushButton(tr("New"));
	but_midi_file_open = new QPushButton(tr("Open"));
	but_midi_file_merge = new QPushButton(tr("Merge"));
	but_midi_file_save = new QPushButton(tr("Save"));
	but_midi_file_save_as = new QPushButton(tr("Save As"));

	gb_midi_file = new MppGroupBox(tr("MIDI File"));
	gb_midi_file->addWidget(but_midi_file_new, 0, 0, 1, 1);
	gb_midi_file->addWidget(but_midi_file_open, 1, 0, 1, 1);
	gb_midi_file->addWidget(but_midi_file_merge, 2, 0, 1, 1);
	gb_midi_file->addWidget(but_midi_file_save, 3, 0, 1, 1);
	gb_midi_file->addWidget(but_midi_file_save_as, 4, 0, 1, 1);

	for (x = 0; x != MPP_MAX_VIEWS; x++) {
		but_midi_file_import[x] = new MppButton(QString("To %1-Scores").arg(QChar('A' + x)), x);
		gb_midi_file->addWidget(but_midi_file_import[x], 5 + x, 0, 1, 1);
		connect(but_midi_file_import[x], SIGNAL(released(int)), this, SLOT(handle_midi_file_import(int)));
	}

	gb_gpro_file_import = new MppGroupBox(tr("GPro v3/4"));

	for (x = 0; x != MPP_MAX_VIEWS; x++) {
		but_gpro_file_import[x] = new MppButton(QString("Import and\n" "open in %1-Scores").arg(QChar('A' + x)), x);
		gb_gpro_file_import->addWidget(but_gpro_file_import[x], x, 0, 1, 1);
		connect(but_gpro_file_import[x], SIGNAL(released(int)), this, SLOT(handle_gpro_file_import(int)));
	}
#if MPP_MAX_VIEWS > 0
	tab_file_gl->addWidget(scores_main[0]->gbScoreFile, 0, 0, 2, 1);
#endif
#if MPP_MAX_VIEWS > 1
	tab_file_gl->addWidget(scores_main[1]->gbScoreFile, 0, 1, 2, 1);
#endif
#if MPP_MAX_VIEWS > 2
	tab_file_gl->addWidget(scores_main[2]->gbScoreFile, 0, 2, 2, 1);
#endif
	tab_file_gl->addWidget(tab_import->gbImport, 0, MPP_MAX_VIEWS, 1, 1);
	tab_file_gl->addWidget(gb_gpro_file_import, 1, MPP_MAX_VIEWS, 1, 1);
	tab_file_gl->addWidget(gb_midi_file, 0, MPP_MAX_VIEWS + 1, 2, 1);

	tab_file_gl->setRowStretch(2, 1);
	tab_file_gl->setColumnStretch(4, 1);

	tab_file_gl->addWidget(but_quit, 3, 0, 1, 4);

	/* <Play> Tab */

	but_bpm = new QPushButton(tr("BP&M generator"));
	connect(but_bpm, SIGNAL(released()), this, SLOT(handle_bpm()));

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
		but_jump[n] = new MppButton(tr("J%1").arg(n), n);

	but_compile = new QPushButton(tr("Com&pile"));

	but_midi_pause = new QPushButton(tr("Pau&se"));
	but_midi_trigger = new QPushButton(tr("Tri&gger"));
	but_midi_rewind = new QPushButton(tr("Re&wind"));

	mbm_midi_play = new MppButtonMap("MIDI playback\0" "OFF\0" "ON\0", 2, 2);
	connect(mbm_midi_play, SIGNAL(selectionChanged(int)), this, SLOT(handle_midi_play(int)));

	mbm_midi_record = new MppButtonMap("MIDI recording\0" "OFF\0" "ON\0", 2, 2);
	connect(mbm_midi_record, SIGNAL(selectionChanged(int)), this, SLOT(handle_midi_record(int)));

	mbm_score_record = new MppButtonMap("Score recording\0" "OFF\0" "ON\0", 2, 2);
	connect(mbm_score_record, SIGNAL(selectionChanged(int)), this, SLOT(handle_score_record(int)));

	mbm_key_mode_a = new MppButtonMap("Key Mode for view A\0" "ALL\0" "MIXED\0" "FIXED\0" "TRANSP\0" "CHORD-PIANO\0" "CHORD-GUITAR\0", 6, 3);
	connect(mbm_key_mode_a, SIGNAL(selectionChanged(int)), this, SLOT(handle_key_mode_a(int)));

	mbm_key_mode_b = new MppButtonMap("Key Mode for view B\0" "ALL\0" "MIXED\0" "FIXED\0" "TRANSP\0" "CHORD-PIANO\0" "CHORD-GUITAR\0", 6, 3);
	connect(mbm_key_mode_b, SIGNAL(selectionChanged(int)), this, SLOT(handle_key_mode_b(int)));

	/* First column */

	tab_play_gl->addWidget(gl_time,0,0,1,1);
	tab_play_gl->addWidget(gl_ctrl,1,0,1,1);
	tab_play_gl->addWidget(mbm_midi_play, 2,0,1,1);
	tab_play_gl->addWidget(mbm_midi_record, 3,0,1,1);
	tab_play_gl->addWidget(mbm_key_mode_a, 4,0,1,1);

	/* Second column */

	tab_play_gl->addWidget(gl_synth_play,0,1,2,2);
	tab_play_gl->addWidget(mbm_score_record, 2,1,1,2);
	tab_play_gl->addWidget(gl_bpm, 3,1,1,2);
	tab_play_gl->addWidget(mbm_key_mode_b, 4,1,1,2);

	tab_play_gl->setRowStretch(5, 1);
	tab_play_gl->setColumnStretch(4, 1);

	gl_bpm->addWidget(lbl_bpm_avg_val, 0, 0, 1, 1, Qt::AlignHCenter|Qt::AlignVCenter);

	gl_time->addWidget(lbl_curr_time_val, 0, 0, 1, 1, Qt::AlignHCenter|Qt::AlignVCenter);

	gl_ctrl->addWidget(but_midi_pause, 0, 0, 1, 1);
	gl_ctrl->addWidget(but_midi_trigger, 1, 0, 1, 1);
	gl_ctrl->addWidget(but_midi_rewind, 2, 0, 1, 1);
	gl_ctrl->addWidget(but_compile, 3, 0, 1, 1);

	gl_synth_play->addWidget(but_bpm, 0, 0, 1, 2);

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

	gb_config_device = new MppGroupBox(tr("Device configuration"));

	gb_config_device->addWidget(new QLabel(tr("Device Name")), 0, 1, 1, 1, Qt::AlignHCenter|Qt::AlignVCenter);
	gb_config_device->addWidget(new QLabel(tr("Play")), 0, 2, 1, 1, Qt::AlignHCenter|Qt::AlignVCenter);
	gb_config_device->addWidget(new QLabel(tr("Rec.")), 0, 3, 1, 1, Qt::AlignHCenter|Qt::AlignVCenter);
	gb_config_device->addWidget(new QLabel(tr("Synth")), 0, 4, 1, 1, Qt::AlignHCenter|Qt::AlignVCenter);
	for (x = 0; x != MPP_MAX_VIEWS; x++)
		gb_config_device->addWidget(new QLabel(tr("View-%1").arg(QChar('A' + x))), 0, 5 + x, 1, 1, Qt::AlignHCenter|Qt::AlignVCenter);
	gb_config_device->addWidget(new QLabel(tr("Mute Map")), 0, 5 + MPP_MAX_VIEWS, 1, 1, Qt::AlignHCenter|Qt::AlignVCenter);
	gb_config_device->addWidget(new QLabel(tr("DevSel")), 0, 6 + MPP_MAX_VIEWS, 1, 1, Qt::AlignHCenter|Qt::AlignVCenter);

	gb_config_device->setColumnStretch(1, 1);

	for (n = 0; n != MPP_MAX_DEVS; n++) {
		but_config_mm[n] = new MppButton(tr("MM"), n);
		connect(but_config_mm[n], SIGNAL(released(int)), this, SLOT(handle_mute_map(int)));

		but_config_dev[n] = new MppButton(tr("DEV"), n);
		connect(but_config_dev[n], SIGNAL(released(int)), this, SLOT(handle_config_dev(int)));

		led_config_dev[n] = new QLineEdit();
		led_config_dev[n]->setMaxLength(256);
		connect(led_config_dev[n], SIGNAL(textChanged(const QString &)), this, SLOT(handle_config_changed()));

		for (x = 0; x != (3 + MPP_MAX_VIEWS); x++) {
			cbx_config_dev[n][x] = new MppCheckBox(n);
			connect(cbx_config_dev[n][x], SIGNAL(stateChanged(int,int)), this, SLOT(handle_config_changed()));
			gb_config_device->addWidget(cbx_config_dev[n][x], n + 1, 2 + x, 1, 1, Qt::AlignHCenter|Qt::AlignVCenter);
		}

		gb_config_device->addWidget(new QLabel(tr("Dev%1:").arg(n)), n + 1, 0, 1, 1, Qt::AlignHCenter|Qt::AlignLeft);
		gb_config_device->addWidget(led_config_dev[n], n + 1, 1, 1, 1);
		gb_config_device->addWidget(but_config_mm[n], n + 1, 5 + MPP_MAX_VIEWS, 1, 1, Qt::AlignHCenter|Qt::AlignVCenter);
		gb_config_device->addWidget(but_config_dev[n], n + 1, 6 + MPP_MAX_VIEWS, 1, 1, Qt::AlignHCenter|Qt::AlignVCenter);
	}

	led_config_insert = new QLineEdit(QString());
	led_config_insert->setMaxLength(256);

	gb_config_insert = new MppGroupBox(tr("Scores recording prefix string"));
	gb_config_insert->addWidget(led_config_insert, 0, 0, 1, 1);

	x = 0;

	tab_config_gl->addWidget(gb_config_device, x, 0, 1, 8);

	x++;

	tab_config_gl->addWidget(gb_config_insert, x, 0, 1, 8);

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
	tab_config_gl->addWidget(mpp_settings->but_config_clean, x, 3, 1, 1);
	tab_config_gl->setColumnStretch(8, 1);

	/* <Instrument> tab */

	but_non_channel_mute_all = new MppButtonMap("Mute non-channel specific MIDI events\0NO\0YES\0", 2, 2);
	connect(but_non_channel_mute_all, SIGNAL(selectionChanged(int)), this, SLOT(handle_non_channel_muted_changed(int)));

	but_instr_program = new QPushButton(tr("Program One"));
	but_instr_program_all = new QPushButton(tr("Program All"));
	but_instr_reset = new QPushButton(tr("Reset"));
	but_instr_rem = new QPushButton(tr("Delete muted"));
	but_instr_mute_all = new QPushButton(tr("Mute all"));
	but_instr_unmute_all = new QPushButton(tr("Unmute all"));

	spn_instr_curr_chan = new MppChanSel(0, 0);
	connect(spn_instr_curr_chan, SIGNAL(valueChanged(int)), this, SLOT(handle_instr_channel_changed(int)));

	spn_instr_curr_bank = new QSpinBox();
	spn_instr_curr_bank->setRange(0, 16383);
	spn_instr_curr_bank->setValue(0);

	spn_instr_curr_prog = new QSpinBox();
	spn_instr_curr_prog->setRange(0, 127);
	spn_instr_curr_prog->setValue(0);

	gb_instr_select = new MppGroupBox(tr("Synth and Recording Instrument Selector"));
	gb_instr_select->addWidget(new QLabel(tr("Channel")), 0, 0, 1, 1, Qt::AlignVCenter|Qt::AlignHCenter);
	gb_instr_select->addWidget(new QLabel(tr("Bank")), 0, 1, 1, 1, Qt::AlignVCenter|Qt::AlignHCenter);
	gb_instr_select->addWidget(new QLabel(tr("Program")), 0, 2, 1, 1, Qt::AlignVCenter|Qt::AlignHCenter);
	gb_instr_select->addWidget(spn_instr_curr_chan, 1, 0, 1, 1);
	gb_instr_select->addWidget(spn_instr_curr_bank, 1, 1, 1, 1);
	gb_instr_select->addWidget(spn_instr_curr_prog, 1, 2, 1, 1);
	gb_instr_select->addWidget(but_instr_program, 1, 3, 1, 3);
	gb_instr_select->addWidget(but_instr_program_all, 1, 6, 1, 2);

	gb_instr_table = new MppGroupBox(tr("Synth and Recording Instrument Table"));

	gb_instr_table->addWidget(new QLabel(tr("Bank")), 0, 1, 1, 1, Qt::AlignVCenter|Qt::AlignHCenter);
	gb_instr_table->addWidget(new QLabel(tr("Program")), 0, 2, 1, 1, Qt::AlignVCenter|Qt::AlignHCenter);
	gb_instr_table->addWidget(new QLabel(tr("Mute")), 0, 3, 1, 1, Qt::AlignVCenter|Qt::AlignHCenter);

	gb_instr_table->addWidget(new QLabel(tr("Bank")), 0, 5, 1, 1, Qt::AlignVCenter|Qt::AlignHCenter);
	gb_instr_table->addWidget(new QLabel(tr("Program")), 0, 6, 1, 1, Qt::AlignVCenter|Qt::AlignHCenter);
	gb_instr_table->addWidget(new QLabel(tr("Mute")), 0, 7, 1, 1, Qt::AlignVCenter|Qt::AlignHCenter);

	for (n = 0; n != 16; n++) {
		int y_off = (n & 8) ? 4 : 0;

		spn_instr_bank[n] = new QSpinBox();
		spn_instr_bank[n]->setRange(0, 16383);
		connect(spn_instr_bank[n], SIGNAL(valueChanged(int)), this, SLOT(handle_instr_changed(int)));

		spn_instr_prog[n] = new QSpinBox();
		spn_instr_prog[n]->setRange(0, 127);
		connect(spn_instr_prog[n], SIGNAL(valueChanged(int)), this, SLOT(handle_instr_changed(int)));

		cbx_instr_mute[n] = new MppCheckBox(n);
		connect(cbx_instr_mute[n], SIGNAL(stateChanged(int,int)), this, SLOT(handle_instr_changed(int)));

		gb_instr_table->addWidget(new QLabel(MppChanName(n)), (n & 7) + 1, 0 + y_off, 1, 1, Qt::AlignVCenter|Qt::AlignRight);
		gb_instr_table->addWidget(spn_instr_bank[n], (n & 7) + 1, 1 + y_off, 1, 1, Qt::AlignVCenter|Qt::AlignRight);
		gb_instr_table->addWidget(spn_instr_prog[n], (n & 7) + 1, 2 + y_off, 1, 1, Qt::AlignVCenter|Qt::AlignHCenter);
		gb_instr_table->addWidget(cbx_instr_mute[n], (n & 7) + 1, 3 + y_off, 1, 1, Qt::AlignVCenter|Qt::AlignHCenter);
	}


	tab_instr_gl->addWidget(gb_instr_select, 0, 0, 1, 8);
	tab_instr_gl->addWidget(gb_instr_table, 1, 0, 1, 8);
	tab_instr_gl->addWidget(but_non_channel_mute_all, 2, 0, 1, 8);

	tab_instr_gl->setRowStretch(3, 1);
	tab_instr_gl->setColumnStretch(8, 1);

	tab_instr_gl->addWidget(but_instr_mute_all, 4, 0, 1, 2);
	tab_instr_gl->addWidget(but_instr_unmute_all, 4, 2, 1, 2);
 	tab_instr_gl->addWidget(but_instr_rem, 4, 4, 1, 2);
	tab_instr_gl->addWidget(but_instr_reset, 4, 6, 1, 2);

	/* <Volume> tab */

	gb_volume_play = new MppGroupBox(tr("Playback"));
	gb_volume_synth = new MppGroupBox(tr("Synth and Recording"));

	but_volume_reset = new QPushButton(tr("Reset"));

	for (n = 0; n != 16; n++) {
		int y_off = (n & 8) ? 2 : 0;

		spn_volume_synth[n] = new MppVolume();
		spn_volume_synth[n]->setRange(0, MPP_VOLUME_MAX, MPP_VOLUME_UNIT);
		connect(spn_volume_synth[n], SIGNAL(valueChanged(int)), this, SLOT(handle_volume_changed(int)));

		spn_volume_play[n] = new MppVolume();
		spn_volume_play[n]->setRange(0, MPP_VOLUME_MAX, MPP_VOLUME_UNIT);
		connect(spn_volume_play[n], SIGNAL(valueChanged(int)), this, SLOT(handle_volume_changed(int)));

		gb_volume_play->addWidget(new QLabel(MppChanName(n)), (n & 7) + x, 0 + y_off, 1, 1, Qt::AlignVCenter|Qt::AlignRight);
		gb_volume_play->addWidget(spn_volume_play[n], (n & 7) + x, 1 + y_off, 1, 1, Qt::AlignVCenter|Qt::AlignHCenter);

		gb_volume_synth->addWidget(new QLabel(MppChanName(n)), (n & 7) + x, 0 + y_off, 1, 1, Qt::AlignVCenter|Qt::AlignRight);
		gb_volume_synth->addWidget(spn_volume_synth[n], (n & 7) + x, 1 + y_off, 1, 1, Qt::AlignVCenter|Qt::AlignHCenter);
	}

	tab_volume_gl->addWidget(gb_volume_play, 0, 0, 1, 1);
	tab_volume_gl->addWidget(gb_volume_synth, 0, 1, 1, 1);

	tab_volume_gl->setRowStretch(1, 1);
	tab_volume_gl->setColumnStretch(4, 1);

	tab_volume_gl->addWidget(but_volume_reset, 2, 0, 1, 2);

	/* Connect all */

	for (n = 0; n != MPP_MAX_LBUTTON; n++)
		connect(but_jump[n], SIGNAL(released(int)), this, SLOT(handle_jump(int)));

	connect(but_compile, SIGNAL(released()), this, SLOT(handle_compile()));
	connect(but_quit, SIGNAL(released()), this, SLOT(handle_quit()));

	connect(but_midi_file_new, SIGNAL(released()), this, SLOT(handle_midi_file_new()));
	connect(but_midi_file_open, SIGNAL(released()), this, SLOT(handle_midi_file_new_open()));
	connect(but_midi_file_merge, SIGNAL(released()), this, SLOT(handle_midi_file_merge_open()));
	connect(but_midi_file_save, SIGNAL(released()), this, SLOT(handle_midi_file_save()));
	connect(but_midi_file_save_as, SIGNAL(released()), this, SLOT(handle_midi_file_save_as()));

	connect(but_midi_trigger, SIGNAL(pressed()), this, SLOT(handle_midi_trigger()));
	connect(but_midi_rewind, SIGNAL(pressed()), this, SLOT(handle_rewind()));
	connect(but_config_view_fontsel, SIGNAL(released()), this, SLOT(handle_config_view_fontsel()));
	connect(but_config_edit_fontsel, SIGNAL(released()), this, SLOT(handle_config_edit_fontsel()));

	connect(but_instr_rem, SIGNAL(released()), this, SLOT(handle_instr_rem()));
	connect(but_instr_program, SIGNAL(released()), this, SLOT(handle_instr_program()));
	connect(but_instr_program_all, SIGNAL(released()), this, SLOT(handle_instr_program_all()));
	connect(but_instr_reset, SIGNAL(released()), this, SLOT(handle_instr_reset()));
	connect(but_instr_mute_all, SIGNAL(released()), this, SLOT(handle_instr_mute_all()));
	connect(but_instr_unmute_all, SIGNAL(released()), this, SLOT(handle_instr_unmute_all()));

	connect(but_volume_reset, SIGNAL(released()), this, SLOT(handle_volume_reset()));

	connect(but_midi_pause, SIGNAL(pressed()), this, SLOT(handle_midi_pause()));

	MidiInit();

	version = tr("MIDI Player Pro v1.2.17");

	setWindowTitle(version);
	setWindowIcon(QIcon(QString(MPP_ICON_FILE)));

	handle_tab_changed(1);

	watchdog->start(250);

	mpp_settings->handle_load();
}

MppMainWindow :: ~MppMainWindow()
{
	MidiUnInit();
}

void
MppMainWindow :: closeEvent(QCloseEvent *event)
{
	exit(0);
}

void
MppMainWindow :: handle_quit()
{
	exit(0);
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
	pthread_mutex_lock(&mtx);
	handle_jump_locked(index);
	pthread_mutex_unlock(&mtx);
}

void
MppMainWindow :: handle_non_channel_muted_changed(int value)
{
	pthread_mutex_lock(&mtx);
	nonChannelMuted = value;
	pthread_mutex_unlock(&mtx);
}

void
MppMainWindow :: handle_compile(int force)
{
	int x;
	int y;

	for (y = x = 0; x != MPP_MAX_VIEWS; x++)
		y += scores_main[x]->handleCompile(force);

	if (y != 0) {
		pthread_mutex_lock(&mtx);
		handle_stop();
		pthread_mutex_unlock(&mtx);
	}
}

void
MppMainWindow :: handle_score_record(int value)
{
	pthread_mutex_lock(&mtx);
	scoreRecordOff = value ? 0 : 1;
	pthread_mutex_unlock(&mtx);
}

void
MppMainWindow :: handle_midi_pause()
{
	uint32_t pos;
	uint8_t triggered;
	uint8_t paused;

	pthread_mutex_lock(&mtx);
	pos = (umidi20_get_curr_position() - startPosition) & 0x3FFFFFFFU;
	triggered = midiTriggered;
	paused = midiPaused;
	pthread_mutex_unlock(&mtx);

	if (paused)
		return;		/* nothing to do */

	handle_rewind();

	pthread_mutex_lock(&mtx);
	if (triggered != 0) {
		midiPaused = 1;
		pausePosition = pos;
	}
	pthread_mutex_unlock(&mtx);
}

void
MppMainWindow :: handle_midi_play(int value)
{
	uint8_t triggered;

	pthread_mutex_lock(&mtx);
	midiPlayOff = value ? 0 : 1;
	triggered = midiTriggered;
	update_play_device_no();
	pthread_mutex_unlock(&mtx);

	handle_midi_pause();

	if (triggered)
		handle_midi_trigger();
}

void
MppMainWindow :: handle_midi_record(int value)
{
	uint8_t triggered;

	pthread_mutex_lock(&mtx);
	midiRecordOff = value ? 0 : 1;
	triggered = midiTriggered;
	update_play_device_no();
	pthread_mutex_unlock(&mtx);

	handle_midi_pause();

	if (triggered)
		handle_midi_trigger();
}

void
MppMainWindow :: handle_play_press(int key, int which)
{
	if (which < 0 || which >= MPP_MAX_VIEWS)
		which = 0;

	pthread_mutex_lock(&mtx);
	if (tab_loop->handle_trigN(key, 90) != 0) {
		/* ignore */
	} else {
		MppScoreMain *sm = scores_main[which];
		sm->handleMidiKeyPressLocked(0, key, 90);
	}
	pthread_mutex_unlock(&mtx);
}

void
MppMainWindow :: handle_play_release(int key, int which)
{
	if (which < 0 || which >= MPP_MAX_VIEWS)
		which = 0;

	pthread_mutex_lock(&mtx);
	MppScoreMain *sm = scores_main[which];
	sm->handleMidiKeyReleaseLocked(0, key);
	pthread_mutex_unlock(&mtx);
}

void
MppMainWindow :: handle_sustain_press(int which)
{
	if (which < 0 || which >= MPP_MAX_VIEWS)
		which = 0;

	MppScoreMain *sm = scores_main[which];

	pthread_mutex_lock(&mtx);
	sm->outputControl(0x40, 127);
	pthread_mutex_unlock(&mtx);
}

void
MppMainWindow :: handle_sustain_release(int which)
{
	if (which < 0 || which >= MPP_MAX_VIEWS)
		which = 0;

	MppScoreMain *sm = scores_main[which];

	pthread_mutex_lock(&mtx);
	sm->outputControl(0x40, 0);
	pthread_mutex_unlock(&mtx);
}

void
MppMainWindow :: handle_watchdog_sub(MppScoreMain *sm, int update_cursor)
{
	QTextCursor cursor(sm->editWidget->textCursor());
	int play_line;

	pthread_mutex_lock(&mtx);
	play_line = sm->head.getCurrLine();
	pthread_mutex_unlock(&mtx);

	if (update_cursor) {
		cursor.movePosition(QTextCursor::Start, QTextCursor::MoveAnchor, 1);
		cursor.movePosition(QTextCursor::Down, QTextCursor::MoveAnchor, play_line);
		cursor.movePosition(QTextCursor::StartOfLine, QTextCursor::MoveAnchor, 1);
		sm->editWidget->setTextCursor(cursor);
		sm->watchdog();
	}

	sm->viewWidgetSub->update();
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

	pthread_mutex_lock(&mtx);
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

		pthread_mutex_unlock(&mtx);

		QPlainTextEdit *ped = currEditor();
		QTextCursor cursor;

		mid_sort(events_copy[0], num_events);

		last_duration = 0;

		if (ped != 0) {
			cursor = QTextCursor(ped->textCursor());
			cursor.movePosition(QTextCursor::StartOfLine, QTextCursor::MoveAnchor, 1);
			cursor.beginEditBlock();
			cursor.insertText(led_config_insert->text());
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

		pthread_mutex_lock(&mtx);
	}
	num_events = numControlEvents;
	if (num_events != 0) {
		numControlEvents = 0;
		memcpy(events_copy, controlEvents, num_events * sizeof(controlEvents[0]));
	}
	pthread_mutex_unlock(&mtx);

	for (x = 0; x != num_events; x++) {
		tab_shortcut->handle_record_event(events_copy[x]);
	}

	if (instr_update)
		handle_instr_changed(0);

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
	if (ops & MPP_OPERATION_BPM)
		dlg_bpm->sync();
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
	handle_midi_file_clear_name();

	handle_rewind();

	if (track != NULL) {
		pthread_mutex_lock(&mtx);
		umidi20_event_queue_drain(&(track->queue));
		chanUsageMask = 0;
		pthread_mutex_unlock(&mtx);

		handle_instr_reset();
		handle_instr_channel_changed(0);
	}
}

void
MppMainWindow :: update_play_device_no()
{
	struct umidi20_event *event;

	if (track == NULL)
		return;

	UMIDI20_QUEUE_FOREACH(event, &(track->queue))
		event->device_no = MPP_MAGIC_DEVNO;	/* hint for "MidiEventTxCallback() */
}

void
MppMainWindow :: handle_midi_file_merge_open()
{
	handle_midi_file_open(1);
}

void
MppMainWindow :: handle_midi_file_new_open()
{
	handle_midi_file_open(0);
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

	diag->setAcceptMode(QFileDialog::AcceptOpen);
	diag->setFileMode(QFileDialog::ExistingFile);

	if (diag->exec()) {

		Mpp.HomeDirMid = diag->directory().path();

		if (how == 1) {
			handle_midi_file_clear_name();
			handle_rewind();
		} else {
			handle_midi_file_new();
		}

		CurrMidiFileName = new QString(diag->selectedFiles()[0]);

		if (MppReadRawFile(*CurrMidiFileName, &data) == 0) {
		
			pthread_mutex_lock(&mtx);
			song_copy = umidi20_load_file(&mtx,
			    (const uint8_t *)data.data(), data.size());
			pthread_mutex_unlock(&mtx);

			if (song_copy == NULL) {
				QMessageBox box;

				box.setText(tr("Invalid MIDI file!"));
				box.setStandardButtons(QMessageBox::Ok);
				box.setIcon(QMessageBox::Information);
				box.setWindowIcon(QIcon(QString(MPP_ICON_FILE)));
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

	pthread_mutex_lock(&mtx);

	UMIDI20_QUEUE_FOREACH(track_copy, &(song_copy->queue)) {

	    printf("track %p\n", track_copy);

	    UMIDI20_QUEUE_FOREACH(event, &(track_copy->queue)) {

	        if (umidi20_event_is_voice(event) ||
		    umidi20_event_is_sysex(event)) {

		    if (do_instr_check(event, NULL) != 0) {
			event_copy = NULL;
		    } else {
			if (umidi20_event_get_what(event) & UMIDI20_WHAT_CHANNEL) {
				uint8_t chan;
				chan = umidi20_event_get_channel(event);
				chanUsageMask |= (1 << chan);
			}
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

			umidi20_event_queue_insert(&(track->queue),
			    event_copy, UMIDI20_CACHE_INPUT);
		}
	    }
	}

	umidi20_song_free(song_copy);

	update_play_device_no();

	pthread_mutex_unlock(&mtx);

	if (how == 2)
		handle_midi_file_import(0);
done:
	/* make sure we save into a new file */
	if (how != 0)
		handle_midi_file_clear_name();

	delete diag;
}

/* must be called locked */
void
MppMainWindow :: handle_midi_file_instr_prepend()
{
	struct mid_data *d = &mid_data;
	uint8_t x;

	for (x = 0; x != 16; x++) {

		/* 
		 * Don't insert instrument bank and program commands
		 * for channels without events!
		 */
		if (!(chanUsageMask & (1 << x)))
			continue;

		mid_set_channel(d, x);
		mid_set_position(d, 0);
		mid_set_device_no(d, 0xFF);
		mid_set_bank_program(d, x,
		    instr[x].bank,
		    instr[x].prog);
	}
}

/* must be called locked */
void
MppMainWindow :: handle_midi_file_instr_delete()
{
	umidi20_event_queue_move(&(track->queue), NULL, 0,
	    MPP_MIN_POS, 0, 0-1, UMIDI20_CACHE_INPUT);
}

void
MppMainWindow :: handle_midi_file_save()
{
	uint8_t *data;
	uint32_t len;
	uint8_t status;

	if (CurrMidiFileName != NULL) {

		pthread_mutex_lock(&mtx);
		handle_midi_file_instr_prepend();
		status = umidi20_save_file(song, &data, &len);
		handle_midi_file_instr_delete();
		pthread_mutex_unlock(&mtx);

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
				box.setWindowIcon(QIcon(QString(MPP_ICON_FILE)));
				box.exec();
			}
		} else {
			QMessageBox box;

			box.setText(tr("Could not get MIDI data!"));
			box.setStandardButtons(QMessageBox::Ok);
			box.setIcon(QMessageBox::Information);
			box.setWindowIcon(QIcon(QString(MPP_ICON_FILE)));

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
		pthread_mutex_lock(&mtx);
		/* kill all leftover notes */
		handle_stop();
		/* send song stop event */
		send_song_stop_locked();
		pthread_mutex_unlock(&mtx);

		/* wait for MIDI events to propagate */
		MppSleep::msleep(100 /* ms */);
	}

	pthread_mutex_lock(&mtx);

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
	pthread_mutex_unlock(&mtx);
}

void
MppMainWindow :: handle_midi_trigger()
{
	pthread_mutex_lock(&mtx);

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

		/* XXX single recursing point */
		send_song_trigger_locked();

		midiTriggered = 1;
		midiPaused = 0;
		pausePosition = 0;

		dlg_bpm->handle_update(1);
		tab_replay->metronome->handleUpdateLocked();
	}
	pthread_mutex_unlock(&mtx);
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
		if ((deviceBits & (MPP_DEV0_RECORD << (3 * n))) && 
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
			default:
				cfg.cfg_dev[n].rec_enabled_cfg = UMIDI20_DISABLE_CFG;
				break;
			}
		} else {
			cfg.cfg_dev[n].rec_enabled_cfg = UMIDI20_DISABLE_CFG;
		}

		if ((deviceBits & ((MPP_DEV0_SYNTH | MPP_DEV0_PLAY) << (3 * n))) && 
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
	n = MPP_MAGIC_DEVNO;
	STRLCPY(cfg.cfg_dev[n].play_fname, "/dev/null",
	    sizeof(cfg.cfg_dev[n].play_fname));
	cfg.cfg_dev[n].play_enabled_cfg = 1;

	umidi20_config_import(&cfg);

	handle_compile();
}

void
MppMainWindow :: handle_config_apply(int devno)
{
	uint8_t ScMidiTriggered;
	uint32_t devInputMaskCopy[MPP_MAX_DEVS];
	int n;
	int x;

	deviceBits = 0;
	memset(devInputMaskCopy, 0, sizeof(devInputMaskCopy));

	for (n = 0; n != MPP_MAX_DEVS; n++) {

		if (deviceName[n] != NULL)
			free(deviceName[n]);

		deviceName[n] = MppQStringToAscii(led_config_dev[n]->text());

		if (cbx_config_dev[n][0]->isChecked())
			deviceBits |= 1UL << ((3*n)+0);
		if (cbx_config_dev[n][1]->isChecked())
			deviceBits |= 1UL << ((3*n)+1);
		if (cbx_config_dev[n][2]->isChecked())
			deviceBits |= 1UL << ((3*n)+2);
		for (x = 0; x != MPP_MAX_VIEWS; x++) {
			if (cbx_config_dev[n][3 + x]->isChecked())
				devInputMaskCopy[n] |= (1U << x);
		}
	}

	handle_config_reload();

	/* wait for MIDI devices to be opened */
	MppSleep::msleep(100 /* ms */);

	pthread_mutex_lock(&mtx);
	memcpy(devInputMask, devInputMaskCopy, sizeof(devInputMask));
	ScMidiTriggered = midiTriggered;
	midiTriggered = 1;

	/*
	 * Update local key enable/disable on all devices and
	 * channels:
	 */
	for (n = 0; n != MPP_MAX_DEVS; n++) {

		if (devno != -1 && devno != n)
			continue;

		for (x = 0; x != 16; x++) {
			uint8_t buf[4];

			if (check_synth(n, x, 0) == 0)
				continue;

			if (enableLocalKeys[n]) {
				buf[0] = 0xB0 | x;
				buf[1] = 0x7A;
				buf[2] = 0x7F;
				mid_add_raw(&mid_data, buf, 3, x);
			}
			if (disableLocalKeys[n]) {
				buf[0] = 0xB0 | x;
				buf[1] = 0x7A;
				buf[2] = 0x00;
				mid_add_raw(&mid_data, buf, 3, x);
			}
		}
	}

	midiTriggered = ScMidiTriggered;
	pthread_mutex_unlock(&mtx);
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

		for (x = 0; x != MPP_MAX_VIEWS; x++)
			scores_main[x]->editWidget->setFont(font);

		tab_help->setFont(font);
		tab_import->editWidget->setFont(font);
	}
}

uint8_t
MppMainWindow :: check_synth(uint8_t device_no, uint8_t chan, uint32_t off)
{
	struct mid_data *d = &mid_data;
	uint32_t pos;

	if (device_no >= MPP_MAX_DEVS)
		return (0);

	if (deviceBits & (MPP_DEV0_SYNTH << (3 * device_no))) {

		handle_midi_trigger();

		/* compute relative time distance */
		pos = umidi20_get_curr_position() - startPosition + off;

		/* compensate for processing delay */
		if (pos != 0)
			pos--;

		mid_set_channel(d, chan & 0xF);
		mid_set_position(d, pos);
		mid_set_device_no(d, device_no);

		return (1);
	}
	return (0);
}

uint8_t
MppMainWindow :: check_play(uint8_t chan, uint32_t off)
{
	struct mid_data *d = &mid_data;
	uint32_t pos;

	handle_midi_trigger();

	/* compute relative time distance */
	pos = umidi20_get_curr_position() - startPosition + off;

	/* compensate for processing delay */
	if (pos != 0)
		pos--;

	mid_set_channel(d, chan & 0xF);
	mid_set_position(d, pos);
	mid_set_device_no(d, MPP_MAGIC_DEVNO);

	return (1);
}

uint8_t
MppMainWindow :: check_record(uint8_t chan, uint32_t off)
{
	struct mid_data *d = &mid_data;
	uint32_t pos;

	if (midiRecordOff)
		return (0);

	chan &= 0xF;

	handle_midi_trigger();

	pos = (umidi20_get_curr_position() - startPosition + off) & 0x3FFFFFFFU;
	if (pos < MPP_MIN_POS)
		pos = MPP_MIN_POS;

	chanUsageMask |= (1 << chan);

	mid_set_channel(d, chan);
	mid_set_position(d, pos);
	mid_set_device_no(d, 0xFF);

	return (1);
}

void
MppMainWindow :: do_key_press(int key, int vel, int dur)
{
	struct mid_data *d = &mid_data;
	int ch = mid_get_channel(d);

	if (vel != 0) {
		vel = (synthVolume[ch] * vel) / MPP_VOLUME_UNIT;

		if (vel > 127)
			vel = 127;
		else if (vel < 0)
			vel = 0;
		else if (vel == 1)
			vel = 2;
	}

	if (key > 127)
		return;
	else if (key < 0)
		return;

	if (dur < 0)
		return;

	mid_key_press(d, key, vel, dur);
}

/* must be called locked */
void
MppMainWindow :: handle_stop(int flag)
{
	uint32_t *pkey;
	uint8_t ScMidiTriggered;
	uint8_t ScMidiRecordOff;
	uint8_t out_key;
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

			out_key = (*pkey >> 8) & 0xFF;
			chan = (*pkey >> 16) & 0xFF;
			delay = (*pkey >> 24) & 0xFF;

			/* only release once */
			*pkey = 0;

			output_key(chan, out_key, 0, delay, 0);
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

		output_key(ps->channel, ps->key, 0, 0, 0);

		/* check for secondary event */
		if (ps->channelSec != 0)
			output_key(ps->channelSec - 1, ps->key, 0, 0, 0);
	    }

	    /* check if we should kill the pedal, modulation and pitch */
	    if (!(flag & 1)) {
		scores_main[z]->outputControl(0x40, 0);
		scores_main[z]->outputControl(0x01, 0);
		scores_main[z]->outputPitch(1U << 13);
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

	pthread_mutex_lock(&mtx);
	time_offset = get_time_offset();
	pthread_mutex_unlock(&mtx);

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

	pthread_mutex_lock(&mw->mtx);

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

		key = umidi20_event_get_key(event) & 0x7F;
		vel = umidi20_event_get_velocity(event);

		if (mw->scoreRecordOff == 0) {
			if (mw->numInputEvents < MPP_MAX_QUEUE) {
				mw->inputEvents[mw->numInputEvents] = key;
				mw->numInputEvents++;
				mw->lastInputEvent = umidi20_get_curr_position();
			}
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

			key = umidi20_event_get_key(event) & 0x7F;
			vel = umidi20_event_get_pressure(event) & 0x7F;

			switch (sm->keyMode) {
			case MM_PASS_ALL:
				sm->outputKeyPressure(sm->synthChannel, key, vel);
				break;
			case MM_PASS_NONE_CHORD_PIANO:
			case MM_PASS_NONE_CHORD_GUITAR:
				sm->handleKeyPressureChord(key, vel);
				break;
			default:
				break;
			}

		} else if (what & UMIDI20_WHAT_CHANNEL_PRESSURE) {

			vel = umidi20_event_get_pressure(event) & 0x7F;

			switch (sm->keyMode) {
			case MM_PASS_ALL:
			case MM_PASS_NONE_CHORD_PIANO:
			case MM_PASS_NONE_CHORD_GUITAR:
				sm->outputChanPressure(vel);
				break;
			default:
				break;
			}

		} else if (umidi20_event_is_key_start(event)) {

			key = umidi20_event_get_key(event) & 0x7F;
			vel = umidi20_event_get_velocity(event);

			sm->handleMidiKeyPressLocked(chan, key, vel);

		} else if (umidi20_event_is_key_end(event)) {

			key = umidi20_event_get_key(event) & 0x7F;

			sm->handleMidiKeyReleaseLocked(chan, key);

		} else if (mw->do_instr_check(event, &chan)) {

		} else if ((what & UMIDI20_WHAT_CONTROL_VALUE) &&
		    (ctrl < 120)) {

			/* Only pass non-channel-mode control messages */

			vel = umidi20_event_get_control_value(event);

			sm->outputControl(ctrl, vel);
		}
	}
done:
	pthread_mutex_unlock(&mw->mtx);
}

/* NOTE: Is called unlocked */
static void
MidiEventTxCallback(uint8_t device_no, void *arg, struct umidi20_event *event, uint8_t *drop)
{
	MppMainWindow *mw = (MppMainWindow *)arg;
	struct umidi20_event *p_event;
	uint32_t what;
	int vel;
	int do_drop = (device_no == MPP_MAGIC_DEVNO);

	pthread_mutex_lock(&mw->mtx);

	what = umidi20_event_get_what(event);

	if (what & UMIDI20_WHAT_CHANNEL) {
		uint8_t chan;

		chan = umidi20_event_get_channel(event) & 0xF;

		vel = umidi20_event_get_velocity(event);

		/* check for playback event */
		if (do_drop) {

			uint8_t x;

			if (vel != 0) {

				vel = (vel * mw->playVolume[chan]) / MPP_VOLUME_UNIT;

				if (vel > 127)
					vel = 127;
				else if (vel < 0)
					vel = 0;

				umidi20_event_set_velocity(event, vel);
			}

			/* check if we should duplicate events for other devices */
			for (x = 0; x < MPP_MAX_DEVS; x++) {
				if (mw->deviceBits & (MPP_DEV0_PLAY << (3 * x))) {
					p_event = umidi20_event_copy(event, 1);
					if (p_event != NULL) {
						p_event->device_no = x;
						umidi20_event_queue_insert(&(root_dev.play[x].queue),
							p_event, UMIDI20_CACHE_INPUT);
					}
				}
			}
		}

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
			/* check for channel mute */
			if (mw->muteMap[device_no][chan]) {
				do_drop = 1;
			}
		}

	} else if (event->cmd[1] != 0xFF) {

		/* check for playback event */
		if (do_drop) {

			uint8_t x;

			/* check if we should duplicate events for other devices */
			for (x = 0; x < MPP_MAX_DEVS; x++) {
				if (mw->deviceBits & (MPP_DEV0_PLAY << (3 * x))) {
					p_event = umidi20_event_copy(event, 1);
					if (p_event != NULL) {
						p_event->device_no = x;
						umidi20_event_queue_insert(&(root_dev.play[x].queue),
							p_event, UMIDI20_CACHE_INPUT);
					}
				}
			}
		}

		if (mw->nonChannelMuted) {
			do_drop = 1;
		} else if (device_no < MPP_MAX_DEVS) {
			if (mw->muteAllNonChannel[device_no])
				do_drop = 1;
		}
	}
	*drop = do_drop;
	pthread_mutex_unlock(&mw->mtx);
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
MppMainWindow :: handle_instr_channel_changed(int chan)
{
	int temp[2];

	temp[0] = spn_instr_bank[chan]->value();
	temp[1] = spn_instr_prog[chan]->value();

	spn_instr_curr_bank->setValue(temp[0]);
	spn_instr_curr_prog->setValue(temp[1]);

	spn_instr_curr_chan->blockSignals(1);
	spn_instr_curr_chan->setValue(chan);
	spn_instr_curr_chan->blockSignals(0);
}

void
MppMainWindow :: handle_instr_program()
{
	int chan = spn_instr_curr_chan->value();
	int bank = spn_instr_curr_bank->value();
	int prog = spn_instr_curr_prog->value();

	spn_instr_bank[chan]->blockSignals(1);
	spn_instr_prog[chan]->blockSignals(1);
	cbx_instr_mute[chan]->blockSignals(1);

	spn_instr_bank[chan]->setValue(bank);
	spn_instr_prog[chan]->setValue(prog);
	cbx_instr_mute[chan]->setChecked(0);

	spn_instr_bank[chan]->blockSignals(0);
	spn_instr_prog[chan]->blockSignals(0);
	cbx_instr_mute[chan]->blockSignals(0);

	pthread_mutex_lock(&mtx);
	instr[chan].updated |= 1;
	pthread_mutex_unlock(&mtx);

	handle_instr_changed(0);
}

void
MppMainWindow :: handle_instr_program_all()
{
	uint8_t x;

	pthread_mutex_lock(&mtx);
	for (x = 0; x != 16; x++)
		instr[x].updated |= 1;
	pthread_mutex_unlock(&mtx);

	handle_instr_changed(0);
}

void 
MppMainWindow :: handle_instr_changed(int dummy)
{
	struct mid_data *d = &mid_data;
	int temp[3];
	uint8_t curr_chan;
	uint8_t x;
	uint8_t y;
	uint8_t update_curr;
	uint8_t trig;

	curr_chan = spn_instr_curr_chan->value();

	for (x = 0; x != 16; x++) {

		temp[0] = spn_instr_bank[x]->value();
		temp[1] = spn_instr_prog[x]->value();
		temp[2] = cbx_instr_mute[x]->isChecked();

		pthread_mutex_lock(&mtx);

		update_curr = 0;

		if (instr[x].bank != temp[0]) {
			if (instr[x].updated & 2) {
				temp[0] = instr[x].bank;
			} else {
				instr[x].bank = temp[0];
				instr[x].updated |= 1;
			}
			update_curr = 1;
		}
		if (instr[x].prog != temp[1]) {
			if (instr[x].updated & 2) {
				temp[1] = instr[x].prog;
			} else {
				instr[x].prog = temp[1];
				instr[x].updated |= 1;
			}
			update_curr = 1;
		}
		if (instr[x].muted != temp[2]) {
			if (instr[x].updated & 2) {
				temp[2] = instr[x].muted;
			} else {
				instr[x].muted = temp[2];
				instr[x].updated |= 1;
			}
			update_curr = 1;
		}
		pthread_mutex_unlock(&mtx);

		if (update_curr) {
			spn_instr_bank[x]->blockSignals(1);
			spn_instr_prog[x]->blockSignals(1);
			cbx_instr_mute[x]->blockSignals(1);

			spn_instr_bank[x]->setValue(temp[0]);
			spn_instr_prog[x]->setValue(temp[1]);
			cbx_instr_mute[x]->setChecked(temp[2]);

			spn_instr_bank[x]->blockSignals(0);
			spn_instr_prog[x]->blockSignals(0);
			cbx_instr_mute[x]->blockSignals(0);

			pthread_mutex_lock(&mtx);
			update_curr = (curr_chan == x);
			pthread_mutex_unlock(&mtx);
		}

		if (update_curr) {
			spn_instr_curr_chan->blockSignals(1);
			spn_instr_curr_bank->blockSignals(1);
			spn_instr_curr_prog->blockSignals(1);

			spn_instr_curr_chan->setValue(x);
			spn_instr_curr_bank->setValue(temp[0]);
			spn_instr_curr_prog->setValue(temp[1]);

			spn_instr_curr_chan->blockSignals(0);
			spn_instr_curr_bank->blockSignals(0);
			spn_instr_curr_prog->blockSignals(0);
		}
	}

	/* Do the real programming */

	pthread_mutex_lock(&mtx);
	trig = midiTriggered;
	midiTriggered = 1;

	for (x = 0; x != 16; x++) {
		if (instr[x].updated == 0)
			continue;

		instr[x].updated = 0;
		for (y = 0; y != MPP_MAX_DEVS; y++) {
			if (muteProgram[y] != 0)
				continue;
			if (check_synth(y, x, 0) == 0)
				continue;
			mid_delay(d, (4 * x));
			mid_set_bank_program(d, x, 
			    instr[x].bank,
			    instr[x].prog);
		}
	}

	midiTriggered = trig;
	pthread_mutex_unlock(&mtx);
}

void 
MppMainWindow :: handle_instr_reset()
{
	uint8_t x;

	but_non_channel_mute_all->setSelection(0);

	for (x = 0; x != 16; x++) {
		spn_instr_bank[x]->blockSignals(1);
		spn_instr_prog[x]->blockSignals(1);
		cbx_instr_mute[x]->blockSignals(1);

		spn_instr_bank[x]->setValue(0);
		spn_instr_prog[x]->setValue(0);
		cbx_instr_mute[x]->setChecked(0);

		spn_instr_bank[x]->blockSignals(0);
		spn_instr_prog[x]->blockSignals(0);
		cbx_instr_mute[x]->blockSignals(0);

		pthread_mutex_lock(&mtx);
		instr[x].updated = 1;
		pthread_mutex_unlock(&mtx);
	}

	spn_instr_curr_chan->blockSignals(1);
	spn_instr_curr_bank->blockSignals(1);
	spn_instr_curr_prog->blockSignals(1);

	spn_instr_curr_chan->setValue(0);
	spn_instr_curr_bank->setValue(0);
	spn_instr_curr_prog->setValue(0);

	spn_instr_curr_chan->blockSignals(0);
	spn_instr_curr_bank->blockSignals(0);
	spn_instr_curr_prog->blockSignals(0);

	handle_instr_changed(0);
}

void
MppMainWindow :: handle_volume_changed(int dummy)
{
	int play_temp[16];
	int synth_temp[16];
	int x;

	for (x = 0; x != 16; x++) {
		play_temp[x] = spn_volume_play[x]->value();
		synth_temp[x] = spn_volume_synth[x]->value();
	}

	pthread_mutex_lock(&mtx);
	for (x = 0; x != 16; x++) {
		playVolume[x] = play_temp[x];
		synthVolume[x] = synth_temp[x];
	}
	pthread_mutex_unlock(&mtx);
}

void
MppMainWindow :: handle_make_scores_visible(MppScoreMain *sm)
{
	if (sm->visual_max == 0)
		handle_make_tab_visible(sm->editWidget);
	else
		handle_make_tab_visible(&sm->viewWidget);
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
		if (main_tb->isVisible(&scores_main[x]->viewWidget)) {
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
	lastViewIndex = 0;

	QString *ps = scores_main[x/2]->currScoreFileName;
	if (ps != NULL)
		setWindowTitle(version + " - " + MppBaseName(*ps));
	else
		setWindowTitle(version);

	if ((force != 0) || (x & 1))
		handle_compile();
}

void 
MppMainWindow :: handle_volume_reset()
{
	uint8_t x;

	for (x = 0; x != 16; x++) {
		spn_volume_play[x]->blockSignals(1);
		spn_volume_synth[x]->blockSignals(1);

		spn_volume_play[x]->setValue(MPP_VOLUME_UNIT);
		spn_volume_synth[x]->setValue(MPP_VOLUME_UNIT);

		spn_volume_play[x]->blockSignals(0);
		spn_volume_synth[x]->blockSignals(0);
	}

	handle_volume_changed(0);
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

	UMIDI20_QUEUE_FOREACH(event, &(im_track->queue)) {

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

	snprintf(buf, sizeof(buf), "W%u.%u /* %u */",
	    retval - lend, lend, retval);

	if (psum != 0)
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
	QTextCursor cursor(scores_main[view]->editWidget->textCursor());

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

	pthread_mutex_lock(&mtx);

	UMIDI20_QUEUE_FOREACH(event, &(im_track->queue)) {

		if (!(umidi20_event_get_what(event) & UMIDI20_WHAT_CHANNEL))
			continue;

		if (umidi20_event_is_key_start(event)) {
			chan = umidi20_event_get_channel(event) & 0xF;
			chan_mask |= (1 << chan);
		}
	}

	pthread_mutex_unlock(&mtx);

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

	pthread_mutex_lock(&mtx);

	umidi20_track_compute_max_min(im_track);

	max_index = convert_midi_duration(im_track, thres, chan_mask);

	UMIDI20_QUEUE_FOREACH(event, &(im_track->queue)) {

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

			out_block += mid_key_str[umidi20_event_get_key(event)];
			out_block += " ";
		}
	}

	pthread_mutex_unlock(&mtx);

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

	cursor.insertText(output);
	
	handle_compile();
	handle_make_tab_visible(scores_main[view]->editWidget);
}

void
MppMainWindow :: handle_midi_file_import(int n)
{
	import_midi_track(track, MIDI_FLAG_DIALOG | MIDI_FLAG_MULTI_CHAN, 0, n);
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
			box.setWindowIcon(QIcon(QString(MPP_ICON_FILE)));

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
MppMainWindow :: MidiInit(void)
{
	int n;

	cbx_config_dev[0][0]->setChecked(1);	/* Play */
	cbx_config_dev[0][1]->setChecked(1);	/* Record */
	cbx_config_dev[0][2]->setChecked(1);	/* Synth */
	for (n = 0; n != MPP_MAX_DEVS; n++)
		cbx_config_dev[n][3]->setChecked(1);	/* View-A */

	led_config_dev[0]->setText(QString("X:"));

	handle_midi_record(0);
	handle_midi_play(0);
	handle_score_record(0);
	handle_instr_reset();
	handle_volume_reset();

	for (n = 0; n != UMIDI20_N_DEVICES; n++) {
		umidi20_set_record_event_callback(n, &MidiEventRxCallback, this);
		umidi20_set_play_event_callback(n, &MidiEventTxCallback, this);
	}

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

	umidi20_song_start(song, 0x40000000, 0x80000000,
	    UMIDI20_FLAG_PLAY | UMIDI20_FLAG_RECORD);

	startPosition = umidi20_get_curr_position() - 0x40000000;

	pthread_mutex_unlock(&mtx);

	/* reload the configuration */

	handle_config_reload();
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
		if (deviceName[n] != NULL) {
			free(deviceName[n]);
			deviceName[n] = NULL;
		}
	}
}

MppPlayGridLayout :: MppPlayGridLayout(MppMainWindow *parent)
  : QWidget(parent), QGridLayout(this)
{
	mw = parent;
}

MppPlayGridLayout :: ~MppPlayGridLayout()
{
}

void
MppPlayGridLayout :: keyPressEvent(QKeyEvent *event)
{
	/* fake pedal down event */
	switch (event->key()) {
	case Qt::Key_0:
	case Qt::Key_1:
	case Qt::Key_2:
	case Qt::Key_3:
	case Qt::Key_4:
	case Qt::Key_5:
	case Qt::Key_6:
	case Qt::Key_7:
	case Qt::Key_8:
	case Qt::Key_9:
		mw->handle_jump(event->key() - Qt::Key_0);
		break;
	default:
		break;
	}
}

void
MppPlayGridLayout :: keyReleaseEvent(QKeyEvent *event)
{
}

int
MppMainWindow :: handle_mute_map(int n)
{
	int retval;

	MppMuteMap diag(this, this, n);

	retval = diag.exec();

	if (retval == QDialog::Accepted)
		handle_config_apply(n);

	return (retval);
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
		led_config_dev[n]->blockSignals(1);
		led_config_dev[n]->setText(diag.result_dev);
		led_config_dev[n]->blockSignals(0);
		handle_config_apply(n);
	}
	return (retval);
}

void
MppMainWindow :: handle_instr_rem()
{
	struct umidi20_event *event;
	struct umidi20_event *event_next;
	uint8_t chan;

	if (track == NULL)
		return;

	handle_instr_changed(0);

	pthread_mutex_lock(&mtx);

	UMIDI20_QUEUE_FOREACH_SAFE(event, &(track->queue), event_next) {
		if (umidi20_event_get_what(event) & UMIDI20_WHAT_CHANNEL) {
			chan = umidi20_event_get_channel(event) & 0xF;
			if (instr[chan].muted) {
				UMIDI20_IF_REMOVE(&(track->queue), event);
				umidi20_event_free(event);
			}
		} else if (nonChannelMuted) {
			UMIDI20_IF_REMOVE(&(track->queue), event);
			umidi20_event_free(event);
		}
	}
	pthread_mutex_unlock(&mtx);
}

void
MppMainWindow :: handle_instr_mute_all()
{
	uint8_t n;

	but_non_channel_mute_all->setSelection(1);

	for (n = 0; n != 16; n++) {
		cbx_instr_mute[n]->blockSignals(1);
		cbx_instr_mute[n]->setChecked(1);
		cbx_instr_mute[n]->blockSignals(0);
	}

	handle_instr_changed(0);
}

void
MppMainWindow :: handle_instr_unmute_all()
{
	uint8_t n;

	but_non_channel_mute_all->setSelection(0);

	for (n = 0; n != 16; n++) {
		cbx_instr_mute[n]->blockSignals(1);
		cbx_instr_mute[n]->setChecked(0);
		cbx_instr_mute[n]->blockSignals(0);
	}

	handle_instr_changed(0);
}

void
MppMainWindow :: output_key(int chan, int key, int vel, int delay, int dur)
{
	struct mid_data *d = &mid_data;
	uint8_t y;

	/* check for time scaling */
	if (dlg_bpm->period_cur != 0 && dlg_bpm->bpm_other != 0)
		delay = (dlg_bpm->period_ref * delay) / dlg_bpm->bpm_other;

	/* output note to all synths first */
	for (y = 0; y != MPP_MAX_DEVS; y++) {
		if (check_synth(y, chan, 0)) {
			mid_delay(d, delay);
			do_key_press(key, vel, dur);
		}
	}

	/* output note to recording device */
	if (check_record(chan, 0)) {
		mid_delay(d, delay);
		do_key_press(key, vel, dur);
	}

	tab_loop->add_key(key, vel);
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
		pthread_mutex_lock(&mtx);
		value = scores_main[0]->keyMode;
		pthread_mutex_unlock(&mtx);
		mbm_key_mode_a->setSelection(value);
	} else 	if (index == 1) {
		pthread_mutex_lock(&mtx);
		value = scores_main[1]->keyMode;
		pthread_mutex_unlock(&mtx);
		mbm_key_mode_b->setSelection(value);
	}
}

void
MppMainWindow :: handle_key_mode_a(int value)
{
	if (value < 0 || value >= MM_PASS_MAX)
		value = 0;

	pthread_mutex_lock(&mtx);
	scores_main[0]->keyMode = value;
	pthread_mutex_unlock(&mtx);
}

void
MppMainWindow :: handle_key_mode_b(int value)
{
	if (value < 0 || value >= MM_PASS_MAX)
		value = 0;

	pthread_mutex_lock(&mtx);
	scores_main[1]->keyMode = value;
	pthread_mutex_unlock(&mtx);
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
	if (qedit != 0)
		qedit->copy();
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
	int n;

	trig = midiTriggered;
	midiTriggered = 1;

	buf[0] = which;

	for (n = 0; n != MPP_MAX_DEVS; n++) {
		if (check_synth(n, 0, 0) != 0)
			mid_add_raw(&mid_data, buf, 1, 0);
	}
	if (check_record(0, 0) != 0)
		mid_add_raw(&mid_data, buf, 1, 0);
	midiTriggered = trig;
}

void
MppMainWindow :: send_song_select_locked(uint8_t which)
{
	uint8_t buf[2];
	uint8_t trig;
	int n;

	trig = midiTriggered;
	midiTriggered = 1;

	buf[0] = 0xF3;
	buf[1] = which & 0x7F;

	for (n = 0; n != MPP_MAX_DEVS; n++) {
		if (check_synth(n, 0, 0) != 0)
			mid_add_raw(&mid_data, buf, 2, 0);
	}
	if (check_record(0, 0) != 0)
		mid_add_raw(&mid_data, buf, 1, 0);
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

	tab_show_control->curr_st = MPP_SHOW_ST_LYRICS;
	tab_show_control->handle_show();
	MppScreenShot(tab_show_control->wg_show, app);
}
#endif
