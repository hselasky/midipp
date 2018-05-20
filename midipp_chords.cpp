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

#include "midipp_chords.h"

/* allowed chord modifier characters */

const QString MppChordModChars = QString::fromUtf8("%./+-#Δ&|^°");

/* common public chord standard */

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
	"$d", "°", "dim", 0
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

static const char *score_sharp[] = {
	"$S", "+", "#", 0
};

static const char *score_flat[] = {
	"$f", "-", "b", "°", "dim", 0
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
	score_half_dim,
	score_seventh,
	0
};

/*
 * C D E F G A B C D E  F  G  A  B  C
 * 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15
 * =   =   =
 */

#define	MASK(x) (1U << (x))
#define	INIT(...) MppScoreVariant(__VA_ARGS__)

/* The list is sorted by priority */

static const class MppScoreVariant MppScoreVariants12[] = {
	/* one */
	INIT(MASK(MPP_C0), "1", 0),

	/* third */
	INIT(MASK(MPP_C0) | MASK(MPP_G0), "5", 0),

	/* triads */
	INIT(MASK(MPP_C0) | MASK(MPP_D0B) | MASK(MPP_D0), "$A", 0),	/* custom */
	INIT(MASK(MPP_C0) | MASK(MPP_D0B) | MASK(MPP_E0B), "$O", 0),	/* custom */
	INIT(MASK(MPP_C0) | MASK(MPP_D0) | MASK(MPP_E0B), "$X", 0),	/* custom */
	INIT(MASK(MPP_C0) | MASK(MPP_E0B) | MASK(MPP_G0B), "$d", "$m$f5", 0),
	INIT(MASK(MPP_C0) | MASK(MPP_E0B) | MASK(MPP_G0), "$m", "sus$S2", "sus$f3", 0),
	INIT(MASK(MPP_C0) | MASK(MPP_E0) | MASK(MPP_A0B), "$a", "$M$S5", 0),
	INIT(MASK(MPP_C0) | MASK(MPP_E0) | MASK(MPP_G0), "", "$M", 0),

	/* augumented and major triads with susXXX */
	INIT(MASK(MPP_C0) | MASK(MPP_D0B) | MASK(MPP_G0), "sus$S1", "sus$f2", 0),
	INIT(MASK(MPP_C0) | MASK(MPP_D0B) | MASK(MPP_A0B), "$asus$S1", "$asus$f2", 0),
	INIT(MASK(MPP_C0) | MASK(MPP_D0) | MASK(MPP_G0), "sus2", 0),
	INIT(MASK(MPP_C0) | MASK(MPP_D0) | MASK(MPP_A0B), "$asus2", 0),
	INIT(MASK(MPP_C0) | MASK(MPP_E0B) | MASK(MPP_G0), "sus$S2", "sus$f3", 0),
	INIT(MASK(MPP_C0) | MASK(MPP_E0B) | MASK(MPP_A0B), "$asus$S2", "$asus$f3", 0),
	INIT(MASK(MPP_C0) | MASK(MPP_F0) | MASK(MPP_G0), "sus4", "sus", 0),
	INIT(MASK(MPP_C0) | MASK(MPP_F0) | MASK(MPP_A0B), "$asus4", "$asus", 0),
	INIT(MASK(MPP_C0) | MASK(MPP_G0B) | MASK(MPP_G0), "sus$S4", "sus$f5", 0),
	INIT(MASK(MPP_C0) | MASK(MPP_G0B) | MASK(MPP_A0B), "$asus$S4", "$asus$f5", 0),
	INIT(MASK(MPP_C0) | MASK(MPP_G0) | MASK(MPP_A0B), "$asus5", 0),
	INIT(MASK(MPP_C0) | MASK(MPP_G0) | MASK(MPP_A0B), "sus$S5", "sus$f6", 0),
	INIT(MASK(MPP_C0) | MASK(MPP_G0) | MASK(MPP_A0), "sus6", 0),
	INIT(MASK(MPP_C0) | MASK(MPP_A0B) | MASK(MPP_A0), "$asus6", 0),
	INIT(MASK(MPP_C0) | MASK(MPP_G0) | MASK(MPP_H0B), "sus$S6", "sus$f7", 0),
	INIT(MASK(MPP_C0) | MASK(MPP_A0B) | MASK(MPP_H0B), "$asus$S6", "$asus$f7", 0),
	INIT(MASK(MPP_C0) | MASK(MPP_G0) | MASK(MPP_H0), "sus7", 0),
	INIT(MASK(MPP_C0) | MASK(MPP_A0B) | MASK(MPP_H0), "$asus7", 0),

	/* seventh */
	INIT(MASK(MPP_C0) | MASK(MPP_E0) | MASK(MPP_G0) | MASK(MPP_H0B), "$7", 0),
	INIT(MASK(MPP_C0) | MASK(MPP_E0) | MASK(MPP_A0B) | MASK(MPP_H0B), "$a7", "$7$S5", "$7$S", 0),
	INIT(MASK(MPP_C0) | MASK(MPP_E0B) | MASK(MPP_G0B) | MASK(MPP_H0B), "$h", "$h7", "$m7$f5", 0),
	INIT(MASK(MPP_C0) | MASK(MPP_E0) | MASK(MPP_G0B) | MASK(MPP_H0B), "$7$f5", 0),
	INIT(MASK(MPP_C0) | MASK(MPP_E0) | MASK(MPP_G0) | MASK(MPP_D0B) | MASK(MPP_H0B), "$7$f9", "$7$f2", 0),
	INIT(MASK(MPP_C0) | MASK(MPP_E0) | MASK(MPP_G0) | MASK(MPP_E0B) | MASK(MPP_H0B), "$7$S9", "$7$S2", 0),
	INIT(MASK(MPP_C0) | MASK(MPP_E0) | MASK(MPP_G0) | MASK(MPP_H0), "$M7", 0),
	INIT(MASK(MPP_C0) | MASK(MPP_E0) | MASK(MPP_A0B) | MASK(MPP_H0), "$a$M7", "$M7$S5", "$M7$S", 0),
	INIT(MASK(MPP_C0) | MASK(MPP_E0B) | MASK(MPP_G0) | MASK(MPP_H0), "$m$M7", 0),
	INIT(MASK(MPP_C0) | MASK(MPP_E0B) | MASK(MPP_G0) | MASK(MPP_H0B), "$m7", 0),
	INIT(MASK(MPP_C0) | MASK(MPP_E0B) | MASK(MPP_G0B) | MASK(MPP_A0), "$d7", 0),

	/* second */
	INIT(MASK(MPP_C0) | MASK(MPP_E0) | MASK(MPP_G0) | MASK(MPP_D0), "2", "$M2", "dom2", 0),
	INIT(MASK(MPP_C0) | MASK(MPP_E0) | MASK(MPP_A0B) | MASK(MPP_D0), "$a2", "2$S5", 0),
	INIT(MASK(MPP_C0) | MASK(MPP_E0B) | MASK(MPP_G0) | MASK(MPP_D0), "$m$M2", 0),
	INIT(MASK(MPP_C0) | MASK(MPP_E0B) | MASK(MPP_G0) | MASK(MPP_D0), "$m2", 0),
	INIT(MASK(MPP_C0) | MASK(MPP_E0) | MASK(MPP_A0B) | MASK(MPP_D0), "$a$M2", 0),
	INIT(MASK(MPP_C0) | MASK(MPP_E0B) | MASK(MPP_G0B) | MASK(MPP_D0), "$h2", 0),
	INIT(MASK(MPP_C0) | MASK(MPP_E0B) | MASK(MPP_G0B) | MASK(MPP_D0B), "$h$f2", 0),

	/* agumented and major seconds and sevenths susXXX */
	INIT(MASK(MPP_C0) | MASK(MPP_D0B) | MASK(MPP_G0) | MASK(MPP_H0B), "7sus$S1", "7sus$f2", 0),
	INIT(MASK(MPP_C0) | MASK(MPP_D0B) | MASK(MPP_A0B) | MASK(MPP_H0B), "$a7sus$S1", "$a7sus$f2", 0),
	INIT(MASK(MPP_C0) | MASK(MPP_D0B) | MASK(MPP_D0) | MASK(MPP_G0), "2sus$S1", "2sus$f2", 0),
	INIT(MASK(MPP_C0) | MASK(MPP_D0B) | MASK(MPP_D0) | MASK(MPP_A0B), "$a2sus$S1", "$a2sus$f2", 0),
	INIT(MASK(MPP_C0) | MASK(MPP_D0) | MASK(MPP_G0) | MASK(MPP_H0B), "7sus2", 0),
	INIT(MASK(MPP_C0) | MASK(MPP_D0) | MASK(MPP_A0B) | MASK(MPP_H0B), "$a7sus2", 0),
	INIT(MASK(MPP_C0) | MASK(MPP_E0B) | MASK(MPP_G0) | MASK(MPP_H0B), "7sus$S2", "7sus$f3", 0),
	INIT(MASK(MPP_C0) | MASK(MPP_E0B) | MASK(MPP_A0B) | MASK(MPP_H0B), "$a7sus$S2", "$a7sus$f3", 0),
	INIT(MASK(MPP_C0) | MASK(MPP_D0) | MASK(MPP_E0B) | MASK(MPP_G0), "2sus$S2", "2sus$f3", 0),
	INIT(MASK(MPP_C0) | MASK(MPP_D0) | MASK(MPP_E0B) | MASK(MPP_A0B), "$a2sus$S2", "$a2sus$f3", 0),
	INIT(MASK(MPP_C0) | MASK(MPP_F0) | MASK(MPP_G0) | MASK(MPP_H0B), "7sus4", "7sus", 0),
	INIT(MASK(MPP_C0) | MASK(MPP_F0) | MASK(MPP_A0B) | MASK(MPP_H0B), "$a7sus4", "$a7sus", 0),
	INIT(MASK(MPP_C0) | MASK(MPP_D0) | MASK(MPP_F0) | MASK(MPP_G0), "2sus4", "2sus", 0),
	INIT(MASK(MPP_C0) | MASK(MPP_D0) | MASK(MPP_F0) | MASK(MPP_A0B), "$a2sus4", "$a2sus", 0),
	INIT(MASK(MPP_C0) | MASK(MPP_G0B) | MASK(MPP_G0) | MASK(MPP_H0B), "7sus$S4", "7sus$f5", 0),
	INIT(MASK(MPP_C0) | MASK(MPP_G0B) | MASK(MPP_A0B) | MASK(MPP_H0B), "$a7sus$S4", "$a7sus$f5", 0),
	INIT(MASK(MPP_C0) | MASK(MPP_D0) | MASK(MPP_G0B) | MASK(MPP_G0), "2sus$S4", "2sus$f5", 0),
	INIT(MASK(MPP_C0) | MASK(MPP_D0) | MASK(MPP_G0B) | MASK(MPP_A0B), "$a2sus$S4", "$a2sus$f5", 0),
	INIT(MASK(MPP_C0) | MASK(MPP_G0) | MASK(MPP_A0B) | MASK(MPP_H0B), "$a7sus5", 0),
	INIT(MASK(MPP_C0) | MASK(MPP_D0) | MASK(MPP_G0) | MASK(MPP_A0B), "$a2sus5", 0),
	INIT(MASK(MPP_C0) | MASK(MPP_G0) | MASK(MPP_A0B) | MASK(MPP_H0B), "7sus$S5", "7sus$f6", 0),
	INIT(MASK(MPP_C0) | MASK(MPP_D0) | MASK(MPP_G0) | MASK(MPP_A0B), "2sus$S5", "2sus$f6", 0),
	INIT(MASK(MPP_C0) | MASK(MPP_G0) | MASK(MPP_A0) | MASK(MPP_H0B), "7sus6", 0),
	INIT(MASK(MPP_C0) | MASK(MPP_A0B) | MASK(MPP_A0) | MASK(MPP_H0B), "$a7sus6", 0),
	INIT(MASK(MPP_C0) | MASK(MPP_D0) | MASK(MPP_G0) | MASK(MPP_A0), "2sus6", 0),
	INIT(MASK(MPP_C0) | MASK(MPP_D0) | MASK(MPP_A0B) | MASK(MPP_A0), "$a2sus6", 0),
	INIT(MASK(MPP_C0) | MASK(MPP_D0) | MASK(MPP_G0) | MASK(MPP_H0B), "2sus$S6", "2sus$f7", 0),
	INIT(MASK(MPP_C0) | MASK(MPP_D0) | MASK(MPP_A0B) | MASK(MPP_H0B), "$a2sus$S6", "$a2sus$f7", 0),
	INIT(MASK(MPP_C0) | MASK(MPP_G0) | MASK(MPP_H0B) | MASK(MPP_H0), "7sus7", 0),
	INIT(MASK(MPP_C0) | MASK(MPP_A0B) | MASK(MPP_H0B) | MASK(MPP_H0), "$a7sus7", 0),
	INIT(MASK(MPP_C0) | MASK(MPP_D0) | MASK(MPP_G0) | MASK(MPP_H0), "2sus7", 0),
	INIT(MASK(MPP_C0) | MASK(MPP_D0) | MASK(MPP_A0B) | MASK(MPP_H0), "$a2sus7", 0),

	/* fourth */
	INIT(MASK(MPP_C0) | MASK(MPP_E0) | MASK(MPP_G0) | MASK(MPP_D0) | MASK(MPP_F0), "4", "$M4", "dom4", 0),
	INIT(MASK(MPP_C0) | MASK(MPP_E0B) | MASK(MPP_G0) | MASK(MPP_D0) | MASK(MPP_F0), "$m4", "$m$M4", 0),
	INIT(MASK(MPP_C0) | MASK(MPP_E0) | MASK(MPP_A0B) | MASK(MPP_D0) | MASK(MPP_F0), "$a4", "4$S5", "$a$M4", 0),
	INIT(MASK(MPP_C0) | MASK(MPP_E0B) | MASK(MPP_G0B) | MASK(MPP_D0B) | MASK(MPP_F0), "$h4", 0),

	/* ninth */
	INIT(MASK(MPP_C0) | MASK(MPP_E0) | MASK(MPP_G0) | MASK(MPP_H0B) | MASK(MPP_D0), "9", "dom9", 0),
	INIT(MASK(MPP_C0) | MASK(MPP_E0) | MASK(MPP_G0) | MASK(MPP_H0) | MASK(MPP_D0), "$M9", 0),
	INIT(MASK(MPP_C0) | MASK(MPP_E0B) | MASK(MPP_G0) | MASK(MPP_H0) | MASK(MPP_D0), "$m$M9", 0),
	INIT(MASK(MPP_C0) | MASK(MPP_E0B) | MASK(MPP_G0) | MASK(MPP_H0B) | MASK(MPP_D0), "$m9", 0),
	INIT(MASK(MPP_C0) | MASK(MPP_E0) | MASK(MPP_A0B) | MASK(MPP_H0) | MASK(MPP_D0), "$a$M9", 0),
	INIT(MASK(MPP_C0) | MASK(MPP_E0) | MASK(MPP_A0B) | MASK(MPP_H0B) | MASK(MPP_D0), "$a9", "9$S5", 0),
	INIT(MASK(MPP_C0) | MASK(MPP_E0B) | MASK(MPP_G0B) | MASK(MPP_H0B) | MASK(MPP_D0), "$h9", 0),
	INIT(MASK(MPP_C0) | MASK(MPP_E0B) | MASK(MPP_G0B) | MASK(MPP_H0B) | MASK(MPP_D0B), "$h$f9", 0),
	INIT(MASK(MPP_C0) | MASK(MPP_E0B) | MASK(MPP_G0B) | MASK(MPP_A0) | MASK(MPP_D0), "$d9", 0),
	INIT(MASK(MPP_C0) | MASK(MPP_E0B) | MASK(MPP_G0B) | MASK(MPP_A0) | MASK(MPP_D0B), "$db9", 0),

	/* agumented and major fourths and ninths susXXX */
	INIT(MASK(MPP_C0) | MASK(MPP_D0B) | MASK(MPP_D0) | MASK(MPP_F0) | MASK(MPP_G0), "4sus$S1", "4sus$f2", 0),
	INIT(MASK(MPP_C0) | MASK(MPP_D0B) | MASK(MPP_D0) | MASK(MPP_F0) | MASK(MPP_A0B), "$a4sus$S1", "$a4sus$f2", 0),
	INIT(MASK(MPP_C0) | MASK(MPP_D0B) | MASK(MPP_D0) | MASK(MPP_G0) | MASK(MPP_H0B), "9sus$S1", "9sus$f2", 0),
	INIT(MASK(MPP_C0) | MASK(MPP_D0B) | MASK(MPP_D0) | MASK(MPP_A0B) | MASK(MPP_H0B), "$a9sus$S1", "$a9sus$f2", 0),
	INIT(MASK(MPP_C0) | MASK(MPP_D0) | MASK(MPP_E0B) | MASK(MPP_F0) | MASK(MPP_G0), "4sus$S2", "4sus$f3", 0),
	INIT(MASK(MPP_C0) | MASK(MPP_D0) | MASK(MPP_E0B) | MASK(MPP_F0) | MASK(MPP_A0B), "$a4sus$S2", "$a4sus$f3", 0),
	INIT(MASK(MPP_C0) | MASK(MPP_D0) | MASK(MPP_E0B) | MASK(MPP_G0) | MASK(MPP_H0B), "9sus$S2", "9sus$f3", 0),
	INIT(MASK(MPP_C0) | MASK(MPP_D0) | MASK(MPP_E0B) | MASK(MPP_A0B) | MASK(MPP_H0B), "$a9sus$S2", "$a9sus$f3", 0),
	INIT(MASK(MPP_C0) | MASK(MPP_D0) | MASK(MPP_F0) | MASK(MPP_G0) | MASK(MPP_H0B), "9sus4", "9sus", 0),
	INIT(MASK(MPP_C0) | MASK(MPP_D0) | MASK(MPP_F0) | MASK(MPP_A0B) | MASK(MPP_H0B), "$a9sus4", "$a9sus", 0),
	INIT(MASK(MPP_C0) | MASK(MPP_D0) | MASK(MPP_F0) | MASK(MPP_G0B) | MASK(MPP_G0), "4sus$S4", "4sus$f5", 0),
	INIT(MASK(MPP_C0) | MASK(MPP_D0) | MASK(MPP_F0) | MASK(MPP_G0B) | MASK(MPP_A0B), "$a4sus$S4", "$a4sus$f5", 0),
	INIT(MASK(MPP_C0) | MASK(MPP_D0) | MASK(MPP_G0B) | MASK(MPP_G0) | MASK(MPP_H0B), "9sus$S4", "9sus$f5", 0),
	INIT(MASK(MPP_C0) | MASK(MPP_D0) | MASK(MPP_G0B) | MASK(MPP_A0B) | MASK(MPP_H0B), "$a9sus$S4", "$a9sus$f5", 0),
	INIT(MASK(MPP_C0) | MASK(MPP_D0) | MASK(MPP_F0) | MASK(MPP_G0) | MASK(MPP_A0B), "$a4sus5", 0),
	INIT(MASK(MPP_C0) | MASK(MPP_D0) | MASK(MPP_G0) | MASK(MPP_A0B) | MASK(MPP_H0B), "$a9sus5", 0),
	INIT(MASK(MPP_C0) | MASK(MPP_D0) | MASK(MPP_F0) | MASK(MPP_G0) | MASK(MPP_A0B), "4sus$S5", "4sus$f6", 0),
	INIT(MASK(MPP_C0) | MASK(MPP_D0) | MASK(MPP_G0) | MASK(MPP_A0B) | MASK(MPP_H0B), "9sus$S5", "9sus$f6", 0),
	INIT(MASK(MPP_C0) | MASK(MPP_D0) | MASK(MPP_F0) | MASK(MPP_G0) | MASK(MPP_A0), "4sus6", 0),
	INIT(MASK(MPP_C0) | MASK(MPP_D0) | MASK(MPP_F0) | MASK(MPP_A0B) | MASK(MPP_A0), "$a4sus6", 0),
	INIT(MASK(MPP_C0) | MASK(MPP_D0) | MASK(MPP_G0) | MASK(MPP_A0) | MASK(MPP_H0B), "9sus6", 0),
	INIT(MASK(MPP_C0) | MASK(MPP_D0) | MASK(MPP_A0B) | MASK(MPP_A0) | MASK(MPP_H0B), "$a9sus6", 0),
	INIT(MASK(MPP_C0) | MASK(MPP_D0) | MASK(MPP_F0) | MASK(MPP_G0) | MASK(MPP_H0B), "4sus$S6", "4sus$f7", 0),
	INIT(MASK(MPP_C0) | MASK(MPP_D0) | MASK(MPP_F0) | MASK(MPP_A0B) | MASK(MPP_H0B), "$a4sus$S6", "$a4sus$f7", 0),
	INIT(MASK(MPP_C0) | MASK(MPP_D0) | MASK(MPP_F0) | MASK(MPP_G0) | MASK(MPP_H0), "4sus7", 0),
	INIT(MASK(MPP_C0) | MASK(MPP_D0) | MASK(MPP_F0) | MASK(MPP_A0B) | MASK(MPP_H0), "$a4sus7", 0),
	INIT(MASK(MPP_C0) | MASK(MPP_D0) | MASK(MPP_G0) | MASK(MPP_H0B) | MASK(MPP_H0), "9sus7", 0),
	INIT(MASK(MPP_C0) | MASK(MPP_D0) | MASK(MPP_A0B) | MASK(MPP_H0B) | MASK(MPP_H0), "$a9sus7", 0),

	/* sixth */
	INIT(MASK(MPP_C0) | MASK(MPP_E0) | MASK(MPP_G0) | MASK(MPP_D0) | MASK(MPP_F0) | MASK(MPP_A0), "6", "$M6", "dom6", 0),
	INIT(MASK(MPP_C0) | MASK(MPP_E0) | MASK(MPP_G0) | MASK(MPP_A0) | MASK(MPP_D0), "62", 0),
	INIT(MASK(MPP_C0) | MASK(MPP_E0) | MASK(MPP_G0) | MASK(MPP_A0) | MASK(MPP_D0), "69", 0),
	INIT(MASK(MPP_C0) | MASK(MPP_E0) | MASK(MPP_G0) | MASK(MPP_A0) | MASK(MPP_F0), "64", 0),
	INIT(MASK(MPP_C0) | MASK(MPP_E0B) | MASK(MPP_G0) | MASK(MPP_D0) | MASK(MPP_F0) | MASK(MPP_A0), "$m6", "$m$M6", 0),
	INIT(MASK(MPP_C0) | MASK(MPP_E0) | MASK(MPP_A0B) | MASK(MPP_D0) | MASK(MPP_F0) | MASK(MPP_A0), "$a6", "6$S5", "$a$M6", 0),
	INIT(MASK(MPP_C0) | MASK(MPP_E0B) | MASK(MPP_G0B) | MASK(MPP_D0) | MASK(MPP_F0) | MASK(MPP_A0), "$h6", 0),

	/* eleventh */
	INIT(MASK(MPP_C0) | MASK(MPP_E0) | MASK(MPP_G0) | MASK(MPP_H0B) | MASK(MPP_D0) | MASK(MPP_F0), "11", "dom11", 0),
	INIT(MASK(MPP_C0) | MASK(MPP_E0) | MASK(MPP_G0) | MASK(MPP_H0) | MASK(MPP_D0) | MASK(MPP_F0), "$M11", 0),
	INIT(MASK(MPP_C0) | MASK(MPP_E0B) | MASK(MPP_G0) | MASK(MPP_H0) | MASK(MPP_D0) | MASK(MPP_F0), "$m$M11", 0),
	INIT(MASK(MPP_C0) | MASK(MPP_E0B) | MASK(MPP_G0) | MASK(MPP_H0B) | MASK(MPP_D0) | MASK(MPP_F0), "$m11", 0),
	INIT(MASK(MPP_C0) | MASK(MPP_E0) | MASK(MPP_A0B) | MASK(MPP_H0) | MASK(MPP_D0) | MASK(MPP_F0), "$a$M11", 0),
	INIT(MASK(MPP_C0) | MASK(MPP_E0) | MASK(MPP_A0B) | MASK(MPP_H0B) | MASK(MPP_D0) | MASK(MPP_F0), "$a11", "11$S5", 0),
	INIT(MASK(MPP_C0) | MASK(MPP_E0B) | MASK(MPP_G0B) | MASK(MPP_H0B) | MASK(MPP_D0B) | MASK(MPP_F0), "$h11", 0),
	INIT(MASK(MPP_C0) | MASK(MPP_E0B) | MASK(MPP_G0B) | MASK(MPP_A0) | MASK(MPP_D0B) | MASK(MPP_E0), "$d11", 0),

	/* agumented and major sixths and elevenths susXXX */
	INIT(MASK(MPP_C0) | MASK(MPP_D0B) | MASK(MPP_D0) | MASK(MPP_F0) | MASK(MPP_G0) | MASK(MPP_A0), "6sus$S1", "6sus$f2", 0),
	INIT(MASK(MPP_C0) | MASK(MPP_D0B) | MASK(MPP_D0) | MASK(MPP_F0) | MASK(MPP_A0B) | MASK(MPP_A0), "$a6sus$S1", "$a6sus$f2", 0),
	INIT(MASK(MPP_C0) | MASK(MPP_D0B) | MASK(MPP_D0) | MASK(MPP_F0) | MASK(MPP_G0) | MASK(MPP_H0B), "11sus$S1", "11sus$f2", 0),
	INIT(MASK(MPP_C0) | MASK(MPP_D0B) | MASK(MPP_D0) | MASK(MPP_F0) | MASK(MPP_A0B) | MASK(MPP_H0B), "$a11sus$S1", "$a11sus$f2", 0),
	INIT(MASK(MPP_C0) | MASK(MPP_D0) | MASK(MPP_E0B) | MASK(MPP_F0) | MASK(MPP_G0) | MASK(MPP_A0), "6sus$S2", "6sus$f3", 0),
	INIT(MASK(MPP_C0) | MASK(MPP_D0) | MASK(MPP_E0B) | MASK(MPP_F0) | MASK(MPP_A0B) | MASK(MPP_A0), "$a6sus$S2", "$a6sus$f3", 0),
	INIT(MASK(MPP_C0) | MASK(MPP_D0) | MASK(MPP_E0B) | MASK(MPP_F0) | MASK(MPP_G0) | MASK(MPP_H0B), "11sus$S2", "11sus$f3", 0),
	INIT(MASK(MPP_C0) | MASK(MPP_D0) | MASK(MPP_E0B) | MASK(MPP_F0) | MASK(MPP_A0B) | MASK(MPP_H0B), "$a11sus$S2", "$a11sus$f3", 0),
	INIT(MASK(MPP_C0) | MASK(MPP_D0) | MASK(MPP_F0) | MASK(MPP_G0B) | MASK(MPP_G0) | MASK(MPP_A0), "6sus$S4", "6sus$f5", 0),
	INIT(MASK(MPP_C0) | MASK(MPP_D0) | MASK(MPP_F0) | MASK(MPP_G0B) | MASK(MPP_A0B) | MASK(MPP_A0), "$a6sus$S4", "$a6sus$f5", 0),
	INIT(MASK(MPP_C0) | MASK(MPP_D0) | MASK(MPP_F0) | MASK(MPP_G0B) | MASK(MPP_G0) | MASK(MPP_H0B), "11sus$S4", "11sus$f5", 0),
	INIT(MASK(MPP_C0) | MASK(MPP_D0) | MASK(MPP_F0) | MASK(MPP_G0B) | MASK(MPP_A0B) | MASK(MPP_H0B), "$a11sus$S4", "$a11sus$f5", 0),
	INIT(MASK(MPP_C0) | MASK(MPP_D0) | MASK(MPP_F0) | MASK(MPP_G0) | MASK(MPP_A0B) | MASK(MPP_A0), "$a6sus5", 0),
	INIT(MASK(MPP_C0) | MASK(MPP_D0) | MASK(MPP_F0) | MASK(MPP_G0) | MASK(MPP_A0B) | MASK(MPP_H0B), "$a11sus5", 0),
	INIT(MASK(MPP_C0) | MASK(MPP_D0) | MASK(MPP_F0) | MASK(MPP_G0) | MASK(MPP_A0B) | MASK(MPP_A0), "6sus$S5", "6sus$f6", 0),
	INIT(MASK(MPP_C0) | MASK(MPP_D0) | MASK(MPP_F0) | MASK(MPP_G0) | MASK(MPP_A0B) | MASK(MPP_H0B), "11sus$S5", "11sus$f6", 0),
	INIT(MASK(MPP_C0) | MASK(MPP_D0) | MASK(MPP_F0) | MASK(MPP_G0) | MASK(MPP_A0) | MASK(MPP_H0B), "11sus6", 0),
	INIT(MASK(MPP_C0) | MASK(MPP_D0) | MASK(MPP_F0) | MASK(MPP_A0B) | MASK(MPP_A0) | MASK(MPP_H0B), "$a11sus6", 0),
	INIT(MASK(MPP_C0) | MASK(MPP_D0) | MASK(MPP_F0) | MASK(MPP_G0) | MASK(MPP_A0) | MASK(MPP_H0B), "6sus$S6", "6sus$f7", 0),
	INIT(MASK(MPP_C0) | MASK(MPP_D0) | MASK(MPP_F0) | MASK(MPP_A0B) | MASK(MPP_A0) | MASK(MPP_H0B), "$a6sus$S6", "$a6sus$f7", 0),
	INIT(MASK(MPP_C0) | MASK(MPP_D0) | MASK(MPP_F0) | MASK(MPP_G0) | MASK(MPP_A0) | MASK(MPP_H0), "6sus7", 0),
	INIT(MASK(MPP_C0) | MASK(MPP_D0) | MASK(MPP_F0) | MASK(MPP_A0B) | MASK(MPP_A0) | MASK(MPP_H0), "$a6sus7", 0),
	INIT(MASK(MPP_C0) | MASK(MPP_D0) | MASK(MPP_F0) | MASK(MPP_G0) | MASK(MPP_H0B) | MASK(MPP_H0), "11sus7", 0),
	INIT(MASK(MPP_C0) | MASK(MPP_D0) | MASK(MPP_F0) | MASK(MPP_A0B) | MASK(MPP_H0B) | MASK(MPP_H0), "$a11sus7", 0),
 
	/* thirteenth */
	INIT(MASK(MPP_C0) | MASK(MPP_E0) | MASK(MPP_G0) | MASK(MPP_H0B) | MASK(MPP_D0) | MASK(MPP_F0) | MASK(MPP_A0), "13", "dom13", 0),
	INIT(MASK(MPP_C0) | MASK(MPP_E0) | MASK(MPP_G0) | MASK(MPP_H0) | MASK(MPP_D0) | MASK(MPP_F0) | MASK(MPP_A0), "$M13", 0),
	INIT(MASK(MPP_C0) | MASK(MPP_E0B) | MASK(MPP_G0) | MASK(MPP_H0) | MASK(MPP_D0) | MASK(MPP_F0) | MASK(MPP_A0), "$m$M13", 0),
	INIT(MASK(MPP_C0) | MASK(MPP_E0B) | MASK(MPP_G0) | MASK(MPP_H0B) | MASK(MPP_D0) | MASK(MPP_F0) | MASK(MPP_A0), "$m13", 0),
	INIT(MASK(MPP_C0) | MASK(MPP_E0) | MASK(MPP_A0B) | MASK(MPP_H0) | MASK(MPP_D0) | MASK(MPP_F0) | MASK(MPP_A0), "$a$M13", 0),
	INIT(MASK(MPP_C0) | MASK(MPP_E0) | MASK(MPP_A0B) | MASK(MPP_H0B) | MASK(MPP_D0) | MASK(MPP_F0) | MASK(MPP_A0), "$a13", "13$S5", 0),
	INIT(MASK(MPP_C0) | MASK(MPP_E0B) | MASK(MPP_G0B) | MASK(MPP_H0B) | MASK(MPP_D0) | MASK(MPP_F0) | MASK(MPP_A0), "$h13", 0),

	/* agumented and major thirteenths susXXX */
	INIT(MASK(MPP_C0) | MASK(MPP_D0B) | MASK(MPP_D0) | MASK(MPP_F0) | MASK(MPP_G0) | MASK(MPP_A0) | MASK(MPP_H0B), "13sus$S1", "13sus$f2", 0),
	INIT(MASK(MPP_C0) | MASK(MPP_D0B) | MASK(MPP_D0) | MASK(MPP_F0) | MASK(MPP_A0B) | MASK(MPP_A0) | MASK(MPP_H0B), "$a13sus$S1", "$a13sus$f2", 0),
	INIT(MASK(MPP_C0) | MASK(MPP_D0) | MASK(MPP_E0B) | MASK(MPP_F0) | MASK(MPP_G0) | MASK(MPP_A0) | MASK(MPP_H0B), "13sus$S2", "13sus$f3", 0),
	INIT(MASK(MPP_C0) | MASK(MPP_D0) | MASK(MPP_E0B) | MASK(MPP_F0) | MASK(MPP_A0B) | MASK(MPP_A0) | MASK(MPP_H0B), "$a13sus$S2", "$a13sus$f3", 0),
	INIT(MASK(MPP_C0) | MASK(MPP_D0) | MASK(MPP_F0) | MASK(MPP_G0B) | MASK(MPP_G0) | MASK(MPP_A0) | MASK(MPP_H0B), "13sus$S4", "13sus$f5", 0),
	INIT(MASK(MPP_C0) | MASK(MPP_D0) | MASK(MPP_F0) | MASK(MPP_G0B) | MASK(MPP_A0B) | MASK(MPP_A0) | MASK(MPP_H0B), "$a13sus$S4", "$a13sus$f5", 0),
	INIT(MASK(MPP_C0) | MASK(MPP_D0) | MASK(MPP_F0) | MASK(MPP_G0) | MASK(MPP_A0B) | MASK(MPP_A0) | MASK(MPP_H0B), "$a13sus5", 0),
	INIT(MASK(MPP_C0) | MASK(MPP_D0) | MASK(MPP_F0) | MASK(MPP_G0) | MASK(MPP_A0B) | MASK(MPP_A0) | MASK(MPP_H0B), "13sus$S5", "13sus$f6", 0),
	INIT(MASK(MPP_C0) | MASK(MPP_D0) | MASK(MPP_F0) | MASK(MPP_G0) | MASK(MPP_A0) | MASK(MPP_H0B) | MASK(MPP_H0), "13sus7", 0),
	INIT(MASK(MPP_C0) | MASK(MPP_D0) | MASK(MPP_F0) | MASK(MPP_A0B) | MASK(MPP_A0) | MASK(MPP_H0B) | MASK(MPP_H0), "$a13sus7", 0),
};

