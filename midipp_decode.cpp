/*-
 * Copyright (c) 2010-2017 Hans Petter Selasky. All rights reserved.
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

#include "midipp_decode.h"
#include "midipp_mainwindow.h"
#include "midipp_scores.h"
#include "midipp_button.h"
#include "midipp_checkbox.h"
#include "midipp_element.h"
#include "midipp_groupbox.h"

/* The list is sorted by priority */

#define MASK(x) (1U << ((x) % MPP_MAX_BANDS))

/* chord modifier characters */

const QString MppChordModChars = QString::fromUtf8("()/+-#Δ&|^°");

/* common standard */

static const char *score_major[] = {
	"$M", "M", "Δ", "j", "Ma", "maj", "major", 0
};

static const char *score_minor[] = {
	"$m", "m", "-", "Mi", "min", "minor", 0
};

static const char *score_aug[] = {
	"$a", "+", "+5", "aug", 0
};

static const char *score_dim[] = {
	"$d", "°", "o", "dim", 0
};

/* custom */

static const char *score_and[] = {
	"$A", "&", "and", 0
};

static const char *score_orr[] = {
	"$O", "|", "orr", 0
};

static const char *score_xor[] = {
	"$X", "^", "xor", 0
};

/* common standard */

#ifdef HAVE_QUARTERTONE
static const char *score_quarter_sharp[] = {
	"$q", "q", 0
};

static const char *score_quarter_flat[] = {
	"$c", "c", 0
};
#endif

static const char *score_sharp[] = {
	"$S", "+", "#", 0
};

static const char *score_flat[] = {
	"$f", "-", "b", "°", "o", "dim", 0
};

static const char *score_half_dim[] = {
	"$h", "ø", "Ø", 0
};

static const char *score_seventh[] = {
	"$7", "7", "dom7", "dominant7", 0
};

static const char **score_macros[] = {
	score_major,
	score_minor,
	score_aug,
	score_dim,
	score_and,
	score_orr,
	score_xor,
	score_sharp,
	score_flat,
#ifdef HAVE_QUARTERTONE
	score_quarter_sharp,
	score_quarter_flat,
#endif
	score_half_dim,
	score_seventh,
	0
};

#ifdef HAVE_QUARTERTONE
static const char *score_bits[MPP_MAX_BANDS + 1] = {
	"C",
	"Dq", "Db", "Dc", "D",
	"Eq", "Eb", "Ec", "E",
	"Fc", "F",
	"Gq", "Gb", "Gc", "G",
	"Aq", "Ab", "Ac", "A",
	"Hq", "Hb", "Hc", "H",
	"Cc", "?"
};

static const char *score_name_sharp[2 * MPP_MAX_BANDS] = {
	"1", "$q2", "$S1", "$c2", "2", "$q3", "$S2", "$c3", "3", "$c4", "4", "$q5", "$S4", "$c5", "5", "$q6", "$S5", "$c6", "6", "$q7", "$S6", "$c7", "7", "$c8",
	"8", "$q9", "$S8", "$c9", "9", "$q10", "$S9", "$c10", "10", "$c11", "11", "$q12", "$S11", "$c12", "12", "$q13", "$S12", "$c13", "13", "$q14", "$S13", "$c14", "14", "$c15",
};

static const char *score_name_flat[2 * MPP_MAX_BANDS] = {
	"1", "$q2", "$f2", "$c2", "2", "$q3", "$f3", "$c3", "3", "$c4", "4", "$q5", "$f5", "$c5", "5", "$q6", "$f6", "$c6", "6", "$q7", "$f7", "$c7", "7", "$c8",
	"8", "$q9", "$f9", "$c9", "9", "$q10", "$f10", "$c10", "10", "$c11", "11", "$q12", "$f12", "$c12", "12", "$q13", "$f13", "$c13", "13", "$q14", "$f14", "$c14", "14", "$c15",
};
#else
static const char *score_bits[MPP_MAX_BANDS + 1] = {
	"C",
	"Db", "D",
	"Eb", "E",
	"F",
	"Gb", "G",
	"Ab", "A",
	"Hb", "H",
	"?"
};

static const char *score_name_sharp[2 * MPP_MAX_BANDS] = {
	"1", "$S1", "2", "$S2", "3", "4", "$S4", "5", "$S5", "6", "$S6", "7",
	"8", "$S8", "9", "$S9", "10", "11", "$S11", "12", "$S12", "13", "$S13", "14",
};

static const char *score_name_flat[2 * MPP_MAX_BANDS] = {
	"1", "$f2", "2", "$f3", "3", "4", "$f5", "5", "$f6", "6", "$f7", "7",
	"8", "$f9", "9", "$f10", "10", "11", "$f12", "12", "$f13", "13", "$f14", "14",
};
#endif

/*
 * C D E F G A B C D E  F  G  A  B  C
 * 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15
 * =   =   =
 */
