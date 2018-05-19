/*-
 * Copyright (c) 2018 Hans Petter Selasky. All rights reserved.
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

#ifndef _MIDIPP_CHORDS_H_
#define	_MIDIPP_CHORDS_H_

#include "midipp.h"

struct MppChord {
	uint32_t data[(MPP_MAX_BANDS + 1 + 31) / 32];
	void zero() { memset(data, 0, sizeof(data)); };
	void set(size_t x) { data[x / 32] |= 1U << (x % 32); }
	void clr(size_t x) { data[x / 32] &= ~(1U << (x % 32)); };
	void tog(size_t x) { data[x / 32] ^= 1U << (x % 32); };
	bool test(size_t x) const { return ((data[x / 32] >> (x % 32)) & 1); };
	uint32_t order() {
		uint32_t retval = 0;
		for (int x = 0; x != MPP_MAX_BANDS; x++) {
			if (test(x))
				retval++;
		}
		return (retval);
	};
	void inc(int step) {
		int x;
		int y = 0;
		for (x = y = 0; x != MPP_MAX_BANDS; x += step) {
			if (test(x))
				break;
		}
		for (; x != MPP_MAX_BANDS && test(x); x += step) {
			tog(x);
			y += step;
		}
		if (x != MPP_MAX_BANDS) {
			tog(x);
			y -= step;
		}
		/* preserve number of set bits */
		for (x = 0; x != y; x += step)
			set(x);
	};
	void dec(int step) {
		int x;
		int y = 0;
		for (x = y = 0; x != MPP_MAX_BANDS; x += step) {
			if (test(x) == 0)
				break;
		}
		for (; x != MPP_MAX_BANDS && test(x) == 0; x += step) {
			tog(x);
			y += step;
		}
		if (x != MPP_MAX_BANDS) {
			tog(x);
			y -= step;
		}
		/* preserve number of set bits */
		for (x = 0; x != y; x += step)
			clr(x);
	};
	void inv() {
		int x = sizeof(data) / sizeof(data[0]);
		while (x--)
			data[x] ^= -1U;
	};
	void shr() {
		int x = sizeof(data) / sizeof(data[0]);
		int rem = 0;
		while (x--) {
			int next = (data[x] & 1) << 31;
			data[x] = (data[x] >> 1) | rem;
			rem = next;
		}
	};
	void shl() {
		int rem = 0;
		for (int x = 0; x != sizeof(data) / sizeof(data[0]); x++) {
			int next = (data[x] >> 31) & 1;
			data[x] = (data[x] << 1) | rem;
			rem = next;
		}
	};
	void operator &=(const MppChord &other) {
		int x = sizeof(data) / sizeof(data[0]);
		while (x--)
			data[x] &= other.data[x];
	};
	void operator |=(const MppChord &other) {
		int x = sizeof(data) / sizeof(data[0]);
		while (x--)
			data[x] |= other.data[x];
	};
	bool operator <(const MppChord &other) const {
		int x = sizeof(data) / sizeof(data[0]);
		while (x--) {
			if (data[x] < other.data[x])
				return (1);
			else if (data[x] > other.data[x])
				return (0);
		}
		return (0);
	};
	bool operator >(const MppChord &other) const {
		int x = sizeof(data) / sizeof(data[0]);
		while (x--) {
			if (data[x] > other.data[x])
				return (1);
			else if (data[x] < other.data[x])
				return (0);
		}
		return (0);
	};
	bool operator ==(const MppChord &other) const {
		int x = sizeof(data) / sizeof(data[0]);
		while (x--) {
			if (data[x] != other.data[x])
				return (0);
		}
		return (1);
	};
	bool operator !=(const MppChord &other) const {
		int x = sizeof(data) / sizeof(data[0]);
		while (x--) {
			if (data[x] != other.data[x])
				return (1);
		}
		return (0);
	};
	void operator =(const MppChord &other) {
		if (this == &other)
			return;
		int x = sizeof(data) / sizeof(data[0]);
		while (x--) {
			data[x] = other.data[x];
		}
	};
};

class MppScoreVariant {
public:
	MppScoreVariant(uint32_t _chord, const char *a, const char *b = 0,
	    const char *c = 0, const char *d = 0);
	~MppScoreVariant() { };
	const char * pattern[4];
	MppChord_t footprint;
	uint32_t rots;
};

extern MppChord_t midipp_major;
extern MppChord_t midipp_major_rectified;

extern void MppRolUpChord(MppChord_t &input, int &delta);
extern void MppRolDownChord(MppChord_t &input, int &delta);
extern void MppNextChordRoot(MppChord_t &input, int step = 1);
extern void MppPrevChordRoot(MppChord_t &input, int step = 1);
extern MppChord_t MppFindChordRoot(MppChord_t, uint32_t * = 0, uint32_t * = 0);
extern int MppIsChord(QString &);

extern void MppChordToStringGeneric(MppChord_t mask, uint32_t rem, uint32_t bass, uint32_t is_chord, uint32_t step, QString &retval);
extern void MppStringToChordGeneric(MppChord_t &mask, uint32_t &rem, uint32_t &bass, uint32_t step, const QString &str);
extern const QString MppKeyToStringGeneric(int key, int sharp);
extern void MppStepChordGeneric(QString &str, int adjust, uint32_t sharp);

#endif		/* _MIDIPP_CHORDS_H_ */
