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

	void decrementDuration();
	void handleLabelJump(int label);
	void handleKeyPress(int key, int vel);
	void handleKeyRelease(int key);
	void handleParse(const QString &ps);
	void handleParseSub(QPrinter *pd, QPoint orig, float scale_f);
	void handleAutoPlay(int bpm);
	void handleStartTimer(uint32_t ticks_ms);
	void handleStopTimer(void);
	void handleTimerPress();

	void viewPaintEvent(QPaintEvent *event);
	void viewMousePressEvent(QMouseEvent *e);

	void parseAdv(uint8_t delta);
	void parseMax(uint16_t *pmax, float value);
	void newLine();
	void newVisual();
	void updateTimer();
	void watchdog();
	uint32_t resolveJump(uint32_t line);

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
	QString *currScoreFileName;
	QLabel *lblFileStatus;

	struct MppScoreEntry scores[MPP_MAX_LINES][MPP_MAX_SCORES];
	struct MppVisualScore visual[MPP_MAX_LINES];

	uint32_t bpmAutoPlay;
	uint32_t pressedKeys[MPP_PRESSED_MAX];
	uint32_t realLine[MPP_MAX_LINES];
	uint32_t timer_ticks[MPP_MAX_LINES];

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
#define	MPP_JUMP_NOP	0xFFU
	uint8_t pageNext[MPP_MAX_LINES];
#define	MPP_CMD_NOP 0
#define	MPP_CMD_LOCK 1
#define	MPP_CMD_UNLOCK 2
	uint8_t playCommand[MPP_MAX_LINES];
	uint8_t synthChannel;
	uint8_t baseKey;
	uint8_t delayNoise;
	uint8_t isPlayKeyLocked;
	uint8_t whatPlayKeyLocked;
	uint8_t last_key;
	uint8_t last_vel;

protected:

	/* parse state */
	struct {
		const QString *ps;
		int x;
		int line;
		int index;
		int bufIndex;
		int realLine;
		int channel;
		int duration;
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
	void handleChannelChanged(int chan);
	void handleScrollChanged(int value);
};

#endif		/* _MIDIPP_SCORES_H_ */