static const struct score_variant_initial score_initial[] = {
	/* one */
	{ { "1" }, MASK(MPP_C0) },

	/* third */
	{ { "5" }, MASK(MPP_C0) | MASK(MPP_G0) },

	/* triads */
	{ { "$A" }, MASK(MPP_C0) | MASK(MPP_D0B) | MASK(MPP_D0) },	/* custom */
	{ { "$O" }, MASK(MPP_C0) | MASK(MPP_D0B) | MASK(MPP_E0B) },	/* custom */
	{ { "$X" }, MASK(MPP_C0) | MASK(MPP_D0) | MASK(MPP_E0B) },	/* custom */
	{ { "$d", "$m$f5" }, MASK(MPP_C0) | MASK(MPP_E0B) | MASK(MPP_G0B) },
	{ { "$m", "sus$S2", "sus$f3" }, MASK(MPP_C0) | MASK(MPP_E0B) | MASK(MPP_G0) },
	{ { "$a", "$M$S5" }, MASK(MPP_C0) | MASK(MPP_E0) | MASK(MPP_A0B) },
	{ { "", "$M" }, MASK(MPP_C0) | MASK(MPP_E0) | MASK(MPP_G0) },

	/* augumented and major triads with susXXX */
	{ { "sus$S1", "sus$f2" }, MASK(MPP_C0) | MASK(MPP_D0B) | MASK(MPP_G0) },
	{ { "$asus$S1", "$asus$f2" }, MASK(MPP_C0) | MASK(MPP_D0B) | MASK(MPP_A0B) },
	{ { "sus2" }, MASK(MPP_C0) | MASK(MPP_D0) | MASK(MPP_G0) },
	{ { "$asus2" }, MASK(MPP_C0) | MASK(MPP_D0) | MASK(MPP_A0B) },
	{ { "sus$S2", "sus$f3" }, MASK(MPP_C0) | MASK(MPP_E0B) | MASK(MPP_G0) },
	{ { "$asus$S2", "$asus$f3" }, MASK(MPP_C0) | MASK(MPP_E0B) | MASK(MPP_A0B) },
	{ { "sus4", "sus" }, MASK(MPP_C0) | MASK(MPP_F0) | MASK(MPP_G0) },
	{ { "$asus4", "$asus" }, MASK(MPP_C0) | MASK(MPP_F0) | MASK(MPP_A0B) },
	{ { "sus$S4", "sus$f5" }, MASK(MPP_C0) | MASK(MPP_G0B) | MASK(MPP_G0) },
	{ { "$asus$S4", "$asus$f5" }, MASK(MPP_C0) | MASK(MPP_G0B) | MASK(MPP_A0B) },
	{ { "$asus5" }, MASK(MPP_C0) | MASK(MPP_G0) | MASK(MPP_A0B) },
	{ { "sus$S5", "sus$f6" }, MASK(MPP_C0) | MASK(MPP_G0) | MASK(MPP_A0B) },
	{ { "sus6" }, MASK(MPP_C0) | MASK(MPP_G0) | MASK(MPP_A0) },
	{ { "$asus6" }, MASK(MPP_C0) | MASK(MPP_A0B) | MASK(MPP_A0) },
	{ { "sus$S6", "sus$f7" }, MASK(MPP_C0) | MASK(MPP_G0) | MASK(MPP_H0B) },
	{ { "$asus$S6", "$asus$f7" }, MASK(MPP_C0) | MASK(MPP_A0B) | MASK(MPP_H0B) },
	{ { "sus7" }, MASK(MPP_C0) | MASK(MPP_G0) | MASK(MPP_H0) },
	{ { "$asus7" }, MASK(MPP_C0) | MASK(MPP_A0B) | MASK(MPP_H0) },

	/* seventh */
	{ { "$7" }, MASK(MPP_C0) | MASK(MPP_E0) | MASK(MPP_G0) | MASK(MPP_H0B) },
	{ { "$a7", "$7$S5", "$7$S" }, MASK(MPP_C0) | MASK(MPP_E0) | MASK(MPP_A0B) | MASK(MPP_H0B) },
	{ { "$h", "$h7", "$m7$f5" }, MASK(MPP_C0) | MASK(MPP_E0B) | MASK(MPP_G0B) | MASK(MPP_H0B) },
	{ { "$7$f5" }, MASK(MPP_C0) | MASK(MPP_E0) | MASK(MPP_G0B) | MASK(MPP_H0B) },
	{ { "$7$f9", "$7$f2" }, MASK(MPP_C0) | MASK(MPP_E0) | MASK(MPP_G0) | MASK(MPP_D0B) | MASK(MPP_H0B) },
	{ { "$7$S9", "$7$S2" }, MASK(MPP_C0) | MASK(MPP_E0) | MASK(MPP_G0) | MASK(MPP_E0B) | MASK(MPP_H0B) },
	{ { "$M7" }, MASK(MPP_C0) | MASK(MPP_E0) | MASK(MPP_G0) | MASK(MPP_H0) },
	{ { "$a$M7", "$M7$S5", "$M7$S" }, MASK(MPP_C0) | MASK(MPP_E0) | MASK(MPP_A0B) | MASK(MPP_H0) },
	{ { "$m$M7" }, MASK(MPP_C0) | MASK(MPP_E0B) | MASK(MPP_G0) | MASK(MPP_H0) },
	{ { "$m7" }, MASK(MPP_C0) | MASK(MPP_E0B) | MASK(MPP_G0) | MASK(MPP_H0B) },
	{ { "$d7" }, MASK(MPP_C0) | MASK(MPP_E0B) | MASK(MPP_G0B) | MASK(MPP_A0) },

	/* second */
	{ { "2", "$M2", "dom2" }, MASK(MPP_C0) | MASK(MPP_E0) | MASK(MPP_G0) | MASK(MPP_D0) },
	{ { "$a2", "2$S5" }, MASK(MPP_C0) | MASK(MPP_E0) | MASK(MPP_A0B) | MASK(MPP_D0) },
	{ { "$m$M2" }, MASK(MPP_C0) | MASK(MPP_E0B) | MASK(MPP_G0) | MASK(MPP_D0) },
	{ { "$m2" }, MASK(MPP_C0) | MASK(MPP_E0B) | MASK(MPP_G0) | MASK(MPP_D0) },
	{ { "$a$M2" }, MASK(MPP_C0) | MASK(MPP_E0) | MASK(MPP_A0B) | MASK(MPP_D0) },
	{ { "$h2" }, MASK(MPP_C0) | MASK(MPP_E0B) | MASK(MPP_G0B) | MASK(MPP_D0) },
	{ { "$h$f2" }, MASK(MPP_C0) | MASK(MPP_E0B) | MASK(MPP_G0B) | MASK(MPP_D0B) },

	/* agumented and major seconds and sevenths susXXX */
	{ { "7sus$S1", "7sus$f2" }, MASK(MPP_C0) | MASK(MPP_D0B) | MASK(MPP_G0) | MASK(MPP_H0B) },
	{ { "$a7sus$S1", "$a7sus$f2" }, MASK(MPP_C0) | MASK(MPP_D0B) | MASK(MPP_A0B) | MASK(MPP_H0B) },
	{ { "2sus$S1", "2sus$f2" }, MASK(MPP_C0) | MASK(MPP_D0B) | MASK(MPP_D0) | MASK(MPP_G0) },
	{ { "$a2sus$S1", "$a2sus$f2" }, MASK(MPP_C0) | MASK(MPP_D0B) | MASK(MPP_D0) | MASK(MPP_A0B) },
	{ { "7sus2" }, MASK(MPP_C0) | MASK(MPP_D0) | MASK(MPP_G0) | MASK(MPP_H0B) },
	{ { "$a7sus2" }, MASK(MPP_C0) | MASK(MPP_D0) | MASK(MPP_A0B) | MASK(MPP_H0B) },
	{ { "7sus$S2", "7sus$f3" }, MASK(MPP_C0) | MASK(MPP_E0B) | MASK(MPP_G0) | MASK(MPP_H0B) },
	{ { "$a7sus$S2", "$a7sus$f3" }, MASK(MPP_C0) | MASK(MPP_E0B) | MASK(MPP_A0B) | MASK(MPP_H0B) },
	{ { "2sus$S2", "2sus$f3" }, MASK(MPP_C0) | MASK(MPP_D0) | MASK(MPP_E0B) | MASK(MPP_G0) },
	{ { "$a2sus$S2", "$a2sus$f3" }, MASK(MPP_C0) | MASK(MPP_D0) | MASK(MPP_E0B) | MASK(MPP_A0B) },
	{ { "7sus4", "7sus" }, MASK(MPP_C0) | MASK(MPP_F0) | MASK(MPP_G0) | MASK(MPP_H0B) },
	{ { "$a7sus4", "$a7sus" }, MASK(MPP_C0) | MASK(MPP_F0) | MASK(MPP_A0B) | MASK(MPP_H0B) },
	{ { "2sus4", "2sus" }, MASK(MPP_C0) | MASK(MPP_D0) | MASK(MPP_F0) | MASK(MPP_G0) },
	{ { "$a2sus4", "$a2sus" }, MASK(MPP_C0) | MASK(MPP_D0) | MASK(MPP_F0) | MASK(MPP_A0B) },
	{ { "7sus$S4", "7sus$f5" }, MASK(MPP_C0) | MASK(MPP_G0B) | MASK(MPP_G0) | MASK(MPP_H0B) },
	{ { "$a7sus$S4", "$a7sus$f5" }, MASK(MPP_C0) | MASK(MPP_G0B) | MASK(MPP_A0B) | MASK(MPP_H0B) },
	{ { "2sus$S4", "2sus$f5" }, MASK(MPP_C0) | MASK(MPP_D0) | MASK(MPP_G0B) | MASK(MPP_G0) },
	{ { "$a2sus$S4", "$a2sus$f5" }, MASK(MPP_C0) | MASK(MPP_D0) | MASK(MPP_G0B) | MASK(MPP_A0B) },
	{ { "$a7sus5" }, MASK(MPP_C0) | MASK(MPP_G0) | MASK(MPP_A0B) | MASK(MPP_H0B) },
	{ { "$a2sus5" }, MASK(MPP_C0) | MASK(MPP_D0) | MASK(MPP_G0) | MASK(MPP_A0B) },
	{ { "7sus$S5", "7sus$f6" }, MASK(MPP_C0) | MASK(MPP_G0) | MASK(MPP_A0B) | MASK(MPP_H0B) },
	{ { "2sus$S5", "2sus$f6" }, MASK(MPP_C0) | MASK(MPP_D0) | MASK(MPP_G0) | MASK(MPP_A0B) },
	{ { "7sus6" }, MASK(MPP_C0) | MASK(MPP_G0) | MASK(MPP_A0) | MASK(MPP_H0B) },
	{ { "$a7sus6" }, MASK(MPP_C0) | MASK(MPP_A0B) | MASK(MPP_A0) | MASK(MPP_H0B) },
	{ { "2sus6" }, MASK(MPP_C0) | MASK(MPP_D0) | MASK(MPP_G0) | MASK(MPP_A0) },
	{ { "$a2sus6" }, MASK(MPP_C0) | MASK(MPP_D0) | MASK(MPP_A0B) | MASK(MPP_A0) },
	{ { "2sus$S6", "2sus$f7" }, MASK(MPP_C0) | MASK(MPP_D0) | MASK(MPP_G0) | MASK(MPP_H0B) },
	{ { "$a2sus$S6", "$a2sus$f7" }, MASK(MPP_C0) | MASK(MPP_D0) | MASK(MPP_A0B) | MASK(MPP_H0B) },
	{ { "7sus7" }, MASK(MPP_C0) | MASK(MPP_G0) | MASK(MPP_H0B) | MASK(MPP_H0) },
	{ { "$a7sus7" }, MASK(MPP_C0) | MASK(MPP_A0B) | MASK(MPP_H0B) | MASK(MPP_H0) },
	{ { "2sus7" }, MASK(MPP_C0) | MASK(MPP_D0) | MASK(MPP_G0) | MASK(MPP_H0) },
	{ { "$a2sus7" }, MASK(MPP_C0) | MASK(MPP_D0) | MASK(MPP_A0B) | MASK(MPP_H0) },

	/* fourth */
	{ { "4", "$M4", "dom4" }, MASK(MPP_C0) | MASK(MPP_E0) | MASK(MPP_G0) | MASK(MPP_D0) | MASK(MPP_F0) },
	{ { "$m4", "$m$M4" }, MASK(MPP_C0) | MASK(MPP_E0B) | MASK(MPP_G0) | MASK(MPP_D0) | MASK(MPP_F0) },
	{ { "$a4", "4$S5", "$a$M4" }, MASK(MPP_C0) | MASK(MPP_E0) | MASK(MPP_A0B) | MASK(MPP_D0) | MASK(MPP_F0) },
	{ { "$h4" }, MASK(MPP_C0) | MASK(MPP_E0B) | MASK(MPP_G0B) | MASK(MPP_D0B) | MASK(MPP_F0) },

	/* ninth */
	{ { "9", "dom9" }, MASK(MPP_C0) | MASK(MPP_E0) | MASK(MPP_G0) | MASK(MPP_H0B) | MASK(MPP_D0) },
	{ { "$M9" }, MASK(MPP_C0) | MASK(MPP_E0) | MASK(MPP_G0) | MASK(MPP_H0) | MASK(MPP_D0) },
	{ { "$m$M9" }, MASK(MPP_C0) | MASK(MPP_E0B) | MASK(MPP_G0) | MASK(MPP_H0) | MASK(MPP_D0) },
	{ { "$m9" }, MASK(MPP_C0) | MASK(MPP_E0B) | MASK(MPP_G0) | MASK(MPP_H0B) | MASK(MPP_D0) },
	{ { "$a$M9" }, MASK(MPP_C0) | MASK(MPP_E0) | MASK(MPP_A0B) | MASK(MPP_H0) | MASK(MPP_D0) },
	{ { "$a9", "9$S5" }, MASK(MPP_C0) | MASK(MPP_E0) | MASK(MPP_A0B) | MASK(MPP_H0B) | MASK(MPP_D0) },
	{ { "$h9" }, MASK(MPP_C0) | MASK(MPP_E0B) | MASK(MPP_G0B) | MASK(MPP_H0B) | MASK(MPP_D0) },
	{ { "$h$f9" }, MASK(MPP_C0) | MASK(MPP_E0B) | MASK(MPP_G0B) | MASK(MPP_H0B) | MASK(MPP_D0B) },
	{ { "$d9" }, MASK(MPP_C0) | MASK(MPP_E0B) | MASK(MPP_G0B) | MASK(MPP_A0) | MASK(MPP_D0) },
	{ { "$db9" }, MASK(MPP_C0) | MASK(MPP_E0B) | MASK(MPP_G0B) | MASK(MPP_A0) | MASK(MPP_D0B) },

	/* agumented and major fourths and ninths susXXX */
	{ { "4sus$S1", "4sus$f2" }, MASK(MPP_C0) | MASK(MPP_D0B) | MASK(MPP_D0) | MASK(MPP_F0) | MASK(MPP_G0) },
	{ { "$a4sus$S1", "$a4sus$f2" }, MASK(MPP_C0) | MASK(MPP_D0B) | MASK(MPP_D0) | MASK(MPP_F0) | MASK(MPP_A0B) },
	{ { "9sus$S1", "9sus$f2" }, MASK(MPP_C0) | MASK(MPP_D0B) | MASK(MPP_D0) | MASK(MPP_G0) | MASK(MPP_H0B) },
	{ { "$a9sus$S1", "$a9sus$f2" }, MASK(MPP_C0) | MASK(MPP_D0B) | MASK(MPP_D0) | MASK(MPP_A0B) | MASK(MPP_H0B) },
	{ { "4sus$S2", "4sus$f3" }, MASK(MPP_C0) | MASK(MPP_D0) | MASK(MPP_E0B) | MASK(MPP_F0) | MASK(MPP_G0) },
	{ { "$a4sus$S2", "$a4sus$f3" }, MASK(MPP_C0) | MASK(MPP_D0) | MASK(MPP_E0B) | MASK(MPP_F0) | MASK(MPP_A0B) },
	{ { "9sus$S2", "9sus$f3" }, MASK(MPP_C0) | MASK(MPP_D0) | MASK(MPP_E0B) | MASK(MPP_G0) | MASK(MPP_H0B) },
	{ { "$a9sus$S2", "$a9sus$f3" }, MASK(MPP_C0) | MASK(MPP_D0) | MASK(MPP_E0B) | MASK(MPP_A0B) | MASK(MPP_H0B) },
	{ { "9sus4", "9sus" }, MASK(MPP_C0) | MASK(MPP_D0) | MASK(MPP_F0) | MASK(MPP_G0) | MASK(MPP_H0B) },
	{ { "$a9sus4", "$a9sus" }, MASK(MPP_C0) | MASK(MPP_D0) | MASK(MPP_F0) | MASK(MPP_A0B) | MASK(MPP_H0B) },
	{ { "4sus$S4", "4sus$f5" }, MASK(MPP_C0) | MASK(MPP_D0) | MASK(MPP_F0) | MASK(MPP_G0B) | MASK(MPP_G0) },
	{ { "$a4sus$S4", "$a4sus$f5" }, MASK(MPP_C0) | MASK(MPP_D0) | MASK(MPP_F0) | MASK(MPP_G0B) | MASK(MPP_A0B) },
	{ { "9sus$S4", "9sus$f5" }, MASK(MPP_C0) | MASK(MPP_D0) | MASK(MPP_G0B) | MASK(MPP_G0) | MASK(MPP_H0B) },
	{ { "$a9sus$S4", "$a9sus$f5" }, MASK(MPP_C0) | MASK(MPP_D0) | MASK(MPP_G0B) | MASK(MPP_A0B) | MASK(MPP_H0B) },
	{ { "$a4sus5" }, MASK(MPP_C0) | MASK(MPP_D0) | MASK(MPP_F0) | MASK(MPP_G0) | MASK(MPP_A0B) },
	{ { "$a9sus5" }, MASK(MPP_C0) | MASK(MPP_D0) | MASK(MPP_G0) | MASK(MPP_A0B) | MASK(MPP_H0B) },
	{ { "4sus$S5", "4sus$f6" }, MASK(MPP_C0) | MASK(MPP_D0) | MASK(MPP_F0) | MASK(MPP_G0) | MASK(MPP_A0B) },
	{ { "9sus$S5", "9sus$f6" }, MASK(MPP_C0) | MASK(MPP_D0) | MASK(MPP_G0) | MASK(MPP_A0B) | MASK(MPP_H0B) },
	{ { "4sus6" }, MASK(MPP_C0) | MASK(MPP_D0) | MASK(MPP_F0) | MASK(MPP_G0) | MASK(MPP_A0) },
	{ { "$a4sus6" }, MASK(MPP_C0) | MASK(MPP_D0) | MASK(MPP_F0) | MASK(MPP_A0B) | MASK(MPP_A0) },
	{ { "9sus6" }, MASK(MPP_C0) | MASK(MPP_D0) | MASK(MPP_G0) | MASK(MPP_A0) | MASK(MPP_H0B) },
	{ { "$a9sus6" }, MASK(MPP_C0) | MASK(MPP_D0) | MASK(MPP_A0B) | MASK(MPP_A0) | MASK(MPP_H0B) },
	{ { "4sus$S6", "4sus$f7" }, MASK(MPP_C0) | MASK(MPP_D0) | MASK(MPP_F0) | MASK(MPP_G0) | MASK(MPP_H0B) },
	{ { "$a4sus$S6", "$a4sus$f7" }, MASK(MPP_C0) | MASK(MPP_D0) | MASK(MPP_F0) | MASK(MPP_A0B) | MASK(MPP_H0B) },
	{ { "4sus7" }, MASK(MPP_C0) | MASK(MPP_D0) | MASK(MPP_F0) | MASK(MPP_G0) | MASK(MPP_H0) },
	{ { "$a4sus7" }, MASK(MPP_C0) | MASK(MPP_D0) | MASK(MPP_F0) | MASK(MPP_A0B) | MASK(MPP_H0) },
	{ { "9sus7" }, MASK(MPP_C0) | MASK(MPP_D0) | MASK(MPP_G0) | MASK(MPP_H0B) | MASK(MPP_H0) },
	{ { "$a9sus7" }, MASK(MPP_C0) | MASK(MPP_D0) | MASK(MPP_A0B) | MASK(MPP_H0B) | MASK(MPP_H0) },

	/* sixth */
	{ { "6", "$M6", "dom6" }, MASK(MPP_C0) | MASK(MPP_E0) | MASK(MPP_G0) | MASK(MPP_D0) | MASK(MPP_F0) | MASK(MPP_A0) },
	{ { "62" }, MASK(MPP_C0) | MASK(MPP_E0) | MASK(MPP_G0) | MASK(MPP_A0) | MASK(MPP_D0) },
	{ { "69" }, MASK(MPP_C0) | MASK(MPP_E0) | MASK(MPP_G0) | MASK(MPP_A0) | MASK(MPP_D0) },
	{ { "64" }, MASK(MPP_C0) | MASK(MPP_E0) | MASK(MPP_G0) | MASK(MPP_A0) | MASK(MPP_F0) },
	{ { "$m6", "$m$M6" }, MASK(MPP_C0) | MASK(MPP_E0B) | MASK(MPP_G0) | MASK(MPP_D0) | MASK(MPP_F0) | MASK(MPP_A0) },
	{ { "$a6", "6$S5", "$a$M6" }, MASK(MPP_C0) | MASK(MPP_E0) | MASK(MPP_A0B) | MASK(MPP_D0) | MASK(MPP_F0) | MASK(MPP_A0) },
	{ { "$h6" }, MASK(MPP_C0) | MASK(MPP_E0B) | MASK(MPP_G0B) | MASK(MPP_D0) | MASK(MPP_F0) | MASK(MPP_A0) },

	/* eleventh */
	{  { "11", "dom11" }, MASK(MPP_C0) | MASK(MPP_E0) | MASK(MPP_G0) | MASK(MPP_H0B) | MASK(MPP_D0) | MASK(MPP_F0) },
	{  { "$M11" }, MASK(MPP_C0) | MASK(MPP_E0) | MASK(MPP_G0) | MASK(MPP_H0) | MASK(MPP_D0) | MASK(MPP_F0) },
	{  { "$m$M11" }, MASK(MPP_C0) | MASK(MPP_E0B) | MASK(MPP_G0) | MASK(MPP_H0) | MASK(MPP_D0) | MASK(MPP_F0) },
	{  { "$m11" }, MASK(MPP_C0) | MASK(MPP_E0B) | MASK(MPP_G0) | MASK(MPP_H0B) | MASK(MPP_D0) | MASK(MPP_F0) },
	{  { "$a$M11" }, MASK(MPP_C0) | MASK(MPP_E0) | MASK(MPP_A0B) | MASK(MPP_H0) | MASK(MPP_D0) | MASK(MPP_F0) },
	{  { "$a11", "11$S5" }, MASK(MPP_C0) | MASK(MPP_E0) | MASK(MPP_A0B) | MASK(MPP_H0B) | MASK(MPP_D0) | MASK(MPP_F0) },
	{  { "$h11" }, MASK(MPP_C0) | MASK(MPP_E0B) | MASK(MPP_G0B) | MASK(MPP_H0B) | MASK(MPP_D0B) | MASK(MPP_F0) },
	{  { "$d11" }, MASK(MPP_C0) | MASK(MPP_E0B) | MASK(MPP_G0B) | MASK(MPP_A0) | MASK(MPP_D0B) | MASK(MPP_E0) },

	/* agumented and major sixths and elevenths susXXX */
	{ { "6sus$S1", "6sus$f2" }, MASK(MPP_C0) | MASK(MPP_D0B) | MASK(MPP_D0) | MASK(MPP_F0) | MASK(MPP_G0) | MASK(MPP_A0) },
	{ { "$a6sus$S1", "$a6sus$f2" }, MASK(MPP_C0) | MASK(MPP_D0B) | MASK(MPP_D0) | MASK(MPP_F0) | MASK(MPP_A0B) | MASK(MPP_A0) },
	{ { "11sus$S1", "11sus$f2" }, MASK(MPP_C0) | MASK(MPP_D0B) | MASK(MPP_D0) | MASK(MPP_F0) | MASK(MPP_G0) | MASK(MPP_H0B) },
	{ { "$a11sus$S1", "$a11sus$f2" }, MASK(MPP_C0) | MASK(MPP_D0B) | MASK(MPP_D0) | MASK(MPP_F0) | MASK(MPP_A0B) | MASK(MPP_H0B) },
	{ { "6sus$S2", "6sus$f3" }, MASK(MPP_C0) | MASK(MPP_D0) | MASK(MPP_E0B) | MASK(MPP_F0) | MASK(MPP_G0) | MASK(MPP_A0) },
	{ { "$a6sus$S2", "$a6sus$f3" }, MASK(MPP_C0) | MASK(MPP_D0) | MASK(MPP_E0B) | MASK(MPP_F0) | MASK(MPP_A0B) | MASK(MPP_A0) },
	{ { "11sus$S2", "11sus$f3" }, MASK(MPP_C0) | MASK(MPP_D0) | MASK(MPP_E0B) | MASK(MPP_F0) | MASK(MPP_G0) | MASK(MPP_H0B) },
	{ { "$a11sus$S2", "$a11sus$f3" }, MASK(MPP_C0) | MASK(MPP_D0) | MASK(MPP_E0B) | MASK(MPP_F0) | MASK(MPP_A0B) | MASK(MPP_H0B) },
	{ { "6sus$S4", "6sus$f5" }, MASK(MPP_C0) | MASK(MPP_D0) | MASK(MPP_F0) | MASK(MPP_G0B) | MASK(MPP_G0) | MASK(MPP_A0) },
	{ { "$a6sus$S4", "$a6sus$f5" }, MASK(MPP_C0) | MASK(MPP_D0) | MASK(MPP_F0) | MASK(MPP_G0B) | MASK(MPP_A0B) | MASK(MPP_A0) },
	{ { "11sus$S4", "11sus$f5" }, MASK(MPP_C0) | MASK(MPP_D0) | MASK(MPP_F0) | MASK(MPP_G0B) | MASK(MPP_G0) | MASK(MPP_H0B) },
	{ { "$a11sus$S4", "$a11sus$f5" }, MASK(MPP_C0) | MASK(MPP_D0) | MASK(MPP_F0) | MASK(MPP_G0B) | MASK(MPP_A0B) | MASK(MPP_H0B) },
	{ { "$a6sus5" }, MASK(MPP_C0) | MASK(MPP_D0) | MASK(MPP_F0) | MASK(MPP_G0) | MASK(MPP_A0B) | MASK(MPP_A0) },
	{ { "$a11sus5" }, MASK(MPP_C0) | MASK(MPP_D0) | MASK(MPP_F0) | MASK(MPP_G0) | MASK(MPP_A0B) | MASK(MPP_H0B) },
	{ { "6sus$S5", "6sus$f6" }, MASK(MPP_C0) | MASK(MPP_D0) | MASK(MPP_F0) | MASK(MPP_G0) | MASK(MPP_A0B) | MASK(MPP_A0) },
	{ { "11sus$S5", "11sus$f6" }, MASK(MPP_C0) | MASK(MPP_D0) | MASK(MPP_F0) | MASK(MPP_G0) | MASK(MPP_A0B) | MASK(MPP_H0B) },
	{ { "11sus6" }, MASK(MPP_C0) | MASK(MPP_D0) | MASK(MPP_F0) | MASK(MPP_G0) | MASK(MPP_A0) | MASK(MPP_H0B) },
	{ { "$a11sus6" }, MASK(MPP_C0) | MASK(MPP_D0) | MASK(MPP_F0) | MASK(MPP_A0B) | MASK(MPP_A0) | MASK(MPP_H0B) },
	{ { "6sus$S6", "6sus$f7" }, MASK(MPP_C0) | MASK(MPP_D0) | MASK(MPP_F0) | MASK(MPP_G0) | MASK(MPP_A0) | MASK(MPP_H0B) },
	{ { "$a6sus$S6", "$a6sus$f7" }, MASK(MPP_C0) | MASK(MPP_D0) | MASK(MPP_F0) | MASK(MPP_A0B) | MASK(MPP_A0) | MASK(MPP_H0B) },
	{ { "6sus7" }, MASK(MPP_C0) | MASK(MPP_D0) | MASK(MPP_F0) | MASK(MPP_G0) | MASK(MPP_A0) | MASK(MPP_H0) },
	{ { "$a6sus7" }, MASK(MPP_C0) | MASK(MPP_D0) | MASK(MPP_F0) | MASK(MPP_A0B) | MASK(MPP_A0) | MASK(MPP_H0) },
	{ { "11sus7" }, MASK(MPP_C0) | MASK(MPP_D0) | MASK(MPP_F0) | MASK(MPP_G0) | MASK(MPP_H0B) | MASK(MPP_H0) },
	{ { "$a11sus7" }, MASK(MPP_C0) | MASK(MPP_D0) | MASK(MPP_F0) | MASK(MPP_A0B) | MASK(MPP_H0B) | MASK(MPP_H0) },
 
	/* thirteenth */
	{  { "13", "dom13" }, MASK(MPP_C0) | MASK(MPP_E0) | MASK(MPP_G0) | MASK(MPP_H0B) | MASK(MPP_D0) | MASK(MPP_F0) | MASK(MPP_A0) },
	{  { "$M13" }, MASK(MPP_C0) | MASK(MPP_E0) | MASK(MPP_G0) | MASK(MPP_H0) | MASK(MPP_D0) | MASK(MPP_F0) | MASK(MPP_A0) },
	{  { "$m$M13" }, MASK(MPP_C0) | MASK(MPP_E0B) | MASK(MPP_G0) | MASK(MPP_H0) | MASK(MPP_D0) | MASK(MPP_F0) | MASK(MPP_A0) },
	{  { "$m13" }, MASK(MPP_C0) | MASK(MPP_E0B) | MASK(MPP_G0) | MASK(MPP_H0B) | MASK(MPP_D0) | MASK(MPP_F0) | MASK(MPP_A0) },
	{  { "$a$M13" }, MASK(MPP_C0) | MASK(MPP_E0) | MASK(MPP_A0B) | MASK(MPP_H0) | MASK(MPP_D0) | MASK(MPP_F0) | MASK(MPP_A0) },
	{  { "$a13", "13$S5" }, MASK(MPP_C0) | MASK(MPP_E0) | MASK(MPP_A0B) | MASK(MPP_H0B) | MASK(MPP_D0) | MASK(MPP_F0) | MASK(MPP_A0) },
	{  { "$h13" }, MASK(MPP_C0) | MASK(MPP_E0B) | MASK(MPP_G0B) | MASK(MPP_H0B) | MASK(MPP_D0) | MASK(MPP_F0) | MASK(MPP_A0) },

	/* agumented and major thirteenths susXXX */
	{ { "13sus$S1", "13sus$f2" }, MASK(MPP_C0) | MASK(MPP_D0B) | MASK(MPP_D0) | MASK(MPP_F0) | MASK(MPP_G0) | MASK(MPP_A0) | MASK(MPP_H0B) },
	{ { "$a13sus$S1", "$a13sus$f2" }, MASK(MPP_C0) | MASK(MPP_D0B) | MASK(MPP_D0) | MASK(MPP_F0) | MASK(MPP_A0B) | MASK(MPP_A0) | MASK(MPP_H0B) },
	{ { "13sus$S2", "13sus$f3" }, MASK(MPP_C0) | MASK(MPP_D0) | MASK(MPP_E0B) | MASK(MPP_F0) | MASK(MPP_G0) | MASK(MPP_A0) | MASK(MPP_H0B) },
	{ { "$a13sus$S2", "$a13sus$f3" }, MASK(MPP_C0) | MASK(MPP_D0) | MASK(MPP_E0B) | MASK(MPP_F0) | MASK(MPP_A0B) | MASK(MPP_A0) | MASK(MPP_H0B) },
	{ { "13sus$S4", "13sus$f5" }, MASK(MPP_C0) | MASK(MPP_D0) | MASK(MPP_F0) | MASK(MPP_G0B) | MASK(MPP_G0) | MASK(MPP_A0) | MASK(MPP_H0B) },
	{ { "$a13sus$S4", "$a13sus$f5" }, MASK(MPP_C0) | MASK(MPP_D0) | MASK(MPP_F0) | MASK(MPP_G0B) | MASK(MPP_A0B) | MASK(MPP_A0) | MASK(MPP_H0B) },
	{ { "$a13sus5" }, MASK(MPP_C0) | MASK(MPP_D0) | MASK(MPP_F0) | MASK(MPP_G0) | MASK(MPP_A0B) | MASK(MPP_A0) | MASK(MPP_H0B) },
	{ { "13sus$S5", "13sus$f6" }, MASK(MPP_C0) | MASK(MPP_D0) | MASK(MPP_F0) | MASK(MPP_G0) | MASK(MPP_A0B) | MASK(MPP_A0) | MASK(MPP_H0B) },
	{ { "13sus7" }, MASK(MPP_C0) | MASK(MPP_D0) | MASK(MPP_F0) | MASK(MPP_G0) | MASK(MPP_A0) | MASK(MPP_H0B) | MASK(MPP_H0) },
	{ { "$a13sus7" }, MASK(MPP_C0) | MASK(MPP_D0) | MASK(MPP_F0) | MASK(MPP_A0B) | MASK(MPP_A0) | MASK(MPP_H0B) | MASK(MPP_H0) },
};

