/*-
 * Copyright (c) 2009-2013 Hans Petter Selasky. All rights reserved.
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

	void decrementDuration(uint32_t timeout = 0);
	void handleLabelJump(int label);
	void handleChordsLoad(void);
	void handleMidiKeyPressLocked(int chan, int key, int vel);
	void handleMidiKeyReleaseLocked(int chan, int key);
	void handleKeyPressChord(int key, int vel, uint32_t key_delay = 0);
	void handleKeyPressureChord(int key, int vel, uint32_t key_delay = 0);
	void handleKeyReleaseChord(int key, uint32_t key_delay = 0);
	void handleKeyPressSub(int, int, uint32_t, int, int);
	void handleKeyPress(int key, int vel, uint32_t key_delay = 0);
	void handleKeyRelease(int key, uint32_t key_delay = 0);
	void handleParse(const QString &ps);
	uint8_t handleKeyRemovePast(MppScoreEntry *pn, uint32_t key_delay = 0);
	void handleScoreFileOpenRaw(char *, uint32_t);
	void handlePrintSub(QPrinter *pd, QPoint orig, float scale_f);
	int handleScoreFileOpenSub(QString fname);
	uint16_t outputMaskGet(void);
	void outputControl(uint8_t ctrl, uint8_t val);
	void outputKeyPressure(uint8_t chan, uint8_t key, uint8_t pressure);
	void outputChanPressure(uint8_t pressure);
	void outputPitch(uint16_t val);
	int getCurrLabel(void);
	void handleScoreFileEffect(int, int, int);
	void handleEditLine(void);

	void viewPaintEvent(QPaintEvent *event);
	void viewMousePressEvent(QMouseEvent *e);
	void locateVisual(MppElement *, int *, MppVisualDot **);

	void watchdog();

	int checkLabelJump(int label);
	int checkHalfPassThru(int key);

	int setPressedKey(int chan, int out_key, int dur, int delay);

	QWidget viewWidget;

	MppHead head;

	uint8_t auto_zero_start[0];

	MppVisualScore *pVisual;

	QGridLayout *viewGrid;
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
	MppSpinBox *spnScoreFileAlign;
	QPushButton *butScoreFileScale;
	QSpinBox *spnScoreFileScale;
	QPushButton *butScoreFileStepUp;
	QPushButton *butScoreFileStepDown;
	QPushButton *butScoreFileSetSharp;
	QPushButton *butScoreFileSetFlat;
	QPushButton *butScoreFileExport;
	QPushButton *butScoreFileExportNoChords;
	MppButton *butScoreFileAutoMel[2];
	QPushButton *butScoreFileReplaceAll;

	QString *currScoreFileName;
	QLabel *lblFileStatus;
	QPicture *picChord[2];

	MppScoreEntry score_past[MPP_MAX_CHORD_MAP];
	MppScoreEntry score_pressure[MPP_MAX_CHORD_MAP];
	MppScoreEntry score_future_base[12];
	MppScoreEntry score_future_treble[12];

	int visual_max;
	int visual_p_max;
	int visual_y_max;
	int unit;

	uint32_t pressedKeys[MPP_PRESSED_MAX];
	uint32_t frozenKeys[MPP_PRESSED_MAX];

	int picScroll;
	uint16_t active_channels;

	int8_t inputChannel;
	uint8_t synthChannel;
	int8_t synthChannelBase;
	int8_t synthChannelTreb;
	uint8_t baseKey;
	uint8_t delayNoise;
	uint8_t whatPlayKeyLocked;
	uint8_t last_key;
	uint8_t last_vel;
	uint8_t keyMode;
	uint8_t pressed_future;
	uint8_t num_base;
	uint8_t chordContrast;
	uint8_t chordNormalize;
	uint8_t songEventsOn;

	uint8_t auto_zero_end[0];

	QString editText;

public slots:

	int handleCompile(int force = 0);
	void handleScoreFileNew(int invisible = 0);
	void handleScoreFileOpen();
	void handleScoreFileSave();
	void handleScoreFileSaveAs();
	void handleScorePrint();
	void handleScoreFileAlign(void);
	void handleScoreFileStepUp(void);
	void handleScoreFileStepDown(void);
	void handleScoreFileSetSharp(void);
	void handleScoreFileSetFlat(void);
	void handleScoreFileScale(void);
	void handleScoreFileAutoMelody(int);
	void handleScoreFileExport(void);
	void handleScoreFileExportNoChords(void);
	void handleScrollChanged(int value);
	void handleScoreFileReplaceAll(void);
};

#endif		/* _MIDIPP_SCORES_H_ */
