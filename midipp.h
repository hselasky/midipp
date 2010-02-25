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

#ifndef _MIDIPP_H_
#define	_MIDIPP_H_

#include <QDialog>
#include <QPushButton>
#include <QGridLayout>
#include <QTextEdit>
#include <QTabWidget>
#include <QLabel>
#include <QSpinBox>
#include <QTextCursor>
#include <QTimer>
#include <QKeyEvent>
#include <QFile>
#include <QFileDialog>
#include <QCheckBox>
#include <QLineEdit>
#include <QSpacerItem>
#include <QLCDNumber>
#include <QPicture>
#include <QColor>
#include <QPaintEvent>
#include <QPainter>
#include <QFont>
#include <QPrintDialog>
#include <QPrinter>
#include <QFontDialog>
#include <QFileInfo>

#include <umidi20.h>

#define	MPP_MAX_LINES	1024
#define	MPP_MAX_SCORES	32
#define	MPP_MAX_LABELS	32
#define	MPP_MAX_QUEUE	32
#define	MPP_MAX_DEVS	4
#define	MPP_MAX_BPM	32
#define	MPP_MIN_POS	4	/* ticks */
#define	MPP_PRESSED_MAX	128

#define	MPP_VISUAL_MARGIN	8
#define	MPP_VISUAL_Y_MAX	80
#define	MPP_VISUAL_R_MAX	8
#define	MPP_VOLUME_UNIT		127

struct MppScore {
	uint8_t key;
	uint8_t dur;
	uint8_t channel;
};

struct MppInstr {
	uint16_t bank;
	uint8_t prog;
	uint8_t muted;
	uint8_t updated;
};

struct MppScoreEntry {
	char *pstr;
	QPicture *pic;
	int32_t x_off;
	int32_t y_off;
};

struct MppSoftc {
	struct MppScore ScScores[MPP_MAX_LINES][MPP_MAX_SCORES];
	struct MppScoreEntry ScVisualScores[MPP_MAX_LINES];
	struct MppInstr ScInstr[16];

	pthread_mutex_t mtx;

	QFont *ScFont;

	uint32_t ScBpmData[MPP_MAX_BPM];
	uint32_t ScLastKeyPress;
	uint32_t ScBpmAutoPlay;
	uint32_t ScBpmAutoPlayOld;
	uint32_t ScLastInputEvent;
	uint32_t ScNoiseRem;

	/* parse state */
	int line;
	int index;
	int buf_index;
	int buf_line;

	/* parse buffer */
	char buf[128];

	uint32_t ScPressed[MPP_PRESSED_MAX];
	uint32_t ScChanUsageMask;
	uint32_t ScStartPosition;
	uint32_t ScPausePosition;
	uint32_t ScDeviceBits;
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

	uint16_t ScJumpNext[MPP_MAX_LINES];
	uint16_t ScJumpTable[MPP_MAX_LABELS];
	uint16_t ScLinesMax;
	uint16_t ScCurrPos;
	uint16_t ScLastPos;
	uint16_t ScMaxScoresWidth;
	uint16_t ScPlayVolume[16];
	uint16_t ScSynthVolume[16];

	uint8_t ScPageNext[MPP_MAX_LINES];
	uint8_t ScInputEvents[MPP_MAX_QUEUE];
	uint8_t ScNumInputEvents;
	uint8_t ScPlayDevice;
	uint8_t ScCmdKey;
	uint8_t ScBaseKey;
	uint8_t ScPlayKey;
	uint8_t ScSynthChannel;
	uint8_t ScBpmAvgLength;
	uint8_t ScBpmAvgPos;
	uint8_t ScSynthIsLocal;

	uint8_t ScScoreRecordOff;
	uint8_t ScInstrUpdated;
	uint8_t ScMidiRecordOff;
	uint8_t ScMidiPlayOff;
	uint8_t ScMidiPassThruOff;
	uint8_t ScMidiTriggered;
	uint8_t ScMidiPaused;
	uint8_t ScDelayNoise;

	char *ScDeviceName[MPP_MAX_DEVS];
};

class MppVisualScores : public QWidget
{

public:
	MppVisualScores(struct MppSoftc *sc_init);

	void paintEvent(QPaintEvent *event);
	struct MppSoftc *sc;
};

class MppMainWindow : public QWidget
{
	Q_OBJECT;

public:
	MppMainWindow(QWidget *parent = 0);
	~MppMainWindow();

	void MidiInit(void);
	void MidiUnInit(void);

	void handle_key_press(int in_key, int vel);
	void handle_key_release(int in_key);
	void handle_stop(void);
	void handle_midi_file_open(int merge);
	void handle_midi_file_clear_name(void);
	void handle_midi_file_instr_prepend(void);
	void handle_midi_file_instr_delete(void);
	void update_play_device_no(void);
	void do_bpm_stats(void);
	void do_clock_stats(void);
	void do_update_bpm(void);
	void do_key_press(struct mid_data *d, int key, int vel, int dur);

	int set_pressed_key(int, int, int, int);

	uint8_t do_instr_check(struct umidi20_event *event);
	uint8_t handle_jump(int pos, int do_jump);
	uint8_t check_record(void);
	uint8_t check_synth(uint8_t device_no);

	struct MppSoftc main_sc;

	QGridLayout *main_gl;
	QTabWidget *main_tw;

	QTabWidget *scores_tw;

	QFont default_font;

	QTimer *watchdog;

	/* visual scores */

	MppVisualScores *scores_wg;

	/* editor */

	QTextEdit *main_edit;

	/* tab <File> */

	QWidget *tab_file_wg;
	QGridLayout *tab_file_gl;
	QLabel *lbl_score_file;
	QLabel *lbl_midi_file;
	QLabel *lbl_file_status;

	QPushButton *but_score_file_new;
	QPushButton *but_score_file_open;
	QPushButton *but_score_file_save;
	QPushButton *but_score_file_save_as;
	QPushButton *but_score_file_print;
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

	QString *CurrScoreFileName;
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
	void handle_score_file_new();
	void handle_score_file_open();
	void handle_score_file_save();
	void handle_score_file_save_as();
	void handle_score_print();
	void handle_midi_file_new();
	void handle_midi_file_merge_open();
	void handle_midi_file_new_open();
	void handle_midi_file_save();
	void handle_midi_file_save_as();
	void handle_rewind();
	void handle_midi_trigger();
	void handle_auto_play(int bpm);
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

protected:
	void keyPressEvent(QKeyEvent *event);
	void keyReleaseEvent(QKeyEvent *event);
};

#endif	/* _MIDIPP_H_ */
