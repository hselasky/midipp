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

static int
MppCompareValue(const void *pa, const void *pb)
{
	MppElement *ma = *((MppElement **)pa);
	MppElement *mb = *((MppElement **)pb);
	if (ma->value[0] > mb->value[0])
		return (1);
	if (ma->value[0] < mb->value[0])
		return (-1);
	return (0);
}

Q_DECL_EXPORT int
MppSpaceOnly(QString &str)
{
	int x;

	for (x = 0; x != str.size(); x++) {
		if (str[x] != ' ' && str[x] != '\t')
			return (0);
	}
	return (1);
}

Q_DECL_EXPORT int
MppIsChord(QString &str)
{
	int x;
	int retval = 0;

	if (str.size() > 1) {
		QChar ch = str[1];

		if (ch == 'A' ||
		    ch == 'B' ||
		    ch == 'C' ||
		    ch == 'D' ||
		    ch == 'E' ||
		    ch == 'F' ||
		    ch == 'G' ||
		    ch == 'H')
			retval = 1;
	}

	for (x = 0; x != str.size(); x++) {
		QChar ch = str[x];

		if (ch.isDigit() || ch.isLetter() ||
		    ch == '+' || ch == '#' ||
		    ch == '(' || ch == ')' ||
		    ch == '/')
			continue;

		retval = 0;
		break;
	}
	return (retval);
}

Q_DECL_EXPORT QString
MppDeQuoteChord(QString &str)
{
	int n = str.size();
	if (n >= 2) {
		if (str[0] == '(' && str[n - 1] == ')') {
			return (str.mid(1, n - 2));
		}
	}
	return (str);
}

Q_DECL_EXPORT QString
MppDeQuoteString(QString &str)
{
	int n = str.size();
	if (n >= 3) {
		if (str[0] == 'S' && str[1] == '"' &&
		    str[n - 1] == '"') {
			return (str.mid(2, n - 3));
		}
	}
	return (str);
}

MppElement :: MppElement(MppElementType _type, int _line, int v0, int v1, int v2)
{
	memset(&entry, 0, sizeof(entry));
	type = _type;
	line = _line;
	sequence = 0;
	value[0] = v0;
	value[1] = v1;
	value[2] = v2;
}

MppElement :: ~MppElement()
{
}

QChar
MppElement :: getChar(int *poffset) const
{
	if (*poffset < txt.size())
		return (txt[(*poffset)++]);
	else
		return (' ');
}

int
MppElement :: getIntValue(int *poffset) const
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
	clear();
}

void
MppHead :: clear()
{
	MppElement *elem;

	while ((elem = TAILQ_FIRST(&head)) != 0) {
		TAILQ_REMOVE(&head, elem, entry);
		delete elem;
	}

	reset();
}

void
MppHead :: reset()
{
	MppElement *elem;

	delete (state.elem);
	last = ' ';
	memset(&state, 0, sizeof(state));

	TAILQ_FOREACH(elem, &head, entry) {
		if (elem->type == MPP_T_LABEL)
			state.label_start[elem->value[0]] = elem;
	}
}

