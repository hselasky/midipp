/*-
 * Copyright (c) 2009-2022 Hans Petter Selasky
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

#include <QStackedLayout>

class MppMainWindow : QObject
{
	Q_OBJECT

public:
	MppMainWindow();
	~MppMainWindow();

#ifdef HAVE_SCREENSHOT
	void ScreenShot(QApplication &);
#endif
	void MidiInit(void);
	void MidiUnInit(void);

	void atomic_lock(void);
	void atomic_unlock(void);

	void closeEvent(QCloseEvent *event);
	void handle_stop(int flag = 0);
	void handle_midi_file_open(int);
	void handle_midi_file_clear_name(void);
	void handle_midi_file_instr_prepend(void);
	void handle_midi_file_instr_delete(void);
	void handle_jump_locked(int index);
	void handle_make_scores_visible(MppScoreMain *);
	void handle_make_tab_visible(QWidget *);

	QString get_midi_score_duration(uint32_t *psum);
	int log_midi_score_duration();
	int convert_midi_duration(struct umidi20_track *, uint32_t thres, uint32_t chan_mask);
	void import_midi_track(struct umidi20_track *, uint32_t = 0, int = -1, int = 0);

	void update_play_device_no(void);

	void do_clock_stats(void);
	int do_extended_alloc(int key, int refcount);
	void do_key_press(int key, int vel, int dur);
  	void do_key_pitch(int key, int pressure);
	void do_key_pressure(int key, int pressure);
	void do_key_control(int key, uint8_t control, int value);
	void output_key(int index, int chan, int key, int vel, int delay, int dur);
	void output_key_pitch(int index, int chan, int key, int amount, uint32_t delay = 0);
	void output_key_control(int index, int chan, int key, uint8_t control, int value, uint32_t delay = 0);
	void output_key_pressure(int index, int chan, int key, int pressure, uint32_t delay = 0);

	uint32_t get_time_offset(void);

	uint8_t noise8(uint8_t factor);
	uint8_t do_instr_check(struct umidi20_event *event, int = 0);
	bool check_play(uint8_t index, uint8_t chan, uint32_t off, uint8_t = MPP_MAGIC_DEVNO);
	bool check_record(uint8_t index, uint8_t chan, uint32_t off);

	void handle_watchdog_sub(MppScoreMain *, int);

	void send_song_stop_locked();
	void send_song_trigger_locked();
	void send_song_select_locked(uint8_t);

	void send_byte_event_locked(uint8_t);

	MppScoreMain *getCurrTransposeView(void);

	QPlainTextEdit *currEditor();
	MppScoreMain *currScores();

	pthread_mutex_t mtx;

	QFont defaultFont;
	QFont editFont;
	QFont printFont;
	QFont showFont;

	QTimer tim_config_init;
	QTimer tim_config_apply;
	QTimer watchdog;

	uint8_t auto_zero_start[0];

	struct MppInstr instr[16];

	int extended_keys[128][2];
  
	uint32_t convLineStart[MPP_MAX_LINES];
	uint32_t convLineEnd[MPP_MAX_LINES];
	uint32_t convIndex;

	uint32_t lastKeyPress;
	uint32_t noiseRem;

	uint32_t devInputMask[MPP_MAX_DEVS];
	uint32_t startPosition;
	uint32_t pausePosition;
	uint32_t deviceBits;
#define	MPP_DEV0_PLAY	0x0001UL
#define	MPP_DEV0_RECORD	0x0002UL

	uint16_t trackVolume[MPP_MAX_TRACKS];
	int16_t masterPitchBend;

	uint8_t devSelMap[MPP_MAX_DEVS];
	uint8_t muteProgram[MPP_MAX_DEVS];
	uint8_t mutePedal[MPP_MAX_DEVS];
	uint8_t enableLocalKeys[MPP_MAX_DEVS];
	uint8_t disableLocalKeys[MPP_MAX_DEVS];
	uint8_t muteAllControl[MPP_MAX_DEVS];
	uint8_t muteAllNonChannel[MPP_MAX_DEVS];
	uint8_t muteMap[MPP_MAX_DEVS][16];
	uint8_t cursorUpdate;

	uint8_t scoreRecordOn;
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

	uint8_t noteMode;

	char *deviceName[MPP_MAX_DEVS];

	QWidget *super_w;
	QStackedLayout *super_l;

	QWidget *main_w;
	QGridLayout *main_gl;

	MppTabBar *main_tb;
	int main_tb_state;

	/* main <> */

	QPushButton *mwRewind;
	QPushButton *mwPlay;
	QPushButton *mwReload;
	QPushButton *mwPaste;
	QPushButton *mwCopy;
	QPushButton *mwUndo;
	QPushButton *mwRedo;
	QPushButton *mwEdit;
	QPushButton *mwUpDown;
	QPushButton *mwLeftRight;

	/* tab <Scores> */

	MppScoreMain *scores_main[MPP_MAX_VIEWS];

	/* tab <Import> */

	MppImportTab *tab_import;

	/* tab <File> */

	MppGridLayout *tab_file_gl;

	MppGroupBox *gb_midi_file;
	QPushButton *but_midi_file_new;
	QPushButton *but_midi_file_open_single;
	QPushButton *but_midi_file_open_multi;
	QPushButton *but_midi_file_merge_single;
	QPushButton *but_midi_file_merge_multi;
	QPushButton *but_midi_file_save;
	QPushButton *but_midi_file_save_as;
	MppButton *but_midi_file_import[MPP_MAX_VIEWS];

	MppGroupBox *gb_gpro_file_import;
	MppButton *but_gpro_file_import[MPP_MAX_VIEWS];

	MppGroupBox *gb_mxml_file_import;
	MppButton *but_mxml_file_import[MPP_MAX_VIEWS];
	
	/* tab <Play> */

	MppGroupBox *gl_ctrl;
	MppGroupBox *gl_time;
	MppGroupBox *gl_bpm;
	MppGroupBox *gl_synth_play;
	MppGroupBox *gl_tuning;

	QLCDNumber *lbl_curr_time_val;
	QLCDNumber *lbl_bpm_avg_val;

	MppGridLayout *tab_play_gl;

	MppVolume *spn_tuning;

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

	MppDevSel *but_config_sel[MPP_MAX_DEVS];
	MppButton *but_config_dev[MPP_MAX_DEVS];
	MppButton *but_config_mm_ch[MPP_MAX_DEVS];
	MppButton *but_config_mm_other[MPP_MAX_DEVS];
	QLineEdit *led_config_dev[MPP_MAX_DEVS];
	MppCheckBox *cbx_config_dev[MPP_MAX_DEVS][1 + MPP_MAX_VIEWS];

	uint32_t dirty_config_mask;

	QPushButton *but_config_view_fontsel;
	QPushButton *but_config_edit_fontsel;
	QPushButton *but_config_print_fontsel;

	QString *CurrMidiFileName;

	/* tab <Shortcut> */

	MppShortcutTab *tab_shortcut;

	/* tab <Custom> */

	MppCustomTab *tab_custom;

	/* tab <Instrument> */

	MppInstrumentTab *tab_instrument;

	/* tab <Loop> */

	MppLoopTab *tab_loop;

	/* tab <RePlay> */

	MppReplayTab *tab_replay;
	
	/* tab <DataBase> */

	MppDataBase *tab_database;

	/* tab <OnlineTabs> */

	MppOnlineTabs *tab_onlinetabs;

	/* tab <Show> */
