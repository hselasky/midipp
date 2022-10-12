/*-
 * Copyright (c) 2013-2022 Hans Petter Selasky
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
#include <stdlib.h>

#include "midipp_element.h"
#include "midipp_chords.h"
#include "midipp_decode.h"

static int
MppNormSpace(MppElement *ptr, int last)
{
	if (last == MPP_T_SPACE && ptr->type == MPP_T_SPACE)
		ptr->txt = ptr->txt.trimmed();
	else if (last == MPP_T_UNKNOWN && ptr->type == MPP_T_SPACE)
		ptr->txt = ptr->txt.trimmed();
	else if (last != MPP_T_SPACE && ptr->type == MPP_T_SPACE)
		ptr->txt = QString(" ") + ptr->txt.trimmed();
	return (ptr->type);
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

MppElement :: MppElement(MppElementType _type, int _line,
    int v0, int v1, int v2, int v3)
{
	memset(&entry, 0, sizeof(entry));
	type = _type;
	line = _line;
	sequence = 0;
	value[0] = v0;
	value[1] = v1;
	value[2] = v2;
	value[3] = v3;
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
	state.text_curr.reset();
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
	state.text_curr.reset();

	TAILQ_FOREACH(elem, &head, entry) {
		switch (elem->type) {
		case MPP_T_LABEL:
			state.label_start[elem->value[0]] = elem;
			break;
		default:
			break;
		}
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
	int x;

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
		case MPP_CMD_IMAGE_PROPS:
			for (x = 1; x != 4; x++) {
				ch = elem->getChar(&off);
				if (ch == '.') {
					elem->value[x] = elem->getIntValue(&off);
					if (elem->value[x] > 65535)
						elem->value[x] = 65535;
					else if (elem->value[x] < 0)
						elem->value[x] = 0;
				} else {
					break;
				}
			}
			break;
		case MPP_CMD_TEXT_PROPS:
			for (x = 1; x != 4; x++) {
				ch = elem->getChar(&off);
				if (ch == '.') {
					elem->value[x] = elem->getIntValue(&off);
					if (elem->value[x] > 255)
						elem->value[x] = 255;
					else if (elem->value[x] < 0)
						elem->value[x] = 0;
				} else {
					break;
				}
			}
			break;
		case MPP_CMD_IMAGE_BG_COLOR:
		case MPP_CMD_IMAGE_FG_COLOR:
		case MPP_CMD_TEXT_BG_COLOR:
		case MPP_CMD_TEXT_FG_COLOR:
			for (x = 1; x != 4; x++) {
				ch = elem->getChar(&off);
				if (ch == '.') {
					elem->value[x] = elem->getIntValue(&off);
					if (elem->value[x] > 255)
						elem->value[x] = 255;
					else if (elem->value[x] < 0)
						elem->value[x] = 0;
				} else {
					break;
				}
			}
			break;
		case MPP_CMD_AUTO_MELODY:
		case MPP_CMD_MICRO_TUNE:
		case MPP_CMD_KEY_MODE:
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

	case MPP_T_SCORE_SUBDIV:
		elem->value[0] += 12 * elem->getIntValue(&off);
		ch = elem->getChar(&off);
		if (ch == 'B' || ch == 'b') {
			elem->value[0] -= 1;
			ch = elem->getChar(&off);
		}
		elem->value[0] *= MPP_BAND_STEP_12;
		if (ch == '.') {
			int rem = elem->getIntValue(&off);
			rem = MPP_SUBDIV_REM_BITREV(rem);
			elem->value[0] += rem;
		}
		break;

	case MPP_T_TRANSPOSE:
		elem->value[0] = elem->getIntValue(&off);
		elem->value[0] *= MPP_BAND_STEP_12;
		ch = elem->getChar(&off);
		if (ch == '.') {
			int rem = elem->getIntValue(&off);
			rem = MPP_SUBDIV_REM_BITREV(rem);
			elem->value[0] += rem;
			ch = elem->getChar(&off);
		}
		if (ch == '.') {
			elem->value[1] = elem->getIntValue(&off);
		}
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
				state.elem = new MppElement(MPP_T_SCORE_SUBDIV, state.line, MPP_C0);
			} else if (ch == 'D') {
				*this += state.elem;
				state.elem = new MppElement(MPP_T_SCORE_SUBDIV, state.line, MPP_D0);
			} else if (ch == 'E') {
				*this += state.elem;
				state.elem = new MppElement(MPP_T_SCORE_SUBDIV, state.line, MPP_E0);
			} else if (ch == 'F') {
				*this += state.elem;
				state.elem = new MppElement(MPP_T_SCORE_SUBDIV, state.line, MPP_F0);
			} else if (ch == 'G') {
				*this += state.elem;
				state.elem = new MppElement(MPP_T_SCORE_SUBDIV, state.line, MPP_G0);
			} else if (ch == 'A') {
				*this += state.elem;
				state.elem = new MppElement(MPP_T_SCORE_SUBDIV, state.line, MPP_A0);
			} else if (ch == 'H' || ch == 'B') {
				*this += state.elem;
				state.elem = new MppElement(MPP_T_SCORE_SUBDIV, state.line, MPP_H0);
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
			} else if (ch == 'V') {
				*this += state.elem;
				state.elem = new MppElement(MPP_T_COMMENT_CMD, state.line);
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

		/* Flush at end of string, if any. */
		switch (state.elem->type) {
		case MPP_T_STRING_DESC:
		case MPP_T_STRING_DOT:
		case MPP_T_STRING_CHORD:
			*this += state.elem;
			state.elem = new MppElement(MPP_T_STRING_CMD, state.line);
			break;
		case MPP_T_COMMENT_DESC:
			*this += state.elem;
			state.elem = new MppElement(MPP_T_COMMENT_CMD, state.line);
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
			switch (state.elem->type) {
			case MPP_T_STRING_CHORD:
			case MPP_T_STRING_DOT:
			case MPP_T_STRING_CMD:
			case MPP_T_STRING_DESC:
				/* flush previous string type, if any */
				if (ch == '.') {
					*this += state.elem;
					state.elem = new MppElement(MPP_T_STRING_DOT, state.line);
				} else if (ch == '(') {
					*this += state.elem;
					state.elem = new MppElement(MPP_T_STRING_CHORD, state.line);
				} else if (ch == '[' || ch == '"') {
					/* ignore - same as previous */
				} else if (state.elem->type != MPP_T_STRING_DESC) {
					*this += state.elem;
					state.elem = new MppElement(MPP_T_STRING_DESC, state.line);
				}
				break;
			case MPP_T_COMMENT_CMD:
			case MPP_T_COMMENT_DESC:
				if (ch == '"') {
					/* ignore - same as previous */
				} else if (state.elem->type != MPP_T_COMMENT_DESC) {
					*this += state.elem;
					state.elem = new MppElement(MPP_T_COMMENT_DESC, state.line);
				}
				break;
			default:
				break;
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
		ptr = start->next();
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

	/* set default values */
	memset(pinfo, 0, sizeof(*pinfo));
	pinfo->key_max = MPP_KEY_MIN;

	start = stop = 0;

	while (foreachLine(&start, &stop) != 0) {

		for (ptr = start; ptr != stop; ptr = ptr->next()) {
			if (ptr->type == MPP_T_STRING_CHORD) {
				string_start = start;
				string_stop = stop;
				counter = 0;
				break;
			}
		}
		for (ptr = start; ptr != stop; ptr = ptr->next()) {
			if (ptr->type == MPP_T_SCORE_SUBDIV)
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
			     ptr = ptr->next()) {

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
			for (ptr = start; ptr != stop; ptr = ptr->next()) {
				if (ptr->type == MPP_T_SCORE_SUBDIV) {
					int key = ptr->value[0];
					if (key > pinfo->key_max)
						pinfo->key_max = key;
					pinfo->stats[MPP_BAND_REM(key, MPP_MAX_CHORD_BANDS)]++;
				}
			}

			for (x = y = 0; x != MPP_MAX_CHORD_BANDS; x++) {
				if (pinfo->stats[x] > pinfo->stats[y])
					y = x;
			}

			/*
			 * The key having the most hits typically is
			 * the base:
			 */
			pinfo->key_base = y * MPP_BAND_STEP_CHORD;

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
				ptr->txt = "";
			} else {
				duration = ptr->value[0];
			}
		} else if (ptr->type == MPP_T_CHANNEL) {
			if (ptr->value[0] == channel) {
				ptr->type = MPP_T_UNKNOWN;
				ptr->txt = "";
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
				ptr->txt = "";
			}
		} else if (ptr->type == MPP_T_CHANNEL) {
			if (has_score == 0) {
				ptr->type = MPP_T_UNKNOWN;
				ptr->txt = "";
			}
		} else if (ptr->type == MPP_T_SCORE_SUBDIV ||
		    ptr->type == MPP_T_MACRO) {
			has_score = 1;
		}
	}

	/* normalize spaces */
	for (start = stop = 0; foreachLine(&start, &stop); ) {
		int last = MPP_T_UNKNOWN;
		for (ptr = start; ptr != stop; ptr = ptr->next())
			last = MppNormSpace(ptr, last);
	}

	/* remove unused entries, except labels */
	for (ptr = TAILQ_FIRST(&head); ptr != NULL; ptr = next) {
		next = ptr->next();
		if (ptr->txt.isEmpty()) {
			TAILQ_REMOVE(&head, ptr, entry);
			delete ptr;
		}
	}

	reset();
}

