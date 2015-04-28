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

#ifndef _MIDIPP_MAINWINDOW_H_
#define	_MIDIPP_MAINWINDOW_H_

#include "midipp.h"

class MppPlayGridLayout : public QWidget, public QGridLayout
{
	MppMainWindow *mw;

public:
	MppPlayGridLayout(MppMainWindow *parent = 0);
	~MppPlayGridLayout();


protected:
	void keyPressEvent(QKeyEvent *event);
	void keyReleaseEvent(QKeyEvent *event);
};

class MppMainWindow : public QWidget
{
	Q_OBJECT;

public:
	MppMainWindow(QWidget *parent = 0);
	~MppMainWindow();

#ifdef HAVE_SCREENSHOT
	void ScreenShot(QApplication &);
#endif
	void MidiInit(void);
	void MidiUnInit(void);

	void closeEvent(QCloseEvent *event);
	void handle_stop(int flag = 0);
	void handle_midi_file_open(int);
	void handle_midi_file_clear_name(void);
	void handle_midi_file_instr_prepend(void);
	void handle_midi_file_instr_delete(void);
	void handle_jump_locked(int index);
	void handle_make_scores_visible(MppScoreMain *);
	void handle_make_tab_visible(QWidget *);

	QString get_midi_score_duration(uint32_t *psum = 0);
	int log_midi_score_duration();
	int convert_midi_duration(struct umidi20_track *, uint32_t thres, uint32_t chan_mask);
	void import_midi_track(struct umidi20_track *, uint32_t = 0, int = -1, int = 0);

	void update_play_device_no(void);

	void do_clock_stats(void);
	void do_key_press(int key, int vel, int dur);
	void output_key(int chan, int key, int vel, int delay, int dur);

	uint32_t get_time_offset(void);

	uint8_t noise8(uint8_t factor);
	uint8_t do_instr_check(struct umidi20_event *event, uint8_t *pchan);
	uint8_t check_play(uint8_t chan, uint32_t off);
	uint8_t check_record(uint8_t chan, uint32_t off);
	uint8_t check_synth(uint8_t device_no, uint8_t chan, uint32_t off);

	void handle_watchdog_sub(MppScoreMain *, int);

	void send_song_stop_locked();
	void send_song_trigger_locked();
	void send_song_select_locked(uint8_t);

	void send_byte_event_locked(uint8_t);

	QPlainTextEdit *currEditor();
	MppScoreMain *currScores();

	pthread_mutex_t mtx;

	QFont defaultFont;
	QFont editFont;
	QFont showFont;

	QString version;

	uint8_t auto_zero_start[0];

	struct MppInstr instr[16];

	uint32_t convLineStart[MPP_MAX_LINES];
	uint32_t convLineEnd[MPP_MAX_LINES];
	uint32_t convIndex;

	uint32_t lastKeyPress;
	uint32_t lastInputEvent;
	uint32_t noiseRem;

	uint32_t devInputMask[MPP_MAX_DEVS];
	uint32_t chanUsageMask;
	uint32_t startPosition;
	uint32_t pausePosition;
	uint32_t deviceBits;
#define	MPP_DEV0_PLAY	0x0001UL
#define	MPP_DEV0_RECORD	0x0002UL
#define	MPP_DEV0_SYNTH	0x0004UL
#define	MPP_DEV1_PLAY	0x0008UL
#define	MPP_DEV1_RECORD	0x0010UL
#define	MPP_DEV1_SYNTH	0x0020UL
#define	MPP_DEV2_PLAY	0x0040UL
#define	MPP_DEV2_RECORD	0x0080UL
#define	MPP_DEV2_SYNTH	0x0100UL
#define	MPP_DEV3_PLAY	0x0200UL
#define	MPP_DEV3_RECORD	0x0400UL
#define	MPP_DEV3_SYNTH	0x0800UL

	uint16_t playVolume[16];
	uint16_t synthVolume[16];

