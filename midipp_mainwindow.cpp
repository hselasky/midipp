/*-
 * Copyright (c) 2009-2012 Hans Petter Selasky. All rights reserved.
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
#include "midipp_scores.h"
#include "midipp_mutemap.h"
#include "midipp_looptab.h"
#include "midipp_database.h"
#include "midipp_decode.h"
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

	editFont.fromString(QString("Monospace,-1,14,5,50,0,0,0,0,0"));
	editFont.setStyleHint(QFont::TypeWriter);

	/* Main GUI */

	mwRight = new QPushButton();
	mwLeft = new QPushButton();
	mwRewind = new QPushButton();
	mwPlay = new QPushButton();
	mwReload = new QPushButton();

	mwRight->setIcon(QIcon(QString(":/right_arrow.png")));
	mwLeft->setIcon(QIcon(QString(":/left_arrow.png")));
	mwRewind->setIcon(QIcon(QString(":/stop.png")));
	mwPlay->setIcon(QIcon(QString(":/play.png")));
	mwReload->setIcon(QIcon(QString(":/reload.png")));

	connect(mwRight, SIGNAL(released()), this, SLOT(handle_move_right()));
	connect(mwLeft, SIGNAL(released()), this, SLOT(handle_move_left()));
	connect(mwRewind, SIGNAL(released()), this, SLOT(handle_rewind()));
	connect(mwPlay, SIGNAL(released()), this, SLOT(handle_midi_trigger()));
	connect(mwReload, SIGNAL(released()), this, SLOT(handle_compile()));

	/* Setup GUI */

	main_gl = new QGridLayout(this);
	main_tw = new QTabWidget();
	scores_tw = new QTabWidget();

	/* Watchdog */

	watchdog = new QTimer(this);
	connect(watchdog, SIGNAL(timeout()), this, SLOT(handle_watchdog()));

	/* Tabs */

	main_gl->addWidget(scores_tw,0,0,9,1);
	main_gl->addWidget(main_tw,0,2,9,1);

	main_gl->addWidget(mwPlay,1,1,1,1);
	main_gl->addWidget(mwRewind,2,1,1,1);
	main_gl->addWidget(mwReload,4,1,1,1);
	main_gl->addWidget(mwRight,6,1,1,1);
	main_gl->addWidget(mwLeft,7,1,1,1);

	for (x = 0; x != MPP_MAX_VIEWS; x++)
		scores_main[x] = new MppScoreMain(this, x);

	currScoreMain()->devInputMask = -1U;

	tab_import = new MppImportTab(this);

	tab_loop = new MppLoopTab(this, this);

	tab_database = 	new MppDataBase(this);

	tab_help = new QPlainTextEdit();
	tab_help->setFont(editFont);
	tab_help->setLineWrapMode(QPlainTextEdit::NoWrap);
	tab_help->setPlainText(tr(
	    "/*\n"
	    " * Copyright (c) 2009-2013 Hans Petter Selasky. All rights reserved.\n"
	    " */\n"

	    "\n"
	    "/*\n"
	    " * Command syntax:\n"
	    " * ===============\n"
	    " *\n"
	    " * U<number>[.] - specifies the duration of the following scores (0..255).\n"
	    " * T<number> - specifies the track number of the following scores (0..31).\n"
	    " * K<number> - defines a command (0..99).\n"
	    " * W<number>.<number> - defines an autoplay timeout (1..9999ms).\n"
	    " * K0 - no operation.\n"
	    " * K1 - lock play key until next label jump.\n"
	    " * K2 - unlock play key.\n"
	    " * K3.<bpm>.<period_ms> - set reference BPM and period in ms.\n"
	    " * K4.<number> - enable automatic melody effect on the N-th note, if non-zero.\n"
	    " * K5.<number> - set number of base scores for chord mode. Default value is 2.\n"
	    " * M<number> - macro inline the given label. Label definition must come before any references.\n"
	    " * L<number> - defines a label (0..31).\n"
	    " * J<R><P><number> - jumps to the given label (0..31) or \n"
	    " *     Relative(R) line (0..31) and starts a new page(P).\n"
	    " * S\"<string>\" - creates a visual string.\n"
	    " * CDEFGAH<number><B> - defines a score in the given octave (0..10).\n"
	    " * X[+/-]<number> - defines the transpose level of the following scores in half-steps.\n"
	    " */\n"
	    "\n"
	    "/*\n"
	    " * Editor shortcuts:\n"
	    " * =================\n"
	    " *\n"
	    " * By double clicking on a score line in the editor window,\n"
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
	    " */\n") + MppVariantList + tr("\n"));

	tab_file_gl = new MppGridLayout();
	tab_play_gl = new MppPlayGridLayout();
	tab_config_gl = new MppGridLayout();
	tab_instr_gl = new MppGridLayout();
	tab_volume_gl = new MppGridLayout();

	gl_ctrl = new MppGroupBox(tr("Main controls"));
	gl_time = new MppGroupBox(tr("Time counter"));
	gl_bpm = new MppGroupBox(tr("Average Beats Per Minute, BPM"));
	gl_synth_play = new MppGroupBox(tr("Synth and Play controls"));

	scores_tw->addTab(&scores_main[0]->viewWidget, tr("View A-Scores"));
	scores_tw->addTab(scores_main[0]->editWidget, tr("Edit A-Scores"));

	scores_tw->addTab(&scores_main[1]->viewWidget, tr("View B-Scores"));
	scores_tw->addTab(scores_main[1]->editWidget, tr("Edit B-Scores"));

	scores_tw->addTab(tab_import->editWidget, tr("Lyrics"));

	main_tw->addTab(tab_file_gl, tr("File"));
	main_tw->addTab(tab_play_gl, tr("Play"));
	main_tw->addTab(tab_config_gl, tr("Config"));
	main_tw->addTab(tab_instr_gl, tr("Instrument"));
	main_tw->addTab(tab_volume_gl, tr("Volume"));
	main_tw->addTab(tab_loop, tr("Loop"));
	main_tw->addTab(tab_database, tr("Database"));
	main_tw->addTab(tab_help, tr("Help"));

	/* <File> Tab */

	but_quit = new QPushButton(tr("Quit"));

	but_midi_file_new = new QPushButton(tr("New"));
	but_midi_file_open = new QPushButton(tr("Open"));
	but_midi_file_merge = new QPushButton(tr("Merge"));
	but_midi_file_save = new QPushButton(tr("Save"));
	but_midi_file_save_as = new QPushButton(tr("Save As"));
	but_midi_file_import = new QPushButton();

	gb_midi_file = new MppGroupBox(tr("MIDI File"));
	gb_midi_file->addWidget(but_midi_file_new, 0, 0, 1, 1);
	gb_midi_file->addWidget(but_midi_file_open, 1, 0, 1, 1);
	gb_midi_file->addWidget(but_midi_file_merge, 2, 0, 1, 1);
	gb_midi_file->addWidget(but_midi_file_save, 3, 0, 1, 1);
	gb_midi_file->addWidget(but_midi_file_save_as, 4, 0, 1, 1);
	gb_midi_file->addWidget(but_midi_file_import, 5, 0, 1, 1);

	but_gpro_file_import = new QPushButton();

	gb_gpro_file_import = new MppGroupBox(tr("GPro v3/4"));
	gb_gpro_file_import->addWidget(but_gpro_file_import, 0, 0, 1, 1);

	tab_file_gl->addWidget(scores_main[0]->gbScoreFile, 0, 0, 2, 1);
	tab_file_gl->addWidget(scores_main[1]->gbScoreFile, 0, 1, 2, 1);
	tab_file_gl->addWidget(tab_import->gbImport, 0, 2, 1, 1);
	tab_file_gl->addWidget(gb_gpro_file_import, 1, 2, 1, 1);
	tab_file_gl->addWidget(gb_midi_file, 0, 3, 2, 1);

	tab_file_gl->setRowStretch(2, 3);

	tab_file_gl->addWidget(but_quit, 3, 0, 1, 4);

	/* <Play> Tab */

	but_bpm = new QPushButton(tr("BP&M"));
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

	for (n = 0; n != MPP_MAX_LBUTTON; n++) {
		char buf[8];
		snprintf(buf, sizeof(buf), "J%u", n);
		but_jump[n] = new MppButton(tr(buf), n);
	}

	but_insert_chord = new QPushButton(tr("&Insert"));
	but_edit_chord = new QPushButton(tr("&Edit"));
	but_replace = new QPushButton(tr("&ReplaceAll"));
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

	mbm_key_mode = new MppButtonMap("\0" "ALL\0" "MIXED\0" "FIXED\0" "TRANSP\0" "CHORD\0", 5, 3);
	connect(mbm_key_mode, SIGNAL(selectionChanged(int)), this, SLOT(handle_key_mode(int)));

	but_play[0] = new MppButton(tr("Shift+Play+A"), 0);
	but_play[0]->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
	connect(but_play[0], SIGNAL(pressed(int)), this, SLOT(handle_play_press(int)));
	connect(but_play[0], SIGNAL(released(int)), this, SLOT(handle_play_release(int)));

	but_play[1] = new MppButton(tr("Shift+Play+B"), 1);
	but_play[1]->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
	connect(but_play[1], SIGNAL(pressed(int)), this, SLOT(handle_play_press(int)));
	connect(but_play[1], SIGNAL(released(int)), this, SLOT(handle_play_release(int)));

	lbl_play_key = new QLabel(tr("Play Key"));
	spn_play_key = new MppSpinBox();
	connect(spn_play_key, SIGNAL(valueChanged(int)), this, SLOT(handle_play_key_changed(int)));
	spn_play_key->setRange(0, 127);
	spn_play_key->setValue(C4);

	/* First column */

	tab_play_gl->addWidget(gl_time,0,0,1,1);
	tab_play_gl->addWidget(gl_ctrl,1,0,1,1);
	tab_play_gl->addWidget(mbm_midi_play, 2,0,1,1);
	tab_play_gl->addWidget(mbm_midi_record, 3,0,1,1);
	tab_play_gl->addWidget(mbm_key_mode, 4,0,1,1);

	/* Second column */

	tab_play_gl->addWidget(gl_synth_play,0,1,2,2);
	tab_play_gl->addWidget(mbm_score_record, 2,1,1,2);
	tab_play_gl->addWidget(gl_bpm, 3,1,1,2);

	tab_play_gl->addWidget(but_play[0], 4, 1, 1, 1);
	tab_play_gl->addWidget(but_play[1], 4, 2, 1, 1);

	tab_play_gl->setRowStretch(5, 4);

	gl_bpm->addWidget(lbl_bpm_avg_val, 0, 0, 1, 1, Qt::AlignHCenter|Qt::AlignVCenter);

	gl_time->addWidget(lbl_curr_time_val, 0, 0, 1, 1, Qt::AlignHCenter|Qt::AlignVCenter);

	gl_ctrl->addWidget(but_midi_pause, 0, 0, 1, 1);
	gl_ctrl->addWidget(but_midi_trigger, 1, 0, 1, 1);
	gl_ctrl->addWidget(but_midi_rewind, 2, 0, 1, 1);
	gl_ctrl->addWidget(but_compile, 3, 0, 1, 1);

	gl_synth_play->addWidget(but_bpm, 0, 0, 1, 2);
	gl_synth_play->addWidget(lbl_play_key, 0, 2, 1, 1);
	gl_synth_play->addWidget(spn_play_key, 0, 3, 1, 1);

	for (x = 0; x != MPP_MAX_VIEWS; x++)
		gl_synth_play->addWidget(but_mode[x], 1 + x, 2, 1, 2);

	gl_synth_play->addWidget(but_insert_chord, 1, 0, 1, 1);

	gl_synth_play->addWidget(but_edit_chord, 2, 0, 1, 1);
	gl_synth_play->addWidget(but_replace, 2, 1, 1, 1);

	for (x = 0; x != MPP_MAX_LBUTTON; x++)
		gl_synth_play->addWidget(but_jump[x], 1 + MPP_MAX_VIEWS + (x / 4), (x % 4), 1, 1);

	/* <Configuration> Tab */

	mpp_settings = new MppSettings(this, "midipp");

	but_config_apply = new QPushButton(tr("Apply"));
	but_config_revert = new QPushButton(tr("Revert"));
	but_config_view_fontsel = new QPushButton(tr("Change View Font"));
	but_config_edit_fontsel = new QPushButton(tr("Change Editor Font"));

	gb_config_device = new MppGroupBox(tr("Device configuration"));

	gb_config_device->addWidget(new QLabel(tr("Device Name")), 0, 1, 1, 1, Qt::AlignHCenter|Qt::AlignVCenter);
	gb_config_device->addWidget(new QLabel(tr("Play")), 0, 2, 1, 1, Qt::AlignHCenter|Qt::AlignVCenter);
	gb_config_device->addWidget(new QLabel(tr("Rec.")), 0, 3, 1, 1, Qt::AlignHCenter|Qt::AlignVCenter);
	gb_config_device->addWidget(new QLabel(tr("Synth")), 0, 4, 1, 1, Qt::AlignHCenter|Qt::AlignVCenter);
	gb_config_device->addWidget(new QLabel(tr("Mute Map")), 0, 5, 1, 1, Qt::AlignHCenter|Qt::AlignVCenter);
	gb_config_device->addWidget(new QLabel(tr("DevSel")), 0, 6, 1, 1, Qt::AlignHCenter|Qt::AlignVCenter);

	gb_config_device->setColumnStretch(1, 1);

	for (n = 0; n != MPP_MAX_DEVS; n++) {
		char buf[16];

		but_config_mm[n] = new MppButton(tr("MM"), n);
		but_config_dev[n] = new MppButton(tr("DEV"), n);

		connect(but_config_mm[n], SIGNAL(released(int)), this, SLOT(handle_mute_map(int)));
		connect(but_config_dev[n], SIGNAL(released(int)), this, SLOT(handle_config_dev(int)));

		led_config_dev[n] = new QLineEdit(QString());

		cbx_config_dev[n][0] = new QCheckBox();
		cbx_config_dev[n][1] = new QCheckBox();
		cbx_config_dev[n][2] = new QCheckBox();

		snprintf(buf, sizeof(buf), "Dev%d:", n);

		gb_config_device->addWidget(new QLabel(tr(buf)), n + 1, 0, 1, 1, Qt::AlignHCenter|Qt::AlignLeft);
		gb_config_device->addWidget(led_config_dev[n], n + 1, 1, 1, 1);
		gb_config_device->addWidget(cbx_config_dev[n][0], n + 1, 2, 1, 1, Qt::AlignHCenter|Qt::AlignVCenter);
		gb_config_device->addWidget(cbx_config_dev[n][1], n + 1, 3, 1, 1, Qt::AlignHCenter|Qt::AlignVCenter);
		gb_config_device->addWidget(cbx_config_dev[n][2], n + 1, 4, 1, 1, Qt::AlignHCenter|Qt::AlignVCenter);
		gb_config_device->addWidget(but_config_mm[n], n + 1, 5, 1, 1, Qt::AlignHCenter|Qt::AlignVCenter);
		gb_config_device->addWidget(but_config_dev[n], n + 1, 6, 1, 1, Qt::AlignHCenter|Qt::AlignVCenter);
	}

	led_config_insert = new QLineEdit(QString());

	gb_config_insert = new MppGroupBox(tr("Scores recording prefix string"));
	gb_config_insert->addWidget(led_config_insert, 0, 0, 1, 1);

	x = 0;

	tab_config_gl->addWidget(gb_config_device, x, 0, 1, 8);

	x++;

	tab_config_gl->addWidget(gb_config_insert, x, 0, 1, 8);

	x++;

	tab_config_gl->setRowStretch(x, 1);

	x++;

	tab_config_gl->addWidget(mpp_settings->but_config_save, x, 0, 1, 1, Qt::AlignLeft|Qt::AlignVCenter);
	tab_config_gl->addWidget(but_config_view_fontsel, x, 1, 1, 1, Qt::AlignLeft|Qt::AlignVCenter);

	x++;

	tab_config_gl->addWidget(mpp_settings->but_config_what, x, 0, 1, 1, Qt::AlignLeft|Qt::AlignVCenter);
	tab_config_gl->addWidget(but_config_edit_fontsel, x, 1, 1, 1, Qt::AlignLeft|Qt::AlignVCenter);

	x++;

	tab_config_gl->addWidget(mpp_settings->but_config_load, x, 0, 1, 1, Qt::AlignLeft|Qt::AlignVCenter);

	x++;

	tab_config_gl->addWidget(but_config_apply, x, 4, 1, 2);
	tab_config_gl->addWidget(but_config_revert, x, 6, 1, 2);

	tab_config_gl->addWidget(mpp_settings->but_config_clean, x, 2, 1, 1, Qt::AlignLeft|Qt::AlignVCenter);

	/* <Instrument> tab */

	but_instr_program = new QPushButton(tr("Program One"));
	but_instr_program_all = new QPushButton(tr("Program All"));
	but_instr_reset = new QPushButton(tr("Reset"));
	but_instr_rem = new QPushButton(tr("Delete muted"));
	but_instr_mute_all = new QPushButton(tr("Mute all"));
	but_instr_unmute_all = new QPushButton(tr("Unmute all"));

	spn_instr_curr_chan = new QSpinBox();
	connect(spn_instr_curr_chan, SIGNAL(valueChanged(int)), this, SLOT(handle_instr_channel_changed(int)));
	spn_instr_curr_chan->setRange(0, 15);
	spn_instr_curr_chan->setValue(0);

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

		char buf[16];

		snprintf(buf, sizeof(buf), "Ch%X", n);

		spn_instr_bank[n] = new QSpinBox();
		spn_instr_bank[n]->setRange(0, 16383);
		connect(spn_instr_bank[n], SIGNAL(valueChanged(int)), this, SLOT(handle_instr_changed(int)));

		spn_instr_prog[n] = new QSpinBox();
		spn_instr_prog[n]->setRange(0, 127);
		connect(spn_instr_prog[n], SIGNAL(valueChanged(int)), this, SLOT(handle_instr_changed(int)));

		cbx_instr_mute[n] = new QCheckBox();
		connect(cbx_instr_mute[n], SIGNAL(stateChanged(int)), this, SLOT(handle_instr_changed(int)));

		gb_instr_table->addWidget(new QLabel(tr(buf)), (n & 7) + 1, 0 + y_off, 1, 1, Qt::AlignVCenter|Qt::AlignRight);
		gb_instr_table->addWidget(spn_instr_bank[n], (n & 7) + 1, 1 + y_off, 1, 1, Qt::AlignVCenter|Qt::AlignRight);
		gb_instr_table->addWidget(spn_instr_prog[n], (n & 7) + 1, 2 + y_off, 1, 1, Qt::AlignVCenter|Qt::AlignHCenter);
		gb_instr_table->addWidget(cbx_instr_mute[n], (n & 7) + 1, 3 + y_off, 1, 1, Qt::AlignVCenter|Qt::AlignHCenter);
	}


	tab_instr_gl->addWidget(gb_instr_select, 0, 0, 1, 8);
	tab_instr_gl->addWidget(gb_instr_table, 1, 0, 1, 8);

	tab_instr_gl->setRowStretch(2, 1);

	tab_instr_gl->addWidget(but_instr_mute_all, 3, 0, 1, 2);
	tab_instr_gl->addWidget(but_instr_unmute_all, 3, 2, 1, 2);
 	tab_instr_gl->addWidget(but_instr_rem, 3, 4, 1, 2);
	tab_instr_gl->addWidget(but_instr_reset, 3, 6, 1, 2);

	/* <Volume> tab */

	gb_volume_play = new MppGroupBox(tr("Playback"));
	gb_volume_synth = new MppGroupBox(tr("Synth and Recording"));

	but_volume_reset = new QPushButton(tr("Reset"));

	for (n = 0; n != 16; n++) {
		int y_off = (n & 8) ? 2 : 0;

		char buf[16];

		snprintf(buf, sizeof(buf), "Ch%X", n);

		spn_volume_synth[n] = new MppVolume();
		spn_volume_synth[n]->setRange(0, MPP_VOLUME_MAX, MPP_VOLUME_UNIT);
		connect(spn_volume_synth[n], SIGNAL(valueChanged(int)), this, SLOT(handle_volume_changed(int)));

		spn_volume_play[n] = new MppVolume();
		spn_volume_play[n]->setRange(0, MPP_VOLUME_MAX, MPP_VOLUME_UNIT);
		connect(spn_volume_play[n], SIGNAL(valueChanged(int)), this, SLOT(handle_volume_changed(int)));

		gb_volume_play->addWidget(new QLabel(buf), (n & 7) + x, 0 + y_off, 1, 1, Qt::AlignVCenter|Qt::AlignRight);
		gb_volume_play->addWidget(spn_volume_play[n], (n & 7) + x, 1 + y_off, 1, 1, Qt::AlignVCenter|Qt::AlignHCenter);

		gb_volume_synth->addWidget(new QLabel(buf), (n & 7) + x, 0 + y_off, 1, 1, Qt::AlignVCenter|Qt::AlignRight);
		gb_volume_synth->addWidget(spn_volume_synth[n], (n & 7) + x, 1 + y_off, 1, 1, Qt::AlignVCenter|Qt::AlignHCenter);
	}

	tab_volume_gl->addWidget(gb_volume_play, 0, 0, 1, 1);
	tab_volume_gl->addWidget(gb_volume_synth, 0, 1, 1, 1);

	tab_volume_gl->setRowStretch(1, 1);

	tab_volume_gl->addWidget(but_volume_reset, 2, 0, 1, 2);

	/* Connect all */

	connect(but_insert_chord, SIGNAL(released()), this, SLOT(handle_insert_chord()));
	connect(but_edit_chord, SIGNAL(released()), this, SLOT(handle_edit_chord()));
	connect(but_replace, SIGNAL(released()), this, SLOT(handle_replace()));

	for (n = 0; n != MPP_MAX_LBUTTON; n++)
		connect(but_jump[n], SIGNAL(pressed(int)), this, SLOT(handle_jump(int)));

	connect(but_compile, SIGNAL(pressed()), this, SLOT(handle_compile()));
	connect(but_quit, SIGNAL(released()), this, SLOT(handle_quit()));

	connect(but_midi_file_new, SIGNAL(pressed()), this, SLOT(handle_midi_file_new()));
	connect(but_midi_file_open, SIGNAL(pressed()), this, SLOT(handle_midi_file_new_open()));
	connect(but_midi_file_merge, SIGNAL(pressed()), this, SLOT(handle_midi_file_merge_open()));
	connect(but_midi_file_save, SIGNAL(pressed()), this, SLOT(handle_midi_file_save()));
	connect(but_midi_file_save_as, SIGNAL(pressed()), this, SLOT(handle_midi_file_save_as()));
	connect(but_midi_file_import, SIGNAL(pressed()), this, SLOT(handle_midi_file_import()));

	connect(but_gpro_file_import, SIGNAL(pressed()), this, SLOT(handle_gpro_file_import()));

	connect(but_midi_trigger, SIGNAL(pressed()), this, SLOT(handle_midi_trigger()));
	connect(but_midi_rewind, SIGNAL(pressed()), this, SLOT(handle_rewind()));
	connect(but_config_apply, SIGNAL(pressed()), this, SLOT(handle_config_apply()));
	connect(but_config_revert, SIGNAL(pressed()), this, SLOT(handle_config_revert()));
	connect(but_config_view_fontsel, SIGNAL(pressed()), this, SLOT(handle_config_view_fontsel()));
	connect(but_config_edit_fontsel, SIGNAL(pressed()), this, SLOT(handle_config_edit_fontsel()));

	connect(but_instr_rem, SIGNAL(pressed()), this, SLOT(handle_instr_rem()));
	connect(but_instr_program, SIGNAL(pressed()), this, SLOT(handle_instr_program()));
	connect(but_instr_program_all, SIGNAL(pressed()), this, SLOT(handle_instr_program_all()));
	connect(but_instr_reset, SIGNAL(pressed()), this, SLOT(handle_instr_reset()));
	connect(but_instr_mute_all, SIGNAL(pressed()), this, SLOT(handle_instr_mute_all()));
	connect(but_instr_unmute_all, SIGNAL(pressed()), this, SLOT(handle_instr_unmute_all()));

	connect(but_volume_reset, SIGNAL(pressed()), this, SLOT(handle_volume_reset()));

	connect(but_midi_pause, SIGNAL(pressed()), this, SLOT(handle_midi_pause()));

	connect(scores_tw, SIGNAL(currentChanged(int)), this, SLOT(handle_tab_changed()));
	connect(main_tw, SIGNAL(currentChanged(int)), this, SLOT(handle_tab_changed()));

	MidiInit();

	version = tr("MIDI Player Pro v1.1.1");

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
MppMainWindow :: handle_insert_chord()
{
	MppDecode dlg(this, 0);

        if(dlg.exec() == QDialog::Accepted) {
		QTextCursor cursor(currScoreMain()->editWidget->textCursor());
		cursor.beginEditBlock();
		cursor.insertText(led_config_insert->text());
		cursor.insertText(dlg.getText());
		cursor.insertText(QString("\n"));
		cursor.endEditBlock();

		handle_compile();
	}
}

