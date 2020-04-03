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

#ifndef _MIDIPP_SCORES_H_
#define	_MIDIPP_SCORES_H_

#include "midipp.h"
#include "midipp_element.h"

class MppScoreView : public QWidget
{

public:
	MppScoreView(MppScoreMain *parent);

protected:
	void paintEvent(QPaintEvent *);
	void mousePressEvent(QMouseEvent *);
	void wheelEvent(QWheelEvent *);
	void keyPressEvent(QKeyEvent *);
	int delta_v;

	MppScoreMain *pScores;
};

class MppScoreTextEdit : public QPlainTextEdit
{
public:
	MppScoreMain *sm;

	MppScoreTextEdit(MppScoreMain *parent);
	~MppScoreTextEdit(void);

	void mouseDoubleClickEvent(QMouseEvent *e);
};

class MppScoreMain : public QObject
{
	Q_OBJECT;

public:
	MppScoreMain(MppMainWindow *parent, int unit);
	~MppScoreMain();

	void decrementDuration(int vel, uint32_t timeout);
	void handleLabelJump(int label);
	void handleChordsLoad(void);
	void handleMidiKeyPressLocked(int key, int vel);
	void handleMidiKeyReleaseLocked(int key, int vel);
	void handleKeyPressChord(int key, int vel, uint32_t key_delay);
	void handleKeyPitchChord(int in_key, int amount, uint32_t key_delay);
	void handleKeyControlChord(int in_key, uint8_t control, int value, uint32_t key_delay);
	void handleKeyPressureChord(int key, int vel, uint32_t key_delay);
	void handleKeyReleaseChord(int key, int vel, uint32_t key_delay);
	void handleKeyPressSub(int, int, uint32_t, int, int);
	void handleKeyPress(int key, int vel, uint32_t key_delay);
	void handleKeyRelease(int key, int vel, uint32_t key_delay);
	void handleParse(const QString &ps);
	uint8_t handleKeyRemovePast(MppScoreEntry *pn, int vel, uint32_t key_delay);
	void handleScoreFileOpenRaw(char *, uint32_t);
	void handlePrintSub(QPrinter *pd, QPoint orig);
	int handleScoreFileOpenSub(QString fname);
	void outputChannelMaskGet(uint16_t *pmask);
	void outputControl(uint8_t ctrl, uint8_t val);
	void outputChanPressure(uint8_t pressure);
	void outputPitch(uint16_t val);
	int getCurrLabel(void);
	void handleScoreFileEffect(int, int, int);
	void handleEditLine(void);

	void viewPaintEvent(QPaintEvent *event);
	void viewMousePressEvent(QMouseEvent *e);
	void locateVisual(MppElement *, int *, int *, MppVisualDot **);

	void watchdog();

	int checkLabelJump(int label);

	int setPressedKey(int chan, int out_key, int dur, int delay);

	MppHead head;

	uint8_t auto_zero_start[0];

	MppVisualScore *pVisual;
	MppSheet *sheet;
	MppGridLayout *gl_view;
	QScrollBar *viewScroll;
	MppScoreView *viewWidgetSub;

	MppMainWindow *mainWindow;
	MppScoreTextEdit *editWidget;

	MppGroupBox *gbScoreFile;

	QPushButton *butScoreFileNew;
	QPushButton *butScoreFileOpen;
	QPushButton *butScoreFileSave;
	QPushButton *butScoreFileSaveAs;
	QPushButton *butScoreFilePrint;
	QPushButton *butScoreFileAlign;
	QSpinBox *spnScoreFileBassOffset;
	QPushButton *butScoreFileBassOffset;
	MppSpinBox *spnScoreFileAlign;
	QPushButton *butScoreFileScale;
	QSpinBox *spnScoreFileScale;
	QPushButton *butScoreFileStepUpHalf;
	QPushButton *butScoreFileStepDownHalf;
	QPushButton *butScoreFileStepUpSingle;
	QPushButton *butScoreFileStepDownSingle;
	QPushButton *butScoreFileSetSharp;
	QPushButton *butScoreFileSetFlat;
	QPushButton *butScoreFileExport;
	QPushButton *butScoreFileExportNoChords;
	QPushButton *butScoreFileReplaceAll;

	QString *currScoreFileName;
	QLabel *lblFileStatus;
	QPicture *picChord[2];

	MppScoreEntry score_past[MPP_MAX_CHORD_MAP];
	MppScoreEntry score_future_base[MPP_MAX_CHORD_FUTURE];
	MppScoreEntry score_future_treble[MPP_MAX_CHORD_FUTURE];

	int visual_max;
	int visual_p_max;
	int unit;

	uint64_t pressedKeys[MPP_PRESSED_MAX];
	uint64_t frozenKeys[MPP_PRESSED_MAX];

	int picScroll;
	uint32_t active_channels;

	int baseKey;
	int whatPlayKeyLocked;
	int chordTranspose;

	int8_t inputChannel;
	uint8_t inputKeyToChannel[128];
	uint8_t synthChannel;
	int8_t synthChannelBase;
	int8_t synthChannelTreb;
	int8_t auxChannel;
	int8_t auxChannelBase;
	int8_t auxChannelTreb;
	int8_t synthDevice;
	int8_t synthDeviceBase;
	int8_t synthDeviceTreb;
	uint8_t delayNoise;
	uint8_t last_key;
	uint8_t last_vel;
	uint8_t keyMode;
	uint8_t noteMode;
	uint8_t pressed_future;
	uint8_t chordContrast;
	uint8_t chordNormalize;
	uint8_t songEventsOn;

	uint8_t auto_zero_end[0];

	int visual_y_max;

	QString editText;

public slots:

	int handleCompile(int force = 0);
	void handleScoreFileNew(int invisible = 0);
	void handleScoreFileOpen();
	void handleScoreFileSave();
	void handleScoreFileSaveAs();
	void handleScorePrint();
	void handleScoreFileBassOffset(void);
	void handleScoreFileAlign(void);
	void handleScoreFileStepUpHalf(void);
	void handleScoreFileStepDownHalf(void);
	void handleScoreFileStepUpSingle(void);
	void handleScoreFileStepDownSingle(void);
	void handleScoreFileSetSharp(void);
	void handleScoreFileSetFlat(void);
	void handleScoreFileScale(void);
	void handleScoreFileExport(void);
	void handleScoreFileExportNoChords(void);
	void handleScrollChanged(int value);
	void handleScoreFileReplaceAll(void);
};

#endif		/* _MIDIPP_SCORES_H_ */