	uint8_t muteProgram[MPP_MAX_DEVS];
	uint8_t mutePedal[MPP_MAX_DEVS];
	uint8_t enableLocalKeys[MPP_MAX_DEVS];
	uint8_t disableLocalKeys[MPP_MAX_DEVS];
	uint8_t muteAllControl[MPP_MAX_DEVS];
	uint8_t muteAllNonChannel[MPP_MAX_DEVS];
	uint8_t muteMap[MPP_MAX_DEVS][16];
	uint8_t inputEvents[MPP_MAX_QUEUE];
	uint8_t numInputEvents;
	uint8_t controlEvents[MPP_MAX_QUEUE][4];
	uint8_t numControlEvents;
	uint8_t cursorUpdate;

	uint8_t nonChannelMuted;
	uint8_t scoreRecordOff;
	uint8_t controlRecordOn;
	uint8_t instrUpdated;
	uint8_t midiRecordOff;
	uint8_t midiPlayOff;
	uint8_t midiTriggered;
	uint8_t midiPaused;
	uint8_t lastViewIndex;
	uint8_t keyModeUpdated;
	uint8_t doOperation;
#define	MPP_OPERATION_PAUSE 0x01
#define	MPP_OPERATION_REWIND 0x02
#define	MPP_OPERATION_BPM 0x04

	char *deviceName[MPP_MAX_DEVS];

	QGridLayout *main_gl;

	MppTabBar *main_tb;
	int main_tb_state;

	/* main <> */

	QPushButton *mwRight;
	QPushButton *mwLeft;
	QPushButton *mwRewind;
	QPushButton *mwPlay;
	QPushButton *mwReload;
	QPushButton *mwPaste;
	QPushButton *mwCopy;
	QPushButton *mwUndo;
	QPushButton *mwRedo;
	QPushButton *mwEdit;
	QPushButton *mwUpDown;

	/* tab <Scores> */

	MppScoreMain *scores_main[MPP_MAX_VIEWS];

	/* tab <Import> */

	MppImportTab *tab_import;

	QTimer *watchdog;

	/* tab <File> */

	MppGridLayout *tab_file_gl;

	QPushButton *but_quit;

	MppGroupBox *gb_midi_file;
	QPushButton *but_midi_file_new;
	QPushButton *but_midi_file_open;
	QPushButton *but_midi_file_merge;
	QPushButton *but_midi_file_save;
	QPushButton *but_midi_file_save_as;
	MppButton *but_midi_file_import[MPP_MAX_VIEWS];

	MppGroupBox *gb_gpro_file_import;
	MppButton *but_gpro_file_import[MPP_MAX_VIEWS];

	/* tab <Play> */

	MppGroupBox *gl_ctrl;
	MppGroupBox *gl_time;
	MppGroupBox *gl_bpm;
	MppGroupBox *gl_synth_play;

	QLCDNumber *lbl_curr_time_val;
	QLCDNumber *lbl_bpm_avg_val;

	MppPlayGridLayout *tab_play_gl;

	MppButtonMap *mbm_midi_play;
	MppButtonMap *mbm_midi_record;
	MppButtonMap *mbm_score_record;
	MppButtonMap *mbm_key_mode_a;
	MppButtonMap *mbm_key_mode_b;

	QPushButton *but_jump[MPP_MAX_LBUTTON];
	QPushButton *but_compile;
	QPushButton *but_midi_pause;
	QPushButton *but_midi_trigger;
	QPushButton *but_midi_rewind;
	QPushButton *but_bpm;

	MppButton *but_mode[MPP_MAX_VIEWS];
	MppMode *dlg_mode[MPP_MAX_VIEWS];

	MppBpm *dlg_bpm;

	/* tab <Chord> */

	MppDecodeTab *tab_chord_gl;

	/* tab <PianoTab> */

	MppPianoTab *tab_pianotab;

	/* tab <Configuration> */

	MppGridLayout *tab_config_gl;

	MppGroupBox *gb_config_device;

	MppSettings *mpp_settings;

	MppButton *but_config_dev[MPP_MAX_DEVS];
	MppButton *but_config_mm[MPP_MAX_DEVS];
	QLineEdit *led_config_dev[MPP_MAX_DEVS];
	MppCheckBox *cbx_config_dev[MPP_MAX_DEVS][3 + MPP_MAX_VIEWS];

