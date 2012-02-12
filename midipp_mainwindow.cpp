/*-
 * Copyright (c) 2009-2011 Hans Petter Selasky. All rights reserved.
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
#include <midipp_scores.h>
#include <midipp_mutemap.h>
#include <midipp_looptab.h>
#include <midipp_decode.h>
#include <midipp_import.h>
#include <midipp_devices.h>
#include <midipp_spinbox.h>
#include <midipp_bpm.h>
#include <midipp_button.h>
#include <midipp_gpro.h>
#include <midipp_midi.h>
#include <midipp_mode.h>

static void MidiEventRxControl(MppScoreMain *sm, uint8_t ctrl, uint8_t val);
static void MidiEventRxPitch(MppScoreMain *sm, uint16_t val);

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

	defaultFont.fromString(QString("Sans Serif,12,-1,5,75,0,0,0,0,0"));

	/* Setup GUI */

	main_gl = new QGridLayout(this);
	main_tw = new QTabWidget();
	scores_tw = new QTabWidget();

	/* Watchdog */

	watchdog = new QTimer(this);
	connect(watchdog, SIGNAL(timeout()), this, SLOT(handle_watchdog()));

	/* Tabs */

	main_gl->addWidget(scores_tw,0,0,1,1);
	main_gl->addWidget(main_tw,0,1,1,1);

	for (x = 0; x != MPP_MAX_VIEWS; x++)
		scores_main[x] = new MppScoreMain(this);

	currScoreMain()->devInputMask = -1U;

	tab_import = new MppImportTab(this);

	tab_loop = new MppLoopTab(this, this);

	tab_help = new QPlainTextEdit();
	tab_help->setFont(font_fixed);
	tab_help->setLineWrapMode(QPlainTextEdit::NoWrap);
	tab_help->setPlainText(tr(
	    "/*\n"
	    " * Copyright (c) 2009-2012 Hans Petter Selasky. All rights reserved.\n"
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
	    " * M<number> - macro inline the given label.\n"
	    " * L<number> - defines a label (0..31).\n"
	    " * J<R><P><number> - jumps to the given label (0..31) or \n"
	    " *     Relative(R) line (0..31) and starts a new page(P).\n"
	    " * S\"<string>\" - creates a visual string.\n"
	    " * CDEFGAH<number><B> - defines a score in the given octave (0..10).\n"
	    " * X[+/-]<number> - defines the transpose level of the following scores in half-steps.\n"
	    " */\n"
	    "\n"));

	tab_file_wg = new QWidget();
	tab_play_wg = new MppPlayWidget(this);
	tab_config_wg = new QWidget();
	tab_instr_wg = new QWidget();
	tab_volume_wg = new QWidget();

	tab_file_gl = new QGridLayout(tab_file_wg);
	tab_play_gl = new QGridLayout(tab_play_wg);
	tab_config_gl = new QGridLayout(tab_config_wg);
	tab_instr_gl = new QGridLayout(tab_instr_wg);
	tab_volume_gl = new QGridLayout(tab_volume_wg);

	scores_tw->addTab(&scores_main[0]->viewWidget, tr("View A-Scores"));
	scores_tw->addTab(scores_main[0]->editWidget, tr("Edit A-Scores"));

	scores_tw->addTab(&scores_main[1]->viewWidget, tr("View B-Scores"));
	scores_tw->addTab(scores_main[1]->editWidget, tr("Edit B-Scores"));

	scores_tw->addTab(tab_import->editWidget, tr("Lyrics"));

	main_tw->addTab(tab_file_wg, tr("File"));
	main_tw->addTab(tab_play_wg, tr("Play"));
	main_tw->addTab(tab_config_wg, tr("Config"));
	main_tw->addTab(tab_instr_wg, tr("Instrument"));
	main_tw->addTab(tab_volume_wg, tr("Volume"));
	main_tw->addTab(tab_loop, tr("Loop"));

	main_tw->addTab(tab_help, tr("Help"));

	/* <File> Tab */

	lbl_score_Afile = new QLabel(tr("- A-Scores -"));

	lbl_score_Bfile = new QLabel(tr("- B-Scores -"));

	lbl_import_file = new QLabel(tr("- Lyrics -"));

	lbl_midi_file = new QLabel(tr("- MIDI File -"));

	but_quit = new QPushButton(tr("Quit"));

	but_midi_file_new = new QPushButton(tr("New"));
	but_midi_file_open = new QPushButton(tr("Open"));
	but_midi_file_merge = new QPushButton(tr("Merge"));
	but_midi_file_save = new QPushButton(tr("Save"));
	but_midi_file_save_as = new QPushButton(tr("Save As"));
	but_midi_file_import = new QPushButton(tr("To X-Scores"));

	lbl_gpro_title = new QLabel(tr("- GPro v3/4 -"));
	but_gpro_file_import = new QPushButton(tr("Open In X-Scores"));

	n = 0;

	tab_file_gl->addWidget(lbl_score_Afile, n, 0, 1, 2, Qt::AlignHCenter|Qt::AlignVCenter);
	tab_file_gl->addWidget(lbl_score_Bfile, n, 2, 1, 2, Qt::AlignHCenter|Qt::AlignVCenter);
	tab_file_gl->addWidget(lbl_import_file, n, 4, 1, 2, Qt::AlignHCenter|Qt::AlignVCenter);

	n++;

	tab_file_gl->addWidget(scores_main[0]->butScoreFileNew, n, 0, 1, 2);
	tab_file_gl->addWidget(scores_main[1]->butScoreFileNew, n, 2, 1, 2);
	tab_file_gl->addWidget(tab_import->butImportFileNew, n, 4, 1, 2);

	n++;

	tab_file_gl->addWidget(scores_main[0]->butScoreFileOpen, n, 0, 1, 2);
	tab_file_gl->addWidget(scores_main[1]->butScoreFileOpen, n, 2, 1, 2);
	tab_file_gl->addWidget(tab_import->butImportFileOpen, n, 4, 1, 2);

	n++;

	tab_file_gl->addWidget(scores_main[0]->butScoreFileSave, n, 0, 1, 2);
	tab_file_gl->addWidget(scores_main[1]->butScoreFileSave, n, 2, 1, 2);
	tab_file_gl->addWidget(tab_import->butImport, n, 4, 1, 2);

	n++;

	tab_file_gl->addWidget(scores_main[0]->butScoreFileSaveAs, n, 0, 1, 2);
	tab_file_gl->addWidget(scores_main[1]->butScoreFileSaveAs, n, 2, 1, 2);
	tab_file_gl->addWidget(lbl_gpro_title, n, 4, 1, 2, Qt::AlignHCenter|Qt::AlignVCenter);

	n++;

	tab_file_gl->addWidget(scores_main[0]->butScoreFilePrint, n, 0, 1, 2);
	tab_file_gl->addWidget(scores_main[1]->butScoreFilePrint, n, 2, 1, 2);
	tab_file_gl->addWidget(but_gpro_file_import, n, 4, 1, 2);

	n++;

	tab_file_gl->addWidget(scores_main[0]->butScoreFileStepUp, n, 0, 1, 2);
	tab_file_gl->addWidget(scores_main[1]->butScoreFileStepUp, n, 2, 1, 2);

	n++;

	tab_file_gl->addWidget(scores_main[0]->butScoreFileStepDown, n, 0, 1, 2);
	tab_file_gl->addWidget(scores_main[1]->butScoreFileStepDown, n, 2, 1, 2);

	n++;

	tab_file_gl->addWidget(scores_main[0]->butScoreFileSetFlat, n, 0, 1, 1);
	tab_file_gl->addWidget(scores_main[1]->butScoreFileSetFlat, n, 2, 1, 1);

	tab_file_gl->addWidget(scores_main[0]->butScoreFileSetSharp, n, 1, 1, 1);
	tab_file_gl->addWidget(scores_main[1]->butScoreFileSetSharp, n, 3, 1, 1);

	n++;

	tab_file_gl->addWidget(scores_main[0]->butScoreFileExport, n, 0, 1, 2);
	tab_file_gl->addWidget(scores_main[1]->butScoreFileExport, n, 2, 1, 2);

	n++;
	n++;

	tab_file_gl->setRowStretch(n, 3);

	n++;

	tab_file_gl->addWidget(but_quit, n, 0, 1, 8);

	n = 0;

	tab_file_gl->addWidget(lbl_midi_file, n, 6, 1, 2, Qt::AlignHCenter|Qt::AlignVCenter);

	n++;

	tab_file_gl->addWidget(but_midi_file_new, n, 6, 1, 2);

	n++;

	tab_file_gl->addWidget(but_midi_file_open, n, 6, 1, 2);

	n++;

	tab_file_gl->addWidget(but_midi_file_merge, n, 6, 1, 2);

	n++;

	tab_file_gl->addWidget(but_midi_file_save, n, 6, 1, 2);

	n++;

	tab_file_gl->addWidget(but_midi_file_save_as, n, 6, 1, 2);

	n++;

	tab_file_gl->addWidget(but_midi_file_import, n, 6, 1, 2);

	n++;

	/* <Play> Tab */

	but_bpm = new QPushButton(tr("BPM"));
	connect(but_bpm, SIGNAL(released()), this, SLOT(handle_bpm()));

	dlg_bpm = new MppBpm(this);

	lbl_bpm_max = new QLabel(tr("Max"));
	lbl_bpm_min = new QLabel(tr("Min"));
	lbl_bpm_avg = new QLabel(tr("Average BPM"));

	lbl_curr_time_val = new QLCDNumber(8);
	lbl_curr_time_val->setMode(QLCDNumber::Dec);
	lbl_curr_time_val->setFrameShape(QLCDNumber::NoFrame);
	lbl_curr_time_val->setSegmentStyle(QLCDNumber::Flat);
	lbl_curr_time_val->setAutoFillBackground(1);

	lbl_bpm_min_val = new QLCDNumber(4);
	lbl_bpm_min_val->setMode(QLCDNumber::Dec);
	lbl_bpm_min_val->setFrameShape(QLCDNumber::NoFrame);
	lbl_bpm_min_val->setSegmentStyle(QLCDNumber::Flat);
	lbl_bpm_min_val->setAutoFillBackground(1);

	lbl_bpm_avg_val = new QLCDNumber(4);
	lbl_bpm_avg_val->setMode(QLCDNumber::Dec);
	lbl_bpm_avg_val->setFrameShape(QLCDNumber::NoFrame);
	lbl_bpm_avg_val->setSegmentStyle(QLCDNumber::Flat);
	lbl_bpm_avg_val->setAutoFillBackground(1);

	lbl_bpm_max_val = new QLCDNumber(4);
	lbl_bpm_max_val->setMode(QLCDNumber::Dec);
	lbl_bpm_max_val->setFrameShape(QLCDNumber::NoFrame);
	lbl_bpm_max_val->setSegmentStyle(QLCDNumber::Flat);
	lbl_bpm_max_val->setAutoFillBackground(1);

	lbl_score_record = new QLabel(QString());
	lbl_midi_record = new QLabel(QString());
	lbl_midi_play = new QLabel(QString());

	for (n = 0; n != MPP_MAX_VIEWS; n++) {
		dlg_mode[n] = new MppMode(n);
		but_mode[n] = new MppButton(tr("View ") + QChar('A' + n) + tr(" mode"), n);
		connect(but_mode[n], SIGNAL(released(int)), this, SLOT(handle_mode(int)));
	}

	for (n = 0; n != MPP_MAX_LBUTTON; n++) {
		char buf[8];
		snprintf(buf, sizeof(buf), "J%u", n);
		but_jump[n] = new MppButton(tr(buf), n);
	}

	but_insert_chord = new QPushButton(tr("&Insert Chord"));
	but_edit_chord = new QPushButton(tr("&Edit Chord"));
	but_compile = new QPushButton(tr("Compile"));
	but_score_record = new QPushButton(tr("Scores"));
	but_midi_record = new QPushButton(tr("MIDI"));

	but_midi_play = new QPushButton(tr("MIDI"));
	but_midi_pause = new QPushButton(tr("Pause"));
	but_midi_trigger = new QPushButton(tr("Trigger"));
	but_midi_rewind = new QPushButton(tr("Rewind"));

	but_play = new QPushButton(tr("Shift+Play"));
	but_play->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);

	lbl_play_key = new QLabel(tr("Play Key"));
	spn_play_key = new MppSpinBox();
	connect(spn_play_key, SIGNAL(valueChanged(int)), this, SLOT(handle_play_key_changed(int)));
	spn_play_key->setRange(0, 127);
	spn_play_key->setValue(C4);

	lbl_time_counter = new QLabel(tr(" - Time Counter -"));
	lbl_synth = new QLabel(tr("- Synth Play -"));
	lbl_playback = new QLabel(tr("- Playback -"));
	lbl_recording = new QLabel(tr("- Recording -"));
	lbl_scores = new QLabel(tr("- Scores -"));

	lbl_key_mode = new QLabel(QString());
	but_key_mode = new QPushButton(QString());
	connect(but_key_mode, SIGNAL(released()), this, SLOT(handle_key_mode()));

	n = 0;

	tab_play_gl->addWidget(lbl_time_counter, n, 0, 1, 4, Qt::AlignHCenter|Qt::AlignVCenter);

	n++;

	tab_play_gl->addWidget(lbl_curr_time_val, n, 0, 1, 4, Qt::AlignHCenter|Qt::AlignVCenter);

	n++;

	tab_play_gl->addWidget(lbl_playback, n, 0, 1, 4, Qt::AlignHCenter|Qt::AlignVCenter);

	n++;

	tab_play_gl->addWidget(lbl_midi_play, n, 3, 1, 1);
	tab_play_gl->addWidget(but_midi_play, n, 0, 1, 3);

	n++;

	tab_play_gl->addWidget(but_midi_pause, n, 0, 1, 4);

	n++;

	tab_play_gl->addWidget(but_midi_trigger, n, 0, 1, 4);

	n++;

	tab_play_gl->addWidget(but_midi_rewind, n, 0, 1, 4);

	n++;

	tab_play_gl->addWidget(lbl_scores, n, 0, 1, 4, Qt::AlignHCenter|Qt::AlignVCenter);

	n++;

	tab_play_gl->addWidget(but_compile, n, 0, 1, 4);

	n++;

	tab_play_gl->addWidget(lbl_recording, n, 0, 1, 4, Qt::AlignHCenter|Qt::AlignVCenter);

	n++;

	tab_play_gl->addWidget(lbl_score_record, n, 3, 1, 1);
	tab_play_gl->addWidget(but_score_record, n, 0, 1, 3);

	n++;

	tab_play_gl->addWidget(lbl_midi_record, n, 3, 1, 1);
	tab_play_gl->addWidget(but_midi_record, n, 0, 1, 3);

	n++;

	tab_play_gl->addWidget(lbl_key_mode, n, 3, 1, 1);
	tab_play_gl->addWidget(but_key_mode, n, 0, 1, 3);

	n = 0;

	tab_play_gl->addWidget(lbl_synth, n, 4, 1, 4, Qt::AlignHCenter|Qt::AlignVCenter);

	n++;

	tab_play_gl->addWidget(lbl_play_key, n, 4, 1, 1);
	tab_play_gl->addWidget(spn_play_key, n, 5, 1, 1);

	n++;

	tab_play_gl->addWidget(but_bpm, n, 4, 1, 2);

	for (x = 0; x != MPP_MAX_VIEWS; x++)
		tab_play_gl->addWidget(but_mode[x], n + x, 6, 1, 2);

	n++;

	tab_play_gl->addWidget(but_insert_chord, n, 4, 1, 2);

	n++;

	tab_play_gl->addWidget(but_edit_chord, n, 6, 1, 2);

	for (x = 0; x != (MPP_MAX_LBUTTON / 2); x++) {
		tab_play_gl->addWidget(but_jump[x + (MPP_MAX_LBUTTON / 2)], n + x, 5, 1, 1);
		tab_play_gl->addWidget(but_jump[x], n + x, 4, 1, 1);
	}

	n += x;

	tab_play_gl->addWidget(lbl_bpm_max, n, 4, 1, 1, Qt::AlignHCenter|Qt::AlignVCenter);
	tab_play_gl->addWidget(lbl_bpm_avg, n, 5, 1, 2, Qt::AlignHCenter|Qt::AlignVCenter);
	tab_play_gl->addWidget(lbl_bpm_min, n, 7, 1, 1, Qt::AlignHCenter|Qt::AlignVCenter);

	n++;

	tab_play_gl->addWidget(lbl_bpm_max_val, n, 4, 1, 1, Qt::AlignHCenter|Qt::AlignVCenter);
	tab_play_gl->addWidget(lbl_bpm_avg_val, n, 5, 1, 2, Qt::AlignHCenter|Qt::AlignVCenter);
	tab_play_gl->addWidget(lbl_bpm_min_val, n, 7, 1, 1, Qt::AlignHCenter|Qt::AlignVCenter);

	n++;

	tab_play_gl->addWidget(but_play, n, 4, 3, 4);

	n += 3;

	tab_play_gl->setRowStretch(n, 4);

	/* <Configuration> Tab */

	lbl_config_title = new QLabel(tr("- Device configuration -"));
	lbl_config_play = new QLabel(tr("Play"));
	lbl_config_rec = new QLabel(tr("Rec."));
	lbl_config_synth = new QLabel(tr("Synth"));
	lbl_config_mm = new QLabel(tr("MuteMap"));
	lbl_config_dv = new QLabel(tr("DevSel"));
	lbl_bpm_count = new QLabel(tr("BPM average length (0..32)"));

	spn_bpm_length = new QSpinBox();
	spn_bpm_length->setRange(0, MPP_MAX_BPM);
	spn_bpm_length->setValue(0);

	lbl_config_local = new QLabel(tr("Enable local MIDI on synth"));
	cbx_config_local = new QCheckBox();
	connect(cbx_config_local, SIGNAL(stateChanged(int)), this, SLOT(handle_config_local_changed(int)));
	cbx_config_local->setCheckState(Qt::Checked);

	but_config_apply = new QPushButton(tr("Apply"));
	but_config_revert = new QPushButton(tr("Revert"));
	but_config_fontsel = new QPushButton(tr("Select Font"));

	lbl_config_insert = new QLabel(tr("Scores record prefix string"));
	led_config_insert = new QLineEdit(QString());

	x = 0;

	tab_config_gl->addWidget(lbl_config_title, x, 0, 1, 3, Qt::AlignHCenter|Qt::AlignVCenter);
	tab_config_gl->addWidget(lbl_config_play, x, 3, 1, 1, Qt::AlignHCenter|Qt::AlignVCenter);
	tab_config_gl->addWidget(lbl_config_rec, x, 4, 1, 1, Qt::AlignHCenter|Qt::AlignVCenter);
	tab_config_gl->addWidget(lbl_config_synth, x, 5, 1, 1, Qt::AlignHCenter|Qt::AlignVCenter);
	tab_config_gl->addWidget(lbl_config_mm, x, 6, 1, 1, Qt::AlignHCenter|Qt::AlignVCenter);
	tab_config_gl->addWidget(lbl_config_dv, x, 7, 1, 1, Qt::AlignHCenter|Qt::AlignVCenter);

	x++;

	for (n = 0; n != MPP_MAX_DEVS; n++) {
		char buf[16];

		but_config_mm[n] = new MppButton(tr("MM"), n);
		but_config_dev[n] = new MppButton(tr("DEV"), n);

		connect(but_config_mm[n], SIGNAL(released(int)), this, SLOT(handle_mute_map(int)));
		connect(but_config_dev[n], SIGNAL(released(int)), this, SLOT(handle_config_dev(int)));

		led_config_dev[n] = new QLineEdit(QString());

		cbx_config_dev[(3*n)+0] = new QCheckBox();
		cbx_config_dev[(3*n)+1] = new QCheckBox();
		cbx_config_dev[(3*n)+2] = new QCheckBox();

		snprintf(buf, sizeof(buf), "Dev%d:", n);

		lbl_config_dev[n] = new QLabel(tr(buf));

		tab_config_gl->addWidget(lbl_config_dev[n], x, 0, 1, 1, Qt::AlignHCenter|Qt::AlignLeft);
		tab_config_gl->addWidget(led_config_dev[n], x, 1, 1, 2, Qt::AlignHCenter|Qt::AlignLeft);

		tab_config_gl->addWidget(cbx_config_dev[(3*n)+0], x, 3, 1, 1, Qt::AlignHCenter|Qt::AlignVCenter);
		tab_config_gl->addWidget(cbx_config_dev[(3*n)+1], x, 4, 1, 1, Qt::AlignHCenter|Qt::AlignVCenter);
		tab_config_gl->addWidget(cbx_config_dev[(3*n)+2], x, 5, 1, 1, Qt::AlignHCenter|Qt::AlignVCenter);
		tab_config_gl->addWidget(but_config_mm[n], x, 6, 1, 1, Qt::AlignHCenter|Qt::AlignVCenter);
		tab_config_gl->addWidget(but_config_dev[n], x, 7, 1, 1, Qt::AlignHCenter|Qt::AlignVCenter);

		x++;
	}

	tab_config_gl->addWidget(lbl_bpm_count, x, 0, 1, 6, Qt::AlignLeft|Qt::AlignVCenter);
	tab_config_gl->addWidget(spn_bpm_length, x, 6, 1, 2, Qt::AlignRight|Qt::AlignVCenter);

	x++;

	tab_config_gl->addWidget(lbl_config_local, x, 0, 1, 7, Qt::AlignLeft|Qt::AlignVCenter);
	tab_config_gl->addWidget(cbx_config_local, x, 7, 1, 1, Qt::AlignHCenter|Qt::AlignVCenter);

	x++;

	tab_config_gl->addWidget(lbl_config_insert, x, 0, 1, 4);
	tab_config_gl->addWidget(led_config_insert, x, 4, 1, 4);

	x++;

	tab_config_gl->setRowStretch(x, 1);

	x++;

	tab_config_gl->addWidget(but_config_fontsel, x, 0, 1, 2, Qt::AlignLeft|Qt::AlignVCenter);
	tab_config_gl->addWidget(but_config_apply, x, 4, 1, 2);
	tab_config_gl->addWidget(but_config_revert, x, 6, 1, 2);

	/* <Instrument> tab */

	lbl_instr_title[0] = new QLabel(tr("- Bank/Program/Mute -"));
	lbl_instr_title[1] = new QLabel(tr("- Bank/Program/Mute -"));

	lbl_instr_prog = new QLabel(tr("- Synth/Record Channel and Selected Bank/Program -"));

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

	x = 0;

	tab_instr_gl->addWidget(lbl_instr_prog, x, 0, 1, 8, Qt::AlignHCenter|Qt::AlignVCenter);

	x++;

	tab_instr_gl->addWidget(spn_instr_curr_chan, x, 0, 1, 1);
	tab_instr_gl->addWidget(spn_instr_curr_bank, x, 1, 1, 1);
	tab_instr_gl->addWidget(spn_instr_curr_prog, x, 2, 1, 1);
	tab_instr_gl->addWidget(but_instr_program, x, 3, 1, 3);
	tab_instr_gl->addWidget(but_instr_program_all, x, 6, 1, 2);

	x++;

	tab_instr_gl->addWidget(lbl_instr_title[0], x, 0, 1, 4, Qt::AlignHCenter|Qt::AlignVCenter);
	tab_instr_gl->addWidget(lbl_instr_title[1], x, 4, 1, 4, Qt::AlignHCenter|Qt::AlignVCenter);

	x++;

	for (n = 0; n != 16; n++) {
		int y_off = (n & 8) ? 4 : 0;

		char buf[16];

		snprintf(buf, sizeof(buf), "Ch%X", n);

		lbl_instr_desc[n] = new QLabel(tr(buf));

		spn_instr_bank[n] = new QSpinBox();
		spn_instr_bank[n]->setRange(0, 16383);
		connect(spn_instr_bank[n], SIGNAL(valueChanged(int)), this, SLOT(handle_instr_changed(int)));

		spn_instr_prog[n] = new QSpinBox();
		spn_instr_prog[n]->setRange(0, 127);
		connect(spn_instr_prog[n], SIGNAL(valueChanged(int)), this, SLOT(handle_instr_changed(int)));

		cbx_instr_mute[n] = new QCheckBox();
		connect(cbx_instr_mute[n], SIGNAL(stateChanged(int)), this, SLOT(handle_instr_changed(int)));

		tab_instr_gl->addWidget(lbl_instr_desc[n], (n & 7) + x, 0 + y_off, 1, 1, Qt::AlignVCenter|Qt::AlignRight);
		tab_instr_gl->addWidget(spn_instr_bank[n], (n & 7) + x, 1 + y_off, 1, 1, Qt::AlignVCenter|Qt::AlignRight);
		tab_instr_gl->addWidget(spn_instr_prog[n], (n & 7) + x, 2 + y_off, 1, 1, Qt::AlignVCenter|Qt::AlignHCenter);
		tab_instr_gl->addWidget(cbx_instr_mute[n], (n & 7) + x, 3 + y_off, 1, 1, Qt::AlignVCenter|Qt::AlignHCenter);
	}

	x += 8;

	tab_instr_gl->setRowStretch(x, 3);

	x++;

	tab_instr_gl->addWidget(but_instr_mute_all, x, 0, 1, 2);
	tab_instr_gl->addWidget(but_instr_unmute_all, x, 2, 1, 2);
 	tab_instr_gl->addWidget(but_instr_rem, x, 4, 1, 2);
	tab_instr_gl->addWidget(but_instr_reset, x, 6, 1, 2);

	/* <Volume> tab */

	lbl_volume_title[0] = new QLabel(tr("- Playback -"));
	lbl_volume_title[1] = new QLabel(tr("- Synth/Record -"));

	but_volume_reset = new QPushButton(tr("Reset"));

	x = 0;

	tab_volume_gl->addWidget(lbl_volume_title[0], x, 0, 1, 4, Qt::AlignHCenter|Qt::AlignVCenter);
	tab_volume_gl->addWidget(lbl_volume_title[1], x, 4, 1, 4, Qt::AlignHCenter|Qt::AlignVCenter);

	x++;

	for (n = 0; n != 16; n++) {
		int y_off = (n & 8) ? 2 : 0;

		char buf[16];

		snprintf(buf, sizeof(buf), "Ch%X", n);

		lbl_volume_play[n] = new QLabel(tr(buf));
		lbl_volume_synth[n] = new QLabel(tr(buf));

		spn_volume_synth[n] = new QSpinBox();
		spn_volume_synth[n]->setRange(0, 511);
		connect(spn_volume_synth[n], SIGNAL(valueChanged(int)), this, SLOT(handle_volume_changed(int)));

		spn_volume_play[n] = new QSpinBox();
		spn_volume_play[n]->setRange(0, 511);
		connect(spn_volume_play[n], SIGNAL(valueChanged(int)), this, SLOT(handle_volume_changed(int)));

		tab_volume_gl->addWidget(lbl_volume_play[n], (n & 7) + x, 0 + y_off, 1, 1, Qt::AlignVCenter|Qt::AlignRight);
		tab_volume_gl->addWidget(spn_volume_play[n], (n & 7) + x, 1 + y_off, 1, 1, Qt::AlignVCenter|Qt::AlignHCenter);
		tab_volume_gl->addWidget(lbl_volume_synth[n], (n & 7) + x, 4 + y_off, 1, 1, Qt::AlignVCenter|Qt::AlignRight);
		tab_volume_gl->addWidget(spn_volume_synth[n], (n & 7) + x, 5 + y_off, 1, 1, Qt::AlignVCenter|Qt::AlignHCenter);
	}

	x += 8;

	tab_volume_gl->setRowStretch(x, 4);

	x++;

	tab_volume_gl->addWidget(but_volume_reset, x, 6, 1, 2);

	/* Connect all */

	connect(but_insert_chord, SIGNAL(released()), this, SLOT(handle_insert_chord()));
	connect(but_edit_chord, SIGNAL(released()), this, SLOT(handle_edit_chord()));

	for (n = 0; n != MPP_MAX_LBUTTON; n++)
		connect(but_jump[n], SIGNAL(pressed(int)), this, SLOT(handle_jump(int)));

	connect(but_compile, SIGNAL(pressed()), this, SLOT(handle_compile()));
	connect(but_score_record, SIGNAL(pressed()), this, SLOT(handle_score_record()));
	connect(but_midi_record, SIGNAL(pressed()), this, SLOT(handle_midi_record()));
	connect(but_midi_play, SIGNAL(pressed()), this, SLOT(handle_midi_play()));
	connect(but_play, SIGNAL(pressed()), this, SLOT(handle_play_press()));
	connect(but_play, SIGNAL(released()), this, SLOT(handle_play_release()));
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
	connect(but_config_fontsel, SIGNAL(pressed()), this, SLOT(handle_config_fontsel()));

	connect(but_instr_rem, SIGNAL(pressed()), this, SLOT(handle_instr_rem()));
	connect(but_instr_program, SIGNAL(pressed()), this, SLOT(handle_instr_program()));
	connect(but_instr_program_all, SIGNAL(pressed()), this, SLOT(handle_instr_program_all()));
	connect(but_instr_reset, SIGNAL(pressed()), this, SLOT(handle_instr_reset()));
	connect(but_instr_mute_all, SIGNAL(pressed()), this, SLOT(handle_instr_mute_all()));
	connect(but_instr_unmute_all, SIGNAL(pressed()), this, SLOT(handle_instr_unmute_all()));

	connect(but_volume_reset, SIGNAL(pressed()), this, SLOT(handle_volume_reset()));

	connect(but_midi_pause, SIGNAL(pressed()), this, SLOT(handle_midi_pause()));

	connect(scores_tw, SIGNAL(currentChanged(int)), this, SLOT(handle_tab_changed(int)));

	MidiInit();

	sync_key_mode();

	version = tr("MIDI Player Pro v1.0.10");

	setWindowTitle(version);
	setWindowIcon(QIcon(QString(MPP_ICON_FILE)));

	watchdog->start(250);
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
	MppDecode dlg(this, this, 0);

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
MppMainWindow :: handle_jump(int index)
{
	pthread_mutex_lock(&mtx);
	handle_jump_locked(index);
	pthread_mutex_unlock(&mtx);
}