void
MppHead :: bassOffset(int which)
{
	MppElement *ptr;
	MppElement *start;
	MppElement *stop;
	int score[24];
	int base[24];
	int key[24];
	uint8_t ns;
	uint8_t nb;
	uint8_t nk;
	uint8_t x;

	start = stop = 0;
	
	while (foreachLine(&start, &stop) != 0) {

		ns = 0;

		for (ptr = start; ptr != stop;
		    ptr = ptr->next()) {
			if (ptr->type == MPP_T_SCORE_SUBDIV) {
				if (ns < 24)
					score[ns++] = ptr->value[0];
			}
		}

		if (ns == 0)
			continue;

		MppSort(score, ns);
		MppSplitBaseTreble(score, ns, base, &nb, key, &nk);

		if (nb == 0)
			continue;

		for (ptr = start; ptr != stop;
		    ptr = ptr->next()) {
			if (ptr->type != MPP_T_SCORE_SUBDIV)
				continue;
			for (x = 0; x != nb; x++) {
				if (ptr->value[0] != base[x])
					continue;
				if ((base[0] % MPP_MAX_CHORD_BANDS) !=
				    (base[x] % MPP_MAX_CHORD_BANDS)) {
					/* remove all bass scores except first one */
					ptr->type = MPP_T_UNKNOWN;
					ptr->txt = "";
				} else if (which != 0) {
					MppElement *psa;
					MppElement *psb;

					psa = new MppElement(MPP_T_SPACE, start->line);
					psa->txt = QString(" ");

					psb = new MppElement(MPP_T_SCORE_SUBDIV,
					    start->line, ptr->value[0] + which);
					psb->txt = MppKeyStr(ptr->value[0] + which);

					TAILQ_INSERT_AFTER(&head, ptr, psb, entry);
					TAILQ_INSERT_AFTER(&head, ptr, psa, entry);

					ptr = psb;
				}
				break;
			}
		}
	}
	sortScore();
}