static int
MppGetValue(const QString &str, int &off)
{
	int retval = 0;

	while (off < str.length() && str[off].isDigit()) {
		retval *= 10;
		retval += str[off].digitValue();
		off++;
	}
	return (retval);
}

Q_DECL_EXPORT void
MppRolDownChord(MppChord_t &input, int &delta)
{
	if (input.test(0) == 0)
		return;
	input.tog(0);
	input.tog(MPP_MAX_BANDS);

	while (input.test(0) == 0) {
		input.shr();
		delta++;
	}
}

Q_DECL_EXPORT void
MppRolUpChord(MppChord_t &input, int &delta)
{
	if (input.test(0) == 0)
		return;
	while (input.test(MPP_MAX_BANDS) == 0) {
		input.shl();
		delta++;
	}
	input.tog(0);
	input.tog(MPP_MAX_BANDS);
}

Q_DECL_EXPORT void
MppNextChordRoot(MppChord_t &input, int step)
{
	for (int x = 0; x != MPP_MAX_BANDS; x++) {
		if (x % step)
			input.clr(x);
	}

	do {
		input.inc(step);
	} while (input.test(0) == 0 ||
		 MppFindChordRoot(input) != input);
}

Q_DECL_EXPORT void
MppPrevChordRoot(MppChord_t &input, int step)
{
	for (int x = 0; x != MPP_MAX_BANDS; x++) {
		if (x % step)
			input.clr(x);
	}

	do {
		input.dec(step);
	} while (input.test(0) == 0 ||
		 MppFindChordRoot(input) != input);
}