void
MppMainWindow :: handle_config_local_changed(int state)
{
	pthread_mutex_lock(&mtx);
	synthIsLocal = (state == Qt::Checked);
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
MppMainWindow :: handle_compile()
{
	int x;

	pthread_mutex_lock(&mtx);
	handle_stop();
	pthread_mutex_unlock(&mtx);

	for (x = 0; x != MPP_MAX_VIEWS; x++)
		scores_main[x]->handleCompile();
}

void
MppMainWindow :: handle_score_record()
{
	pthread_mutex_lock(&mtx);
	scoreRecordOff = !scoreRecordOff;
	pthread_mutex_unlock(&mtx);

	if (scoreRecordOff == 0)
		lbl_score_record->setText(tr("ON"));
	else
		lbl_score_record->setText(tr("OFF"));
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
MppMainWindow :: handle_midi_play()
{
	uint8_t triggered;

	pthread_mutex_lock(&mtx);
	midiPlayOff = !midiPlayOff;
	triggered = midiTriggered;
	update_play_device_no();
	pthread_mutex_unlock(&mtx);

	if (midiPlayOff == 0)
		lbl_midi_play->setText(tr("ON"));
	else
		lbl_midi_play->setText(tr("OFF"));

	handle_midi_pause();

	if (triggered)
		handle_midi_trigger();
}

void
MppMainWindow :: handle_midi_record()
{
	uint8_t triggered;

	pthread_mutex_lock(&mtx);
	midiRecordOff = !midiRecordOff;
	triggered = midiTriggered;
	update_play_device_no();
	pthread_mutex_unlock(&mtx);

	if (midiRecordOff == 0)
		lbl_midi_record->setText(tr("ON"));
	else
		lbl_midi_record->setText(tr("OFF"));

	handle_midi_pause();

	if (triggered)
		handle_midi_trigger();
}

void
MppMainWindow :: handle_play_press()
{
	MppScoreMain *sm = currScoreMain();

	pthread_mutex_lock(&mtx);
	if (tab_loop->handle_trigN(playKey, 90)) {
		/* ignore */
	} else if (sm->keyMode != MM_PASS_ALL) {
		sm->handleKeyPress(playKey, 90);
	} else {
		output_key(sm->synthChannel, playKey, 90, 0, 0);
	}
	pthread_mutex_unlock(&mtx);
}

void
MppMainWindow :: handle_play_release()
{
	MppScoreMain *sm = currScoreMain();

	pthread_mutex_lock(&mtx);
	if (sm->keyMode != MM_PASS_ALL) {
		sm->handleKeyRelease(playKey);
	} else {
		output_key(sm->synthChannel, playKey, 0, 0, 0);
	}
	pthread_mutex_unlock(&mtx);
}

void
MppMainWindow :: handle_watchdog()
{
	MppScoreMain *sm = currScoreMain();
	QTextCursor cursor(sm->editWidget->textCursor());
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
	uint32_t play_line;
	uint32_t play_pos;

	pthread_mutex_lock(&mtx);
	cursor_update = cursorUpdate;
	cursorUpdate = 0;
	instr_update = instrUpdated;
	instrUpdated = 0;
	num_events = numInputEvents;
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

	if (cursor_update) {
		cursor.movePosition(QTextCursor::Start, QTextCursor::MoveAnchor, 1);
		cursor.movePosition(QTextCursor::Down, QTextCursor::MoveAnchor, play_line);
		cursor.movePosition(QTextCursor::StartOfLine, QTextCursor::MoveAnchor, 1);
		sm->editWidget->setTextCursor(cursor);
		sm->watchdog();
	}

	sm->viewWidgetSub->repaint();

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
				box.exec();
			}
		} else {
			QMessageBox box;

			box.setText(tr("Could not get MIDI data!"));
			box.setStandardButtons(QMessageBox::Ok);
			box.setIcon(QMessageBox::Information);
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
	uint8_t ScMidiTriggered;
	char *p_play;
	char *p_rec;
	int n;
	int x;
	int y;

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

	pthread_mutex_lock(&mtx);
	ScMidiTriggered = midiTriggered;
	midiTriggered = 1;

	for (y = 0; y != MPP_MAX_DEVS; y++) {
		/* set local on all channels */
		for (x = 0; x != 16; x++) {
			if (check_synth(y, x, 0)) {
				uint8_t buf[4];
				buf[0] = 0xB0 | x;
				buf[1] = 0x7A;
				buf[2] = synthIsLocal ? 0x7F : 0x00;
				mid_add_raw(&mid_data, buf, 3, x);
			}
		}
	}

	midiTriggered = ScMidiTriggered;
	pthread_mutex_unlock(&mtx);
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

		cbx_config_dev[(3*n)+0]->setChecked(
		    (deviceBits & (1UL << ((3*n)+0))) ? 1 : 0);

		cbx_config_dev[(3*n)+1]->setChecked(
		    (deviceBits & (1UL << ((3*n)+1))) ? 1 : 0);

		cbx_config_dev[(3*n)+2]->setChecked(
		    (deviceBits & (1UL << ((3*n)+2))) ? 1 : 0);
	}

	spn_bpm_length->setValue(bpmAvgLength);
}