struct MppKeyInfo {
	int channel;
	int duration;
	int key;
};

static int
MppCompareKeyInfo(void *arg, const void *pa, const void *pb)
{
	MppKeyInfo *ma = (MppKeyInfo *)pa;
	MppKeyInfo *mb = (MppKeyInfo *)pb;

	if (ma->channel > mb->channel)
		return (1);
	if (ma->channel < mb->channel)
		return (-1);
	if (ma->duration > mb->duration)
		return (1);
	if (ma->duration < mb->duration)
		return (-1);
	if (ma->key > mb->key)
		return (1);
	if (ma->key < mb->key)
		return (-1);
	return (0);
}

void
MppHead :: sortScore()
{
	MppElement *start;
	MppElement *stop;
	MppElement *ptr;
	int duration;
	int channel;

	start = stop = 0;

	while (foreachLine(&start, &stop) != 0) {
		struct MppKeyInfo *mk;
		size_t num = 0;
		size_t i;

		for (ptr = start; ptr != stop; ptr = ptr->next()) {
			switch (ptr->type) {
			case MPP_T_SCORE_SUBDIV:
				num++;
				break;
			default:
				break;
			}
		}

		if (num == 0)
			continue;

		mk = (struct MppKeyInfo *)malloc(num * sizeof(*mk));
		if (mk == 0)
			continue;

		channel = 0;
		duration = 1;
		num = 0;

		/* extract all keys */
		for (ptr = start; ptr != stop; ptr = ptr->next()) {
			switch (ptr->type) {
			case MPP_T_DURATION:
				duration = ptr->value[0];
				ptr->type = MPP_T_UNKNOWN;
				ptr->txt = "";
				break;
			case MPP_T_CHANNEL:
				channel = ptr->value[0];
				ptr->type = MPP_T_UNKNOWN;
				ptr->txt = "";
				break;
			case MPP_T_SCORE_SUBDIV:
				mk[num].channel = channel;
				mk[num].duration = duration;
				mk[num].key = ptr->value[0];
				num++;
				ptr->type = MPP_T_UNKNOWN;
				ptr->txt = "";
				break;
			default:
				break;
			}
		}

		MppSort(mk, num, sizeof(*mk), MppCompareKeyInfo, 0);

		/* output */

		channel = 0;
		duration = 1;

		for (i = 0; i != num; i++) {
			if (i != 0 && MppCompareKeyInfo(0, mk + i, mk + i - 1) == 0)
				continue;
			if (channel != mk[i].channel) {
				channel = mk[i].channel;
				ptr = new MppElement(MPP_T_CHANNEL, start->line, channel);
				ptr->txt = QString("T%1").arg(channel);
				TAILQ_INSERT_BEFORE(start, ptr, entry);

				ptr = new MppElement(MPP_T_SPACE, start->line);
				ptr->txt = QString(" ");
				TAILQ_INSERT_BEFORE(start, ptr, entry);
			}
			if (duration != mk[i].duration) {
				duration = mk[i].duration;
				ptr = new MppElement(MPP_T_DURATION, start->line, duration);
				ptr->txt = QString("U%1%2").arg((duration + 1) / 2)
				    .arg((duration & 1) ? "" : ".");
				TAILQ_INSERT_BEFORE(start, ptr, entry);

				ptr = new MppElement(MPP_T_SPACE, start->line);
				ptr->txt = QString(" ");
				TAILQ_INSERT_BEFORE(start, ptr, entry);
			}
			ptr = new MppElement(MPP_T_SCORE_SUBDIV, start->line, mk[i].key);
			ptr->txt = MppKeyStr(mk[i].key);
			TAILQ_INSERT_BEFORE(start, ptr, entry);

			ptr = new MppElement(MPP_T_SPACE, start->line);
			ptr->txt = QString(" ");
			TAILQ_INSERT_BEFORE(start, ptr, entry);
		}
		free(mk);
	}
}