QString MppKeyStr[256];

#define	MAX_SCORES 2048

Q_DECL_EXPORT class score_variant *mpp_score_variant;

static unsigned mpp_max_variant;

static uint32_t
MppRor(uint32_t val, uint8_t n)
{
	return (((val >> n) | (val << (MPP_MAX_BANDS - n))) & 0xFFFFFFU);
}

static uint8_t
MppSumbits(uint32_t val)
{
	val = ((val & (1U * 0xAAAAAAAAU)) / 2U) + (val & (1U * 0x55555555U));
	val = ((val & (3U * 0x44444444U)) / 4U) + (val & (3U * 0x11111111U));
	val = ((val & (15U * 0x10101010U)) / 16U) + (val & (15U * 0x01010101U));
	val = ((val & (255U * 0x01000100U)) / 256U) + (val & (255U * 0x00010001U));
	val = ((val & (65535U * 0x00010000U)) / 65536U) + (val & (65535U * 0x00000001U));
	return (val);
}

static const QString
MppBitsToString(uint32_t bits)
{
	QString temp;
	uint8_t x;
	for (x = 0; x != MPP_MAX_BANDS; x++) {
		if (bits & 1) {
			temp += QString(score_bits[x]);
			if (bits != 1)
				temp += QString("-");
		}
		bits >>= 1;
	}
	return (temp);
}

