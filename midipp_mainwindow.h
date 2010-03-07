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

	void update_play_device_no(void);

	void do_bpm_stats(void);
	void do_clock_stats(void);
	void do_update_bpm(void);
	void do_key_press(int key, int vel, int dur);

	uint8_t noise8(uint8_t factor);
	uint8_t do_instr_check(struct umidi20_event *event, uint8_t *pchan);
	uint8_t check_record(uint8_t chan);
	uint8_t check_synth(uint8_t device_no, uint8_t chan);

	pthread_mutex_t mtx;

	QFont defaultFont;

	uint8_t auto_zero_start[0];

	struct MppInstr instr[16];

	MppScoreMain *currScoreMain;

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

	uint8_t inputEvents[MPP_MAX_QUEUE];
	uint8_t numInputEvents;
	uint8_t playDevice;
	uint8_t cmdKey;
	uint8_t playKey;
	uint8_t bpmAvgLength;
	uint8_t bpmAvgPos;
	uint8_t synthIsLocal;
	uint8_t restartBpm;

	uint8_t scoreRecordOff;
	uint8_t instrUpdated;
	uint8_t midiRecordOff;
	uint8_t midiPlayOff;
	uint8_t midiPassThruOff;
	uint8_t midiTriggered;
	uint8_t midiPaused;

	char *deviceName[MPP_MAX_DEVS];

	QGridLayout *main_gl;

	QTabWidget *main_tw;

	QTabWidget *scores_tw;

	MppScoreMain *scores_main[MPP_MAX_VIEWS];

	QTimer *watchdog;

	/* tab <File> */

	QWidget *tab_file_wg;
	QGridLayout *tab_file_gl;
	QLabel *lbl_score_Afile;
	QLabel *lbl_score_Bfile;
	QLabel *lbl_midi_file;
	QLabel *lbl_file_status;

	QPushButton *but_quit;

	QPushButton *but_midi_file_new;
	QPushButton *but_midi_file_open;
	QPushButton *but_midi_file_merge;
	QPushButton *but_midi_file_save;
	QPushButton *but_midi_file_save_as;

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
	QLabel	*lbl_midi_pass_thru;

	QPushButton *but_jump[8];
	QPushButton *but_midi_pass_thru;
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
	QLabel *lbl_config_dev[MPP_MAX_DEVS];
	QLabel *lbl_bpm_count;

	QSpinBox *spn_bpm_length;
	QSpinBox *spn_auto_play;

	QLabel	*lbl_key_delay;
	QSpinBox *spn_key_delay;

	QLineEdit *led_config_dev[MPP_MAX_DEVS];
	QCheckBox *cbx_config_dev[3 * MPP_MAX_DEVS];

	QLabel *lbl_config_local;
	QCheckBox *cbx_config_local;

	QPushButton *but_config_apply;
	QPushButton *but_config_revert;
	QPushButton *but_config_fontsel;

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

	QPushButton *but_instr_program;
	QPushButton *but_instr_apply;
	QPushButton *but_instr_revert;
	QPushButton *but_instr_reset;

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
	void handle_jump_0();
	void handle_jump_1();
	void handle_jump_2();
	void handle_jump_3();
	void handle_jump_4();
	void handle_jump_5();
	void handle_jump_6();
	void handle_jump_7();
	void handle_jump_N(int index);
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

	void handle_volume_changed(int);
	void handle_volume_apply();
	void handle_volume_revert();
	void handle_volume_reset();
	void handle_volume_reload();

	void handle_auto_play(int bpm);
	void handle_tab_changed(int index);

protected:
	void keyPressEvent(QKeyEvent *event);
	void keyReleaseEvent(QKeyEvent *event);
};

#endif		/* _MIDIPP_MAINWINDOW_H_ */
