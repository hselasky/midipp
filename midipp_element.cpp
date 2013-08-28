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

#include <string.h>

#include "midipp_element.h"

MppElement :: MppElement(MppElementType type, int line, int v0, int v1, int v2)
{
	memset(&entry, 0, sizeof(entry));
	type = type;
	line = line;
	value[0] = v0;
	value[1] = v1;
	value[2] = v2;
}

MppElement :: ~MppElement()
{
}

QChar
MppElement :: getChar(int *poffset)
{
	if (*poffset < txt.size())
		return (txt[(*poffset)++]);
	else
		return (' ');
}

int
MppElement :: getIntValue(int *poffset)
{
	QChar ch;
	int value;
	int neg;
	int last;

	last = *poffset;
	ch = getChar(poffset);

	/* check sign, if any */
	if (ch == '-') {
		neg = 1;
		last = *poffset;
		ch = getChar(poffset);
	} else if (ch == '+') {
		neg = 0;
		last = *poffset;
		ch = getChar(poffset);
	} else {
		neg = 0;
	}

	value = 0;

	while (ch.isNumber()) {
		value *= 10;
		value += ch.digitValue();

		last = *poffset;
		ch = getChar(poffset);
	}

	/* rewind last character */
	*poffset = last;

	if (neg)
		value = -value;

	return (value);
}

MppHead :: MppHead()
{
	TAILQ_INIT(&head);
	last = ' ';
	memset(&state, 0, sizeof(state));
}

MppHead :: ~MppHead()
{
	MppElement *elem;

	while ((elem = TAILQ_FIRST(&head)) != 0) {
		TAILQ_REMOVE(&head, elem, entry);
		delete elem;
	}
}

void
MppHead :: operator += (MppElement *elem)
{
	QChar ch;
	int off;

	if (elem == 0)
		return;

	off = 1;

	switch (elem->type) {
	case MPP_T_CHANNEL:
		elem->value[0] = elem->getIntValue(&off);
		if (elem->value[0] < 0)
			elem->value[0] = 0;
		else if (elem->value[0] > 15)
			elem->value[0] = 15;
		break; 

	case MPP_T_COMMAND:
		elem->value[0] = elem->getIntValue(&off);
		switch (elem->value[0]) {
		case 5:
			ch = elem->getChar(&off);
			if (ch == '.') {
				elem->value[1] = elem->getIntValue(&off);
				if (elem->value[1] > 12)
					elem->value[1] = 12;
				else if (elem->value[1] < 0)
					elem->value[1] = 0;
			} else {
				elem->value[1] = 2;
			}
			break;
		case 4:
			ch = elem->getChar(&off);
			if (ch == '.')
				elem->value[1] = elem->getIntValue(&off);
			else
				elem->value[1] = 0;	/* disabled */
			break;
		case 3:
			ch = elem->getChar(&off);
			if (ch == '.') {
				elem->value[1] = elem->getIntValue(&off);
				ch = elem->getChar(&off);
				if (ch == '.')
					elem->value[2] = elem->getIntValue(&off);
				else
					elem->value[2] = 0; /* per */
			} else {
				elem->value[1] = 120;	/* ref */
			}
			if (elem->value[1] < 1)
				elem->value[1] = 1;
			else if (elem->value[1] > 6000)
				elem->value[1] = 6000;
			if (elem->value[2] < 0)
				elem->value[2] = 0;
			else if (elem->value[2] > 60000)
				elem->value[2] = 60000;
			break;
		default:
			break;
		}
		break;

	case MPP_T_DURATION:
		elem->value[0] = 2 * elem->getIntValue(&off);
		ch = elem->getChar(&off);
		if (ch != '.' && elem->value[0] > 0)
			elem->value[0]--;
		if (elem->value[0] < 0)
			elem->value[0] = 0;
		else if (elem->value[0] > 65535)
			elem->value[0] = 65535;
		break;

	case MPP_T_JUMP:
	case MPP_T_MACRO:
	case MPP_T_LABEL:
		elem->value[0] = elem->getIntValue(&off);
		if (elem->value[0] < 0 || elem->value[0] >= MPP_MAX_LABELS)
			elem->value[0] = 0;
		break;

	case MPP_T_SCORE:
		elem->value[0] += 12 * elem->getIntValue(&off);
		ch = elem->getChar(&off);
		if (ch == 'B' || ch == 'b')
			elem->value[0]--;
		if (elem->value[0] > 127)
			elem->value[0] = 127;
		else if (elem->value[0] < 0)
			elem->value[0] = 0;
		break;

	case MPP_T_STRING:
		break;

	case MPP_T_TRANSPOSE:
		elem->value[0] = elem->getIntValue(&off);
		break;

	case MPP_T_TIMER:
		elem->value[0] = elem->getIntValue(&off);
		if (elem->value[0] < 0)
			elem->value[0] = 0;
		else if (elem->value[0] > 0xffffff)
			elem->value[0] = 0xffffff;
		ch = elem->getChar(&off);
		if (ch == '.') {
			elem->value[1] = elem->getIntValue(&off);
			if (elem->value[1] < 0)
				elem->value[1] = 0;
			else if (elem->value[1] > 0xffffff)
				elem->value[1] = 0xffffff;
		}
		break;
	default:
		break;
	}

	TAILQ_INSERT_TAIL(&head, elem, entry);
}