static void
MppScoreStoreAdd(const class score_variant &orig, unsigned &y, const unsigned which)
{
	uint32_t mask = MASK(which) | MASK(MPP_H0);

	/* avoid sus and add together */
	if (orig.pattern.indexOf(QString("sus")) != -1)
		return;
	/* adds don't have 7th */
	if ((orig.footprint & mask) != 0)
		return;

	class score_variant temp = orig;
	temp.footprint |= MASK(which);
	temp.pattern.append(QString("add") + QString(score_name_sharp[which]));

	if (y < MAX_SCORES)
		mpp_score_variant[y++] = temp;

	if (strcmp(score_name_sharp[which], score_name_flat[which])) {
		class score_variant temp = orig;
		temp.footprint |= MASK(which);
		temp.pattern.append(QString("add") + QString(score_name_flat[which]));

		if (y < MAX_SCORES)
			mpp_score_variant[y++] = temp;
	}
}

static const QString
MppScoreExpandPattern(QString temp)
{
	while (1) {
	  	unsigned x;

		for (x = 0; score_macros[x]; x++) {
			if (temp.indexOf(score_macros[x][0]) != -1)
				break;
		}
		if (score_macros[x] == 0)
			return (temp);

		temp.replace(QString(score_macros[x][0]),
		    QString::fromUtf8(score_macros[x][1]));
	}
}

