/*-
 * Copyright (c) 2013 Hans Petter Selasky. All rights reserved.
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

#ifndef _MIDIPP_ELEMENT_H_
#define	_MIDIPP_ELEMENT_H_

#include "midipp.h"

class MppElement;
typedef TAILQ_ENTRY(MppElement) MppElementEntryT;
typedef TAILQ_HEAD(MppElementHead, MppElement) MppElementHeadT;

enum MppElementType {
	MPP_T_CHANNEL,
	MPP_T_COMMAND,
	MPP_T_DURATION,
	MPP_T_JUMP,
	MPP_T_LABEL,
	MPP_T_MACRO,
	MPP_T_NEWLINE,
	MPP_T_SCORE,
	MPP_T_SPACE,
	MPP_T_STRING_CMD,
	MPP_T_STRING_DESC,
	MPP_T_STRING_DOT,
	MPP_T_STRING_CHORD,
	MPP_T_TRANSPOSE,
	MPP_T_TIMER,
	MPP_T_UNKNOWN,
	MPP_T_MAX,
};

enum MppCommandType {
	MPP_CMD_NOP,
	MPP_CMD_LOCK,
	MPP_CMD_UNLOCK,
	MPP_CMD_BPM_REF,
	MPP_CMD_AUTO_MELODY,
	MPP_CMD_NUM_BASE,
	MPP_CMD_MAX,
};

#define	MPP_FLAG_JUMP_PAGE 1
#define	MPP_FLAG_JUMP_REL 2

struct MppChordElement {
	MppElement *chord;
	MppElement *start;
	MppElement *stop;
	int stats[12];
	int key_max;
	int key_base;
};

class MppElement {
public:
	MppElement(MppElementType type, int, int = 0, int = 0, int = 0);
	~MppElement();

	QChar getChar(int *);
	int getIntValue(int *);

	QString txt;

	MppElementEntryT entry;
	enum MppElementType type;
	int value[3];
	int line;
};

class MppHead {
public:
	MppElementHeadT head;

	QChar last;

	struct {
		int offset;
		int command;
		int comment;
		int line;
		int string;
		int level;
		MppElement *curr_start;
		MppElement *curr_stop;
		MppElement *elem;
		MppElement *label_start[MPP_MAX_LABELS];
	} state;

	MppHead();
	~MppHead();

	void move(MppHead *, MppElement *, MppElement *);
	int getChord(int, struct MppChordElement *);
	void optimise(void);
	void autoMelody(int);
	void transposeScore(int, int = 0);
	void limitScore(int);
	void scaleTime(int);
	void alignTime(int);
	int getPlaytime();
	void flush();
	QString toPlain(int = -1);
	QString toLyrics();
	int foreachLine(MppElement **, MppElement **);
	int getMaxLines();
	void stepLine(MppElement **, MppElement **);
	void currLine(MppElement **, MppElement **);
	void jumpLabel(int);
	void jumpPointer(MppElement *);

	void operator += (QChar);
	void operator += (const QString &);
	void operator += (MppElement *);
};

extern int MppSpaceOnly(QString &);
extern int MppHasSpace(QString &);
extern QString MppDeQuoteChord(QString &);
extern QString MppDeQuoteString(QString &);

#endif			/* _MIDIPP_ELEMENT_H_ */
