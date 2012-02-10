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

#ifndef _MIDIPP_SCORES_H_
#define	_MIDIPP_SCORES_H_

#include <midipp.h>

class MppScoreView : public QWidget
{

public:
	MppScoreView(MppScoreMain *parent);

	void paintEvent(QPaintEvent *event);
	void mousePressEvent(QMouseEvent *e);

	MppScoreMain *pScores;
};

class MppScoreMain : public QObject
{
	Q_OBJECT;

public:

	MppScoreMain(MppMainWindow *parent);
	~MppScoreMain();

	void decrementDuration(uint32_t timeout = 0);
	void handleLabelJump(int label);
	void handleChordsLoad(void);
	void handleKeyPressChord(int key, int vel, uint32_t key_delay = 0);
	void handleKeyReleaseChord(int key, uint32_t key_delay = 0);
	void handleKeyPress(int key, int vel, uint32_t key_delay = 0);
	void handleKeyRelease(int key, uint32_t key_delay = 0);
	void handleParse(const QString &ps);
	void handleParseSub(QPrinter *pd, QPoint orig, float scale_f);
	int handleScoreFileOpenSub(QString fname);
	QString handleTextTranspose(const QString &str, int level, int sharp);
	const char *baseKeyToString(int key, int sharp);

	void viewPaintEvent(QPaintEvent *event);
	void viewMousePressEvent(QMouseEvent *e);

	char getChar(uint32_t offset);
	int32_t getIntValue(uint32_t offset);
	void parseAdv(uint8_t delta);
	void parseMax(uint16_t *pmax, float value);
	void resetAutoMelody(void);
	void computeAutoMelody(void);
	void newLine(uint8_t force, uint8_t label, uint8_t new_page);
	void newVisual();
	void watchdog();
	uint32_t resolveJump(uint32_t line);
	uint32_t resolveDuration(uint32_t line, uint32_t line_end, uint8_t dur);

	int checkLabelJump(int label);
	int checkHalfPassThru(int key);

	int setPressedKey(int chan, int out_key, int dur, int delay);

	QWidget viewWidget;

	uint8_t auto_zero_start[0];

	QGridLayout *viewGrid;
	QScrollBar *viewScroll;
	MppScoreView *viewWidgetSub;

	MppMainWindow *mainWindow;
	QPlainTextEdit *editWidget;
	QPushButton *butScoreFileNew;
	QPushButton *butScoreFileOpen;
	QPushButton *butScoreFileSave;
	QPushButton *butScoreFileSaveAs;
	QPushButton *butScoreFilePrint;
	QPushButton *butScoreFileStepUp;
	QPushButton *butScoreFileStepDown;
	QPushButton *butScoreFileSetSharp;
	QPushButton *butScoreFileSetFlat;
	QString *currScoreFileName;
	QLabel *lblFileStatus;
	QPicture *picChord[2];

	struct MppScoreEntry score_past[24];
	struct MppScoreEntry score_future[12];
	struct MppScoreEntry scores[MPP_MAX_LINES][MPP_MAX_SCORES];
	struct MppVisualScore visual[MPP_MAX_LINES];
	struct MppMergeEntry merge[MPP_MAX_MERGE];

	uint32_t pressedKeys[MPP_PRESSED_MAX];
	uint32_t realLine[MPP_MAX_LINES];
	uint32_t timer_ticks_pre[MPP_MAX_LINES];
	uint32_t timer_ticks_post[MPP_MAX_LINES];
	uint32_t devInputMask;

	uint16_t jumpTable[MPP_MAX_LABELS];
	uint16_t mousePressPos[MPP_MAX_LINES];
	uint16_t picMax;
	uint16_t linesMax;
	uint16_t currPos;
	uint16_t lastPos;
	uint16_t picScroll;
	uint16_t maxScoresWidth;
	uint16_t active_channels;

	uint8_t jumpLabel[MPP_MAX_LINES];
#define	MPP_JUMP_REL	0x80U
	uint8_t pageNext[MPP_MAX_LINES];
#define	MPP_CMD_NOP 0
#define	MPP_CMD_LOCK 1
#define	MPP_CMD_UNLOCK 2
	uint8_t playCommand[MPP_MAX_LINES];
	uint8_t synthChannel;
	uint8_t baseKey;
	uint8_t cmdKey;
	uint8_t delayNoise;
	uint8_t isPlayKeyLocked;
	uint8_t whatPlayKeyLocked;
	uint8_t last_key;
	uint8_t last_vel;
	uint8_t timer_was_active;
	uint8_t keyMode;
	uint8_t pressed_future;

protected:

	/* parse state */
	struct {
		uint8_t am_keys[12];
		const QString *ps;
		int x;
		int line;
		int index;
		int bufIndex;
		int realLine;
		int channel;
		int duration;
		int mindex;
		int am_steps;
		int am_mode;
	} ps;

	/* parse buffer */
	char bufData[512];

	uint8_t auto_zero_end[0];

	QString editText;

public slots:

	void handleCompile();
	void handleScoreFileNew();
	void handleScoreFileOpen();
	void handleScoreFileSave();
	void handleScoreFileSaveAs();
	void handleScorePrint();
	void handleScoreFileStepUp(void);
	void handleScoreFileStepDown(void);
	void handleScoreFileSetSharp(void);
	void handleScoreFileSetFlat(void);
	void handleScrollChanged(int value);
};

#endif		/* _MIDIPP_SCORES_H_ */
