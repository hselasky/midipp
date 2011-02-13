/*-
 * Copyright (c) 2009-2010 Hans Petter Selasky. All rights reserved.
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

#include <midipp.h>

enum {
	MM_PASS_ALL,
	MM_PASS_ONE_MIXED,
	MM_PASS_NONE_FIXED,
	MM_PASS_NONE_TRANS,
	MM_PASS_MAX,
};

class MppMainWindow : public QWidget
{
	Q_OBJECT;

public:
	MppMainWindow(QWidget *parent = 0);
	~MppMainWindow();

	void MidiInit(void);
	void MidiUnInit(void);

	void handle_stop(void);
	void handle_midi_file_open(int merge);
	void handle_midi_file_clear_name(void);
	void handle_midi_file_instr_prepend(void);
	void handle_midi_file_instr_delete(void);
	void handle_jump_locked(int index);

	int convert_midi_duration(uint32_t thres);
	int convert_midi_score_duration();

	void update_play_device_no(void);

	void do_bpm_stats(void);
	void do_clock_stats(void);
	void do_update_bpm(void);
	void do_key_press(int key, int vel, int dur);
	void output_key_sub(int chan, int key, int vel, int delay, int dur);
	void output_key(int chan, int key, int vel, int delay, int dur, int seq = 0);

	uint32_t get_time_offset(void);

	uint8_t noise8(uint8_t factor);
	uint8_t do_instr_check(struct umidi20_event *event, uint8_t *pchan);
	uint8_t check_record(uint8_t chan, uint32_t off);
	uint8_t check_synth(uint8_t device_no, uint8_t chan, uint32_t off);

	pthread_mutex_t mtx;

	QFont defaultFont;

	uint8_t auto_zero_start[0];

	struct MppInstr instr[16];

	MppScoreMain *currScoreMain;

	uint32_t convLineStart[MPP_MAX_LINES];
	uint32_t convIndex;

	uint32_t bpmData[MPP_MAX_BPM];
	uint32_t lastKeyPress;
	uint32_t bpmAutoPlayOld;
	uint32_t lastInputEvent;
	uint32_t noiseRem;

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

	uint8_t muteMap[MPP_MAX_DEVS][16];
	uint8_t inputEvents[MPP_MAX_QUEUE];
	uint8_t numInputEvents;
	uint8_t cmdKey;
	uint8_t baseKey;
	uint8_t playKey;
	uint8_t bpmAvgLength;
	uint8_t bpmAvgPos;
	uint8_t synthIsLocal;
	uint8_t restartBpm;
	uint8_t cursorUpdate;

	uint8_t scoreRecordOff;
	uint8_t instrUpdated;
	uint8_t midiRecordOff;
	uint8_t midiPlayOff;
	uint8_t midiMode;
	uint8_t midiTriggered;
	uint8_t midiPaused;

	char *deviceName[MPP_MAX_DEVS];

	QGridLayout *main_gl;

	QTabWidget *main_tw;

	QTabWidget *scores_tw;

	/* tab <Scores> */

	MppScoreMain *scores_main[MPP_MAX_VIEWS];

	/* tab <Import> */

	MppImportTab *tab_import;

	QTimer *watchdog;

	/* tab <File> */

	QWidget *tab_file_wg;
	QGridLayout *tab_file_gl;
	QLabel *lbl_score_Afile;
	QLabel *lbl_score_Bfile;
	QLabel *lbl_import_file;
	QLabel *lbl_midi_file;
	QLabel *lbl_file_status;

	QPushButton *but_quit;

	QPushButton *but_midi_file_new;
	QPushButton *but_midi_file_open;
	QPushButton *but_midi_file_merge;
	QPushButton *but_midi_file_save;
	QPushButton *but_midi_file_save_as;
	QPushButton *but_midi_file_convert;

	/* tab <Play> */

	QLabel *lbl_bpm_min;
	QLabel *lbl_bpm_avg;
	QLabel *lbl_bpm_max;

	QLCDNumber *lbl_curr_time_val;
	QLCDNumber *lbl_bpm_min_val;
	QLCDNumber *lbl_bpm_avg_val;
	QLCDNumber *lbl_bpm_max_val;

	QLabel *lbl_synth;
	QLabel *lbl_time_counter;
	QLabel *lbl_playback;
	QLabel *lbl_recording;
	QLabel *lbl_scores;

	QWidget *tab_play_wg;
	QGridLayout *tab_play_gl;

	QLabel	*lbl_volume;
	QSpinBox *spn_volume;

	QLabel	*lbl_play_key;
	QSpinBox *spn_play_key;

	QLabel	*lbl_cmd_key;
	QSpinBox *spn_cmd_key;

	QLabel	*lbl_base_key;
	QSpinBox *spn_base_key;

	QLabel	*lbl_score_record;
	QLabel	*lbl_midi_record;
	QLabel	*lbl_midi_play;
	QLabel	*lbl_midi_mode;

	QPushButton *but_insert_chord;
	QPushButton *but_jump[MPP_MAX_LBUTTON];
	QPushButton *but_midi_mode;
	QPushButton *but_compile;
	QPushButton *but_score_record;
	QPushButton *but_play;
	QPushButton *but_midi_pause;
	QPushButton *but_midi_record;
	QPushButton *but_midi_play;
	QPushButton *but_midi_trigger;
	QPushButton *but_midi_rewind;

	/* tab <Configuration> */

	QGridLayout *tab_config_gl;
	QWidget *tab_config_wg;

	QLabel *lbl_auto_play;
	QLabel *lbl_config_title;
	QLabel *lbl_config_play;
	QLabel *lbl_config_rec;
	QLabel *lbl_config_synth;
	QLabel *lbl_config_mm;
	QLabel *lbl_config_dev[MPP_MAX_DEVS];
	QLabel *lbl_bpm_count;

	QSpinBox *spn_bpm_length;
	QSpinBox *spn_auto_play;

	QLabel	*lbl_key_delay;
	QSpinBox *spn_key_delay;

	QLabel	*lbl_parse_thres;
	QSpinBox *spn_parse_thres;

	QPushButton *but_config_mm[MPP_MAX_DEVS];
	QLineEdit *led_config_dev[MPP_MAX_DEVS];
	QCheckBox *cbx_config_dev[3 * MPP_MAX_DEVS];

	QLabel *lbl_config_local;
	QCheckBox *cbx_config_local;

	QPushButton *but_config_apply;
	QPushButton *but_config_revert;
	QPushButton *but_config_fontsel;

	QLabel *lbl_config_insert;
	QLineEdit *led_config_insert;

	QString *CurrMidiFileName;

	/* tab <Edit> */

	QGridLayout *tab_edit_gl;
	QWidget *tab_edit_wg;

	QLabel *lbl_edit_title;
	QLabel *lbl_edit_channel;
	QLabel *lbl_edit_transpose;
	QLabel *lbl_edit_volume;

	QSpinBox *spn_edit_channel;
	QSpinBox *spn_edit_transpose;
	QSpinBox *spn_edit_volume;

	QPushButton *but_edit_apply_transpose;
	QPushButton *but_edit_change_volume;
	QPushButton *but_edit_remove_pedal;
	QPushButton *but_edit_remove_keys;
	QPushButton *but_edit_remove_all;

	/* tab <Instrument> */

	QGridLayout *tab_instr_gl;
	QWidget *tab_instr_wg;

	QSpinBox *spn_instr_curr_chan;
	QSpinBox *spn_instr_curr_bank;
	QSpinBox *spn_instr_curr_prog;
	QSpinBox *spn_instr_bank[16];
	QSpinBox *spn_instr_prog[16];

	QLabel *lbl_instr_title[2];
	QLabel *lbl_instr_prog;
	QLabel *lbl_instr_desc[16];

	QCheckBox *cbx_instr_mute[16];

	QPushButton *but_instr_rem;
	QPushButton *but_instr_program;
	QPushButton *but_instr_apply;
	QPushButton *but_instr_revert;
	QPushButton *but_instr_reset;
	QPushButton *but_instr_mute_all;
	QPushButton *but_instr_unmute_all;

	/* tab <Volume> */

	QGridLayout *tab_volume_gl;
	QWidget *tab_volume_wg;

	QSpinBox *spn_volume_play[16];
	QSpinBox *spn_volume_synth[16];

	QLabel *lbl_volume_title[2];
	QLabel *lbl_volume_play[16];
	QLabel *lbl_volume_synth[16];

	QPushButton *but_volume_apply;
	QPushButton *but_volume_revert;
	QPushButton *but_volume_reset;

	/* tab <Loop> */

	MppLoopTab *tab_loop;

	/* tab <Echo> */

	MppEchoTab *tab_echo[MPP_MAX_ETAB];

	/* MIDI stuff */
	struct mid_data mid_data;
	struct umidi20_song *song;
	struct umidi20_track *track;

	uint8_t auto_zero_end[0];