void
MppMainWindow :: handle_config_apply()
{
	int n;

	deviceBits = 0;

	for (n = 0; n != MPP_MAX_DEVS; n++) {

		if (deviceName[n] != NULL)
			free(deviceName[n]);

		deviceName[n] = MppQStringToAscii(led_config_dev[n]->text());

		if (cbx_config_dev[(3*n)+0]->isChecked())
			deviceBits |= 1UL << ((3*n)+0);
		if (cbx_config_dev[(3*n)+1]->isChecked())
			deviceBits |= 1UL << ((3*n)+1);
		if (cbx_config_dev[(3*n)+2]->isChecked())
			deviceBits |= 1UL << ((3*n)+2);
	}

	n = spn_bpm_length->value();

	pthread_mutex_lock(&mtx);
	bpmAvgLength = n;
	pthread_mutex_unlock(&mtx);

	handle_config_reload();
}

void
MppMainWindow :: handle_config_fontsel()
{
	bool success;
	int x;

	QFont font = QFontDialog::getFont(&success, defaultFont, this);

	if (success) {
		defaultFont = font;

		for (x = 0; x != MPP_MAX_VIEWS; x++)
			scores_main[x]->handleCompile();
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
		MidiEventRxControl(scores_main[z], 0x40, 0);
		MidiEventRxControl(scores_main[z], 0x01, 0);
		MidiEventRxPitch(scores_main[z], 1U << 13);
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

	pthread_mutex_lock(&mtx);
	time_offset = get_time_offset();
	pthread_mutex_unlock(&mtx);

	lbl_curr_time_val->display((int)time_offset);
}

void
MppMainWindow :: do_bpm_stats(void)
{
	uint32_t min = 0xFFFFFFFFUL;
	uint32_t max = 0;
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
		if (val > max)
			max = val;
		if (val < min)
			min = val;
	}

	pthread_mutex_unlock(&mtx);

	if (sum == 0)
		sum = 1;
	if (max == 0)
		max = 1;
	if (min == 0)
		min = 1;

	sum = (len * UMIDI20_BPM) / sum;
	max = UMIDI20_BPM / max;
	min = UMIDI20_BPM / min;

	if (sum > 9999)
		sum = 9999;
	if (min > 9999)
		min = 9999;
	if (max > 9999)
		max = 9999;

	lbl_bpm_max_val->display((int)min);
	lbl_bpm_min_val->display((int)max);
	lbl_bpm_avg_val->display((int)sum);
}

