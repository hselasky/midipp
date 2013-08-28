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
typedef TAILQ_HEAD(,MppElement) MppElementHeadT;

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
	MPP_T_STRING,
	MPP_T_TRANSPOSE,
	MPP_T_TIMER,
	MPP_T_MAX,
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
		MppElement *elem;
	} state;

	MppHead();
	~MppHead();

	void flush();
	QString toPlain();

	void operator += (QChar);
	void operator += (QString &);
	void operator += (MppElement *);
};

#endif			/* _MIDIPP_ELEMENT_H_ */
