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

#include <umidi20.h>

#define	MPP_MAX_LINES	1024
#define	MPP_MAX_NOTES	16
#define	MPP_MAX_LABELS	32
#define	MPP_MAX_QUEUE	32

struct MppNote {
	uint8_t key;
	uint8_t dur;
	uint8_t channel;
};

struct MppSoftc {
	struct MppNote ScNotes[MPP_MAX_LINES][MPP_MAX_NOTES];

	uint32_t ScTrackInvMask;
	uint32_t ScPosition;

	uint16_t ScJumpNext[MPP_MAX_LINES];
	uint16_t ScJumpTable[MPP_MAX_LABELS];
	uint16_t ScLinesMax;
	uint16_t ScCurrPos;

	uint8_t ScPressed[128];
	uint8_t ScInputEvents[MPP_MAX_QUEUE];
	uint8_t ScNumInputEvents;
	uint8_t ScCmdKey;

	uint8_t is_note_record_off;
	uint8_t is_midi_record_off;
	uint8_t is_midi_play_off;
	uint8_t is_midi_pass_thru_off;
	uint8_t is_midi_triggered;
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

	uint8_t handle_jump(int pos);
	uint8_t check_record(void);
	uint8_t check_playback(void);

	struct MppSoftc main_sc;

	QGridLayout *main_gl;
	QTabWidget *main_tw;

	QTimer *watchdog;

	/* editor */

	QTextEdit *main_edit;

	/* tab <File> */

	QWidget *tab_file_wg;
	QGridLayout *tab_file_gl;
	QLabel *lbl_note_file;
	QLabel *lbl_midi_file;

	/* tab <Note File> */

	QPushButton *but_note_file_new;
	QPushButton *but_note_file_open;
	QPushButton *but_note_file_save;
	QPushButton *but_note_file_save_as;
	QPushButton *but_note_file_print;
	QPushButton *but_configure;
	QPushButton *but_quit;

	/* tab <MIDI File> */

	QPushButton *but_midi_file_new;
	QPushButton *but_midi_file_open;
	QPushButton *but_midi_file_save;
	QPushButton *but_midi_file_save_as;

	/* tab <Play> */

	QLabel *lbl_synth;
	QLabel *lbl_playback;
	QLabel *lbl_recording;

	QWidget *tab_play_wg;
	QGridLayout *tab_play_gl;

	QLabel	*lbl_volume;
	QSpinBox *spn_volume;

	QLabel	*lbl_play_key;
	QSpinBox *spn_play_key;

	QLabel	*lbl_cmd_key;
	QSpinBox *spn_cmd_key;

	QLabel	*lbl_note_record;
	QLabel	*lbl_midi_record;
	QLabel	*lbl_midi_play;
	QLabel	*lbl_midi_pass_thru;

	QPushButton *but_jump[4];
	QPushButton *but_track[4];

	QLabel *lbl_track[4];

	QPushButton *but_midi_pass_thru;
	QPushButton *but_compile;
	QPushButton *but_note_record;
	QPushButton *but_play;
	QPushButton *but_midi_record;
	QPushButton *but_midi_play;
	QPushButton *but_midi_trigger;
	QPushButton *but_midi_rewind;

	QString *CurrNoteFileName;
	QString *CurrMidiFileName;

	/* MIDI stuff */
	struct mid_data mid_data;
	struct umidi20_song *song;
	struct umidi20_track *track;

	pthread_mutex_t mtx;

 public slots:
	void handle_quit();
	void handle_play_key_changed(int);
	void handle_cmd_key_changed(int);
	void handle_jump_0();
	void handle_jump_1();
	void handle_jump_2();
	void handle_jump_3();
	void handle_jump_N(int index);
	void handle_track_0();
	void handle_track_1();
	void handle_track_2();
	void handle_track_3();
	void handle_track_N(int index);
	void handle_pass_thru();
	void handle_compile();
	void handle_note_record();
	void handle_midi_record();
	void handle_midi_play();
	void handle_play_press();
	void handle_play_release();
	void handle_watchdog();
	void handle_note_file_new();
	void handle_note_file_open();
	void handle_note_file_save();
	void handle_note_file_save_as();
	void handle_midi_file_new();
	void handle_midi_file_open();
	void handle_midi_file_save();
	void handle_midi_file_save_as();
	void handle_rewind();
	void handle_play_trigger();

 protected:
	void keyPressEvent(QKeyEvent *event);
	void keyReleaseEvent(QKeyEvent *event);
};

#endif	/* _MIDIPP_H_ */