static const QString
MppScoreFullPattern(QString temp)
{
	while (1) {
	  	unsigned x;
	  	unsigned y;

		for (x = 0; score_macros[x]; x++) {
			if (temp.indexOf(score_macros[x][0]) != -1)
				break;
		}
		if (score_macros[x] == 0)
			return (temp);

		QString combos;

		combos += "{";
		for (y = 1; score_macros[x][y]; y++) {
			combos += QString::fromUtf8(score_macros[x][y]);
			if (score_macros[x][y+1])
				combos += ",";
		}
		combos += "}";

		temp.replace(QString(score_macros[x][0]), combos);
	}
}

static bool
MppScoreMatchPattern(const QString &pattern, const QString &str, int t = 0, int u = 0)
{
	while (1) {
		if (t >= pattern.length()) {
			return (u == str.length());
		} else if (u >= str.length()) {
			return (false);
		} else if (pattern[t] == '$') {
		  	int x;
			int y;
			for (x = 0; score_macros[x]; x++) {
				if (score_macros[x][0][1] == pattern[t + 1])
					break;
			}
			if (score_macros[x] == 0)
				return (false);
			for (y = 1; score_macros[x][y]; y++) {
				QString temp = QString::fromUtf8(score_macros[x][y]);
				if (str.indexOf(temp, u) == u) {
					/* there might be more matches, need to recurse */
					bool found = MppScoreMatchPattern(pattern, str, t + 2, u + temp.length());
					if (found)
						return (found);
				}
			}
			return (false);
		} else if (pattern[t] == str[u]) {
			t++;
			u++;
		} else {
			return (false);
		}
	}
}