void
MppHead :: transposeScore(int adjust, int sharp)
{
	MppElement *ptr;
	QString str;

	if (sharp == 0) {
		/* figure out sharp or flat */
		TAILQ_FOREACH(ptr, &head, entry) {
			if (ptr->type != MPP_T_STRING_CHORD)
				continue;
			/* If any sharp is seen, select sharp */
			if (ptr->txt.indexOf("#") > -1) {
				sharp = 1;
				break;
			}
		}
	}

	/* convert into boolean */
	sharp = (sharp > 0) ? 1 : 0;

	TAILQ_FOREACH(ptr, &head, entry) {
		switch (ptr->type) {
		case MPP_T_SCORE_SUBDIV:
			ptr->value[0] += adjust;
			ptr->txt = MppKeyStr(ptr->value[0]);
			break;
		case MPP_T_STRING_CHORD:
			str = "";

			for (int x = 0; x != ptr->txt.length(); x++) {
				QChar ch = ptr->txt[x];
				if (x == 0 && ch == '(')
					continue;
				if (x == ptr->txt.length() - 1 && ch == ')')
					continue;
				str += ch;
			}
			MppStepChordGeneric(str, adjust, sharp);
			ptr->txt = QString("(") + str + QString(")");
			break;
		default:
			break;
		}
	}
}