public slots:
	void handle_quit();
	void handle_play_key_changed(int);
	void handle_cmd_key_changed(int);
	void handle_base_key_changed(int);
	void handle_key_delay_changed(int);
	void handle_config_local_changed(int);
	void handle_insert_chord();
	void handle_jump_N(int index);
	void handle_jump_common();
	void handle_pass_thru();
	void handle_compile();
	void handle_score_record();
	void handle_midi_record();
	void handle_midi_pause();
	void handle_midi_play();
	void handle_play_press();
	void handle_play_release();
	void handle_watchdog();
	void handle_midi_file_new();
	void handle_midi_file_merge_open();
	void handle_midi_file_new_open();
	void handle_midi_file_save();
	void handle_midi_file_save_as();
	void handle_rewind();
	void handle_midi_trigger();
	void handle_config_apply();
	void handle_config_revert();
	void handle_config_reload();
	void handle_config_fontsel();

	void handle_instr_apply();
	void handle_instr_revert();
	void handle_instr_reset();
	void handle_instr_reload();
	void handle_instr_program();
	void handle_instr_channel_changed(int);
	void handle_instr_rem();
	void handle_instr_mute_all();
	void handle_instr_unmute_all();

	void handle_volume_changed(int);
	void handle_volume_apply();
	void handle_volume_revert();
	void handle_volume_reset();
	void handle_volume_reload();

	void handle_auto_play(int bpm);
	void handle_tab_changed(int index);

	void handle_midi_file_convert();

	void handle_mute_map();

protected:
	void keyPressEvent(QKeyEvent *event);
	void keyReleaseEvent(QKeyEvent *event);
};

#endif		/* _MIDIPP_MAINWINDOW_H_ */