static void
MidiEventRxControl(MppScoreMain *sm, uint8_t ctrl, uint8_t val)
{
	MppMainWindow *mw = sm->mainWindow;
	struct mid_data *d = &mw->mid_data;
	uint16_t mask;
	uint8_t x;
	uint8_t chan;

	chan = sm->synthChannel;

	if (sm->keyMode == MM_PASS_ALL)
		mask = 1;
	else
		mask = sm->active_channels;

	/* the control event is distributed to all active channels */
	while (mask) {
		if (mask & 1) {
			for (x = 0; x != MPP_MAX_DEVS; x++) {
				if (mw->check_synth(x, chan, 0))
					mid_control(d, ctrl, val);
			}
			if (mw->check_record(chan, 0))
				mid_control(d, ctrl, val);
		}
		mask /= 2;
		chan++;
		chan &= 0xF;
	}

	if (ctrl == 0x40)
		mw->tab_loop->add_pedal(val);
}

static void
MidiEventRxPitch(MppScoreMain *sm, uint16_t val)
{
	MppMainWindow *mw = sm->mainWindow;
	struct mid_data *d = &mw->mid_data;
	uint16_t mask;
	uint8_t x;
	uint8_t chan;

	chan = sm->synthChannel;

	if (sm->keyMode == MM_PASS_ALL)
		mask = 1;
	else
		mask = sm->active_channels;

	/* the control event is distributed to all active channels */
	while (mask) {
		if (mask & 1) {
			for (x = 0; x != MPP_MAX_DEVS; x++) {
				if (mw->check_synth(x, chan, 0))
					mid_pitch_bend(d, val);
			}
			if (mw->check_record(chan, 0))
				mid_pitch_bend(d, val);
		}
		mask /= 2;
		chan++;
		chan &= 0xF;
	}
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

			MidiEventRxPitch(sm, vel);

		} else if (ctrl == 0x40 || ctrl == 0x01) {

			vel = umidi20_event_get_control_value(event);

			MidiEventRxControl(sm, ctrl, vel);

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
			if (check_synth(y, x, 0)) {
				mid_delay(d, (4 * x));
				mid_set_bank_program(d, x, 
				    instr[x].bank,
				    instr[x].prog);
			}
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
MppMainWindow :: handle_tab_changed(int index)
{
	QWidget *pw;
	int x;
	int compile = 0;

	pw = scores_tw->widget(index);

	for (x = 0; x != MPP_MAX_VIEWS; x++) {
		if (pw == &scores_main[x]->viewWidget) {
			compile = 1;
			break;
		}
		if (pw == scores_main[x]->editWidget)
			break;
	}

	if (x == MPP_MAX_VIEWS) {
		currViewIndex = 0;
		setWindowTitle(version);
	} else {
		currViewIndex = x;
		QString *ps = scores_main[x]->currScoreFileName;
		if (ps != NULL)
			setWindowTitle(version + " - " + MppBaseName(*ps));
		else
			setWindowTitle(version);
	}

	handle_instr_channel_changed(currScoreMain()->synthChannel);

	if (compile)
		handle_compile();

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

	handle_midi_record();
	handle_midi_play();
	handle_score_record();
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

MppPlayWidget :: MppPlayWidget(MppMainWindow *parent)
  : QWidget(parent)
{
	mw = parent;
}

MppPlayWidget :: ~MppPlayWidget()
{
}

void
MppPlayWidget :: keyPressEvent(QKeyEvent *event)
{
	/* fake pedal down event */
	switch (event->key()) {
	case Qt::Key_Shift:
		pthread_mutex_lock(&mw->mtx);
		if (mw->midiTriggered != 0)
			MidiEventRxControl(mw->currScoreMain(), 0x40, 127);
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
MppPlayWidget :: keyReleaseEvent(QKeyEvent *event)
{
	/* fake pedal up event */
	if (event->key() == Qt::Key_Shift) {
		pthread_mutex_lock(&mw->mtx);
		if (mw->midiTriggered != 0)
			MidiEventRxControl(mw->currScoreMain(), 0x40, 0);
		pthread_mutex_unlock(&mw->mtx);
	}
}

void
MppMainWindow :: handle_mute_map(int n)
{
	MppMuteMap diag(this, this, n);
	diag.exec();
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
	dlg_mode[index]->exec();

	pthread_mutex_lock(&mtx);
	scores_main[index]->baseKey = dlg_mode[index]->base_key;
	scores_main[index]->cmdKey = dlg_mode[index]->cmd_key;
	scores_main[index]->delayNoise = dlg_mode[index]->key_delay;
	scores_main[index]->devInputMask = dlg_mode[index]->input_mask;
	scores_main[index]->keyMode = dlg_mode[index]->key_mode;
	scores_main[index]->synthChannel = dlg_mode[index]->channel;
	pthread_mutex_unlock(&mtx);

	sync_key_mode();
}

void
MppMainWindow :: handle_key_mode()
{
	MppScoreMain *sm = currScoreMain();
	MppMode *cm = currModeDlg();

	cm->handle_mode();

	pthread_mutex_lock(&mtx);
	sm->keyMode = cm->key_mode;
	pthread_mutex_unlock(&mtx);

	sync_key_mode();
}

void
MppMainWindow :: sync_key_mode()
{
	lbl_key_mode->setText(currModeDlg()->lbl_mode->text());
	but_key_mode->setText(tr("Key Mode ") + QChar('A' + currViewIndex));
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