void
MppHead :: limitScore(int limit)
{
	MppElement *start = 0;
	MppElement *stop = 0;
	MppElement *ptr;

	while (foreachLine(&start, &stop)) {
		struct MppKeyInfo *mk;
		size_t num = 0;
		size_t i;
		size_t j;
		int channel;
		int duration;

		for (ptr = start; ptr != stop; ptr = ptr->next()) {
			if (ptr->type == MPP_T_SCORE_SUBDIV)
				num++;
		}

		if (num == 0)
			continue;

		mk = (struct MppKeyInfo *)malloc(num * sizeof(*mk));
		if (mk == 0)
			continue;
		channel = 0;
		duration = 1;
		num = 0;

		/* extract all keys */
		for (ptr = start; ptr != stop; ptr = ptr->next()) {
			switch (ptr->type) {
			case MPP_T_DURATION:
				duration = ptr->value[0];
				ptr->type = MPP_T_UNKNOWN;
				ptr->txt = "";
				break;
			case MPP_T_CHANNEL:
				channel = ptr->value[0];
				ptr->type = MPP_T_UNKNOWN;
				ptr->txt = "";
				break;
			case MPP_T_SCORE_SUBDIV:
				mk[num].channel = channel;
				mk[num].duration = duration;
				mk[num].key = ptr->value[0];
				num++;
				ptr->type = MPP_T_UNKNOWN;
				ptr->txt = "";
				break;
			default:
				break;
			}
		}

		/* import scores */
		for (i = 0; i != num; i++) {
			mk[i].key = MPP_BAND_REM(mk[i].key, MPP_MAX_BANDS) +
			    limit - MPP_BAND_REM(limit, MPP_MAX_BANDS);
			if (mk[i].key >= limit)
				mk[i].key -= MPP_MAX_BANDS;
		}

		MppSort(mk, num, sizeof(*mk), MppCompareKeyInfo, 0);

		/* figure out the duplicates and move them down */
		for (i = num; i--; ) {
			for (j = i; j--; ) {
				if (mk[i].channel != mk[j].channel)
					break;
				if (mk[i].key == mk[j].key)
					mk[j].key -= MPP_MAX_BANDS;
			}
		}

		MppSort(mk, num, sizeof(*mk), MppCompareKeyInfo, 0);

		/* output */

		channel = 0;
		duration = 1;

		for (i = 0; i != num; i++) {
			if (i != 0 && MppCompareKeyInfo(0, mk + i, mk + i - 1) == 0)
				continue;
			if (channel != mk[i].channel) {
				channel = mk[i].channel;
				ptr = new MppElement(MPP_T_CHANNEL, start->line, channel);
				ptr->txt = QString("T%1").arg(channel);
				TAILQ_INSERT_BEFORE(start, ptr, entry);

				ptr = new MppElement(MPP_T_SPACE, start->line);
				ptr->txt = QString(" ");
				TAILQ_INSERT_BEFORE(start, ptr, entry);
			}
			if (duration != mk[i].duration) {
				duration = mk[i].duration;
				ptr = new MppElement(MPP_T_DURATION, start->line, duration);
				ptr->txt = QString("U%1%2").arg((duration + 1) / 2)
				    .arg((duration & 1) ? "" : ".");
				TAILQ_INSERT_BEFORE(start, ptr, entry);

				ptr = new MppElement(MPP_T_SPACE, start->line);
				ptr->txt = QString(" ");
				TAILQ_INSERT_BEFORE(start, ptr, entry);
			}
			ptr = new MppElement(MPP_T_SCORE_SUBDIV, start->line, mk[i].key);
			ptr->txt = MppKeyStr(mk[i].key);
			TAILQ_INSERT_BEFORE(start, ptr, entry);

			ptr = new MppElement(MPP_T_SPACE, start->line);
			ptr->txt = QString(" ");
			TAILQ_INSERT_BEFORE(start, ptr, entry);
		}
		free(mk);
	}
}