void
MppHead :: operator += (QString &str)
{
	int x;

	for (x = 0; x != str.size(); x++)
		*this += str[x];
}

void
MppHead :: operator += (QChar ch)
{
	if (state.comment == 0 && state.string == 0) {
		if (state.command == 0) {
			ch = ch.toUpper();
			if (ch == 'C') {
				*this += state.elem;
				state.elem = new MppElement(MPP_T_SCORE, state.line, C0);
			} else if (ch == 'D') {
				*this += state.elem;
				state.elem = new MppElement(MPP_T_SCORE, state.line, D0);
			} else if (ch == 'E') {
				*this += state.elem;
				state.elem = new MppElement(MPP_T_SCORE, state.line, E0);
			} else if (ch == 'F') {
				*this += state.elem;
				state.elem = new MppElement(MPP_T_SCORE, state.line, F0);
			} else if (ch == 'G') {
				*this += state.elem;
				state.elem = new MppElement(MPP_T_SCORE, state.line, G0);
			} else if (ch == 'A') {
				*this += state.elem;
				state.elem = new MppElement(MPP_T_SCORE, state.line, A0);
			} else if (ch == 'H' || ch == 'B') {
				*this += state.elem;
				state.elem = new MppElement(MPP_T_SCORE, state.line, H0);
			} else if (ch == 'T') {
				*this += state.elem;
				state.elem = new MppElement(MPP_T_CHANNEL, state.line);
			} else if (ch == 'K') {
				*this += state.elem;
				state.elem = new MppElement(MPP_T_COMMAND, state.line);
			} else if (ch == 'L') {
				*this += state.elem;
				state.elem = new MppElement(MPP_T_LABEL, state.line);
			} else if (ch == 'M') {
				*this += state.elem;
				state.elem = new MppElement(MPP_T_MACRO, state.line);
			} else if (ch == 'J') {
				*this += state.elem;
				state.elem = new MppElement(MPP_T_JUMP, state.line);
			} else if (ch == 'U') {
				*this += state.elem;
				state.elem = new MppElement(MPP_T_DURATION, state.line);
			} else if (ch == 'S') {
				*this += state.elem;
				state.elem = new MppElement(MPP_T_STRING, state.line);
			} else if (ch == 'W') {
				*this += state.elem;
				state.elem = new MppElement(MPP_T_TIMER, state.line);
			} else if (ch == 'X') {
				*this += state.elem;
				state.elem = new MppElement(MPP_T_TRANSPOSE, state.line);
			}
		}
		if (ch == '\n') {
			*this += state.elem;
			state.elem = new MppElement(MPP_T_NEWLINE, state.line);
			state.command = 0;
		} else if (ch == ';') {
			*this += state.elem;
			state.elem = new MppElement(MPP_T_NEWLINE, state.line);
			state.command = 0;
		} else if (ch == ' ' || ch == '\t' || ch == '\r' || ch == '/') {
			if (state.elem != 0 && state.elem->type != MPP_T_SPACE) {
				*this += state.elem;
				state.elem = 0;
			}
			state.command = 0;
		} else {
			state.command = 1;
		}
	}
	if (state.elem == 0)
		state.elem = new MppElement(MPP_T_SPACE, state.line);
	if (state.comment == 0 && ch == '"')
		state.string ^= 1;
	if (state.string == 0) {
		if (ch == '/' && last == '*') {
			state.comment--;
			state.command = 0;
		} else if (ch == '*' && last == '/') {
			state.comment++;
			state.command = 0;
		}
	}
	state.elem->txt += ch;
	last = ch;
	if (ch == '\n')
		state.line++;
}

void
MppHead :: flush()
{
	if (state.elem != 0) {
		*this += state.elem;
		state.elem = 0;
	}
}

QString
MppHead :: toPlain()
{
	MppElement *elem;
	QString retval;

	TAILQ_FOREACH(elem, &head, entry)
		retval += elem->txt;

	return (retval);
}