Q_DECL_EXPORT MppChord_t
MppFindChordRoot(MppChord_t input, uint32_t *rots, uint32_t *steps)
{
	MppChord_t retval = input;
	uint32_t tmprot = 0;
	uint32_t tmpsteps = 0;
	uint32_t sumbits = 0;

	for (uint32_t x = 0; x != MPP_MAX_BANDS; x++) {
		if (input.test(x))
			sumbits++;
	}

	if (rots)
		*rots = 0;
	if (steps)
		*steps = 0;

	for (uint32_t x = 0; x != MPP_MAX_BANDS; x++) {
		if (input.test(0)) {
			input.tog(0);
			input.tog(MPP_MAX_BANDS);
			tmpsteps++;
			tmpsteps %= sumbits;
		}
		input.shr();
		tmprot++;
		tmprot %= MPP_MAX_BANDS;
		if (input == retval)
			break;
		if (input < retval) {
			if (rots)
				*rots = tmprot;
			if (steps)
				*steps = tmpsteps;
			retval = input;
		}
	}
	return (retval);
}

Q_DECL_EXPORT int
MppIsChord(QString &str)
{
	int retval = 0;
	int x;
	int y;

	for (x = 0; x != str.length(); x++) {
		QChar ch = str[x];

		if (retval == 0 && ch.isLetter()) {
			retval = 1;
			if (ch < 'A' || ch > 'H')
				return (0);
		} else if (retval != 0 && ch == '/') {
			retval = 0;
			goto next;
		}
		if (ch.isDigit() || ch.isLetter())
			goto next;
		for (y = 0; y != MppChordModChars.size(); y++) {
			if (ch == MppChordModChars[y])
				goto next;
		}
		return (0);
	next:;
	}
	return (retval);
}