void
MppMainWindow :: handle_edit_chord()
{
	MppScoreMain *sm = currScoreMain();

	handle_compile();

	if (sm->handleEditLine() == 0)
		handle_compile();
}

void
MppMainWindow :: handle_replace()
{
	MppScoreMain *sm = currScoreMain();

	sm->handleReplace();
}

void
MppMainWindow :: handle_jump(int index)
{
	pthread_mutex_lock(&mtx);
	handle_jump_locked(index);
	pthread_mutex_unlock(&mtx);
}

void
MppMainWindow :: handle_play_key_changed(int key)
{
	pthread_mutex_lock(&mtx);
	playKey = key & 0x7F;
	pthread_mutex_unlock(&mtx);
}

void
MppMainWindow :: handle_compile(int force)
{
	int x;

	pthread_mutex_lock(&mtx);
	handle_stop();
	pthread_mutex_unlock(&mtx);

	for (x = 0; x != MPP_MAX_VIEWS; x++)
		scores_main[x]->handleCompile(force);
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
MppMainWindow :: handle_play_press(int which)
{
	if (which < 0 || which >= MPP_MAX_VIEWS)
		which = 0;

	MppScoreMain *sm = scores_main[which];

	pthread_mutex_lock(&mtx);
	if (tab_loop->handle_trigN(playKey, 90)) {
		/* ignore */
	} else if (sm->keyMode != MM_PASS_ALL) {
		sm->handleKeyPress(playKey, 90);
	} else {
		output_key(sm->synthChannel, playKey, 90, 0, 0);
	}
	do_update_bpm();
	pthread_mutex_unlock(&mtx);
}

void
MppMainWindow :: handle_play_release(int which)
{
	if (which < 0 || which >= MPP_MAX_VIEWS)
		which = 0;

	MppScoreMain *sm = scores_main[which];

	pthread_mutex_lock(&mtx);
	if (sm->keyMode != MM_PASS_ALL) {
		sm->handleKeyRelease(playKey);
	} else {
		output_key(sm->synthChannel, playKey, 0, 0, 0);
	}
	pthread_mutex_unlock(&mtx);
}

void
MppMainWindow :: handle_watchdog_sub(MppScoreMain *sm, int update_cursor)
{
	QTextCursor cursor(sm->editWidget->textCursor());
	uint32_t play_line;
	uint32_t play_pos;
	uint32_t x;

	pthread_mutex_lock(&mtx);

	play_pos = sm->currPos;

	/* skip any jumps */
	for (x = 0; x != 128; x++) {
		uint32_t jump_pos;
		jump_pos = sm->resolveJump(play_pos);
		if (jump_pos != 0)
			play_pos = jump_pos - 1;
		else
			break;
	}
	play_line = sm->realLine[play_pos];

	pthread_mutex_unlock(&mtx);

	if (update_cursor) {
		cursor.movePosition(QTextCursor::Start, QTextCursor::MoveAnchor, 1);
		cursor.movePosition(QTextCursor::Down, QTextCursor::MoveAnchor, play_line);
		cursor.movePosition(QTextCursor::StartOfLine, QTextCursor::MoveAnchor, 1);
		sm->editWidget->setTextCursor(cursor);
		sm->watchdog();
	}

	sm->viewWidgetSub->repaint();
}

void
MppMainWindow :: handle_watchdog()
{
	uint32_t delta;
	char buf[32];
	uint8_t events_copy[MPP_MAX_QUEUE];
	uint8_t num_events;
	uint8_t x;
	uint8_t y;
	uint8_t z;
	uint8_t last_duration;
	uint8_t instr_update;
	uint8_t cursor_update;

	/* update focus if any */
	handle_tab_changed();

	pthread_mutex_lock(&mtx);
	cursor_update = cursorUpdate;
	cursorUpdate = 0;
	instr_update = instrUpdated;
	instrUpdated = 0;
	num_events = numInputEvents;

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
	pthread_mutex_unlock(&mtx);

	if (num_events != 0) {
		MppScoreMain *sm = currScoreMain();
		QTextCursor cursor(sm->editWidget->textCursor());

		mid_sort(events_copy, num_events);

		last_duration = 0;

		cursor.movePosition(QTextCursor::StartOfLine, QTextCursor::MoveAnchor, 1);
		cursor.beginEditBlock();
		cursor.insertText(led_config_insert->text());

		for (x = 0; x != num_events; x++) {
			for (y = x; y != num_events; y++) {
				if (events_copy[x] != events_copy[y])
					break;
			}

			z = y - 1;
			y = y - x;

			if (y != last_duration) {
				last_duration = y;
				snprintf(buf, sizeof(buf), "U%d %s ",
				    y, mid_key_str[events_copy[x] & 0x7F]);
			} else {
				snprintf(buf, sizeof(buf), "%s ",
				    mid_key_str[events_copy[x] & 0x7F]);
			}

			cursor.insertText(QString(buf));

			x = z;
		}

		cursor.insertText(QString("\n"));
		cursor.endEditBlock();
		sm->editWidget->setTextCursor(cursor);
	}

	if (instr_update)
		handle_instr_changed(0);

	do_bpm_stats();

	do_clock_stats();

	for (x = 0; x != MPP_MAX_VIEWS; x++)
		handle_watchdog_sub(scores_main[x], cursor_update);

	tab_loop->watchdog();
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
MppMainWindow :: handle_midi_file_open(int merge)
{
	QFileDialog *diag = 
	  new QFileDialog(this, tr("Select MIDI File"), 
		QString(), QString("MIDI File (*.mid *.MID)"));
	struct umidi20_song *song_copy;
	struct umidi20_track *track_copy;
	struct umidi20_event *event;
	struct umidi20_event *event_copy;
	QByteArray data;

	diag->setAcceptMode(QFileDialog::AcceptOpen);
	diag->setFileMode(QFileDialog::ExistingFile);

	if (diag->exec()) {

		if (merge) {
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

		    if (do_instr_check(event, NULL)) {
			event_copy = NULL;
		    } else {
			if (umidi20_event_get_what(event) & UMIDI20_WHAT_CHANNEL) {
				uint8_t chan;
				chan = umidi20_event_get_channel(event);
				chanUsageMask |= (1 << chan);
			}
			event_copy = umidi20_event_copy(event, 0);
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
	}

	umidi20_song_free(song_copy);

	update_play_device_no();

	pthread_mutex_unlock(&mtx);

done:
	/* make sure we save into a new file */
	if (merge)
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
		QString(), QString("MIDI File (*.mid *.MID)"));

	diag->setAcceptMode(QFileDialog::AcceptSave);
	diag->setFileMode(QFileDialog::AnyFile);
	diag->setDefaultSuffix(QString("mid"));

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
		midiTriggered = 1;
		midiPaused = 0;
		pausePosition = 0;

		dlg_bpm->handle_update();
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

	handle_config_revert();
}

void
MppMainWindow :: handle_config_revert()
{
	int n;

	for (n = 0; n != MPP_MAX_DEVS; n++) {

		if (deviceName[n] != NULL)
			led_config_dev[n]->setText(QString(deviceName[n]));
		else
			led_config_dev[n]->setText(QString());

		cbx_config_dev[n][0]->setChecked(
		    (deviceBits & (1UL << ((3*n)+0))) ? 1 : 0);

		cbx_config_dev[n][1]->setChecked(
		    (deviceBits & (1UL << ((3*n)+1))) ? 1 : 0);

		cbx_config_dev[n][2]->setChecked(
		    (deviceBits & (1UL << ((3*n)+2))) ? 1 : 0);
	}
}

void
MppMainWindow :: handle_config_apply()
{
	handle_config_apply_sub(-1);
}

void
MppMainWindow :: handle_config_apply_sub(int devno)
{
	uint8_t ScMidiTriggered;
	int n;
	int x;

	deviceBits = 0;

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
	}

	handle_config_reload();

	/* wait for MIDI devices to be opened */

	usleep(125000);

	pthread_mutex_lock(&mtx);
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

		chan &= 0xF;

		handle_midi_trigger();

		pos = umidi20_get_curr_position() - startPosition + 1 + off;

		mid_set_channel(d, chan);
		mid_set_position(d, pos);
		mid_set_device_no(d, device_no);

		return (1);
	}
	return (0);
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
	uint8_t out_key;
	uint8_t chan;
	uint8_t x;
	uint8_t z;
	uint8_t delay;

	ScMidiTriggered = midiTriggered;
	midiTriggered = 1;

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

	    for (x = 0; x != 24; x++) {
		struct MppScoreEntry *ps;

		ps = &scores_main[z]->score_past[x];

		if (ps->dur != 0) {

			/* only release once */
			ps->dur = 0;

			output_key(ps->channel, ps->key, 0, 0, 0);
		}
	    }

	    /* check if we should kill the pedal, modulation and pitch */
	    if (!(flag & 1)) {
		scores_main[z]->outputControl(0x40, 0);
		scores_main[z]->outputControl(0x01, 0);
		scores_main[z]->outputPitch(1U << 13);
	    }
	}

	midiTriggered = ScMidiTriggered;
}

