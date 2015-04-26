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
typedef struct
#define struct 
TAILQ_ENTRY(MppElement) MppElementEntryT;
#undef struct
typedef struct
#define struct
TAILQ_HEAD(MppElementHead, MppElement) MppElementHeadT;
#undef struct

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
#define	MPP_FLAG_JUMP_DIGIT 4

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

	int compare(const MppElement *) const;

	QChar getChar(int *) const;
	int getIntValue(int *) const;

	QString txt;

	MppElementEntryT entry;
	enum MppElementType type;
	int value[3];
	int line;
	int sequence;
};

class MppHead {
public:
	MppElementHeadT head;

	QChar last;

	struct {
		int command;
		int comment;
		int did_jump;
		int key_lock;
		int level;
		int line;
		int offset;
		int string;
		MppElement *push_start;
		MppElement *push_stop;
		MppElement *curr_start;
		MppElement *curr_stop;
		MppElement *last_start;
		MppElement *last_stop;
		MppElement *elem;
		MppElement *label_start[MPP_MAX_LABELS];
	} state;

	MppHead();
	~MppHead();

	void replace(MppHead *, MppElement *, MppElement *);
	int getChord(int, MppChordElement *);
	void reset();
	void clear();
	void optimise();
	void autoMelody(int);
	void transposeScore(int, int = 0);
	void limitScore(int);
	void scaleTime(int);
	void alignTime(int);
	void dotReorder();
	int getPlaytime();
	void flush();
	QString toPlain(int = -1);
	QString toLyrics(int no_chords = 0);
	void toLyrics(QString *);
	int foreachLine(MppElement **, MppElement **);
	int getMaxLines();
	int isFirst();
	void syncLast();
	void pushLine();
	void popLine();
	void stepLine(MppElement **, MppElement **);
	void currLine(MppElement **, MppElement **);
	void jumpLabel(int);
	void jumpPointer(MppElement *);
	void sequence();
	int getCurrLine();

	void operator += (QChar);
	void operator += (const QString &);
	void operator += (MppElement *);
};

extern int MppSpaceOnly(QString &);
extern int MppIsChord(QString &);
extern QString MppDeQuoteChord(QString &);
extern QString MppDeQuoteString(QString &);

#endif			/* _MIDIPP_ELEMENT_H_ */
