/*-
 * Copyright (c) 2010-2015 Hans Petter Selasky. All rights reserved.
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

/* The list is sorted by priority. C5 is base. */

#define MASK(x) (1U << ((x) % 12))


/* common standard */

static const char *score_major[] = {
	"$M", "M", "Δ", "j", "Ma", "maj", "major", 0
};

static const char *score_minor[] = {
	"$m", "m", "-", "Mi", "min", "minor", 0
};

static const char *score_aug[] = {
	"$a", "+", "aug", 0
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

static const char *score_sharp[] = {
	"$S", "+", "#", 0
};

static const char *score_flat[] = {
	"$f", "b", "°", "o", "dim", 0
};

static const char *score_half_dim[] = {
	"$h", "ø", "Ø", 0
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
	0
};

static const char *score_bits[13] = {
	"C", "Db", "D", "Eb", "E", "F", "Gb", "G", "Ab", "A", "Hb", "H", "?"
};

static const char *score_name_sharp[13] = {
	"1", "$S1", "2", "$S2", "3", "4", "$S4", "5", "$S5", "6", "$S6", "7", "?"
};

static const char *score_name_flat[13] = {
	"1", "$f2", "2", "$f3", "3", "4", "$f5", "5", "$f6", "6", "$f7", "7", "?"
};

static const struct score_variant_initial score_initial[] = {
	/* one */
	{ { "1" }, MASK(C5) },

	/* third */
	{ { "5" }, MASK(C5) | MASK(G5) },

	/* triads */
	{ { "$A" }, MASK(C5) | MASK(D5B) | MASK(D5) },	/* custom */
	{ { "$O" }, MASK(C5) | MASK(D5B) | MASK(E5B) },	/* custom */
	{ { "$X" }, MASK(C5) | MASK(D5) | MASK(E5B) },	/* custom */
	{ { "$d", "$m$f5" }, MASK(C5) | MASK(E5B) | MASK(G5B) },
	{ { "$m", "sus$S2", "sus$f3" }, MASK(C5) | MASK(E5B) | MASK(G5) },
	{ { "$a", "$M$S5" }, MASK(C5) | MASK(E5) | MASK(A5B) },
	{ { "", "$M" }, MASK(C5) | MASK(E5) | MASK(G5) },

	/* augumented and major triads with susXXX */
	{ { "sus$S1", "sus$f2" }, MASK(C5) | MASK(D5B) | MASK(G5) },
	{ { "$asus$S1", "$asus$f2" }, MASK(C5) | MASK(D5B) | MASK(A5B) },
	{ { "sus2" }, MASK(C5) | MASK(D5) | MASK(G5) },
	{ { "$asus2" }, MASK(C5) | MASK(D5) | MASK(A5B) },
	{ { "sus$S2", "sus$f3" }, MASK(C5) | MASK(E5B) | MASK(G5) },
	{ { "$asus$S2", "$asus$f3" }, MASK(C5) | MASK(E5B) | MASK(A5B) },
	{ { "sus4", "sus" }, MASK(C5) | MASK(F5) | MASK(G5) },
	{ { "$asus4", "$asus" }, MASK(C5) | MASK(F5) | MASK(A5B) },
	{ { "sus$S4", "sus$f5" }, MASK(C5) | MASK(G5B) | MASK(G5) },
	{ { "$asus$S4", "$asus$f5" }, MASK(C5) | MASK(G5B) | MASK(A5B) },
	{ { "$asus5" }, MASK(C5) | MASK(G5) | MASK(A5B) },
	{ { "sus$S5", "sus$f6" }, MASK(C5) | MASK(G5) | MASK(A5B) },
	{ { "sus6" }, MASK(C5) | MASK(G5) | MASK(A5) },
	{ { "$asus6" }, MASK(C5) | MASK(A5B) | MASK(A5) },
	{ { "sus$S6", "sus$f7" }, MASK(C5) | MASK(G5) | MASK(H5B) },
	{ { "$asus$S6", "$asus$f7" }, MASK(C5) | MASK(A5B) | MASK(H5B) },
	{ { "sus7" }, MASK(C5) | MASK(G5) | MASK(H5) },
	{ { "$asus7" }, MASK(C5) | MASK(A5B) | MASK(H5) },

	/* seventh */
	{ { "7", "dom7" }, MASK(C5) | MASK(E5) | MASK(G5) | MASK(H5B) },
	{ { "$a7", "7$S" }, MASK(C5) | MASK(E5) | MASK(A5B) | MASK(H5B) },
	{ { "$h", "$h7", "$m7$f5" }, MASK(C5) | MASK(E5B) | MASK(G5B) | MASK(H5B) },
	{ { "7$f5", "dom7$f5" }, MASK(C5) | MASK(E5) | MASK(G5B) | MASK(H5B) },
	{ { "7$f9" }, MASK(C5) | MASK(E5) | MASK(G5) | MASK(D5B) | MASK(H5B) },
	{ { "7$S9" }, MASK(C5) | MASK(E5) | MASK(G5) | MASK(E5B) | MASK(H5B) },
	{ { "7$f2" }, MASK(C5) | MASK(E5) | MASK(G5) | MASK(D5B) | MASK(H5B) },
	{ { "7$S2" }, MASK(C5) | MASK(E5) | MASK(G5) | MASK(E5B) | MASK(H5B) },
	{ { "$M7" }, MASK(C5) | MASK(E5) | MASK(G5) | MASK(H5) },
	{ { "$a$M7", "$M7$S5" }, MASK(C5) | MASK(E5) | MASK(A5B) | MASK(H5) },
	{ { "$m$M7" }, MASK(C5) | MASK(E5B) | MASK(G5) | MASK(H5) },
	{ { "$m7" }, MASK(C5) | MASK(E5B) | MASK(G5) | MASK(H5B) },
	{ { "$d7" }, MASK(C5) | MASK(E5B) | MASK(G5B) | MASK(A5) },

	/* second */
	{ { "2", "$M2", "dom2" }, MASK(C5) | MASK(E5) | MASK(G5) | MASK(D5) },
	{ { "$a2", "2$S5" }, MASK(C5) | MASK(E5) | MASK(A5B) | MASK(D5) },
	{ { "$m$M2" }, MASK(C5) | MASK(E5B) | MASK(G5) | MASK(D5) },
	{ { "$m2" }, MASK(C5) | MASK(E5B) | MASK(G5) | MASK(D5) },
	{ { "$a$M2" }, MASK(C5) | MASK(E5) | MASK(A5B) | MASK(D5) },
	{ { "$h2" }, MASK(C5) | MASK(E5B) | MASK(G5B) | MASK(D5) },
	{ { "$h$f2" }, MASK(C5) | MASK(E5B) | MASK(G5B) | MASK(D5B) },

	/* agumented and major seconds and sevenths susXXX */
	{ { "7sus$S1", "7sus$f2" }, MASK(C5) | MASK(D5B) | MASK(G5) | MASK(H5B) },
	{ { "$a7sus$S1", "$a7sus$f2" }, MASK(C5) | MASK(D5B) | MASK(A5B) | MASK(H5B) },
	{ { "2sus$S1", "2sus$f2" }, MASK(C5) | MASK(D5B) | MASK(D5) | MASK(G5) },
	{ { "$a2sus$S1", "$a2sus$f2" }, MASK(C5) | MASK(D5B) | MASK(D5) | MASK(A5B) },
	{ { "7sus2" }, MASK(C5) | MASK(D5) | MASK(G5) | MASK(H5B) },
	{ { "$a7sus2" }, MASK(C5) | MASK(D5) | MASK(A5B) | MASK(H5B) },
	{ { "7sus$S2", "7sus$f3" }, MASK(C5) | MASK(E5B) | MASK(G5) | MASK(H5B) },
	{ { "$a7sus$S2", "$a7sus$f3" }, MASK(C5) | MASK(E5B) | MASK(A5B) | MASK(H5B) },
	{ { "2sus$S2", "2sus$f3" }, MASK(C5) | MASK(D5) | MASK(E5B) | MASK(G5) },
	{ { "$a2sus$S2", "$a2sus$f3" }, MASK(C5) | MASK(D5) | MASK(E5B) | MASK(A5B) },
	{ { "7sus4", "7sus" }, MASK(C5) | MASK(F5) | MASK(G5) | MASK(H5B) },
	{ { "$a7sus4", "$a7sus" }, MASK(C5) | MASK(F5) | MASK(A5B) | MASK(H5B) },
	{ { "2sus4", "2sus" }, MASK(C5) | MASK(D5) | MASK(F5) | MASK(G5) },
	{ { "$a2sus4", "$a2sus" }, MASK(C5) | MASK(D5) | MASK(F5) | MASK(A5B) },
	{ { "7sus$S4", "7sus$f5" }, MASK(C5) | MASK(G5B) | MASK(G5) | MASK(H5B) },
	{ { "$a7sus$S4", "$a7sus$f5" }, MASK(C5) | MASK(G5B) | MASK(A5B) | MASK(H5B) },
	{ { "2sus$S4", "2sus$f5" }, MASK(C5) | MASK(D5) | MASK(G5B) | MASK(G5) },
	{ { "$a2sus$S4", "$a2sus$f5" }, MASK(C5) | MASK(D5) | MASK(G5B) | MASK(A5B) },
	{ { "$a7sus5" }, MASK(C5) | MASK(G5) | MASK(A5B) | MASK(H5B) },
	{ { "$a2sus5" }, MASK(C5) | MASK(D5) | MASK(G5) | MASK(A5B) },
	{ { "7sus$S5", "7sus$f6" }, MASK(C5) | MASK(G5) | MASK(A5B) | MASK(H5B) },
	{ { "2sus$S5", "2sus$f6" }, MASK(C5) | MASK(D5) | MASK(G5) | MASK(A5B) },
	{ { "7sus6" }, MASK(C5) | MASK(G5) | MASK(A5) | MASK(H5B) },
	{ { "$a7sus6" }, MASK(C5) | MASK(A5B) | MASK(A5) | MASK(H5B) },
	{ { "2sus6" }, MASK(C5) | MASK(D5) | MASK(G5) | MASK(A5) },
	{ { "$a2sus6" }, MASK(C5) | MASK(D5) | MASK(A5B) | MASK(A5) },
	{ { "2sus$S6", "2sus$f7" }, MASK(C5) | MASK(D5) | MASK(G5) | MASK(H5B) },
	{ { "$a2sus$S6", "$a2sus$f7" }, MASK(C5) | MASK(D5) | MASK(A5B) | MASK(H5B) },
	{ { "7sus7" }, MASK(C5) | MASK(G5) | MASK(H5B) | MASK(H5) },
	{ { "$a7sus7" }, MASK(C5) | MASK(A5B) | MASK(H5B) | MASK(H5) },
	{ { "2sus7" }, MASK(C5) | MASK(D5) | MASK(G5) | MASK(H5) },
	{ { "$a2sus7" }, MASK(C5) | MASK(D5) | MASK(A5B) | MASK(H5) },

	/* fourth */
	{ { "4", "$M4", "dom4" }, MASK(C5) | MASK(E5) | MASK(G5) | MASK(D5) | MASK(F5) },
	{ { "$m4", "$m$M4" }, MASK(C5) | MASK(E5B) | MASK(G5) | MASK(D5) | MASK(F5) },
	{ { "$a4", "4$S5", "$a$M4" }, MASK(C5) | MASK(E5) | MASK(A5B) | MASK(D5) | MASK(F5) },
	{ { "$h4" }, MASK(C5) | MASK(E5B) | MASK(G5B) | MASK(D5B) | MASK(F5) },

	/* ninth */
	{ { "9", "dom9" }, MASK(C5) | MASK(E5) | MASK(G5) | MASK(H5B) | MASK(D5) },
	{ { "$M9" }, MASK(C5) | MASK(E5) | MASK(G5) | MASK(H5) | MASK(D5) },
	{ { "$m$M9" }, MASK(C5) | MASK(E5B) | MASK(G5) | MASK(H5) | MASK(D5) },
	{ { "$m9" }, MASK(C5) | MASK(E5B) | MASK(G5) | MASK(H5B) | MASK(D5) },
	{ { "$a$M9" }, MASK(C5) | MASK(E5) | MASK(A5B) | MASK(H5) | MASK(D5) },
	{ { "$a9", "9$S5" }, MASK(C5) | MASK(E5) | MASK(A5B) | MASK(H5B) | MASK(D5) },
	{ { "$h9" }, MASK(C5) | MASK(E5B) | MASK(G5B) | MASK(H5B) | MASK(D5) },
	{ { "$h$f9" }, MASK(C5) | MASK(E5B) | MASK(G5B) | MASK(H5B) | MASK(D5B) },
	{ { "$d9" }, MASK(C5) | MASK(E5B) | MASK(G5B) | MASK(A5) | MASK(D5) },
	{ { "$db9" }, MASK(C5) | MASK(E5B) | MASK(G5B) | MASK(A5) | MASK(D5B) },

	/* agumented and major fourths and ninths susXXX */
	{ { "4sus$S1", "4sus$f2" }, MASK(C5) | MASK(D5B) | MASK(D5) | MASK(F5) | MASK(G5) },
	{ { "$a4sus$S1", "$a4sus$f2" }, MASK(C5) | MASK(D5B) | MASK(D5) | MASK(F5) | MASK(A5B) },
	{ { "9sus$S1", "9sus$f2" }, MASK(C5) | MASK(D5B) | MASK(D5) | MASK(G5) | MASK(H5B) },
	{ { "$a9sus$S1", "$a9sus$f2" }, MASK(C5) | MASK(D5B) | MASK(D5) | MASK(A5B) | MASK(H5B) },
	{ { "4sus$S2", "4sus$f3" }, MASK(C5) | MASK(D5) | MASK(E5B) | MASK(F5) | MASK(G5) },
	{ { "$a4sus$S2", "$a4sus$f3" }, MASK(C5) | MASK(D5) | MASK(E5B) | MASK(F5) | MASK(A5B) },
	{ { "9sus$S2", "9sus$f3" }, MASK(C5) | MASK(D5) | MASK(E5B) | MASK(G5) | MASK(H5B) },
	{ { "$a9sus$S2", "$a9sus$f3" }, MASK(C5) | MASK(D5) | MASK(E5B) | MASK(A5B) | MASK(H5B) },
	{ { "9sus4", "9sus" }, MASK(C5) | MASK(D5) | MASK(F5) | MASK(G5) | MASK(H5B) },
	{ { "$a9sus4", "$a9sus" }, MASK(C5) | MASK(D5) | MASK(F5) | MASK(A5B) | MASK(H5B) },
	{ { "4sus$S4", "4sus$f5" }, MASK(C5) | MASK(D5) | MASK(F5) | MASK(G5B) | MASK(G5) },
	{ { "$a4sus$S4", "$a4sus$f5" }, MASK(C5) | MASK(D5) | MASK(F5) | MASK(G5B) | MASK(A5B) },
	{ { "9sus$S4", "9sus$f5" }, MASK(C5) | MASK(D5) | MASK(G5B) | MASK(G5) | MASK(H5B) },
	{ { "$a9sus$S4", "$a9sus$f5" }, MASK(C5) | MASK(D5) | MASK(G5B) | MASK(A5B) | MASK(H5B) },
	{ { "$a4sus5" }, MASK(C5) | MASK(D5) | MASK(F5) | MASK(G5) | MASK(A5B) },
	{ { "$a9sus5" }, MASK(C5) | MASK(D5) | MASK(G5) | MASK(A5B) | MASK(H5B) },
	{ { "4sus$S5", "4sus$f6" }, MASK(C5) | MASK(D5) | MASK(F5) | MASK(G5) | MASK(A5B) },
	{ { "9sus$S5", "9sus$f6" }, MASK(C5) | MASK(D5) | MASK(G5) | MASK(A5B) | MASK(H5B) },
	{ { "4sus6" }, MASK(C5) | MASK(D5) | MASK(F5) | MASK(G5) | MASK(A5) },
	{ { "$a4sus6" }, MASK(C5) | MASK(D5) | MASK(F5) | MASK(A5B) | MASK(A5) },
	{ { "9sus6" }, MASK(C5) | MASK(D5) | MASK(G5) | MASK(A5) | MASK(H5B) },
	{ { "$a9sus6" }, MASK(C5) | MASK(D5) | MASK(A5B) | MASK(A5) | MASK(H5B) },
	{ { "4sus$S6", "4sus$f7" }, MASK(C5) | MASK(D5) | MASK(F5) | MASK(G5) | MASK(H5B) },
	{ { "$a4sus$S6", "$a4sus$f7" }, MASK(C5) | MASK(D5) | MASK(F5) | MASK(A5B) | MASK(H5B) },
	{ { "4sus7" }, MASK(C5) | MASK(D5) | MASK(F5) | MASK(G5) | MASK(H5) },
	{ { "$a4sus7" }, MASK(C5) | MASK(D5) | MASK(F5) | MASK(A5B) | MASK(H5) },
	{ { "9sus7" }, MASK(C5) | MASK(D5) | MASK(G5) | MASK(H5B) | MASK(H5) },
	{ { "$a9sus7" }, MASK(C5) | MASK(D5) | MASK(A5B) | MASK(H5B) | MASK(H5) },

	/* sixth */
	{ { "6", "$M6", "dom6" }, MASK(C5) | MASK(E5) | MASK(G5) | MASK(D5) | MASK(F5) | MASK(A5) },
	{ { "62" }, MASK(C5) | MASK(E5) | MASK(G5) | MASK(A5) | MASK(D5) },
	{ { "69" }, MASK(C5) | MASK(E5) | MASK(G5) | MASK(A5) | MASK(D5) },
	{ { "64" }, MASK(C5) | MASK(E5) | MASK(G5) | MASK(A5) | MASK(F5) },
	{ { "$m6", "$m$M6" }, MASK(C5) | MASK(E5B) | MASK(G5) | MASK(D5) | MASK(F5) | MASK(A5) },
	{ { "$a6", "6$S5", "$a$M6" }, MASK(C5) | MASK(E5) | MASK(A5B) | MASK(D5) | MASK(F5) | MASK(A5) },
	{ { "$h6" }, MASK(C5) | MASK(E5B) | MASK(G5B) | MASK(D5) | MASK(F5) | MASK(A5) },

	/* eleventh */
	{  { "11", "dom11" }, MASK(C5) | MASK(E5) | MASK(G5) | MASK(H5B) | MASK(D5) | MASK(F5) },
	{  { "$M11" }, MASK(C5) | MASK(E5) | MASK(G5) | MASK(H5) | MASK(D5) | MASK(F5) },
	{  { "$m$M11" }, MASK(C5) | MASK(E5B) | MASK(G5) | MASK(H5) | MASK(D5) | MASK(F5) },
	{  { "$m11" }, MASK(C5) | MASK(E5B) | MASK(G5) | MASK(H5B) | MASK(D5) | MASK(F5) },
	{  { "$a$M11" }, MASK(C5) | MASK(E5) | MASK(A5B) | MASK(H5) | MASK(D5) | MASK(F5) },
	{  { "$a11", "11$S5" }, MASK(C5) | MASK(E5) | MASK(A5B) | MASK(H5B) | MASK(D5) | MASK(F5) },
	{  { "$h11" }, MASK(C5) | MASK(E5B) | MASK(G5B) | MASK(H5B) | MASK(D5B) | MASK(F5) },
	{  { "$d11" }, MASK(C5) | MASK(E5B) | MASK(G5B) | MASK(A5) | MASK(D5B) | MASK(E5) },

	/* agumented and major sixths and elevenths susXXX */
	{ { "6sus$S1", "6sus$f2" }, MASK(C5) | MASK(D5B) | MASK(D5) | MASK(F5) | MASK(G5) | MASK(A5) },
	{ { "$a6sus$S1", "$a6sus$f2" }, MASK(C5) | MASK(D5B) | MASK(D5) | MASK(F5) | MASK(A5B) | MASK(A5) },
	{ { "11sus$S1", "11sus$f2" }, MASK(C5) | MASK(D5B) | MASK(D5) | MASK(F5) | MASK(G5) | MASK(H5B) },
	{ { "$a11sus$S1", "$a11sus$f2" }, MASK(C5) | MASK(D5B) | MASK(D5) | MASK(F5) | MASK(A5B) | MASK(H5B) },
	{ { "6sus$S2", "6sus$f3" }, MASK(C5) | MASK(D5) | MASK(E5B) | MASK(F5) | MASK(G5) | MASK(A5) },
	{ { "$a6sus$S2", "$a6sus$f3" }, MASK(C5) | MASK(D5) | MASK(E5B) | MASK(F5) | MASK(A5B) | MASK(A5) },
	{ { "11sus$S2", "11sus$f3" }, MASK(C5) | MASK(D5) | MASK(E5B) | MASK(F5) | MASK(G5) | MASK(H5B) },
	{ { "$a11sus$S2", "$a11sus$f3" }, MASK(C5) | MASK(D5) | MASK(E5B) | MASK(F5) | MASK(A5B) | MASK(H5B) },
	{ { "6sus$S4", "6sus$f5" }, MASK(C5) | MASK(D5) | MASK(F5) | MASK(G5B) | MASK(G5) | MASK(A5) },
	{ { "$a6sus$S4", "$a6sus$f5" }, MASK(C5) | MASK(D5) | MASK(F5) | MASK(G5B) | MASK(A5B) | MASK(A5) },
	{ { "11sus$S4", "11sus$f5" }, MASK(C5) | MASK(D5) | MASK(F5) | MASK(G5B) | MASK(G5) | MASK(H5B) },
	{ { "$a11sus$S4", "$a11sus$f5" }, MASK(C5) | MASK(D5) | MASK(F5) | MASK(G5B) | MASK(A5B) | MASK(H5B) },
	{ { "$a6sus5" }, MASK(C5) | MASK(D5) | MASK(F5) | MASK(G5) | MASK(A5B) | MASK(A5) },
	{ { "$a11sus5" }, MASK(C5) | MASK(D5) | MASK(F5) | MASK(G5) | MASK(A5B) | MASK(H5B) },
	{ { "6sus$S5", "6sus$f6" }, MASK(C5) | MASK(D5) | MASK(F5) | MASK(G5) | MASK(A5B) | MASK(A5) },
	{ { "11sus$S5", "11sus$f6" }, MASK(C5) | MASK(D5) | MASK(F5) | MASK(G5) | MASK(A5B) | MASK(H5B) },
	{ { "11sus6" }, MASK(C5) | MASK(D5) | MASK(F5) | MASK(G5) | MASK(A5) | MASK(H5B) },
	{ { "$a11sus6" }, MASK(C5) | MASK(D5) | MASK(F5) | MASK(A5B) | MASK(A5) | MASK(H5B) },
	{ { "6sus$S6", "6sus$f7" }, MASK(C5) | MASK(D5) | MASK(F5) | MASK(G5) | MASK(A5) | MASK(H5B) },
	{ { "$a6sus$S6", "$a6sus$f7" }, MASK(C5) | MASK(D5) | MASK(F5) | MASK(A5B) | MASK(A5) | MASK(H5B) },
	{ { "6sus7" }, MASK(C5) | MASK(D5) | MASK(F5) | MASK(G5) | MASK(A5) | MASK(H5) },
	{ { "$a6sus7" }, MASK(C5) | MASK(D5) | MASK(F5) | MASK(A5B) | MASK(A5) | MASK(H5) },
	{ { "11sus7" }, MASK(C5) | MASK(D5) | MASK(F5) | MASK(G5) | MASK(H5B) | MASK(H5) },
	{ { "$a11sus7" }, MASK(C5) | MASK(D5) | MASK(F5) | MASK(A5B) | MASK(H5B) | MASK(H5) },
 
	/* thirteenth */
	{  { "13", "dom13" }, MASK(C5) | MASK(E5) | MASK(G5) | MASK(H5B) | MASK(D5) | MASK(F5) | MASK(A5) },
	{  { "$M13" }, MASK(C5) | MASK(E5) | MASK(G5) | MASK(H5) | MASK(D5) | MASK(F5) | MASK(A5) },
	{  { "$m$M13" }, MASK(C5) | MASK(E5B) | MASK(G5) | MASK(H5) | MASK(D5) | MASK(F5) | MASK(A5) },
	{  { "$m13" }, MASK(C5) | MASK(E5B) | MASK(G5) | MASK(H5B) | MASK(D5) | MASK(F5) | MASK(A5) },
	{  { "$a$M13" }, MASK(C5) | MASK(E5) | MASK(A5B) | MASK(H5) | MASK(D5) | MASK(F5) | MASK(A5) },
	{  { "$a13", "13$S5" }, MASK(C5) | MASK(E5) | MASK(A5B) | MASK(H5B) | MASK(D5) | MASK(F5) | MASK(A5) },
	{  { "$h13" }, MASK(C5) | MASK(E5B) | MASK(G5B) | MASK(H5B) | MASK(D5) | MASK(F5) | MASK(A5) },

	/* agumented and major thirteenths susXXX */
	{ { "13sus$S1", "13sus$f2" }, MASK(C5) | MASK(D5B) | MASK(D5) | MASK(F5) | MASK(G5) | MASK(A5) | MASK(H5B) },
	{ { "$a13sus$S1", "$a13sus$f2" }, MASK(C5) | MASK(D5B) | MASK(D5) | MASK(F5) | MASK(A5B) | MASK(A5) | MASK(H5B) },
	{ { "13sus$S2", "13sus$f3" }, MASK(C5) | MASK(D5) | MASK(E5B) | MASK(F5) | MASK(G5) | MASK(A5) | MASK(H5B) },
	{ { "$a13sus$S2", "$a13sus$f3" }, MASK(C5) | MASK(D5) | MASK(E5B) | MASK(F5) | MASK(A5B) | MASK(A5) | MASK(H5B) },
	{ { "13sus$S4", "13sus$f5" }, MASK(C5) | MASK(D5) | MASK(F5) | MASK(G5B) | MASK(G5) | MASK(A5) | MASK(H5B) },
	{ { "$a13sus$S4", "$a13sus$f5" }, MASK(C5) | MASK(D5) | MASK(F5) | MASK(G5B) | MASK(A5B) | MASK(A5) | MASK(H5B) },
	{ { "$a13sus5" }, MASK(C5) | MASK(D5) | MASK(F5) | MASK(G5) | MASK(A5B) | MASK(A5) | MASK(H5B) },
	{ { "13sus$S5", "13sus$f6" }, MASK(C5) | MASK(D5) | MASK(F5) | MASK(G5) | MASK(A5B) | MASK(A5) | MASK(H5B) },
	{ { "13sus7" }, MASK(C5) | MASK(D5) | MASK(F5) | MASK(G5) | MASK(A5) | MASK(H5B) | MASK(H5) },
	{ { "$a13sus7" }, MASK(C5) | MASK(D5) | MASK(F5) | MASK(A5B) | MASK(A5) | MASK(H5B) | MASK(H5) },
};

#define	MAX_SCORES 2048

static class score_variant *mpp_score_variant;

static unsigned mpp_max_variant;

static uint32_t
MppRor(uint32_t val, uint8_t n)
{
	return (((val >> n) | (val << (12 - n))) & 0xfff);
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
	for (x = 0; x != 12; x++) {
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
MppScoreStoreAdd(const class score_variant &orig, unsigned &y, const int which)
{
	uint32_t mask = MASK(which) | MASK(H5);

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
MppScoreMatchPattern(const QString &pattern, const QString str)
{
	int t = 0;
	int u = 0;

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
			QString temp;
			for (y = 1; score_macros[x][y]; y++) {
				temp = QString::fromUtf8(score_macros[x][y]);
				if (str.indexOf(temp, u) == u)
					break;
			}
			if (score_macros[x][y] == 0)
				return (false);
			t += 2;
			u += temp.length();
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
		for (z = 1; z != 12; z++)
			MppScoreStoreAdd(mpp_score_variant[x], y, z);
	}

	qsort(mpp_score_variant, y, sizeof(mpp_score_variant[0]), &MppScoreVariantCmp);

	for (x = 0; x != y; x++) {
		if (mpp_score_variant[x].duplicate)
			continue;
		for (z = x + 1; z != y; z++) {
			for (t = 0; t != 12; t++) {
				if (MppRor(mpp_score_variant[z].footprint, t) ==
					    mpp_score_variant[x].footprint)
					break;
			}
			if (t != 12)
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

		for (t = 0; t != 12; t++) {
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
		key = C5;
		index++;
	} else if (ptr[index] == 'D') {
		key = D5;
		index++;
	} else if (ptr[index] == 'E') {
		key = E5;
		index++;
	} else if (ptr[index] == 'F') {
		key = F5;
		index++;
	} else if (ptr[index] == 'G') {
		key = G5;
		index++;
	} else if (ptr[index] == 'A') {
		key = A5;
		index++;
	} else if (ptr[index] == 'H' || ptr[index] == 'B') {
		key = H5;
		index++;
	} else {
		return (0);
	}
	if (index < ptr.length()) {
		if (ptr[index] == '#') {
			key++;
			index++;
		} else if (ptr[index] == 'b') {
			key--;
			index++;
		}
	}
	return (key);
}

uint8_t
MppDecodeTab :: parseScoreChord(MppChordElement *pinfo)
{
	QString out;
	uint8_t temp[12];
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

	if (pinfo->chord != 0)
		is_sharp = (pinfo->chord->txt.indexOf('#') > -1);
	else
		is_sharp = 0;

	for (x = 0; x != 12; x++) {
		if (pinfo->stats[x] != 0)
			footprint |= (1 << x);
	}

	if (footprint == 0)
		return (1);	/* not found */

	best_x = 0;
	best_y = 12;
	best_z = 12;

	for (x = 0; x != mpp_max_variant; x++) {
		if (mpp_score_variant[x].duplicate)
			continue;
		for (y = 0; y != 12; y++) {
			z = MppSumbits(mpp_score_variant[x].footprint ^ MppRor(footprint, y));
			if (z < best_z) {
				best_z = z;
				best_x = x;
				best_y = y;
			}
		}
	}
	x = best_x;
	y = best_y;

	if (x == mpp_max_variant || y == 12)
		return (1);	/* not found */

	for (n = z = 0; z != 12; z++) {
		if (mpp_score_variant[x].footprint & (1 << z))
			temp[n++] = z + y + C5;
	}
	if (n == 0)
		return (1);	/* not found */

	rol = 0;
	flags = 0;

	while (flags != 3 && rol < 126 && rol > -126) {

		mid_sort(temp, n);

		if (temp[n-1] < pinfo->key_max) {
			mid_trans(temp, n, 1);
			rol++;
			flags |= 1;
		} else if (temp[n-1] > pinfo->key_max) {
			mid_trans(temp, n, -1);
			rol--;
			flags |= 2;
		} else {
			break;
		}
	}

	rol_value = rol;

	out += MppBaseKeyToString(y, is_sharp);
	out += MppScoreExpandPattern(mpp_score_variant[x].pattern);

	if (y != pinfo->key_base) {
		out += "/";
		out += MppBaseKeyToString(pinfo->key_base, is_sharp);
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
	uint8_t error = 0;
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

	for (y = 0; y != 12; y++) {
		if (!(mpp_score_variant[x].footprint & (1 << y)))
			continue;
		if (n < *pn)
			pout[n++] = y + key;
	}

	mid_trans(pout + 1, n - 1, rol);

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

	gb->addWidget(new QLabel(tr("[CDEFGABH][#b][...][/CDEFGABH[#b]]")),
	    0,0,1,4, Qt::AlignHCenter|Qt::AlignVCenter);

	gb->addWidget(but_rol_down, 2,0,1,1);
	gb->addWidget(but_rol_up, 2,1,1,1);

	gb->addWidget(but_mod_down, 2,2,1,1);
	gb->addWidget(but_mod_up, 2,3,1,1);

	gb->addWidget(lin_edit, 3,0,1,4);
	gb->addWidget(lin_out, 4,0,1,4);

	gb->addWidget(lbl_status, 5,0,1,2, Qt::AlignHCenter|Qt::AlignVCenter);
	gb->addWidget(new QLabel(tr("Add auto base:")), 5,2,1,1, Qt::AlignRight|Qt::AlignVCenter);
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

	pthread_mutex_lock(&mw->mtx);
	for (x = 0; x != MPP_MAX_VAR_OFF; x++) {
		if (auto_base[x] != 0)
			mw->output_key(sm->synthChannel, auto_base[x], vel, 0, 0);
	}
	for (x = 0; current_score[x]; x++) {
		mw->output_key(sm->synthChannel, current_score[x], vel, 0, 0);
	}
	pthread_mutex_unlock(&mw->mtx);
}

void
MppDecodeTab :: handle_play_release(int value)
{
	MppScoreMain *sm = mw->scores_main[value / 128];
	uint8_t x;

	pthread_mutex_lock(&mw->mtx);
	for (x = 0; x != MPP_MAX_VAR_OFF; x++) {
		if (auto_base[x] != 0)
			mw->output_key(sm->synthChannel, auto_base[x], 0, 0, 0);
	}
	for (x = 0; current_score[x]; x++) {
		mw->output_key(sm->synthChannel, current_score[x], 0, 0, 0);
	}
	pthread_mutex_unlock(&mw->mtx);
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
			QChar ch = chord[0];
			int endpos = chord.indexOf("/");
			int startpos = 1;

			if (endpos < 0)
				endpos = chord.length();

			if ((endpos > startpos) &&
			    (chord[startpos] == 'b' ||
			     chord[startpos] == 'B' ||
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

		while (b_auto >= (int)(current_score[1] & 0x7F))
			b_auto -= 12;

		if (b_auto >= 12) {
		  auto_base[0] = (b_auto - 12) & 0x7F;
		  out += QString(mid_key_str[auto_base[0]]) +
			  QString(" ");
		}
		if (b_auto >= 0) {
		  auto_base[1] = (b_auto - 0) & 0x7F;
		  out += QString(mid_key_str[auto_base[1]]) +
			  QString(" ");
		}
	}

	for (x = 1; x < n; x++) {
		out += QString(mid_key_str[current_score[x] & 0x7F]) +
		  QString(" ");
	}

done:
	out += QString("/* ") + lin_edit->text().trimmed() 
	  + QString(" */");

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
	int num = event->delta();

	if (event->orientation() == Qt::Vertical && num != 0) {
		handle_parse(num / (8 * 15));
		event->accept();
	} else {
		event->ignore();
	}
}