static int
MppScoreVariantCmp(const void *pa, const void *pb)
{
	const class score_variant *sa = (const class score_variant *)pa;
	const class score_variant *sb = (const class score_variant *)pb;

	if (sa->pattern.length() > sb->pattern.length())
		return (1);
	if (sa->pattern.length() < sb->pattern.length())
		return (-1);
	if (sa->pattern > sb->pattern)
		return (1);
	if (sa->pattern < sb->pattern)
		return (-1);
	return (0);
}

void
MppScoreVariantInit(void)
{
	class score_variant temp;
	unsigned x;
	unsigned y;
	unsigned z;
	unsigned t;

	for (x = 0; x != 256; x++) {
		const char *ptr = MppBaseKeyToString24(x % MPP_MAX_BANDS, 0);
		MppKeyStr[x] = QString("%1%2%3").arg(QChar(ptr[0])).arg((x + 1) / MPP_MAX_BANDS)
		    .arg(ptr + 1).toUpper();
	}

	/* allocate array for score variants */
	mpp_score_variant = new score_variant [MAX_SCORES];

	for (x = y = 0; x != (sizeof(score_initial) / sizeof(score_initial[0])); x++) {
		const struct score_variant_initial *ps = score_initial + x;
		for (z = 0; z != MPP_SCORE_KEYMAX; z++) {
			if (ps->pattern[z] == 0)
				continue;

			temp.pattern = QString(ps->pattern[z]);
			temp.footprint = ps->footprint;

			if (y < MAX_SCORES)
				mpp_score_variant[y++] = temp;
		}
	}

	mpp_max_variant = y;

	/* compute all adds */
	
	for (x = 0; x != mpp_max_variant; x++) {
		for (z = 0; z != 2 * MPP_MAX_BANDS; z++)
			MppScoreStoreAdd(mpp_score_variant[x], y, z);
	}

	qsort(mpp_score_variant, y, sizeof(mpp_score_variant[0]), &MppScoreVariantCmp);

	for (x = 0; x != y; x++) {
		if (mpp_score_variant[x].duplicate)
			continue;
		for (z = x + 1; z != y; z++) {
			for (t = 0; t != MPP_MAX_BANDS; t++) {
				if (MppRor(mpp_score_variant[z].footprint, t) ==
					    mpp_score_variant[x].footprint)
					break;
			}
			if (t != MPP_MAX_BANDS)
				mpp_score_variant[z].duplicate = 1 + x;
		}
	}

	for (x = z = 0; x != y; x++) {
		if (mpp_score_variant[x].duplicate != 0)
			continue;
		Mpp.VariantList += QString("/* C") +
		    MppScoreFullPattern(mpp_score_variant[x].pattern) +
		    QString(" = C") +
		    MppScoreExpandPattern(mpp_score_variant[x].pattern) +
		    QString(" = ") +
		    MppBitsToString(mpp_score_variant[x].footprint) + QString(" */\n");
		z++;
	}
	Mpp.VariantList += QString("\n/* Number of supported uniqe chords is %1 */\n\n").arg(z);

	for (x = z = 0; x != y; x++) {
		if (mpp_score_variant[x].duplicate == 0)
			continue;
		temp = mpp_score_variant[mpp_score_variant[x].duplicate - 1];

		for (t = 0; t != MPP_MAX_BANDS; t++) {
			if (MppRor(mpp_score_variant[x].footprint, t) == temp.footprint)
				break;
		}
		
		Mpp.VariantList += QString("/* C") +
		    MppScoreFullPattern(mpp_score_variant[x].pattern) +
		    QString(" = C") +
		    MppScoreExpandPattern(mpp_score_variant[x].pattern) +
		    QString(" = ") + QString(score_bits[t]) +
		    MppScoreExpandPattern(temp.pattern) +
		    QString(" */\n");
		z++;
	}
	Mpp.VariantList += QString("\n/* Number of supported chord variants is %1 */\n\n").arg(z);

	for (x = 0; x != y; x++) {
		if (mpp_score_variant[x].duplicate)
			continue;
	}
	/* store maximum number of variants */
	mpp_max_variant = y;
}

static uint8_t
mpp_get_key(const QString &ptr, int &index)
{
	uint8_t key;
	if (index >= ptr.length())
		return (0);
	if (ptr[index] == 'C') {
		key = MPP_C0;
		index++;
	} else if (ptr[index] == 'D') {
		key = MPP_D0;
		index++;
	} else if (ptr[index] == 'E') {
		key = MPP_E0;
		index++;
	} else if (ptr[index] == 'F') {
		key = MPP_F0;
		index++;
	} else if (ptr[index] == 'G') {
		key = MPP_G0;
		index++;
	} else if (ptr[index] == 'A') {
		key = MPP_A0;
		index++;
	} else if (ptr[index] == 'H' || ptr[index] == 'B') {
		key = MPP_H0;
		index++;
	} else {
		return (0);
	}
	key += (MPP_MAX_BANDS * 5);
	if (index < ptr.length()) {
		switch (ptr[index].toLatin1()) {
#ifdef HAVE_QUARTERTONE
		case 'q':
			key -= 3;
			index++;
			break;
		case 'c':
			key -= 1;
			index++;
			break;
#endif
		case '#':
			key += MPP_BAND_STEP_12;
			index++;
			break;
		case 'b':
			key -= MPP_BAND_STEP_12;
			index++;
			break;
		default:
			break;
		}
	}
	return (key);
}