	uint32_t dirty_config_mask;

	QTimer *tim_config_apply;
	QPushButton *but_config_view_fontsel;
	QPushButton *but_config_edit_fontsel;

	MppGroupBox *gb_config_insert;
	QLineEdit *led_config_insert;

	QString *CurrMidiFileName;

	/* tab <Shortcut> */

	MppShortcutTab *tab_shortcut;

	/* tab <Custom> */

	MppCustomTab *tab_custom;

	/* tab <Instrument> */

	MppGridLayout *tab_instr_gl;

	MppGroupBox *gb_instr_select;
	MppGroupBox *gb_instr_table;

	MppChanSel *spn_instr_curr_chan;
	QSpinBox *spn_instr_curr_bank;
	QSpinBox *spn_instr_curr_prog;
	QSpinBox *spn_instr_bank[16];
	QSpinBox *spn_instr_prog[16];

	MppCheckBox *cbx_instr_mute[16];

	MppButtonMap *but_non_channel_mute_all;

	QPushButton *but_instr_rem;
	QPushButton *but_instr_program;
	QPushButton *but_instr_program_all;
	QPushButton *but_instr_reset;
	QPushButton *but_instr_mute_all;
	QPushButton *but_instr_unmute_all;

	/* tab <Volume> */

	MppGridLayout *tab_volume_gl;

	MppGroupBox *gb_volume_play;
	MppGroupBox *gb_volume_synth;

	MppVolume *spn_volume_play[16];
	MppVolume *spn_volume_synth[16];

	QPushButton *but_volume_reset;

	/* tab <Loop> */

	MppLoopTab *tab_loop;

	/* tab <RePlay> */

	MppReplayTab *tab_replay;
	
	/* tab <DataBase> */

	MppDataBase *tab_database;

	/* tab <Show> */
#ifndef HAVE_NO_SHOW
	MppShowControl *tab_show_control;
#endif
	/* tab <Help> */

	QPlainTextEdit *tab_help;

	/* MIDI stuff */
	struct mid_data mid_data;
	struct umidi20_song *song;
	struct umidi20_track *track;

	uint8_t auto_zero_end[0];

public slots:
	void handle_quit();
	void handle_jump(int index);
	void handle_compile(int force = 0);
	void handle_score_record(int);
	void handle_midi_record(int);
	void handle_midi_pause();
	void handle_midi_play(int);
	void handle_play_press(int, int);
	void handle_play_release(int, int);
	void handle_sustain_press(int);
	void handle_sustain_release(int);
	void handle_watchdog();
	void handle_midi_file_new();
	void handle_midi_file_merge_open();
	void handle_midi_file_new_open();
	void handle_midi_file_save();
	void handle_midi_file_save_as();
	void handle_rewind();
	void handle_midi_trigger();
	void handle_config_changed();
	void handle_config_apply(int = -1);
	void handle_config_reload();
	void handle_config_view_fontsel();
	void handle_config_edit_fontsel();

	void handle_non_channel_muted_changed(int);
	void handle_instr_changed(int);
	void handle_instr_reset();
	void handle_instr_program();
	void handle_instr_program_all();
	void handle_instr_channel_changed(int);
	void handle_instr_rem();
	void handle_instr_mute_all();
	void handle_instr_unmute_all();

	void handle_volume_changed(int);
	void handle_volume_reset();

	void handle_midi_file_import(int);
	void handle_gpro_file_import(int);

	int handle_mute_map(int);
	int handle_config_dev(int, int = 0);

	void handle_bpm();
	void handle_mode(int,int = 1);

	void handle_key_mode_a(int);
	void handle_key_mode_b(int);

	void handle_move_right();
	void handle_move_left();

	void handle_tab_changed(int force = 0);

	void handle_copy();
	void handle_paste();
	void handle_redo();
	void handle_undo();
	void handle_edit();
	void handle_up_down();
};

#endif		/* _MIDIPP_MAINWINDOW_H_ */