void
MppHead :: tuneScore()
{
	MppElement *start = 0;
	MppElement *stop = 0;
	MppElement *ptr;

	while (foreachLine(&start, &stop)) {
		MppChord_t mask;
		int base = -1;
		int rem;
		int adjust[MPP_MAX_CHORD_BANDS] = {};

		mask.zero();

		/* extract all keys */
		for (ptr = start; ptr != stop; ptr = ptr->next()) {
			switch (ptr->type) {
			case MPP_T_SCORE_SUBDIV:
				rem = MPP_BAND_REM(ptr->value[0], MPP_MAX_CHORD_BANDS);
				/* first repeated remainder is the base */
				if (base == -1 && mask.test(rem))
					base = rem;
				mask.set(rem);
				adjust[rem] = rem * MPP_BAND_STEP_CHORD;
				break;
			default:
				break;
			}
		}

		/* if there are no keys, or base is undefined, skip */
		if (mask.order() == 0 || base == -1)
			continue;

		/* compute adjustment */
		for (int x = 0; x != MPP_MAX_CHORD_BANDS; x++) {
			if (mask.test(x) == 0)
				continue;
			rem = (MPP_MAX_CHORD_BANDS + x - base) % MPP_MAX_CHORD_BANDS;
			if (rem % (MPP_MAX_CHORD_BANDS / 12))
				continue;
			rem /= (MPP_MAX_CHORD_BANDS / 12);
			adjust[x] = x * MPP_BAND_STEP_CHORD + Mpp.KeyAdjust[rem];
		}

		/* update all keys */
		for (ptr = start; ptr != stop; ptr = ptr->next()) {
			switch (ptr->type) {
			int x;
			case MPP_T_SCORE_SUBDIV:
				x = MPP_BAND_REM(ptr->value[0], MPP_MAX_CHORD_BANDS);

				ptr->value[0] -= MPP_BAND_REM(ptr->value[0], MPP_MAX_BANDS);
				ptr->value[0] += adjust[x];
				ptr->txt = MppKeyStr(ptr->value[0]);
				break;
			default:
				break;
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

	for ( ; ptr != 0; ptr = ptr->next()) {
		if (ptr->type == MPP_T_NEWLINE ||
		    ptr->type == MPP_T_JUMP ||
		    ptr->type == MPP_T_LABEL) {
			ptr = ptr->next();
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

		linebuf[0] = "";
		linebuf[1] = "";

		for (ptr = start; ptr != stop; ptr = ptr->next()) {
			switch (ptr->type) {
			case MPP_T_STRING_DESC:
			case MPP_T_COMMENT_DESC:
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
			out += linebuf[0].replace(QRegularExpression("\\s*$"), "");
			out += '\n';
		}

		/* Export Text Line */
		if (linebuf[1].size() > 0) {
			/* check for label tags */
			if (linebuf[1].size() > 1 &&
			    linebuf[1][0] == 'L' &&
			    linebuf[1][1].isDigit()) {
				int x;
				for (x = 2; x != linebuf[1].size(); x++) {
					if (linebuf[1][x].isDigit())
						continue;
					break;
				}
				for (; x != linebuf[1].size(); x++) {
					if (linebuf[1][x].isSpace() ||
					    linebuf[1][x] == '-')
						continue;
					linebuf[1] = linebuf[1]
					  .mid(x, linebuf[1].size() - x);
					break;
				}
				out += '\n';
				out += '[';
				out += linebuf[1].replace(QRegularExpression("\\s*$"), "");
				out += ']';
			} else {
				/* remove white space at end of line */
				out += linebuf[1].replace(QRegularExpression("\\s*$"), "");
			}
			out += '\n';
		}
	}
	return (out);
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
			    ptr = ptr->next()) {
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
				} else if (ptr->type == MPP_T_SCORE_SUBDIV ||
				    ptr->type == MPP_T_MACRO ||
				    ptr->type == MPP_T_TIMER) {
					/* valid event */
					goto done;
				} else if (ptr->type == MPP_T_COMMAND) {
					switch (ptr->value[0]) {
					case MPP_CMD_LOCK:
						state.key_lock = -1;
						break;
					case MPP_CMD_UNLOCK:
						state.key_lock = 0;
						break;
					case MPP_CMD_IMAGE_PROPS:
						state.image_curr.num = ptr->value[1];
						state.image_curr.how = ptr->value[2];
						state.image_curr.align = ptr->value[3];
						break;
					case MPP_CMD_IMAGE_BG_COLOR:
						state.image_curr.color.bg_red = ptr->value[1];
						state.image_curr.color.bg_green = ptr->value[2];
						state.image_curr.color.bg_blue = ptr->value[3];
						break;
					case MPP_CMD_IMAGE_FG_COLOR:
						state.image_curr.color.fg_red = ptr->value[1];
						state.image_curr.color.fg_green = ptr->value[2];
						state.image_curr.color.fg_blue = ptr->value[3];
						break;
					case MPP_CMD_TEXT_PROPS:
						state.text_curr.align = ptr->value[1];
						state.text_curr.space = ptr->value[2];
						state.text_curr.shadow = ptr->value[3];
						break;
					case MPP_CMD_TEXT_BG_COLOR:
						state.text_curr.color.bg_red = ptr->value[1];
						state.text_curr.color.bg_green = ptr->value[2];
						state.text_curr.color.bg_blue = ptr->value[3];
						break;
					case MPP_CMD_TEXT_FG_COLOR:
						state.text_curr.color.fg_red = ptr->value[1];
						state.text_curr.color.fg_green = ptr->value[2];
						state.text_curr.color.fg_blue = ptr->value[3];
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

		next = ptr->next();
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
	const MppElement * const ne = 0;

	/* handle special case */
	if (this == ne && other == ne)
		return (0);
	if (other == ne)
		return (-1);
	if (this == ne)
		return (1);

	/* compare sequence number */
	if (sequence > other->sequence)
		return (1);
	if (sequence < other->sequence)
		return (-1);
	return (0);
}

MppElement *
MppElement :: next() const
{
	return (TAILQ_NEXT(this, entry));
}