static int
MppGetJumpFlags(QChar ch)
{
	if (ch == 'P' || ch == 'p')	/* new page */
		return (MPP_FLAG_JUMP_PAGE);
	if (ch == 'R' || ch == 'r')	/* relative jump */
		return (MPP_FLAG_JUMP_REL);
	if (ch.isDigit())
		return (MPP_FLAG_JUMP_DIGIT);
	return (0);
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
		case MPP_CMD_NUM_BASE:
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
		case MPP_CMD_AUTO_MELODY:
			ch = elem->getChar(&off);
			if (ch == '.')
				elem->value[1] = elem->getIntValue(&off);
			else
				elem->value[1] = 0;	/* disabled */
			break;
		case MPP_CMD_BPM_REF:
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
		while (1) {
			int last = off;
			int flags;
			ch = elem->getChar(&off);
			flags = MppGetJumpFlags(ch);
			elem->value[1] |= flags;
			if (flags == 0 ||
			    flags == MPP_FLAG_JUMP_DIGIT) {
				off = last;
				break;
			}
		}
		elem->value[0] = elem->getIntValue(&off);
		if (elem->value[0] < 0 || elem->value[0] >= MPP_MAX_LABELS)
			elem->value[0] = 0;
		break;

	case MPP_T_MACRO:
		elem->value[0] = elem->getIntValue(&off);
		if (elem->value[0] < 0 || elem->value[0] >= MPP_MAX_LABELS)
			elem->value[0] = 0;
		break;

	case MPP_T_LABEL:
		elem->value[0] = elem->getIntValue(&off);
		if (elem->value[0] < 0 || elem->value[0] >= MPP_MAX_LABELS)
			elem->value[0] = 0;
		break;

	case MPP_T_STRING_DOT:
		if (elem->txt.size() > 2 &&
		    elem->txt[0] == '.' && elem->txt[1] == '[') {
			off = 2;
			elem->value[0] = elem->getIntValue(&off);
		} else {
			elem->value[0] = 0;
		}
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
MppHead :: operator += (const QString &str)
{
	int x;

	for (x = 0; x != str.size(); x++)
		*this += str[x];
}

void
MppHead :: operator += (QChar ch)
{
	if (ch == QChar::ParagraphSeparator)
		ch = '\n';

	if (state.comment == 0 && state.string == 0) {
		if (state.command == 0) {
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
				state.elem = new MppElement(MPP_T_STRING_CMD, state.line);
			} else if (ch == 'W') {
				*this += state.elem;
				state.elem = new MppElement(MPP_T_TIMER, state.line);
			} else if (ch == 'X') {
				*this += state.elem;
				state.elem = new MppElement(MPP_T_TRANSPOSE, state.line);
			} else if (ch != ' ' && ch != '\t' && ch != '\r' &&
			    ch != '/' && ch != '\n' && ch != ';') {
				*this += state.elem;
				state.elem = new MppElement(MPP_T_UNKNOWN, state.line);
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
	if (state.comment == 0 && ch == '"') {
		state.string ^= 1;
		if (state.string != 0)
			state.level = 0;

		switch (state.elem->type) {
		case MPP_T_STRING_DESC:
		case MPP_T_STRING_DOT:
		case MPP_T_STRING_CHORD:
			*this += state.elem;
			state.elem = new MppElement(MPP_T_STRING_CMD, state.line);
			break;
		default:
			break;
		}
	}
	if (state.string == 0) {
		if (ch == '/' && last == '*') {
			state.comment--;
			state.command = 0;
		} else if (ch == '*' && last == '/') {
			state.comment++;
			state.command = 0;
		}
	} else {
		if (state.level == 0) {
			/* flush previous string type, if any */
			if (ch == '.') {
				*this += state.elem;
				state.elem = new MppElement(MPP_T_STRING_DOT, state.line);
			} else if (ch == '(') {
				*this += state.elem;
				state.elem = new MppElement(MPP_T_STRING_CHORD, state.line);
			} else if (ch == '[' || ch == '"') {
				/* ignore - same as previous */
			} else if (state.elem->type == MPP_T_STRING_CHORD ||
				   state.elem->type == MPP_T_STRING_DOT ||
				   state.elem->type == MPP_T_STRING_CMD) {
				*this += state.elem;
				state.elem = new MppElement(MPP_T_STRING_DESC, state.line);
			}
		}
		if (ch == '(' || ch == '[')
			state.level ++;
		else if (ch == ')' || ch == ']')
			state.level --;
	}
	state.elem->txt += ch;
	last = ch;
	if (ch == '\n')
		state.line++;
}

void
MppHead :: flush()
{
	*this += state.elem;
	state.elem = 0;

	reset();
}

QString
MppHead :: toPlain(int line)
{
	MppElement *elem;
	QString retval;

	TAILQ_FOREACH(elem, &head, entry) {
		if (line < 0 || elem->line == line)
			retval += elem->txt;
	}
	return (retval);
}

int
MppHead :: getPlaytime()
{
	MppElement *elem;
	int retval = 0;

	TAILQ_FOREACH(elem, &head, entry) {
		if (elem->type != MPP_T_TIMER)
			continue;
		retval += elem->value[0] + elem->value[1];
	}
	/* simple check for overflow */
	if (retval < 0)
		retval = 0;

	return (retval);
}

void
MppHead :: replace(MppHead *phead, MppElement *start, MppElement *stop)
{
	MppElement *ptr;

	if (start == 0)
		return;

	while ((ptr = TAILQ_FIRST(&phead->head)) != 0) {
		TAILQ_REMOVE(&phead->head, ptr, entry);
		TAILQ_INSERT_BEFORE(start, ptr, entry);
	}

	phead->clear();

	while (start != stop) {
		ptr = TAILQ_NEXT(start, entry);
		TAILQ_REMOVE(&head, start, entry);
		delete start;
		start = ptr;
	}

	reset();
}

int
MppHead :: getChord(int line, MppChordElement *pinfo)
{
	MppElement *ptr;
	MppElement *start;
	MppElement *stop;
	MppElement *string_start = 0;
	MppElement *string_stop = 0;
	int counter = 0;
	int x;
	int y;

	memset(pinfo, 0, sizeof(*pinfo));

	start = stop = 0;

	while (foreachLine(&start, &stop) != 0) {

		for (ptr = start; ptr != stop;
		    ptr = TAILQ_NEXT(ptr, entry)) {
			if (ptr->type == MPP_T_STRING_CHORD &&
			    MppIsChord(ptr->txt) != 0) {
				string_start = start;
				string_stop = stop;
				counter = 0;
				break;
			}
		}
		for (ptr = start; ptr != stop;
		    ptr = TAILQ_NEXT(ptr, entry)) {
			if (ptr->type == MPP_T_SCORE)
				break;
		}

		/* if no scores, continue */
		if (ptr == stop)
			continue;

		if (start->line == line) {
			/* compute pointer to chord info, if any */
			int dot_first = 0;
			int num_dot = 0;

			for (ptr = string_start; ptr != string_stop;
			     ptr = TAILQ_NEXT(ptr, entry)) {

				if (ptr->type == MPP_T_STRING_DOT) {
					if (dot_first == 0)
						dot_first = 1;
					num_dot++;
				} else if (ptr->type == MPP_T_STRING_CHORD) {
					if (dot_first == 0)
						dot_first = -1;
					if (dot_first == 1) {
						/* dot is before the chord */
						if ((num_dot - 1) == counter)
							break;
					} else {
						/* dot is after the chord */
						if (num_dot == counter)
							break;
					}
				}
			}
			if (ptr != string_stop)
				pinfo->chord = ptr;
			pinfo->start = start;
			pinfo->stop = stop;

			/* compute chord profile */
			for (ptr = start; ptr != stop;
			     ptr = TAILQ_NEXT(ptr, entry)) {
				int key;
				if (ptr->type != MPP_T_SCORE)
					continue;
				key = ptr->value[0];
				pinfo->stats[key % 12]++;
				if (key > pinfo->key_max)
					pinfo->key_max = key;
			}

			for (x = y = 0; x != 12; x++) {
				if (pinfo->stats[x] > pinfo->stats[y])
					y = x;
			}

			/*
			 * The key having the most hits typically is
			 * the base:
			 */
			pinfo->key_base = y;

			/* valid chord/score found */
			return (1);
		}
		counter++;
	}
	return (0);
}

void
MppHead :: optimise()
{
	MppElement *start;
	MppElement *stop;
	MppElement *ptr;
	MppElement *next;
	int duration = 1;
	int channel = 0;
	int has_score = 0;

	/* accumulate same duration and channel */
	TAILQ_FOREACH(ptr, &head, entry) {
		if (ptr->type == MPP_T_NEWLINE) {
			duration = 1;
			channel = 0;
		} else if (ptr->type == MPP_T_DURATION) {
			if (ptr->value[0] == duration) {
				ptr->type = MPP_T_UNKNOWN;
				ptr->txt = QString();
			} else {
				duration = ptr->value[0];
			}
		} else if (ptr->type == MPP_T_CHANNEL) {
			if (ptr->value[0] == channel) {
				ptr->type = MPP_T_UNKNOWN;
				ptr->txt = QString();
			} else {
				channel = ptr->value[0];
			}
		}
	}

	/* remove unused channel and duration */
	TAILQ_FOREACH_REVERSE(ptr, &head, MppElementHead, entry) {
		if (ptr->type == MPP_T_NEWLINE) {
			has_score = 0;
		} else if (ptr->type == MPP_T_DURATION) {
			if (has_score == 0) {
				ptr->type = MPP_T_UNKNOWN;
				ptr->txt = QString();
			}
		} else if (ptr->type == MPP_T_CHANNEL) {
			if (has_score == 0) {
				ptr->type = MPP_T_UNKNOWN;
				ptr->txt = QString();
			}
		} else if (ptr->type == MPP_T_SCORE ||
		    ptr->type == MPP_T_MACRO) {
			has_score = 1;
		}
	}

	/* accumulate spaces */
	TAILQ_FOREACH(ptr, &head, entry) {
		next = TAILQ_NEXT(ptr, entry);
		if (next != 0 && MppSpaceOnly(ptr->txt) != 0 &&
		    MppSpaceOnly(next->txt) != 0) {
			ptr->txt = QString();
			next->txt = QString(" ");
		}
	}

	for (start = stop = 0; foreachLine(&start, &stop); ) {
		/* remove space at beginning of each line */
		ptr = start;
		while (ptr != stop) {
			if (MppSpaceOnly(ptr->txt) != 0) {
				next = TAILQ_NEXT(ptr, entry);
				TAILQ_REMOVE(&head, ptr, entry);
				delete ptr;
				ptr = next;
			} else {
				break;
			}
		}
	}

	/* remove unused entries, except labels */
	for (ptr = TAILQ_FIRST(&head); ptr != NULL; ptr = next) {
		next = TAILQ_NEXT(ptr, entry);
		if (ptr->txt.size() != 0)
			continue;
		TAILQ_REMOVE(&head, ptr, entry);
		delete ptr;
	}

	reset();
}

void
MppHead :: autoMelody(int which)
{
	MppElement *ptr;
	MppElement *psc;
	MppElement *pdr;
	MppElement *start;
	MppElement *stop;
	MppElement *am_start;
	MppElement *am_stop;
	int duration;
	int steps;
	int num;
	int dup;
	int am_keys[12];
	int cur_keys[12];
	int sort_keys[12];
	int last = 0;
	int am_step;
	int x;
	int y;
	int key;

	memset(am_keys, 0, sizeof(am_keys));
	am_start = am_stop = 0;
	am_step = steps = 0;
	start = stop = 0;

	while (foreachLine(&start, &stop) != 0) {

		memset(cur_keys, 0, sizeof(cur_keys));

		duration = 1;

		for (dup = num = 0, ptr = start; ptr != stop;
		    ptr = TAILQ_NEXT(ptr, entry)) {
			if (ptr->type == MPP_T_SCORE) {
				int rem;

				last = ptr->value[0];
				rem = last % 12;
				if (cur_keys[rem] == 0)
					num++;
				else
					dup++;
				cur_keys[rem]++;
			} else if (ptr->type == MPP_T_JUMP) {
				/* reset auto melody */
				memset(am_keys, 0, sizeof(am_keys));
				am_start = am_stop = 0;
				am_step = 0;
			}
		}
		if (num != 0)
			steps++;

		if (num >= 3) {
			/* this looks like a chord */
			memcpy(am_keys, cur_keys, sizeof(am_keys));
			am_start = start;
			am_stop = stop;
			am_step = steps - 1;
			continue;
		} else if ((dup + num) != 1) {
			/* not a single score event */
			continue;
		}

		/* compute second score */
		memset(sort_keys, 0xFF, sizeof(sort_keys));

		/* gather candidates */
		for (x = 0; x != 12; x++) {
			int delta;
			int rem;

			if (am_keys[x] == 0 || cur_keys[x] != 0)
				continue;

			rem = last % 12;
			if (x > rem)
				delta = x - rem;
			else
				delta = rem - x;

			rem = x + last - rem;

			while (rem >= last)
				rem -= 12;

			sort_keys[delta] = rem;
		}

		/* decide closest candidate */
		for (x = y = 0; x != 12; x++) {
			if (sort_keys[x] > -1) {
				if (y == which)
					break;
				y++;
			}
		}

		/* add melody, if any */
		if (x == 12)
			continue;

		key = sort_keys[x];

		ptr = new MppElement(MPP_T_SCORE, start->line, key);
		ptr->txt = QString(mid_key_str[key]);

		psc = new MppElement(MPP_T_SPACE, start->line);
		psc->txt = QString(" ");

		TAILQ_INSERT_BEFORE(start, ptr, entry);
		TAILQ_INSERT_BEFORE(start, psc, entry);

		/* update stop pointer, if any */
		if (am_stop == start)
			am_stop = ptr;

		duration = 1;

		/* update duration of conflicting scores */
		for (ptr = am_start; ptr != am_stop;
		    ptr = TAILQ_NEXT(ptr, entry)) {
			if (ptr->type == MPP_T_SCORE) {
				if (ptr->value[0] != key)
					continue;
				if (duration < (2 * (steps - am_step - 1)))
					continue;

				psc = new MppElement(MPP_T_SPACE, am_start->line);
				psc->txt = QString(" ");

				pdr = new MppElement(MPP_T_DURATION, am_start->line, 1);
				pdr->txt = QString("U1");

				TAILQ_INSERT_BEFORE(ptr, pdr, entry);
				TAILQ_INSERT_BEFORE(ptr, psc, entry);

				/* update start pointer, if any */
				if (am_start == ptr)
					am_start = pdr;

				psc = new MppElement(MPP_T_SPACE, am_start->line);
				psc->txt = QString(" ");

				pdr = new MppElement(MPP_T_DURATION, am_start->line, 1);
				pdr->txt = QString("U%1").arg((duration + 1) / 2);
				if (!(duration & 1))
					pdr->txt += QChar('.');

				TAILQ_INSERT_AFTER(&head, ptr, psc, entry);
				TAILQ_INSERT_AFTER(&head, ptr, pdr, entry);

			} else if (ptr->type == MPP_T_DURATION) {
				duration = ptr->value[0];
			}
		}
	}
}

void
MppHead :: transposeScore(int adjust, int sharp)
{
	MppElement *ptr;
	int x;

	if (sharp == 0) {
		/* figure out sharp or flat */
		TAILQ_FOREACH(ptr, &head, entry) {

			if (ptr->type != MPP_T_STRING_CHORD)
				continue;			

			/* Chords should not contain spaces of any kind */
			if (MppIsChord(ptr->txt) == 0)
				continue;

			for (x = 0; x < ptr->txt.size() - 1; x++) {
				QChar ch = ptr->txt[x];
				QChar cn = ptr->txt[x + 1];

				if (ch == 'A' ||
				    ch == 'B' ||
				    ch == 'C' ||
				    ch == 'D' ||
				    ch == 'E' ||
				    ch == 'F' ||
				    ch == 'G' ||
				    ch == 'H') {
					if (cn == '#') {
						sharp++;
						x++;
					} else if (cn == 'b') {
						sharp--;
						x++;
					}
				}
			}
		}
	}

	/* convert to boolean */
	if (sharp >= 0)
		sharp = 1;	/* sharp */
	else
		sharp = 0;	/* flat */

	TAILQ_FOREACH(ptr, &head, entry) {

		if (ptr->type == MPP_T_SCORE) {
			ptr->value[0] += adjust;
			if (ptr->value[0] < 0 || ptr->value[0] > 127) {
				ptr->txt = QString();
				ptr->type = MPP_T_UNKNOWN;
			} else {
				ptr->txt = QString(mid_key_str[ptr->value[0]]);
			}
			continue;
		}

		if (ptr->type != MPP_T_STRING_CHORD)
			continue;

		/* Chords should not contain spaces of any kind */
		if (MppIsChord(ptr->txt) == 0)
			continue;

		QString out;

		for (x = 0; x < ptr->txt.size() - 1; x++) {
			int key;

			QChar ch = ptr->txt[x];
			QChar cn = ptr->txt[x + 1];

			if (ch == 'A')
				key = A0;
			else if (ch == 'B' || ch == 'H')
				key = H0;
			else if (ch == 'C')
				key = C0;
			else if (ch == 'D')
				key = D0;
			else if (ch == 'E')
				key = E0;
			else if (ch == 'F')
				key = F0;
			else if (ch == 'G')
				key = G0;
			else
				key = -1;

			if (key > -1) {
				/* can be negative */
				key += adjust % 12;

				if (cn == 'b') {
					key = (12 + key + 11) % 12;
					out += MppBaseKeyToString(key, sharp);
					x++;
				} else if (cn == '#') {
					key = (12 + key + 1) % 12;
					out += MppBaseKeyToString(key, sharp);
					x++;
				} else {
					key = (12 + key) % 12;
					out += MppBaseKeyToString(key, sharp);
				}
				continue;
			}
			out += ch;
		}
		/* last character too */
		out += ptr->txt[ptr->txt.size() - 1];

		/* put new text in place */
		ptr->txt = out;
	}
}

void
MppHead :: limitScore(int limit)
{
	MppElement *start;
	MppElement *stop;
	MppElement *ptr;
	MppElement *array[128];
	int num;
	int x;
	int y;
	int z;
	int temp;
	int map[12];

	if (limit < 0 || limit > 127)
		return;

	start = stop = 0;

	while (foreachLine(&start, &stop) != 0) {
		for (num = 0, ptr = start; ptr != stop;
		    ptr = TAILQ_NEXT(ptr, entry)) {
			if (ptr->type == MPP_T_SCORE && num < 128) {
				array[num++] = ptr;
			}
		}
		if (num == 0)
			continue;

		/* sort scores by value */

		qsort(array, num, sizeof(void *), MppCompareValue);

		/* setup map */

		for (x = 0; x != 12; x++)
			map[x] = limit;

		/* re-align */

		for (x = num - 1; x != -1; ) {

			/* get current score */
			temp = array[x]->value[0];
			y = temp % 12;

			/* compute length */

			for (z = x - 1; z != -1; z--) {
				if (array[x]->value[0] != array[z]->value[0])
					break;
			}

			/* adjust */

			while (temp < map[y])
				temp += 12;
			while (temp >= map[y])
				temp -= 12;

			/* range check */

			if (temp < 0 || temp > 127) {
				for (; x != z; x--) {
					array[x]->type = MPP_T_UNKNOWN;
					array[x]->value[0] = 0;
					array[x]->txt = QString();
				}
			} else {
				map[y] = temp;
				for (; x != z; x--) {
					array[x]->value[0] = temp;
					array[x]->txt = QString(mid_key_str[temp]);
				}
			}
		}
	}
}

void
MppHead :: alignTime(int align)
{
	MppElement *elem;
	int rem;

	if (align <= 0)
		return;

	TAILQ_FOREACH(elem, &head, entry) {
		if (elem->type != MPP_T_TIMER)
			continue;

		rem = elem->value[0] % align;
		if (rem < (align / 2))
			elem->value[0] -= rem;
		else
			elem->value[0] += (align - rem) % align;

		rem = elem->value[1] % align;
		if (rem < (align / 2))
			elem->value[1] -= rem;
		else
			elem->value[1] += (align - rem) % align;

		elem->txt = QString("W%1.%2").arg(elem->value[0]).arg(elem->value[1]);
	}
}

void
MppHead :: scaleTime(int max)
{
	MppElement *elem;
	int offset;
	int playtime;
	int last;

	if (max < 0)
		return;

	playtime = getPlaytime();
	if (playtime <= 0)
		return;

	last = 0;
	offset = 0;
	TAILQ_FOREACH(elem, &head, entry) {
		int curr;

		if (elem->type != MPP_T_TIMER)
			continue;

		offset += elem->value[0];
		curr = ((int64_t)offset * (int64_t)max) / (int64_t)playtime;
		elem->value[0] = curr - last;
		last = curr;

		offset += elem->value[1];
		curr = ((int64_t)offset * (int64_t)max) / (int64_t)playtime;
		elem->value[1] = curr - last;
		last = curr;

		elem->txt = QString("W%1.%2").arg(elem->value[0]).arg(elem->value[1]);
	}
}

int
MppHead :: foreachLine(MppElement **ppstart, MppElement **ppstop)
{
	MppElement *ptr;

	if (*ppstart == 0)
		ptr = *ppstart = *ppstop = TAILQ_FIRST(&head);
	else
		ptr = *ppstart = *ppstop;

	if (ptr == 0)
		return (0);

	for ( ; ptr != 0; ptr = TAILQ_NEXT(ptr, entry)) {
		if (ptr->type == MPP_T_NEWLINE ||
		    ptr->type == MPP_T_JUMP ||
		    ptr->type == MPP_T_LABEL) {
			ptr = TAILQ_NEXT(ptr, entry);
			break;
		}
	}
	*ppstop = ptr;

	return (1);
}

int
MppHead :: getMaxLines()
{
	return (state.line + 1);
}

QString
MppHead :: toLyrics(int no_chords)
{
	MppElement *start;
	MppElement *stop;
	MppElement *ptr;
	QString linebuf[2];
	QString out;

	for (start = stop = 0; foreachLine(&start, &stop); ) {

		linebuf[0] = QString();
		linebuf[1] = QString();

		for (ptr = start; ptr != stop;
		    ptr = TAILQ_NEXT(ptr, entry)) {
			switch (ptr->type) {
			case MPP_T_STRING_DESC:
				linebuf[1] += ptr->txt;
				break;
			case MPP_T_STRING_DOT:
				break;
			case MPP_T_STRING_CHORD:
				while (linebuf[1].size() > linebuf[0].size())
					linebuf[0] += ' ';
				linebuf[0] += MppDeQuoteChord(ptr->txt);
				linebuf[0] += ' ';
				break;
			default:
				break;
			}
		}

		/* Export Chord Line */
		if (no_chords == 0 && linebuf[0].size() > 0) {
			/* remove white space at end of line */
			out += linebuf[0].replace(QRegExp("\\s*$"), "");
			out += '\n';
		}

		/* Export Text Line */
		if (linebuf[1].size() > 0) {
			/* remove white space at end of line */
			out += linebuf[1].replace(QRegExp("\\s*$"), "");
			out += '\n';
		}
	}
	return (out);
}

void
MppHead :: toLyrics(QString *pstr)
{
	MppElement *start;
	MppElement *stop;
	MppElement *ptr;
	QString linebuf;
	int x;
	int label = 0;

	for (x = 0; x != MPP_MAX_LABELS; x++) {
		pstr[x] = QString();
	}

	for (start = stop = 0; foreachLine(&start, &stop); ) {

		linebuf = QString();

		for (ptr = start; ptr != stop;
		    ptr = TAILQ_NEXT(ptr, entry)) {
			switch (ptr->type) {
			case MPP_T_STRING_CHORD:
				if (MppIsChord(ptr->txt))
					break;
				linebuf += MppDeQuoteChord(ptr->txt) + '\n';
				break;
			case MPP_T_STRING_DESC:
				linebuf += ptr->txt;
				break;
			default:
				break;
			}
		}

		/* Export Text Line */
		if (linebuf.size() > 0) {
			/* Hide text lines starting with "L%d" */
			if ((linebuf.size() > 1) &&
			    (linebuf[0] == 'L') && linebuf[1].isDigit()) {
				label = 0;
				for (x = 1; (x != linebuf.size()) &&
				    (linebuf[x].isDigit()); x++) {
					label *= 10;
					label += linebuf[x].digitValue();
				}
				if (label < 0 || label >= MPP_MAX_LABELS)
					label = 0;
			} else {
				if (MppSpaceOnly(linebuf) == 0)
					pstr[label] += linebuf + '\n';
			}
		}
	}
}

int
MppHead :: isFirst()
{
	return (state.curr_start == state.last_start);
}

void
MppHead :: syncLast()
{
	state.last_start = state.curr_start;
	state.last_stop = state.curr_stop;
}

void
MppHead :: pushLine()
{
	state.push_start = state.curr_start;
	state.push_stop = state.curr_stop;
}

void
MppHead :: popLine()
{
	state.curr_start = state.push_start;
	state.curr_stop = state.push_stop;
}

void
MppHead :: stepLine(MppElement **ppstart, MppElement **ppstop)
{
	MppElement *ptr;
	int to = 8;

	while (to--) {
		while (foreachLine(&state.curr_start, &state.curr_stop)) {
			for (ptr = state.curr_start; ptr != state.curr_stop;
			    ptr = TAILQ_NEXT(ptr, entry)) {
				if (ptr->type == MPP_T_JUMP) {
					if (!(ptr->value[1] & MPP_FLAG_JUMP_DIGIT))
						continue;
					if (ptr->value[1] & MPP_FLAG_JUMP_REL)
						;
					else
						jumpLabel(ptr->value[0]);
					if (!to--)
						goto done;
					break;
				} else if (ptr->type == MPP_T_SCORE ||
				    ptr->type == MPP_T_MACRO) {
					goto done;
				} else if (ptr->type == MPP_T_COMMAND) {
					switch (ptr->value[0]) {
					case MPP_CMD_LOCK:
						state.key_lock = -1;
						break;
					case MPP_CMD_UNLOCK:
						state.key_lock = 0;
						break;
					default:
						break;
					}
				}
			}
		}
		state.did_jump = 1;
		state.key_lock = 0;
	}
done:
	*ppstart = state.curr_start;
	*ppstop = state.curr_stop;
}

void
MppHead :: currLine(MppElement **ppstart, MppElement **ppstop)
{
	/* check if after jump */
	if (state.curr_start == state.curr_stop) {
		stepLine(ppstart, ppstop);
		syncLast();
	} else {
		*ppstart = state.curr_start;
		*ppstop = state.curr_stop;
	}
}

void
MppHead :: jumpLabel(int label)
{
	if (label < 0 || label >= MPP_MAX_LABELS)
		return;

	jumpPointer(state.label_start[label]);
}

void
MppHead :: jumpPointer(MppElement *ptr)
{
	if (ptr == 0)
		return;

	state.did_jump = 1;
	state.key_lock = 0;
	state.curr_start = state.curr_stop = ptr;
}

void
MppHead :: sequence()
{
	MppElement *elem;
	int count = 0;

	TAILQ_FOREACH(elem, &head, entry) {
		elem->sequence = count;
		count++;
	}
}

/* must be called locked */
int
MppHead :: getCurrLine()
{
	MppElement *start;
	MppElement *stop;

	currLine(&start, &stop);

	if (start != 0)
		return (start->line);
	return (0);
}

void
MppHead :: dotReorder()
{
	MppElement *ptr;
	MppElement *next;

	TAILQ_FOREACH(ptr, &head, entry) {
		if (ptr->type != MPP_T_STRING_CHORD)
			continue;

		next = TAILQ_NEXT(ptr, entry);
		if (next == 0 || next->type != MPP_T_STRING_DOT)
			continue;

		/* move dot before chord */
		TAILQ_REMOVE(&head, next, entry);
		TAILQ_INSERT_BEFORE(ptr, next, entry);
	}
}

int
MppElement :: compare(const MppElement *other) const
{
	/* handle special case */
	if (this == 0 && other == 0)
		return (0);
	if (other == 0)
		return (-1);
	if (this == 0)
		return (1);

	/* compare sequence number */
	if (this->sequence > other->sequence)
		return (1);
	if (this->sequence < other->sequence)
		return (-1);
	return (0);
}