/* This function must be called locked */

void
MppMainWindow :: do_update_bpm(void)
{
	uint32_t delta;
	uint32_t curr;

	curr = umidi20_get_curr_position();
	delta = curr - lastKeyPress;
	lastKeyPress = curr;

	/* too big delay */
	if (delta >= (UMIDI20_BPM / 15))
		return;

	/* store statistics */
	bpmData[bpmAvgPos] = delta;
	bpmAvgPos++;

	if (bpmAvgPos >= bpmAvgLength)
		bpmAvgPos = 0;
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

void
MppMainWindow :: do_bpm_stats(void)
{
	uint32_t sum = 0;
	uint32_t val;
	uint8_t x;
	uint8_t len;

	len = bpmAvgLength;

	if (len == 0)
		return;

	pthread_mutex_lock(&mtx);

	for (x = 0; x != len; x++) {
		val = bpmData[x];

		sum += val;
	}

	pthread_mutex_unlock(&mtx);

	if (sum == 0)
		sum = 1;

	sum = (len * UMIDI20_BPM) / sum;

	if (sum > 9999)
		sum = 9999;

	lbl_bpm_avg_val->display((int)sum);
}

/* is called locked */
static void
MidiEventRxCallback(uint8_t device_no, void *arg, struct umidi20_event *event, uint8_t *drop)
{
	MppMainWindow *mw = (MppMainWindow *)arg;
	MppScoreMain *sm;
	uint8_t chan;
	uint8_t ctrl;
	int key;
	int vel;
	int lbl;
	int n;

	*drop = 1;

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

		if (mw->tab_loop->handle_trigN(key, vel)) {
			mw->do_update_bpm();
			return;
		}
	}

	for (n = 0; n != MPP_MAX_VIEWS; n++) {
		sm = mw->scores_main[n];

		if (!(sm->devInputMask & (1U << device_no)))
			continue;

		chan = sm->synthChannel;

		ctrl = umidi20_event_get_control_address(event);

		if (umidi20_event_is_pitch_bend(event)) {

			vel = umidi20_event_get_pitch_value(event);

			sm->outputPitch(vel);

		} else if (umidi20_event_get_what(event) &
		    UMIDI20_WHAT_KEY_PRESSURE) {

			key = umidi20_event_get_key(event) & 0x7F;
			vel = umidi20_event_get_pressure(event) & 0x7F;

			if (sm->cmdKey != 0)
				lbl = key - sm->cmdKey;
			else
				lbl = -1;

			switch (sm->keyMode) {
			case MM_PASS_ALL:
				sm->outputKeyPressure(sm->synthChannel, key, vel);
				break;
			case MM_PASS_NONE_CHORD:
				if (sm->checkLabelJump(lbl) == 0)
					sm->handleKeyPressureChord(key, vel);
				break;
			default:
				break;
			}

		} else if (umidi20_event_get_what(event) &
		    UMIDI20_WHAT_CHANNEL_PRESSURE) {

			vel = umidi20_event_get_pressure(event) & 0x7F;

			switch (sm->keyMode) {
			case MM_PASS_ALL:
			case MM_PASS_NONE_CHORD:
				sm->outputChanPressure(sm->synthChannel, vel);
				break;
			default:
				break;
			}

		} else if (ctrl == 0x40 || ctrl == 0x01) {

			vel = umidi20_event_get_control_value(event);

			sm->outputControl(ctrl, vel);

		} else if (umidi20_event_is_key_start(event)) {

			key = umidi20_event_get_key(event) & 0x7F;
			vel = umidi20_event_get_velocity(event);

			if (sm->cmdKey != 0)
				lbl = key - sm->cmdKey;
			else
				lbl = -1;

			if (sm->keyMode != MM_PASS_ALL) {

				if (sm->checkLabelJump(lbl)) {
					mw->handle_jump_locked(lbl);
				} else {
					if ((sm->keyMode == MM_PASS_NONE_FIXED) ||
					    (sm->keyMode == MM_PASS_NONE_TRANS) ||
					    (sm->keyMode == MM_PASS_NONE_CHORD)) {

						if (sm->keyMode == MM_PASS_NONE_FIXED)
							key = mw->playKey;

						if (sm->keyMode != MM_PASS_NONE_CHORD)
							sm->handleKeyPress(key, vel);
						else
							sm->handleKeyPressChord(key, vel);

					} else if (sm->checkHalfPassThru(key) != 0) {

						if (key == sm->baseKey)
							sm->handleKeyPress(key, vel);

					} else if (sm->setPressedKey(chan, key, 255, 0) == 0) {
						mw->output_key(chan, key, vel, 0, 0);
						mw->do_update_bpm();
					}
				}

			} else if (sm->setPressedKey(chan, key, 255, 0) == 0) {

				mw->output_key(chan, key, vel, 0, 0);
				mw->do_update_bpm();
			}

		} else if (umidi20_event_is_key_end(event)) {

			key = umidi20_event_get_key(event) & 0x7F;

			if (sm->cmdKey != 0)
				lbl = key - sm->cmdKey;
			else
				lbl = -1;

			if (sm->keyMode != MM_PASS_ALL) {

				if (sm->checkLabelJump(lbl) == 0) {
					if ((sm->keyMode == MM_PASS_NONE_FIXED) ||
					    (sm->keyMode == MM_PASS_NONE_TRANS) ||
					    (sm->keyMode == MM_PASS_NONE_CHORD)) {

						if (sm->keyMode == MM_PASS_NONE_FIXED)
							key = mw->playKey;

						if (sm->keyMode != MM_PASS_NONE_CHORD)
							sm->handleKeyRelease(key);
						else
							sm->handleKeyReleaseChord(key);

					} else if (sm->checkHalfPassThru(key) != 0) {

						if (key == sm->baseKey)
							sm->handleKeyRelease(key);

					} else if (sm->setPressedKey(chan, key, 0, 0) == 0) {
						mw->output_key(chan, key, 0, 0, 0);
					}
				}

			} else if (sm->setPressedKey(chan, key, 0, 0) == 0) {

				mw->output_key(chan, key, 0, 0, 0);
			}

		} else if (mw->do_instr_check(event, &chan)) {

		}
	}
}