#ifndef HAVE_NO_SHOW
	MppShowControl *tab_show_control;
#endif
	/* tab <Help> */

	QPlainTextEdit *tab_help;

	/* MIDI stuff */
	struct mid_data mid_data;
	struct umidi20_song *song;
	struct umidi20_track *track[MPP_MAX_TRACKS];

	uint8_t auto_zero_end[0];

	void show() {
		super_w->show();
	};

	operator QWidget *() const {
		return (super_w);
	};

public slots:
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
	void handle_midi_file_merge_single_open();
	void handle_midi_file_new_single_open();
	void handle_midi_file_merge_multi_open();
	void handle_midi_file_new_multi_open();
	void handle_midi_file_save();
	void handle_midi_file_save_as();
	void handle_rewind();
	void handle_midi_trigger();
	void handle_config_changed();
	void handle_config_init();
	void handle_config_apply();
  	void handle_config_local_keys();
	void handle_config_reload();
	void handle_config_view_fontsel();
	void handle_config_edit_fontsel();
	void handle_config_print_fontsel();

	void handle_midi_file_import(int);
	void handle_gpro_file_import(int);
	void handle_mxml_file_import(int);

	void handle_mute_map_ch(int);
	void handle_mute_map_other(int);

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
	void handle_left_right();
	void handle_tuning();
};

#endif		/* _MIDIPP_MAINWINDOW_H_ */