static void
MppExpandPattern12(const QString &pattern, QString &str)
{
	int x;
	int y;

	str = QString();

	for (y = 0; y < pattern.length(); y++) {
		if (pattern[y] == '$') {
			for (x = 0; score_macros[x]; x++) {
				if (score_macros[x][0][1] == pattern[y + 1])
					break;
			}
			if (score_macros[x] != 0)
				str += QString::fromUtf8(score_macros[x][1]);
			y++;
		} else {
			str += pattern[y];
		}
	}
}

static bool
MppScoreMatchPattern12(const QString &pattern, const QString &str, int &add, int t, int &u)
{
	while (1) {
		if (t >= pattern.length()) {
			/* check for add in the end */
			if (str.indexOf("add", u) == u) {
				const uint8_t map[14] = {
					MPP_C0, MPP_D0, MPP_E0, MPP_F0, MPP_G0, MPP_A0, MPP_H0,
					MPP_C0, MPP_D0, MPP_E0, MPP_F0, MPP_G0, MPP_A0, MPP_H0,
				};

				add = 0;
				u += 3;
				for (int x = 1; score_sharp[x]; x++) {
					QString pat = QString::fromUtf8(score_sharp[x]);
					if (str.indexOf(pat, u) != u)
						continue;
					u += pat.length();
					add++;
					goto add_num;
				}
				for (int x = 1; score_flat[x]; x++) {
					QString pat = QString::fromUtf8(score_flat[x]);
					if (str.indexOf(pat, u) != u)
						continue;
					u += pat.length();
					add--;
					goto add_num;
				}
			add_num:
				if (u + 1 == str.length() || str[u + 1] == '/') {
					if (str[u].isDigit() == 0 ||
					    str[u].digitValue() == 0)
						return (false);
					add += map[str[u].digitValue() - 1];
					u++;
				} else if (u + 2 == str.length() || str[u + 2] == '/') {
					if (str[u].isDigit() == 0 ||
					    str[u].digitValue() != 1 ||
					    str[u+1].isDigit() == 0 ||
					    str[u+1].digitValue() > 4)
						return (false);
					add += map[10 + str[u+1].digitValue() - 1];
					u += 2;
				} else {
					return (false);
				}
				add = (add + 12) % 12;
			}
			return (u == str.length() || str[u] == '/' || str[u] == '%');
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
					int nu = u + temp.length();
					/* there might be more matches, need to recurse */
					bool found = MppScoreMatchPattern12(
					    pattern, str, add, t + 2, nu);
					if (found) {
						u = nu;
						return (found);
					}
				}
			}
			return (false);
		} else if (u >= str.length()) {
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
MppStringDecodeKey(const QString &str, int &off)
{
	const uint8_t map[8] = { MPP_A0, MPP_H0, MPP_C0, MPP_D0,
				 MPP_E0, MPP_F0, MPP_G0, MPP_H0 };
	int x;
	int y = off;
	char ch;

	if (y >= str.length())
		goto error;

	ch = str[y].toLatin1();
	if (ch < 'A' || ch > 'H')
		goto error;

	/* regular chord notation */
	x = map[ch - 'A'];
	y++;
	if (y != str.length()) {
		if (str[y] == 'b') {
			x = (11 + x) % 12;
			/* advance */
			y++;
		} else if (str[y] == '#') {
			x = (1 + x) % 12;
			/* advance */
			y++;
		}
	}
	x *= MPP_BAND_STEP_12;

	/* check for subdivision marker */
	if (y != str.length() && str[y].isDigit()) {
		int z;
		int rem;

		z = y;
		rem = MppGetValue(str, z);
		if (rem > 0 && z != str.length() && str[z] == '.') {
			x += MPP_BAND_REM_BITREV(rem);
			/* advance */
			y = z + 1;
		}
	}
	off = y;
	return (x);
error:
	return (-1);
}

static int
MppStringDecodeNumeric(const QString &str, int &off)
{
	int x;
	int y;

	y = off;
	x = MppGetValue(str, y);
	if (x < 1)
		goto error;
	off = y;
	return (MPP_KEY_TO_BAND(x));
error:
	return (-1);
}

Q_DECL_EXPORT void
MppStringToChordGeneric(MppChord_t &mask, uint32_t &rem, uint32_t &bass, uint32_t step, const QString &str)
{
	uint32_t diff;
	int add;
	int y;

	if (str.isEmpty())
		goto error;

	y = 0;
	rem = bass = MppStringDecodeKey(str, y);
	if (rem == -1U)
		goto error;

	mask.zero();
	mask.set(0);

	for (size_t x = 0; x != (sizeof(MppScoreVariants12) / sizeof(MppScoreVariants12[0])); x++) {
	    for (size_t z = 0; MppScoreVariants12[x].pattern[z]; z++) {
		QString pat = QString::fromUtf8(MppScoreVariants12[x].pattern[z]);
		int nu = y;
		add = -1;
		if (MppScoreMatchPattern12(pat, str, add, 0, nu) == 0)
			continue;
		y = nu;
		/* set mask */
		mask = MppScoreVariants12[x].footprint;
		/* check for add */
		if (add > -1) {
			if (mask.test(add * MPP_BAND_STEP_12))
				goto error;
			mask.set(add * MPP_BAND_STEP_12);
		}
		/* adjust for rotation */
		rem = (rem + MppScoreVariants12[x].rots) % MPP_MAX_BANDS;
		goto next;
	    }
	}
next:
	while (y != str.length() && str[y] == '%') {
		y++;
		diff = MppStringDecodeNumeric(str, y);
		if (diff == -1U || (diff % step) || mask.test(diff))
			goto error;
		mask.set(diff);
	}
	if (y != str.length()) {
		if (str[y] != '/')
			goto error;
		y++;
		bass = MppStringDecodeKey(str, y);
		if (bass == -1U || y != str.length())
			goto error;
	}
	if ((rem % step) || (bass % step))
		goto error;

	/* rectify the 2nd tone to be more accurate when possible */
	if (step <= MPP_BAND_STEP_96) {
		uint32_t bass_rel = (MPP_MAX_BANDS - (rem % MPP_MAX_BANDS) +
		    (bass % MPP_MAX_BANDS)) % MPP_MAX_BANDS;

		for (y = 0; y != MPP_MAX_BANDS; y += MPP_BAND_STEP_12) {
			uint32_t pa = y;
			uint32_t pb = (y + MPP_BAND_STEP_12 * 4) % MPP_MAX_BANDS;
			uint32_t pc = (y + MPP_BAND_STEP_12 * 7) % MPP_MAX_BANDS;

			if (mask.test(pa) && mask.test(pb) && mask.test(pc)) {
				uint32_t rots;

				/* adjust treble tone, if any */
				mask.clr(pb);
				mask.set((pb + MPP_MAX_BANDS - MPP_BAND_STEP_96) % MPP_MAX_BANDS);

				mask = MppFindChordRoot(mask, &rots);
				
				/* adjust for rotation */
				rem = (rem + rots) % MPP_MAX_BANDS;

				/* adjust bass tone, if any */
				if (bass_rel == pb)
					bass = (bass + MPP_MAX_BANDS - MPP_BAND_STEP_96) % MPP_MAX_BANDS;
				break;
			}
		}
	}
	return;
error:
	mask.zero();
	rem = 0;
	bass = 0;
}

Q_DECL_EXPORT void
MppChordToStringGeneric(MppChord_t mask, uint32_t rem, uint32_t bass, uint32_t sharp, uint32_t step, QString &retval)
{
	uint32_t rots;

	rem = rem % MPP_MAX_BANDS;
	bass = bass % MPP_MAX_BANDS;

	/* check if conversion is valid */
	if ((rem % step) || (bass % step) || mask.test(0) == 0)
		goto error;

	/* get root chord */
	mask = MppFindChordRoot(mask, &rots);

	/* adjust for rotations */
	rem = (rem + rots) % MPP_MAX_BANDS;
	
	/* look for known chords */
	for (size_t x = 0; x != (sizeof(MppScoreVariants12) / sizeof(MppScoreVariants12[0])); x++) {
		if (MppScoreVariants12[x].footprint != mask)
			continue;
		QString pat = QString::fromUtf8(MppScoreVariants12[x].pattern[0]);
		uint32_t y = MppScoreVariants12[x].rots;

		/* expand pattern */
		MppExpandPattern12(pat, retval);

		/* adjust for rotations, again */
		rem = (rem + MPP_MAX_BANDS - y) % MPP_MAX_BANDS;

		retval = MppKeyToStringGeneric(rem, sharp) + retval;
		if (rem != bass)
			retval += QString("/") + MppKeyToStringGeneric(bass, sharp);
		return;
	}

	retval = MppKeyToStringGeneric(rem, sharp) + QString("1");

	/* use the generic way */
	for (int x = 1; x != MPP_MAX_BANDS; x++) {
		if (mask.test(x)) {
			if (x % step)
				goto error;
			retval += QString("%%1").arg(MPP_BAND_TO_KEY(x));
		}
	}
	if (rem != bass) {
		retval += "/";
		retval += MppKeyToStringGeneric(bass, sharp);
	}
	return;
error:
	retval = QString();
}

Q_DECL_EXPORT const QString
MppKeyToStringGeneric(int key, int sharp)
{
	const char *map_sharp[12] = {
		/* majors */
		/* [MPP_C0] = */ "C",
		/* [MPP_D0B] = */ "C#",
		/* [MPP_D0] = */ "D",
		/* [MPP_E0B] = */ "D#",
		/* [MPP_E0] = */ "E",
		/* [MPP_F0] = */ "F",
		/* [MPP_G0B] = */ "F#",
		/* [MPP_G0] = */ "G",
		/* [MPP_A0B] = */ "G#",
		/* [MPP_A0] = */ "A",
		/* [MPP_H0B] = */ "A#",
		/* [MPP_H0] = */ "H",
	};
	const char *map_flat[12] = {
		/* majors */
		/* [MPP_C0] = */ "C",
		/* [MPP_D0B] = */ "Db",
		/* [MPP_D0] = */ "D",
		/* [MPP_E0B] = */ "Eb",
		/* [MPP_E0] = */ "E",
		/* [MPP_F0] = */ "F",
		/* [MPP_G0B] = */ "Gb",
		/* [MPP_G0] = */ "G",
		/* [MPP_A0B] = */ "Ab",
		/* [MPP_A0] = */ "A",
		/* [MPP_H0B] = */ "Hb",
		/* [MPP_H0] = */ "H",
	};
	QString retval;
	int rem;

	key = MPP_BAND_REM(key);
	rem = MPP_BAND_REM_BITREV(key);
	key /= MPP_BAND_STEP_12;

	if (rem != 0) {
		if (sharp)
			return (QString("%1%2.").arg(map_sharp[key]).arg(rem));
		else
			return (QString("%1%2.").arg(map_flat[key]).arg(rem));
	} else {
		if (sharp)
			return (QString("%1").arg(map_sharp[key]));
		else
			return (QString("%1").arg(map_flat[key]));
	}
}

Q_DECL_EXPORT void
MppStepChordGeneric(QString &input, int adjust, uint32_t sharp)
{
	QString str;
	MppChord_t mask;
	uint32_t rem;
	uint32_t bass;

	MppStringToChordGeneric(mask, rem, bass, 1, input);

	/* only step chords we understand */
	if (mask.test(0) == 0)
		return;

	adjust = MPP_BAND_REM(adjust);

	rem += adjust;
	bass += adjust;
	rem = MPP_BAND_REM(rem);
	bass = MPP_BAND_REM(bass);

	MppChordToStringGeneric(mask, rem, bass, sharp, 1, str);

	if (str.isEmpty())
		return;

	input = str;
}

MppScoreVariant :: MppScoreVariant(uint32_t _chord, const char *a,
    const char *b, const char *c, const char *d)
{
	footprint.zero();
	for (int x = 0; x != 12; x++) {
		if ((_chord >> x) & 1)
			footprint.set(x * MPP_BAND_STEP_12);
	}
	footprint = MppFindChordRoot(footprint, &rots);
	pattern[0] = a;
	pattern[1] = b;
	pattern[2] = c;
	pattern[3] = d;
}