/* is called locked */
static void
MidiEventTxCallback(uint8_t device_no, void *arg, struct umidi20_event *event, uint8_t *drop)
{
	MppMainWindow *mw = (MppMainWindow *)arg;
	struct umidi20_event *p_event;
	int vel;
	int do_drop;

	if (umidi20_event_get_what(event) & UMIDI20_WHAT_CHANNEL) {
		uint8_t chan;

		chan = umidi20_event_get_channel(event) & 0xF;

		vel = umidi20_event_get_velocity(event);

		do_drop = (device_no == MPP_MAGIC_DEVNO);

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

		/* check mute map */
		if ((device_no < MPP_MAX_DEVS) &&
		    (mw->muteMap[device_no][chan])) {
			do_drop = 1;
		}

		*drop = mw->instr[chan].muted || do_drop;
	}
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
MppMainWindow :: handle_tab_changed(int force)
{
	int x;
	int compile = 0;

	for (x = 0; x != MPP_MAX_VIEWS; x++) {
		if (scores_main[x]->editWidget->hasFocus())
			break;
	}
	if (x == MPP_MAX_VIEWS) {
		for (x = 0; x != MPP_MAX_VIEWS; x++) {
			if (scores_tw->currentWidget() == &scores_main[x]->viewWidget ||
			    main_tw->currentWidget() == &scores_main[x]->viewWidget) {
				compile = 1;
				break;
			}
			if (scores_tw->currentWidget() == scores_main[x]->editWidget ||
			    main_tw->currentWidget() == scores_main[x]->editWidget) {
				break;
			}
		}
	}
	if (x == MPP_MAX_VIEWS)
		x = 0;

	if (currViewIndex == x && force == 0)
		return;

	currViewIndex = x;
	QString *ps = scores_main[x]->currScoreFileName;
	if (ps != NULL)
		setWindowTitle(version + " - " + MppBaseName(*ps));
	else
		setWindowTitle(version);

	handle_instr_channel_changed(currScoreMain()->synthChannel);

	if (compile != 0 || force != 0)
		handle_compile();

	but_midi_file_import->setText(tr("To ") +
	    QChar('A' + currViewIndex) + tr("-Scores"));

	but_gpro_file_import->setText(tr("Open In ") +
	    QChar('A' + currViewIndex) + tr("-Scores"));

	tab_import->butImport->setText(tr("To ") +
	    QChar('A' + currViewIndex) + tr("-Scores"));

	sync_key_mode();
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
MppMainWindow :: import_midi_track(struct umidi20_track *im_track, uint32_t flags, int label)
{
	QTextCursor cursor(currScoreMain()->editWidget->textCursor());

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
}

void
MppMainWindow :: handle_midi_file_import()
{
	import_midi_track(track, MIDI_FLAG_DIALOG | MIDI_FLAG_MULTI_CHAN, 0);
}

void
MppMainWindow :: handle_gpro_file_import()
{
	QFileDialog *diag = 
	  new QFileDialog(this, tr("Select GPro v3 or v4 File"), 
		QString(), QString("GPro File (*.gp *.gp3 *.gp4 *.GP *.GP3 *.GP4)"));
	QByteArray data;
	MppGPro *gpro;
	QTextCursor *cursor;

	diag->setAcceptMode(QFileDialog::AcceptOpen);
	diag->setFileMode(QFileDialog::ExistingFile);

	if (diag->exec()) {

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

	cursor = new QTextCursor(currScoreMain()->editWidget->textCursor());
	cursor->beginEditBlock();
	cursor->insertText(gpro->output);
	cursor->endEditBlock();

	delete gpro;
	delete cursor;

	handle_compile();

done:
	delete diag;
}

void
MppMainWindow :: MidiInit(void)
{
	int n;

	deviceBits = MPP_DEV0_SYNTH | MPP_DEV0_PLAY | 
	  MPP_DEV1_RECORD | MPP_DEV2_RECORD | MPP_DEV3_RECORD;
	deviceName[0] = strdup("D:/midi");
	deviceName[1] = strdup("D:/dev/umidi0.0");
	deviceName[2] = strdup("D:/dev/umidi1.0");
	deviceName[3] = strdup("D:/dev/umidi2.0");
	bpmAvgLength = 4;

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
	case Qt::Key_Shift:
		pthread_mutex_lock(&mw->mtx);
		if (mw->midiTriggered != 0) {
			unsigned int x;
			for (x = 0; x != MPP_MAX_VIEWS; x++)
				mw->scores_main[x]->outputControl(0x40, 127);
		}
		pthread_mutex_unlock(&mw->mtx);
		break;
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
	/* fake pedal up event */
	if (event->key() == Qt::Key_Shift) {
		pthread_mutex_lock(&mw->mtx);
		if (mw->midiTriggered != 0) {
			unsigned int x;
			for (x = 0; x != MPP_MAX_VIEWS; x++)
				mw->scores_main[x]->outputControl(0x40, 0);
		}
		pthread_mutex_unlock(&mw->mtx);
	}
}

void
MppMainWindow :: handle_mute_map(int n)
{
	/* mutemap dialog */

	MppMuteMap diag(this, this, n);

	if (diag.exec() == QDialog::Accepted)
		handle_config_apply_sub(n);
}

void
MppMainWindow :: handle_config_dev(int n)
{
	MppDevices diag(this);
	if (diag.exec() == QDialog::Accepted) {
		led_config_dev[n]->setText(diag.result_dev);
	}
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
		}
	}
	pthread_mutex_unlock(&mtx);
}

void
MppMainWindow :: handle_instr_mute_all()
{
	uint8_t n;

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
	if (dlg_bpm->period != 0 && dlg_bpm->bpm != 0)
		delay = (dlg_bpm->ref * delay) / dlg_bpm->bpm;

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
MppMainWindow :: handle_mode(int index)
{
	dlg_mode[index]->update_all();
	dlg_mode[index]->exec();

	sync_key_mode();
}

void
MppMainWindow :: handle_key_mode(int value)
{
	MppScoreMain *sm = currScoreMain();
	MppMode *cm = currModeDlg();

	if (value < 0 || value >= MM_PASS_MAX)
		value = 0;

	pthread_mutex_lock(&mtx);
	sm->keyMode = value;
	pthread_mutex_unlock(&mtx);

	cm->update_all();

	sync_key_mode();
}

void
MppMainWindow :: sync_key_mode()
{
	mbm_key_mode->setSelection(currModeDlg()->but_mode->currSelection);
	mbm_key_mode->setTitle(tr("Key Mode for view ") + QChar('A' + currViewIndex));
}

MppScoreMain *
MppMainWindow :: currScoreMain()
{
	return (scores_main[currViewIndex]);
}

MppMode *
MppMainWindow :: currModeDlg()
{
	return (dlg_mode[currViewIndex]);
}

void
MppMainWindow :: handle_move_right()
{
	QWidget *pw;
	int index;

	index = scores_tw->currentIndex();
	if (index < 0)
		return;

	QString desc = scores_tw->tabText(index);
	pw = scores_tw->widget(index);
	scores_tw->removeTab(index);

	if (scores_tw->widget(0) == NULL)
		scores_tw->setVisible(0);

	main_tw->addTab(pw, desc);
	main_tw->setCurrentWidget(pw);
	main_tw->setVisible(1);
}

void
MppMainWindow :: handle_move_left()
{
	QWidget *pw;
	int index;

	index = main_tw->currentIndex();
	if (index < 0)
		return;

	QString desc = main_tw->tabText(index);
	pw = main_tw->widget(index);
	main_tw->removeTab(index);

	if (main_tw->widget(0) == NULL)
		main_tw->setVisible(0);

	scores_tw->addTab(pw, desc);
	scores_tw->setCurrentWidget(pw);
	scores_tw->setVisible(1);
}