uint8_t
MppDecodeTab :: parseScoreChord(MppChordElement *pinfo)
{
	QString out;
	uint8_t temp[MPP_MAX_BANDS];
	uint32_t footprint = 0;
	unsigned int x;
	int y;
	int z;
	int n;
	int is_sharp;
	int rol;
	int flags;
	unsigned best_x;
	int best_y;
	int best_z;
	int best_l;

	if (pinfo->chord != 0)
		is_sharp = (pinfo->chord->txt.indexOf('#') > -1);
	else
		is_sharp = 0;

	for (x = 0; x != MPP_MAX_BANDS; x++) {
		if (pinfo->stats[x] != 0)
			footprint |= (1 << x);
	}

	if (footprint == 0)
		return (1);	/* not found */

	best_l = 0;
	best_x = mpp_max_variant;
	best_y = MPP_MAX_BANDS;
	best_z = MPP_MAX_BANDS;

	for (x = 0; x != mpp_max_variant; x++) {
		if (mpp_score_variant[x].duplicate)
			continue;
		int plen = mpp_score_variant[x].pattern.length();
		for (y = 0; y != MPP_MAX_BANDS; y++) {
			z = MppSumbits(mpp_score_variant[x].footprint ^ MppRor(footprint, y));
			/* if possible, get rid of the slash */
			if (z < best_z || (z == best_z && mpp_max_variant != best_x &&
			    best_l == plen && best_y != y && pinfo->key_base == y)) {
				best_z = z;
				best_x = x;
				best_y = y;
				best_l = plen;
			}
		}
	}
	x = best_x;
	y = best_y;

	if (x == mpp_max_variant || y == MPP_MAX_BANDS)
		return (1);	/* not found */

	for (n = z = 0; z != MPP_MAX_BANDS; z++) {
		if (mpp_score_variant[x].footprint & (1 << z))
			temp[n++] = z + y + (MPP_MAX_BANDS * 5);
	}
	if (n == 0)
		return (1);	/* not found */

	rol = 0;
	flags = 0;

	while (flags != 3 && rol < 126 && rol > -126) {

		mid_sort(temp, n);

		if (temp[n-1] < pinfo->key_max) {
			MppTrans(temp, n, 1);
			rol++;
			flags |= 1;
		} else if (temp[n-1] > pinfo->key_max) {
			MppTrans(temp, n, -1);
			rol--;
			flags |= 2;
		} else {
			break;
		}
	}

	rol_value = rol;

	out += MppBaseKeyToString24(y, is_sharp);
	out += MppScoreExpandPattern(mpp_score_variant[x].pattern);

	if (y != pinfo->key_base) {
		out += "/";
		out += MppBaseKeyToString24(pinfo->key_base, is_sharp);
	}

	lin_edit->setText(out);

	return (0);
}

Q_DECL_EXPORT uint8_t
mpp_find_chord(QString input, uint8_t *pbase,
    uint8_t *pkey, uint32_t *pvar)
{
	int index = 0;
	int temp;
	uint32_t x;
	uint8_t key;
	uint8_t base;

	if (pbase != NULL)
		*pbase = 0;
	if (pkey != NULL)
		*pkey = 0;
	if (pvar != NULL)
		*pvar = 0;

	key = base = mpp_get_key(input, index);
	if (key == 0)
		return (1);

	temp = input.indexOf("/", index);
	if (temp > -1) {
		int basepos = temp + 1;
		base = mpp_get_key(input, basepos);
		if (base == 0)
			base = key;
		input.truncate(temp);
	}
	input.remove(0, index);

	for (x = 0; x != mpp_max_variant; x++) {
		if (MppScoreMatchPattern(mpp_score_variant[x].pattern, input))
			break;
	}

	if (x == mpp_max_variant)
		return (1);

	if (pkey != NULL)
		*pkey = key;
	if (pbase != NULL)
		*pbase = base;
	if (pvar != NULL)
		*pvar = x;

	return (0);
}

Q_DECL_EXPORT uint8_t
mpp_parse_chord(const QString &input, int8_t rol,
    uint8_t *pout, uint8_t *pn, uint32_t *pvar,
    int change_var)
{
	uint32_t x;
	uint8_t error;
	uint8_t base;
	uint8_t key;
	uint8_t y;
	uint8_t n;

	error = mpp_find_chord(input, &base, &key, &x);
	if (error) {
		*pn = 0;
		*pvar = 0;
		return (1);
	}

	if (change_var != 0) {
		change_var %= (int)mpp_max_variant;
		x = (mpp_max_variant + change_var + x) % mpp_max_variant;

		/* skip duplicates */
		while (mpp_score_variant[x].duplicate) {
			if (change_var < 0)
				x = (mpp_max_variant + x - 1) % mpp_max_variant;
			else
				x = (mpp_max_variant + x + 1) % mpp_max_variant;
		}
	}

	n = 0;

	pout[n++] = base;

	for (y = 0; y != MPP_MAX_BANDS; y++) {
		if (!(mpp_score_variant[x].footprint & (1 << y)))
			continue;
		if (n < *pn)
			pout[n++] = y + key;
	}

	MppTrans(pout + 1, n - 1, rol);

	*pn = n;
	*pvar = x;

	return (0);
}

MppDecodeTab :: MppDecodeTab(MppMainWindow *_mw)
  : QWidget()
{
	int x;
	int n;

	rol_value = 0;
	delta_v = 0;

	mw = _mw;

	gl = new QGridLayout(this);

	gb = new MppGroupBox(tr("Chord Selector"));
	gl->addWidget(gb, 0,0,1,1);
	gl->setRowStretch(1,1);
	gl->setColumnStretch(1,1);

	lin_edit = new QLineEdit(QString('C'));
	lin_edit->setMaxLength(256);

	lin_out = new QLineEdit();
	lin_out->setMaxLength(256);

	lbl_status = new QLabel(tr(""));

	cbx_auto_base = new MppCheckBox();
	cbx_auto_base->setChecked(1);

	but_insert = new QPushButton(tr("Insert"));
	but_rol_up = new QPushButton(tr("Roll Up"));
	but_rol_down = new QPushButton(tr("Roll Down"));
	but_mod_up = new QPushButton(tr("Mod Up"));
	but_mod_down = new QPushButton(tr("Mod Down"));

	for (x = 0; x != MPP_MAX_VIEWS; x++) {
		char ch = 'A' + x;

		but_play[x][0] = new MppButton(tr("View %1\nPlay &90").arg(ch), 90 + (128 * x));
		but_play[x][1] = new MppButton(tr("View %1\nPlay &60").arg(ch), 60 + (128 * x));
		but_play[x][2] = new MppButton(tr("View %1\nPlay &30").arg(ch), 30 + (128 * x));

		for (n = 0; n != 3; n++) {
			connect(but_play[x][n], SIGNAL(pressed(int)), this, SLOT(handle_play_press(int)));
			connect(but_play[x][n], SIGNAL(released(int)), this, SLOT(handle_play_release(int)));
		}
	}
	connect(but_insert, SIGNAL(released()), this, SLOT(handle_insert()));
	connect(but_rol_up, SIGNAL(released()), this, SLOT(handle_rol_up()));
	connect(but_rol_down, SIGNAL(released()), this, SLOT(handle_rol_down()));
	connect(but_mod_up, SIGNAL(released()), this, SLOT(handle_mod_up()));
	connect(but_mod_down, SIGNAL(released()), this, SLOT(handle_mod_down()));
	connect(lin_edit, SIGNAL(textChanged(const QString &)), this, SLOT(handle_parse()));
	connect(cbx_auto_base, SIGNAL(stateChanged(int,int)), this, SLOT(handle_parse()));

	memset(current_score, 0, sizeof(current_score));
	memset(auto_base, 0, sizeof(auto_base));

	gb->addWidget(
#ifdef HAVE_QUARTERTONE
	    new QLabel(tr("[CDEFGABH][#qbc][...][/CDEFGABH[#qbc]]")),
#else
	    new QLabel(tr("[CDEFGABH][#b][...][/CDEFGABH[#b]]")),
#endif
	    0,0,1,4, Qt::AlignHCenter|Qt::AlignVCenter);

	gb->addWidget(but_rol_down, 2,0,1,1);
	gb->addWidget(but_rol_up, 2,1,1,1);

	gb->addWidget(but_mod_down, 2,2,1,1);
	gb->addWidget(but_mod_up, 2,3,1,1);

	gb->addWidget(lin_edit, 3,0,1,4);
	gb->addWidget(lin_out, 4,0,1,4);

	gb->addWidget(lbl_status, 5,0,1,2, Qt::AlignHCenter|Qt::AlignVCenter);
	gb->addWidget(new QLabel(tr("Add bass scores:")), 5,2,1,1, Qt::AlignRight|Qt::AlignVCenter);
	gb->addWidget(cbx_auto_base, 5,3,1,1, Qt::AlignHCenter|Qt::AlignVCenter);

	for (x = 0; x != MPP_MAX_VIEWS; x++) {
		for (n = 0; n != 3; n++)
			gb->addWidget(but_play[x][n], 6 + x, n, 1, 1);
	}
	gb->addWidget(but_insert, 6, 3, MPP_MAX_VIEWS, 2);

	handle_parse();
    
	but_insert->setFocus();
}

