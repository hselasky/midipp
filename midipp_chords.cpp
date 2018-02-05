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
MppRolUpChord(MppChord_t &input, int &delta)
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
MppRolDownChord(MppChord_t &input, int &delta)
{
	if (input.test(0) == 0)
		return;

	while (input.test(MPP_MAX_BANDS) == 0) {
		input.shl();
		delta--;
	}
	input.tog(0);
	input.tog(MPP_MAX_BANDS);
}

Q_DECL_EXPORT void
MppNextChordRoot(MppChord_t &input, int step)
{
	uint32_t rots;

	for (int x = 0; x != MPP_MAX_BANDS; x++) {
		if (x % step)
			input.clr(x);
	}

	do {
		input.inc(step);
	} while (MppFindChordRoot(input, rots) != input);
}

Q_DECL_EXPORT void
MppPrevChordRoot(MppChord_t &input, int step)
{
	uint32_t rots;

	for (int x = 0; x != MPP_MAX_BANDS; x++) {
		if (x % step)
			input.clr(x);
	}

	do {
		input.dec(step);
	} while (MppFindChordRoot(input, rots) != input);
}

Q_DECL_EXPORT MppChord_t
MppFindChordRoot(MppChord_t input, uint32_t &rots)
{
	MppChord_t retval = input;
	uint32_t tmprot = 0;

	rots = tmprot = 0;
	while (1) {
		if (input.test(0)) {
			input.tog(0);
			input.tog(MPP_MAX_BANDS);
		}
		input.shr();
		tmprot++;
		tmprot %= MPP_MAX_BANDS;
		if (input == retval)
			break;
		if (input < retval) {
			rots = tmprot;
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

	for (x = 0; x != str.size(); x++) {
		QChar ch = str[x];

		if (retval == 0 && ch.isLetter()) {
			retval = 1;
			if (ch < 'A' || ch > 'H')
				return (0);
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
MppScoreMatchPattern12(const QString &pattern, const QString &str, int &add, int t = 0, int u = 0)
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
			return (u == str.length() || str[u] == '/');
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
					bool found = MppScoreMatchPattern12(
					    pattern, str, add, t + 2, u + temp.length());
					if (found)
						return (found);
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

static void
MppStringToChord12(MppChord_t &mask, uint32_t &rem, uint32_t &bass, const QString &str)
{
	const uint8_t map[8] = { MPP_A0, MPP_H0, MPP_C0, MPP_D0,
				 MPP_E0, MPP_F0, MPP_G0, MPP_H0 };
	uint32_t diff;
	char ch;
	int x;
	int y;
	int add;

	if (str.isEmpty())
		goto error;

	ch = str[0].toLatin1();
	if (ch < 'A' || ch > 'H')
		goto error;

	x = map[ch - 'A'];
	y = 1;
	if (str.length() > y) {
		if (str[y] == 'b') {
			x = (11 + x) % 12;
			y++;
		} else if (str[y] == '#') {
			x = (1 + x) % 12;
			y++;
		}
	}

	rem = bass = x * MPP_BAND_STEP_12;
	mask.zero();
	mask.set(0);

	for (x = 0; x != (sizeof(MppScoreVariants12) / sizeof(MppScoreVariants12[0])); x++) {
	    for (int z = 0; MppScoreVariants12[x].pattern[z]; z++) {
		QString pat = QString::fromUtf8(MppScoreVariants12[x].pattern[z]);
		add = -1;
		if (MppScoreMatchPattern12(pat, str, add, 0, y)) {
			y = str.indexOf("/");
			if (y < 0)
				y = str.length();
			mask = MppScoreVariants12[x].footprint;
			if (add > -1)
				mask.set(add * MPP_BAND_STEP_12);
			mask = MppFindChordRoot(mask, diff);
			rem += diff;
			rem %= MPP_MAX_BANDS;
			goto next;
		}
	    }
	}
next:
	for (x = y; x != str.length(); x++) {
		if (str[x] == '/') {
			if ((x + 2) == str.length()) {
				if (str[x + 1] < QChar('A') || str[x + 1] > QChar('H'))
					goto error;
				bass = map[str[x + 1].toLatin1() - 'A'] * MPP_BAND_STEP_12;
				break;
			} else if ((x + 3) == str.length()) {
				if (str[x + 1] < QChar('A') || str[x + 1] > QChar('H'))
					goto error;
				if (str[x + 2] == 'b') {
					bass = (11 + map[str[x + 1].toLatin1() - 'A']) % 12;
					bass *= MPP_BAND_STEP_12;
					break;
				} else if (str[x + 2] == '#') {
					bass = (1 + map[str[x + 1].toLatin1() - 'A']) % 12;
					bass *= MPP_BAND_STEP_12;
					break;
				}
			}
		}
		goto error;
	}
	if (MppFindChordRoot(mask, diff) != mask)
		goto error;
	return;
error:
	mask.zero();
	rem = 0;
	bass = 0;
}

static const char *
MppKeyToString12(uint32_t key, uint32_t sharp)
{

	switch (key / MPP_BAND_STEP_12) {
	/* majors */
	case MPP_A0:
		return ("A");
	case MPP_H0:
		return ("H");
	case MPP_C0:
		return ("C");
	case MPP_D0:
		return ("D");
	case MPP_E0:
		return ("E");
	case MPP_F0:
		return ("F");
	case MPP_G0:
		return ("G");
	/* flats */
	case MPP_H0B:
		if (sharp)
			return ("A#");
		else
			return ("Hb");
	case MPP_A0B:
		if (sharp)
			return ("G#");
		else
			return ("Ab");
	case MPP_G0B:
		if (sharp)
			return ("F#");
		else
			return ("Gb");
	case MPP_E0B:
		if (sharp)
			return ("D#");
		else
			return ("Eb");
	case MPP_D0B:
		if (sharp)
			return ("C#");
		else
			return ("Db");
	default:
		return ("");
	}
}

Q_DECL_EXPORT void
MppChordToStringGeneric(MppChord_t mask, uint32_t rem, uint32_t bass, uint32_t sharp, uint32_t step, QString &retval)
{
	uint32_t rots;
	uint32_t x;

	/* get root chord */
	mask = MppFindChordRoot(mask, rots);

	/* adjust for rotation */
	rem = (rem + rots + MPP_MAX_BANDS - (MPP_BAND_STEP_12 * 9)) % MPP_MAX_BANDS;
	bass = (bass + MPP_MAX_BANDS - (MPP_BAND_STEP_12 * 9)) % MPP_MAX_BANDS;

	/* check if conversion is valid */
	if ((rem % step) || (bass % step))
		goto error;

	/* look for known chords */
	if ((rem % MPP_BAND_STEP_12) == 0 || (bass % MPP_BAND_STEP_12) == 0) {
		for (x = 0; x != (sizeof(MppScoreVariants12) / sizeof(MppScoreVariants12[0])); x++) {
			if (MppScoreVariants12[x].footprint == mask) {	
				QString pat = QString::fromUtf8(MppScoreVariants12[x].pattern[0]);
				MppExpandPattern12(pat, retval);
				rem = (rem + (MPP_BAND_STEP_12 * 9)) % MPP_MAX_BANDS;
				bass = (bass + (MPP_BAND_STEP_12 * 9)) % MPP_MAX_BANDS;
				if (rem != bass) {
					retval = MppKeyToString12(rem, sharp) + retval + QString("/") +
					    MppKeyToString12(bass, sharp);
				} else {
					retval = MppKeyToString12(rem, sharp) + retval;
				}
				return;
			}
		}
	}

	rem = MPP_BAND_TO_KEY(rem);
	bass = MPP_BAND_TO_KEY(bass);

	if (mask.test(0) == 0)
		goto error;

	retval = QString("%1").arg(rem);

	for (x = 1; x != MPP_MAX_BANDS; x++) {
		if (mask.test(x)) {
			if (x % step)
				goto error;
			retval += QString("-%1").arg(MPP_BAND_TO_KEY(x));
		}
	}
	if (rem != bass) {
		retval += "/";
		retval += QString("%1").arg(bass);
	}
	return;
error:
	retval = QString();
}

Q_DECL_EXPORT void
MppStringToChordGeneric(MppChord_t &mask, uint32_t &rem, uint32_t &bass, uint32_t step, const QString &str)
{
	int last = 0;
	int off = 0;
	int diff;
	uint32_t rots;

	if (str.isEmpty())
		goto error;

	if (step <= MPP_BAND_STEP_12) {
		MppStringToChord12(mask, rem, bass, str);
		if (mask.test(0))
			return;
	}
	
	rem = MppGetValue(str, off);
	rem = bass = (MPP_KEY_TO_BAND(rem) + (MPP_BAND_STEP_12 * 9)) % MPP_MAX_BANDS;
	if ((int)rem < 0 || (rem % step))
		goto error;
	mask.zero();
	mask.set(0);

	while (1) {
		if (off == str.length())
			break;
		if (str[off] != '-')
			break;
		off++;

		diff = MppGetValue(str, off);
		diff = MPP_KEY_TO_BAND(diff);
		if ((diff <= last) || (diff % step))
			goto error;
		mask.set(diff);
		last = diff;
	}

	if (off != str.length() && str[off] == '/') {
		off++;
		bass = MppGetValue(str, off);
		if ((int)bass < 0)
			goto error;
		bass = (MPP_KEY_TO_BAND(bass) + (MPP_BAND_STEP_12 * 9)) % MPP_MAX_BANDS;
	}

	if (off != str.length())
		goto error;
	if (MppFindChordRoot(mask, rots) != mask)
		goto error;
	return;
error:
	mask.zero();
	rem = 0;
	bass = 0;
}

Q_DECL_EXPORT const QString
MppKeyToStringGeneric(uint32_t key)
{
	/* 440.0Hz = "A" = 9 is zero reference */
	key += MPP_MAX_BANDS - (MPP_BAND_STEP_12 * 9);

	/* get absolute remainder */
	key = MPP_BAND_REM(key);

	/* bitreverse */
	key = MPP_BAND_TO_KEY(key);

	return (QString("%1").arg(key));
}

Q_DECL_EXPORT void
MppStepChordGeneric(QString &input, int adjust, uint32_t sharp)
{
	QString str;
	MppChord_t mask;
	uint32_t rem;
	uint32_t bass;
	uint32_t wrap;

	/* Chords should not contain spaces of any kind */
	if (MppIsChord(input) == 0)
		return;

	wrap = 0;
	for (int off = 0; off != input.length(); off++) {
		if (input[off] == '(' || input[off] == ')') {
			wrap = 1;
			continue;
		}
		str += input[off];
	}

	MppStringToChordGeneric(mask, rem, bass, 1, str);

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

	if (wrap)
		input = QString("(") + str + QString(")");
	else
		input = str;
}