MppDecodeTab :: ~MppDecodeTab()
{

}

void
MppDecodeTab :: handle_play_press(int value)
{
	MppScoreMain *sm = mw->scores_main[value / 128];
	uint8_t vel = value & 127;
	uint8_t x;

	mw->atomic_lock();
	for (x = 0; x != MPP_MAX_BANDS; x++) {
		if (auto_base[x] != 0)
			mw->output_key(sm->synthChannel, auto_base[x], vel, 0, 0);
	}
	for (x = 0; current_score[x]; x++) {
		mw->output_key(sm->synthChannel, current_score[x], vel, 0, 0);
	}
	mw->atomic_unlock();
}

void
MppDecodeTab :: handle_play_release(int value)
{
	MppScoreMain *sm = mw->scores_main[value / 128];
	uint8_t x;

	mw->atomic_lock();
	for (x = 0; x != MPP_MAX_BANDS; x++) {
		if (auto_base[x] != 0)
			mw->output_key(sm->synthChannel, auto_base[x], 0, 0, 0);
	}
	for (x = 0; current_score[x]; x++) {
		mw->output_key(sm->synthChannel, current_score[x], 0, 0, 0);
	}
	mw->atomic_unlock();
}

void
MppDecodeTab :: handle_insert()
{
	QPlainTextEdit *qedit = mw->currEditor();
	if (qedit == 0)
		return;

	QTextCursor cursor(qedit->textCursor());
	MppChordElement info;
	MppElement *ptr;
	MppHead temp;
	int row;

	temp += qedit->toPlainText();
	temp.flush();

	row = cursor.blockNumber();

	cursor.beginEditBlock();

	/* check if the chord is valid */
	if (temp.getChord(row, &info) != 0) {
		if (info.chord != 0) {
			info.chord->txt = QChar('(') + lin_edit->text().trimmed() + QChar(')');

			cursor.movePosition(QTextCursor::Start, QTextCursor::MoveAnchor, 1);
			cursor.movePosition(QTextCursor::Down, QTextCursor::MoveAnchor, info.chord->line);
			cursor.movePosition(QTextCursor::EndOfLine, QTextCursor::MoveAnchor, 1);
			cursor.movePosition(QTextCursor::StartOfLine, QTextCursor::KeepAnchor, 1);
			cursor.removeSelectedText();
			cursor.insertText(temp.toPlain(info.chord->line).replace("\n", ""));
		}
		if (info.start != 0) {
			for (ptr = info.start; ptr != info.stop;
			    ptr = TAILQ_NEXT(ptr, entry)) {
				ptr->txt = QString();
			}

			info.start->txt = mw->led_config_insert->text() +
			    getText();

			cursor.movePosition(QTextCursor::Start, QTextCursor::MoveAnchor, 1);
			cursor.movePosition(QTextCursor::Down, QTextCursor::MoveAnchor, row);
			cursor.movePosition(QTextCursor::EndOfLine, QTextCursor::MoveAnchor, 1);
			cursor.movePosition(QTextCursor::StartOfLine, QTextCursor::KeepAnchor, 1);
			cursor.removeSelectedText();
			cursor.insertText(temp.toPlain(info.start->line).replace("\n", ""));
		}
	} else {
		cursor.removeSelectedText();
		cursor.insertText(mw->led_config_insert->text() + getText() + QChar('\n'));
	}
	cursor.endEditBlock();

	mw->handle_compile();
}

void
MppDecodeTab :: handle_rol_up()
{
	rol_value++;
	if (rol_value > 127)
		rol_value = 127;
	handle_parse();
}

void
MppDecodeTab :: handle_rol_down()
{
	rol_value--;
	if (rol_value < -127)
		rol_value = -127;
	handle_parse();
}

void
MppDecodeTab :: handle_mod_up()
{
	handle_parse(1);
}

void
MppDecodeTab :: handle_mod_down()
{
	handle_parse(-1);
}

void
MppDecodeTab :: handle_parse(int change_var)
{
	QString out;

	int error;
	int b_auto;
	int x;

	uint32_t var;
	uint8_t n;

	QString chord = lin_edit->text().trimmed();
	if (chord.isEmpty())
		return;

	out += "U1 ";

	memset(current_score, 0, sizeof(current_score));
	memset(auto_base, 0, sizeof(auto_base));

	n = sizeof(current_score) / sizeof(current_score[0]);
	error = mpp_parse_chord(chord, rol_value, current_score,
	    &n, &var, change_var);

	if (error == 0) {
		if (change_var && chord.length() > 0) {
			int endpos = chord.indexOf("/");
			int startpos = 1;

			if (endpos < 0)
				endpos = chord.length();

			if ((endpos > startpos) &&
			    (chord[startpos] == 'b' ||
			     chord[startpos] == 'B' ||
#ifdef HAVE_QUARTERTONE
			     chord[startpos] == 'q' ||
			     chord[startpos] == 'c' ||
#endif
			     chord[startpos] == '#'))
				startpos++;	/* keep modifier too */

			QString temp = MppScoreExpandPattern(mpp_score_variant[var].pattern);

			/* replace chord type */
			chord.replace(startpos, endpos - startpos, temp);

			/* store chord */
			lin_edit->setText(chord);
		}
		lbl_status->setText(tr("OK"));
	} else {
		lbl_status->setText(tr("ERROR"));
		goto done;
	}

	b_auto = current_score[0];

	if (cbx_auto_base->isChecked()) {

		while (b_auto >= (int)current_score[1])
			b_auto -= MPP_MAX_BANDS;

		if (b_auto >= MPP_MAX_BANDS) {
			auto_base[0] = b_auto - MPP_MAX_BANDS;
			out += QString(MppKeyStr[auto_base[0]]) + QString(" ");
		}
		if (b_auto >= 0) {
			auto_base[1] = b_auto;
			out += QString(MppKeyStr[auto_base[1]]) + QString(" ");
		}
	}

	for (x = 1; x < n; x++) {
		out += QString(MppKeyStr[current_score[x]]) + QString(" ");
	}

done:
	out += QString("/* ") + lin_edit->text().trimmed() + QString(" */");

	lin_out->setText(out);
}

QString
MppDecodeTab :: getText()
{
	return (lin_out->text());
}

void
MppDecodeTab :: setText(QString str)
{
	lin_edit->setText(str);

	handle_parse();
}

void
MppDecodeTab :: keyPressEvent(QKeyEvent *event)
{
	switch (event->key()) {
	case Qt::Key_Up:
		handle_parse(1);
		break;
	case Qt::Key_Down:
		handle_parse(-1);
		break;
	default:
		break;
	}
}

void
MppDecodeTab :: wheelEvent(QWheelEvent *event)
{
	if (event->orientation() == Qt::Vertical) {
		delta_v -= event->delta();
		int delta = delta_v / MPP_WHEEL_STEP;
		delta_v %= MPP_WHEEL_STEP;
		if (delta != 0)
			handle_parse(delta);
	}
	event->accept();
}
