/*-
 * Copyright (c) 2011 Hans Petter Selasky. All rights reserved.
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

/*
 * GuitarPro format documentation: http://dguitar.sourceforge.net
 *
 * Mauricio Gracia Gutierrez
 * Matthieu Wipliez
 */

#include "midipp_gpro.h"
#include "midipp_checkbox.h"

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <sys/queue.h>

struct gpro_event;
typedef TAILQ_ENTRY(gpro_event) gpro_event_entry_t;
typedef TAILQ_HEAD(gpro_event_head,gpro_event) gpro_event_head_t;

#ifdef HAVE_DEBUG
#define	DPRINTF(fmt, ...) do { \
	printf("%s:%d: " fmt, __FUNCTION__, __LINE__,## __VA_ARGS__); \
} while (0)
#else
#define	DPRINTF(...) do { } while (0)
#endif

struct gpro_measure {
	gpro_event_head_t head;
	uint32_t start_time;
	uint8_t start_repeat;
	uint8_t end_repeat;
};

struct gpro_file {
	gpro_event_head_t head;
	gpro_event_head_t temp;
	QString *out;
	const uint8_t *ptr;
	char *track_str[GPRO_MAX_TRACKS];
	struct gpro_measure *pmeas;
	uint32_t rem;
	uint32_t str_len;
	uint32_t ticks_sub;
	uint32_t imeas;
	uint32_t nmeas;
	uint32_t chan_mask;
	uint8_t dur_num;
	uint8_t dur_div;
	uint8_t string_max[GPRO_MAX_TRACKS];
	uint8_t track;
	uint8_t is_v4;
	uint8_t capo[GPRO_MAX_TRACKS];
	uint8_t tuning[GPRO_MAX_TRACKS][8];
};

struct gpro_event {
	gpro_event_entry_t entry;
	uint8_t key;
	uint8_t chan;
	uint32_t time;
	uint32_t dur;
};

static uint32_t
gpro_duration_to_ticks(uint8_t dur)
{
	dur += 2;

	if (dur < GPRO_LHZ)
		return (GPRO_HZ >> dur);
	else
		return (1);
}

static void
gpro_skip(struct gpro_file *pgf, uint32_t nskip)
{
	if (nskip > pgf->rem)
		nskip = pgf->rem;

	pgf->ptr += nskip;
	pgf->rem -= nskip;
}

static uint8_t
gpro_get_1(struct gpro_file *pgf)
{
	uint8_t temp;

	if (pgf->rem != 0) {
		temp = *(pgf->ptr);
		pgf->ptr++;
		pgf->rem--;
	} else {
		temp = 0;
	}
	return (temp);
}

static uint32_t
gpro_get_4(struct gpro_file *pgf)
{
	uint32_t temp;

	temp = gpro_get_1(pgf);
	temp |= gpro_get_1(pgf) << 8;
	temp |= gpro_get_1(pgf) << 16;
	temp |= gpro_get_1(pgf) << 24;

	return (temp);
}

#if 0
static uint16_t
gpro_get_2(struct gpro_file *pgf)
{
	uint16_t temp;

	temp = gpro_get_1(pgf);
	temp |= gpro_get_1(pgf) << 8;

	return (temp);
}
#endif

static char *
gpro_get_string_1(struct gpro_file *pgf)
{
	uint32_t len;
	uint32_t x;
	char *ptr;

	len = gpro_get_1(pgf);

	ptr = (char *)malloc(len + 1);

	if (ptr == NULL) {
		gpro_skip(pgf, len);
	} else {

		for (x = 0; x != len; x++)
			ptr[x] = gpro_get_1(pgf);
		ptr[x] = 0;

		DPRINTF("%s %d\n", ptr, x);
	}

	pgf->str_len = len;
	return (ptr);
}

static uint8_t
gpro_eof(struct gpro_file *pgf)
{
	return (pgf->rem == 0);
}

static char *
gpro_get_string_4_wrap(struct gpro_file *pgf)
{
	uint32_t len;
	uint32_t x;
	char *ptr;

	len = gpro_get_4(pgf);

	gpro_get_1(pgf);

	if (len)
		len--;

	if (len <= pgf->rem) {
		ptr = (char *)malloc(len + 1);
	} else {
		ptr = NULL;
	}

	if (ptr == NULL) {
		gpro_skip(pgf, len);
	} else {
		for (x = 0; (x != len); x++) {
				ptr[x] = gpro_get_1(pgf);
		}
		ptr[x] = 0;

		DPRINTF("%s %d\n", ptr, x);
	}

	pgf->str_len = len;

	return (ptr);
}

static char *
gpro_get_string_4(struct gpro_file *pgf)
{
	uint32_t len;
	uint32_t x;
	char *ptr;

	len = gpro_get_4(pgf);

	if (len <= pgf->rem) {
		ptr = (char *)malloc(len + 1);
	} else {
		ptr = NULL;
	}

	if (ptr == NULL) {
		gpro_skip(pgf, len);
	} else {
		for (x = 0; (x != len); x++) {
				ptr[x] = gpro_get_1(pgf);
		}
		ptr[x] = 0;

		DPRINTF("%s %d\n", ptr, x);
	}

	pgf->str_len = len;

	return (ptr);
}

static void
gpro_pad_string(struct gpro_file *pgf, uint32_t len)
{
	/* skip reset of header */

	while (pgf->str_len < len) {
		gpro_get_1(pgf);
		pgf->str_len++;
	}
}

static void
gpro_new_event(gpro_event_head_t *phead, uint32_t time,
    uint32_t dur, uint8_t key, uint8_t chan)
{
	struct gpro_event *pev;
	struct gpro_event *pin;

	pev = (struct gpro_event *)malloc(sizeof(*pev));
	if (pev == NULL)
		return;

	pev->key = key;
	pev->chan = chan;
	pev->time = time;
	pev->dur = dur;

	DPRINTF("key=%s chan=%d time=%d dur=%d\n",
	    mid_key_str[key & 0x7F], chan, time, dur);

	TAILQ_FOREACH(pin, phead, entry) {
		if (pin->time > time)
			break;
	}

	if (pin == NULL)
		TAILQ_INSERT_TAIL(phead, pev, entry);
	else
		TAILQ_INSERT_BEFORE(pin, pev, entry);
}

static void
gpro_copy_merge_head(gpro_event_head_t *phead1, gpro_event_head_t *phead2, uint32_t time_offset)
{
	struct gpro_event *pev;
	struct gpro_event *pin;
	struct gpro_event *pxv;
	struct gpro_event *plv;

	pin = TAILQ_FIRST(phead1);
	if (pin != NULL) {
		pev = TAILQ_FIRST(phead2);
		if (pev != NULL) {
			plv = TAILQ_LAST(phead1, gpro_event_head);
			if ((pev->time + time_offset) > plv->time)
				pin = NULL;
		}
	}

	TAILQ_FOREACH(pev, phead2, entry) {

		while ((pin != NULL) && ((pev->time + time_offset) > pin->time))
			pin = TAILQ_NEXT(pin, entry);

		pxv = (struct gpro_event *)malloc(sizeof(*pxv));
		if (pxv == NULL)
			continue;

		pxv->key = pev->key;
		pxv->chan = pev->chan;
		pxv->time = pev->time + time_offset;
		pxv->dur = pev->dur;

		if (pin == NULL)
			TAILQ_INSERT_TAIL(phead1, pxv, entry);
		else
			TAILQ_INSERT_BEFORE(pin, pxv, entry);
	}
}

static uint32_t
gpro_event_duration(struct gpro_file *pgf, struct gpro_event *pev)
{
	uint32_t tim = pev->time;
	uint32_t end = tim + pev->dur;
	uint32_t dur = 1;
	uint8_t chan;
	uint8_t key;

	chan = pev->chan;
	key = pev->key;

	while ((pev = TAILQ_NEXT(pev, entry))) {

		if (pev->chan >= GPRO_MAX_TRACKS)
			continue;
		if ((pgf->chan_mask & (1 << pev->chan)) == 0)
			continue;
		if (pev->time > end)
			break;
		if (pev->chan == chan && pev->key == key)
			break;
		if (pev->time != tim) {
			tim = pev->time;
			dur++;
			if (dur == GPRO_MAX_DURATION)
				break;
		}
	}
	return (dur);
}

static void
gpro_clean_events(gpro_event_head_t *phead)
{
	struct gpro_event *pev;

	while ((pev = TAILQ_FIRST(phead)) != NULL) {
		TAILQ_REMOVE(phead, pev, entry);
		free(pev);
	}
}

static void
gpro_get_chord_diagram(struct gpro_file *pgf)
{
	char *ptr;
	uint8_t hdr;
	uint32_t x;
	uint32_t base;

	hdr = gpro_get_1(pgf);

	DPRINTF("hdr=%d\n", hdr);

	if (hdr & (1 << 0)) {

		if (pgf->is_v4) {

			/* sharp/flat */
			gpro_get_1(pgf);

			/* blank */
			gpro_get_1(pgf);
			gpro_get_1(pgf);
			gpro_get_1(pgf);

			/* root */
			gpro_get_1(pgf);

			/* chord type */
			gpro_get_1(pgf);

			/* 9,11,13 */
			gpro_get_1(pgf);

			/* bass */
			gpro_get_4(pgf);

			/* tonality */
			gpro_get_4(pgf);

			/* added note */
			gpro_get_1(pgf);

			/* chord name */
			ptr = gpro_get_string_1(pgf);
			free(ptr);

			gpro_pad_string(pgf, 20);

			/* blank */
			gpro_get_1(pgf);
			gpro_get_1(pgf);

			/* tonality - 5 */
			gpro_get_1(pgf);

			/* tonality - 9 */
			gpro_get_1(pgf);

			/* tonality - 11 */
			gpro_get_1(pgf);

			/* base fret */
			base = gpro_get_4(pgf);

			/* frets */
			for (x = 0; x != 7; x++)
				gpro_get_4(pgf);

			/* barres */
			gpro_get_1(pgf);

			/* fret of barre */
			for (x = 0; x != 5; x++)
				gpro_get_1(pgf);

			/* start of barre */
			for (x = 0; x != 5; x++)
				gpro_get_1(pgf);

			/* end of barre */
			for (x = 0; x != 5; x++)
				gpro_get_1(pgf);

			/* blank */
			gpro_get_4(pgf);
			gpro_get_4(pgf);

			/* fingering */
			for (x = 0; x != 7; x++)
				gpro_get_1(pgf);

			/* finger display */
			gpro_get_1(pgf);

		} else {
			/* sharp or flat */
			gpro_get_1(pgf);

			/* blank */
			gpro_get_1(pgf);
			gpro_get_1(pgf);
			gpro_get_1(pgf);

			/* root */
			gpro_get_4(pgf);

			/* major/minor */
			gpro_get_4(pgf);

			/* nine, eleven or thirteen */
			gpro_get_4(pgf);

			/* bass */
			gpro_get_4(pgf);

			/* diminished/augmented */
			gpro_get_1(pgf);

			/* blank */
			gpro_get_1(pgf);
			gpro_get_1(pgf);
			gpro_get_1(pgf);

			/* add */
			gpro_get_1(pgf);

			/* chord name */
			ptr = gpro_get_string_1(pgf);
			free(ptr);

			gpro_pad_string(pgf, 34);

			/* base fret */
			base = gpro_get_4(pgf);

			/* frets */
			for (x = 0; x != 6; x++) {
				gpro_get_4(pgf);
			}

			/* blank */
			for (x = 0; x != 28; x++)
				gpro_get_1(pgf);

			/* notes in chord */
			for (x = 0; x != 7; x++)
				gpro_get_1(pgf);

			gpro_get_1(pgf);
		}
		return;
	}

	/* chord name */
	ptr = gpro_get_string_4_wrap(pgf);
	free(ptr);

	base = gpro_get_4(pgf);
	if (base != 0) {
		for (x = 0; x != 6; x++) {
			/* read fret */
			gpro_get_4(pgf);
		}
	}
}

static void
gpro_get_bend(struct gpro_file *pgf)
{
	uint32_t npoint;
	uint32_t x;

	/* type */
	gpro_get_1(pgf);

	/* value */
	gpro_get_4(pgf);

	npoint = gpro_get_4(pgf);

	DPRINTF("npoint=%d\n", npoint);

	for (x = 0; (x != npoint) && (gpro_eof(pgf) == 0); x++) {
		gpro_get_4(pgf);
		gpro_get_4(pgf);
		gpro_get_1(pgf);
	}
}

static void
gpro_get_marker(struct gpro_file *pgf)
{
	char *ptr;

	DPRINTF("\n");

	ptr = gpro_get_string_4_wrap(pgf);
	free(ptr);

	gpro_get_4(pgf);
}

static void
gpro_get_beat_effect_v3(struct gpro_file *pgf)
{
	uint8_t hdr;

	DPRINTF("\n");

	hdr = gpro_get_1(pgf);

	if (hdr & (1 << 5)) {
		gpro_get_1(pgf);
		gpro_get_4(pgf);
	}

	if (hdr & (1 << 4)) {
		gpro_get_1(pgf);
		gpro_get_1(pgf);
	}

	if (hdr & (1 << 6)) {
		gpro_get_1(pgf);
		gpro_get_1(pgf);
	}
}

static void
gpro_get_beat_effect_v4(struct gpro_file *pgf)
{
	uint8_t hdr1;
	uint8_t hdr2;

	hdr1 = gpro_get_1(pgf);
	hdr2 = gpro_get_1(pgf);

	DPRINTF("hdr1 = 0x%x hdr2 = 0x%x\n", hdr1, hdr2);

	/* tapping/slapping/popping */
	if (hdr1 & (1 << 5)) {
		gpro_get_1(pgf);
	}

	if (hdr2 & (1 << 2))
		gpro_get_bend(pgf);

	/* stroke effect */
	if (hdr1 & (1 << 6)) {
		gpro_get_1(pgf);
		gpro_get_1(pgf);
	}

	/* pickstroke */
	if (hdr2 & (1 << 1)) {
		gpro_get_1(pgf);
	}
}

static void
gpro_get_mix_table(struct gpro_file *pgf)
{
	uint32_t n;
	uint32_t x;
	uint32_t temp;

	DPRINTF("\n");

	for (n = x = 0; x != 7; x++) {
		temp = gpro_get_1(pgf);
		if ((x != 0) && (temp != (uint8_t)-1))
			n++;
	}

	/* tempo */
	temp = gpro_get_4(pgf);
	if (temp != (uint32_t)-1)
		n++;

	DPRINTF("tempo = %d\n", temp);

	for (x = 0; x != n; x++)
		gpro_get_1(pgf);

	/* apply to all tracks */
	if (pgf->is_v4)
		gpro_get_1(pgf);
}

static void
gpro_get_grace(struct gpro_file *pgf)
{
	DPRINTF("\n");

	gpro_get_1(pgf);
	gpro_get_1(pgf);
	gpro_get_1(pgf);
	gpro_get_1(pgf);
}

static uint8_t
gpro_get_note_effect_v3(struct gpro_file *pgf)
{
	uint8_t hdr;

	hdr = gpro_get_1(pgf);

	DPRINTF("hdr=0x%x\n", hdr);

	if (hdr & (1 << 0))
		gpro_get_bend(pgf);

	if (hdr & (1 << 4))
		gpro_get_grace(pgf);

	return ((hdr & 0x08) ? 2 : 0);
}

static uint8_t
gpro_get_note_effect_v4(struct gpro_file *pgf)
{
	uint8_t hdr1;
	uint8_t hdr2;
	uint8_t flags = 0;

	hdr1 = gpro_get_1(pgf);
	hdr2 = gpro_get_1(pgf);

	DPRINTF("hdr1=0x%x hdr2=0x%x\n", hdr1, hdr2);

	if (hdr1 & (1 << 0))
		gpro_get_bend(pgf);

	if (hdr1 & (1 << 4))
		gpro_get_grace(pgf);

	/* tremolo picking */
	if (hdr2 & (1 << 2))
		gpro_get_1(pgf);

	/* */
	if (hdr2 & (1 << 3))
		gpro_get_1(pgf);

	/* */
	if (hdr2 & (1 << 4))
		gpro_get_1(pgf);

	/* */
	if (hdr2 & (1 << 5)) {
		gpro_get_1(pgf);
		gpro_get_1(pgf);
	}

	if (hdr1 & (1 << 5))
		flags |= 2;

	return (flags);
}

static uint32_t
gpro_get_note(struct gpro_file *pgf)
{
	uint8_t hdr;
	uint8_t fret = 0;
	uint8_t dur;
	uint8_t ntup;
	uint8_t flags;
	uint8_t type = 0;

	hdr = gpro_get_1(pgf);

	flags = (hdr & (1 << 1)) ? 1 : 0;

	/* note type */
	if (hdr & (1 << 5)) {
		type = gpro_get_1(pgf);

		/* check for tie note */
		if (type == 0x02)
			flags |= 4;
	}

	DPRINTF("hdr = 0x%x type = 0x%x\n", hdr, type);

	/* duration */
	if (hdr & (1 << 0)) {
		dur = gpro_get_1(pgf);
		ntup = gpro_get_1(pgf);
		DPRINTF("dur = %d, ntup = %d\n", dur, ntup);
	} else {
		dur = 0x80;
		ntup = 0;
	}

	/* note dynamic */
	if (hdr & (1 << 4))
		gpro_get_1(pgf);

	/* fret number */
	if (hdr & (1 << 5)) {
		fret = gpro_get_1(pgf);
		DPRINTF("fret=%d\n", fret);
	}

	/* fingering */
	if (hdr & (1 << 7)) {
		gpro_get_1(pgf);
		gpro_get_1(pgf);
	}

	/* effect on note */
	if (hdr & (1 << 3)) {
		if (pgf->is_v4)
			flags |= gpro_get_note_effect_v4(pgf);
		else
			flags |= gpro_get_note_effect_v3(pgf);
	}
	return (fret | (dur << 8) | (ntup << 16) | (flags << 24));
}

static void
gpro_get_measure(struct gpro_file *pgf)
{
	uint8_t hdr;
	uint8_t x;

	memset(&pgf->pmeas[pgf->imeas], 0, sizeof(pgf->pmeas[0]));

	TAILQ_INIT(&pgf->pmeas[pgf->imeas].head);

	hdr = gpro_get_1(pgf);

	DPRINTF("hdr = 0x%x\n", hdr);

	/* start of repeat */
	if (hdr & (1 << 2)) {
		DPRINTF("Start repeat\n");
		pgf->pmeas[pgf->imeas].start_repeat = 1;
	}

	/* numerator key signature */
	if (hdr & (1 << 0)) {
		x = gpro_get_1(pgf);
		DPRINTF("Num = %d\n", x);
		pgf->dur_num = x;
	}

	/* denominator key signature */
	if (hdr & (1 << 1)) {
		x = gpro_get_1(pgf);
		DPRINTF("DeNum = %d\n", x);
		pgf->dur_div = x;
	}

	/* end of repeat */
	if (hdr & (1 << 3)) {
		x = gpro_get_1(pgf);
		DPRINTF("End repeat %d\n", x);
		pgf->pmeas[pgf->imeas].end_repeat = x;
	}

	/* number of alternate ending */
	if (hdr & (1 << 4))
		gpro_get_1(pgf);

	/* marker string */
	if (hdr & (1 << 5))
		gpro_get_marker(pgf);

	/* tonality */
	if (hdr & (1 << 6)) {
		gpro_get_1(pgf);
		gpro_get_1(pgf);
	}
}

static void
gpro_get_beat(struct gpro_file *pgf)
{
	char *ptr;
	uint32_t beat_dur;
	uint32_t note_dur;
	uint32_t beat_tup;
	uint32_t note_tup;
	uint8_t note_key;
	uint8_t hdr;
	uint8_t str;
	uint8_t n;
	uint8_t dur;
	uint8_t dotted;
	uint8_t status;
	uint8_t smax;

	hdr = gpro_get_1(pgf);	/* header */

	DPRINTF("hdr = 0x%x\n", hdr);

	if (hdr & 0x80)
		hdr &= 0x3f;

	/* beat status */
	if (hdr & (1 << 6)) {
		status = gpro_get_1(pgf);
	} else {
		status = 255;
	}

	/* beat duration */
	dur = gpro_get_1(pgf);

	/* dotted notes */
	dotted = (hdr & (1 << 0)) ? 1 : 0;

	/* get ntuplet */
	if (hdr & (1 << 5)) {
		beat_tup = gpro_get_4(pgf);
	} else {
		beat_tup = 0;
	}

	DPRINTF("dur = %d, dotted = %d, tuplet = %d, status = %d\n",
	    dur, dotted, beat_tup, status);

	/* get chord diagram */
	if (hdr & (1 << 1))
		gpro_get_chord_diagram(pgf);

	if (hdr & (1 << 2)) {
		/* text */
		ptr = gpro_get_string_4_wrap(pgf);
		free(ptr);
	}

	if (hdr & (1 << 3)) {
		if (pgf->is_v4)
			gpro_get_beat_effect_v4(pgf);
		else
			gpro_get_beat_effect_v3(pgf);
	}

	if (hdr & (1 << 4))
		gpro_get_mix_table(pgf);

	/* strings played */
	str = ~gpro_get_1(pgf);

	DPRINTF("str = 0x%x\n", 0xFF ^ str);

	/* convert beat duration */
	beat_dur = gpro_duration_to_ticks(dur);

	/* maximum number of strings */
	smax = pgf->string_max[pgf->track];

	/* check for dotted */
	if (dotted)
		beat_dur += beat_dur >> 1;

	/* check for tuplet */
	if (beat_tup != 0) {
		uint32_t msb = beat_tup;

		/* compute msb */
		while (msb != ((-msb) & msb))
			msb &= msb - 1;

		/* scale */
		beat_dur *= msb;
		beat_dur /= beat_tup;
	}

	for (n = 1; n != 7; n++) {

		uint32_t flags;

		if (str & (1 << n))
			continue;

		flags = gpro_get_note(pgf);

		if (n > smax)
			continue;

		if (flags & (1U << 25)) {
			/* ring */
			note_dur = -1U;
		} else if ((flags & 0xff00) == 0x8000) {
			/* use beat duration */
			note_dur = beat_dur - 1;
		} else {
			/* use note duration */
			note_dur = gpro_duration_to_ticks((flags >> 8) & 0xFF);
			note_tup = (flags >> 16) & 0xFF;

			/* check for dotted */
			if (flags & (1U << 24))
				note_dur += note_dur >> 1;

			/* check for tuplet */
			if (note_tup != 0) {
				uint32_t msb = note_tup;

				/* compute msb */
				while (msb != ((-msb) & msb))
					msb &= msb - 1;

				/* scale */
				note_dur *= msb;
				note_dur /= note_tup;
			}

			note_dur -= 1;
		}

		/* get key */
		note_key = (flags & 0xFF) + pgf->capo[pgf->track] +
		    pgf->tuning[pgf->track][smax - n];

		/* special grip */
		if (0 && (n == 5)) {
			DPRINTF("Special\n");
			note_key --;
		}

		/* check for tie to previous measure */
		if ((flags & (1U << 26)) && (pgf->imeas != 0)) {
			struct gpro_event *pev;
			struct gpro_event *pev_last = NULL;

			TAILQ_FOREACH(pev, &pgf->pmeas[pgf->imeas - 1].head, entry) {
				if (pev->key == note_key && pev->chan == pgf->track) {
					pev_last = pev;
				}
			}
			if (pev_last != NULL) {
			    pev_last->dur += note_dur + 1;
			    continue;
			}
		}

		/* insert note into list of notes */
		gpro_new_event(&pgf->pmeas[pgf->imeas].head, pgf->ticks_sub,
		    note_dur, note_key, pgf->track);
	}

	pgf->ticks_sub += beat_dur;
}

static void
gpro_get_midi_info(struct gpro_file *pgf)
{
	uint32_t x;
	x = gpro_get_4(pgf);	/* instrument */

	if (x)
		DPRINTF("Instrument: 0x%08x\n", x);

	gpro_get_1(pgf);	/* volume */
	gpro_get_1(pgf);	/* balance */
	gpro_get_1(pgf);	/* chorus */
	gpro_get_1(pgf);	/* reverb */
	gpro_get_1(pgf);	/* phaser */
	gpro_get_1(pgf);	/* tremlo */
	gpro_get_1(pgf);	/* blank1 */
	gpro_get_1(pgf);	/* blank2 */
}

static void
gpro_get_track(struct gpro_file *pgf)
{
	char *ptr;
	uint8_t hdr;
	uint32_t x;
	uint32_t y;

	hdr = gpro_get_1(pgf);

	DPRINTF("hdr = 0x%x\n", hdr);

	/* track name */
	ptr = gpro_get_string_1(pgf);

	if (pgf->track < GPRO_MAX_TRACKS) {
		free(pgf->track_str[pgf->track]);
		pgf->track_str[pgf->track] = ptr;
	} else {
		free(ptr);
	}

	gpro_pad_string(pgf, 40);

	/* number of strings */
	x = gpro_get_4(pgf);

	if (x > 7)
		x = 7;

	pgf->string_max[pgf->track] = x;

	DPRINTF("Number of strings %d\n", x);

	/* tuning of strings */
	for (x = 0; x != 7; x++) {
		y = gpro_get_4(pgf);
		DPRINTF("String tuning[%d] = %d = %s\n",
		    x, y, mid_key_str[y & 0x7F]);
		pgf->tuning[pgf->track][x] = y;
	}

	/* port */
	gpro_get_4(pgf);

	/* channel */
	gpro_get_4(pgf);

	/* channel effects */
	gpro_get_4(pgf);

	/* number of frets */
	gpro_get_4(pgf);

	/* height of capo */
	x = gpro_get_4(pgf);

	DPRINTF("CAPO = %d\n", x);

	/* color */
	gpro_get_4(pgf);
}

static void
gpro_dump_events(struct gpro_file *pgf, QString &out, uint8_t single_track)
{
	struct gpro_event *pev;
	char buf[64];
	uint32_t chan_last = 0;
	uint32_t dur_last = -1U;
	uint32_t dur;
	uint32_t time_last;
	uint32_t nevent = 0;

	pev = TAILQ_FIRST(&pgf->head);
	if (pev != 0)
		time_last = ~pev->time;
	else
		time_last = 0;

	TAILQ_FOREACH(pev, &pgf->head, entry) {

		if (pev->chan >= GPRO_MAX_TRACKS)
			continue;
		if ((pgf->chan_mask & (1 << pev->chan)) == 0)
			continue;

		if (pev->time != time_last) {
			time_last = pev->time;
			chan_last = 0;
			dur_last = -1U;
			out += "\n";
#ifdef HAVE_DEBUG
			snprintf(buf, sizeof(buf), "/* %d */ ", pev->time);
			out += buf;
#endif
			nevent++;
			if (!(nevent & 15))
				out += "\n";
		}

		if (pev->chan != chan_last) {
			chan_last = pev->chan;
			if (single_track == 0) {
				snprintf(buf, sizeof(buf), "T%u ", chan_last);
				out += buf;
			}
		}

		dur = gpro_event_duration(pgf, pev);

		if (dur != dur_last) {
			dur_last = dur;
			snprintf(buf, sizeof(buf), "U%u ", dur_last);
			out += buf;
		}

		out += mid_key_str[pev->key & 0x7F];
		out += " ";
	}

	out += "\n";
}

static void
gpro_parse(struct gpro_file *pgf, QString *out)
{
	char *ptr;
	uint32_t nmeas;
	uint32_t ntrack;
	uint32_t value;
	uint32_t nbeat;
	uint32_t nscore;
	uint32_t y_repeat;
	uint32_t x;
	uint32_t y;
	uint32_t z;
	uint32_t acc_time;

	pgf->out = out;

	pgf->pmeas = 0;
	pgf->nmeas = 0;

	TAILQ_INIT(&pgf->head);
	TAILQ_INIT(&pgf->temp);

	memset(pgf->track_str, 0, sizeof(pgf->track_str));
	pgf->chan_mask = 0;

	/* version */

	ptr = gpro_get_string_1(pgf);

	if (strstr(ptr, "3.") == NULL) {
		if (strstr(ptr, "4.") == NULL) {
			free(ptr);
			return;
		} else {
			pgf->is_v4 = 1;
			DPRINTF("V4\n");
		}

	} else {
		pgf->is_v4 = 0;
		DPRINTF("V3\n");
	}
	free(ptr);

	gpro_pad_string(pgf, 30);

	/* title */
	ptr = gpro_get_string_4_wrap(pgf);
	if (out && ptr && ptr[0]) {
		*out += "/* Title: ";
		*out += ptr;
		*out += " */\n";
	}
	free(ptr);

	/* subtitle */
	ptr = gpro_get_string_4_wrap(pgf);
	if (out && ptr && ptr[0]) {
		*out += "/* Subtitle: ";
		*out += ptr;
		*out += " */\n";
	}
	free(ptr);

	/* interpret */
	ptr = gpro_get_string_4_wrap(pgf);
	if (out && ptr && ptr[0]) {
		*out += "/* Interpret: ";
		*out += ptr;
		*out += " */\n";
	}
	free(ptr);

	/* album */
	ptr = gpro_get_string_4_wrap(pgf);
	if (out && ptr && ptr[0]) {
		*out += "/* Album: ";
		*out += ptr;
		*out += " */\n";
	}
	free(ptr);

	/* author of the song */
	ptr = gpro_get_string_4_wrap(pgf);
	if (out && ptr && ptr[0]) {
		*out += "/* Song author: ";
		*out += ptr;
		*out += " */\n";
	}
	free(ptr);

	/* copyright */
	ptr = gpro_get_string_4_wrap(pgf);
	if (out && ptr && ptr[0]) {
		*out += "/* Copyright: ";
		*out += ptr;
		*out += " */\n";
	}
	free(ptr);

	/* author of the tablature */
	ptr = gpro_get_string_4_wrap(pgf);
	if (out && ptr && ptr[0]) {
		*out += "/* Author of tablature: ";
		*out += ptr;
		*out += " */\n";
	}
	free(ptr);

	/* instructional */
	ptr = gpro_get_string_4_wrap(pgf);
	if (out && ptr && ptr[0]) {
		*out += "/* Instructional: ";
		*out += ptr;
		*out += " */\n";
	}
	free(ptr);

	nscore = gpro_get_4(pgf);

	for (x = 0; (x != nscore) && (gpro_eof(pgf) == 0); x++) {
		/* scores */
		ptr = gpro_get_string_4_wrap(pgf);
		free(ptr);
	}

	/* triplet Feel */
	value = gpro_get_1(pgf);

	if (pgf->is_v4) {

		/* lyrics */
		x = gpro_get_4(pgf);

		DPRINTF("Lyrics measure = %d\n", x);

		*out += "/* Lyrics: \n\n";

		for (x = 0; x != 5; x++) {
			gpro_get_4(pgf);
			ptr = gpro_get_string_4(pgf);

			*out += ptr;

			free(ptr);
		}

		*out += "\n*/\n";
	}

	*out += "\nL0:\n";

	/* tempo */
	value = gpro_get_4(pgf);

	DPRINTF("tempo = %d\n", value);

	if (pgf->is_v4 == 0) {

		/* key */
		value = gpro_get_4(pgf);

		DPRINTF("key = %d\n", value);
	} else {
		/* key */
		value = gpro_get_1(pgf);

		DPRINTF("key = %d\n", value);

		/* octave */
		value = gpro_get_4(pgf);

		DPRINTF("octave = %d\n", value);
	}

	for (x = 0; x != (4 * 16); x++)
		gpro_get_midi_info(pgf);

	nmeas = gpro_get_4(pgf);	/* number of measures */
	ntrack = gpro_get_4(pgf);	/* number of tracks */

	DPRINTF("nmeas = %d, ntrack = %d\n", nmeas, ntrack);

	if (nmeas > (1024 * 1024))
		return;

	pgf->nmeas = nmeas;
	pgf->pmeas = (struct gpro_measure *)malloc(sizeof(pgf->pmeas[0]) * nmeas);

	if (pgf->pmeas == 0)
		return;

	/* compute ticks */

	for (acc_time = x = 0; x != nmeas; x++) {

		pgf->imeas = x;

		gpro_get_measure(pgf);

		pgf->pmeas[pgf->imeas].start_time = acc_time;

		if (pgf->dur_num == 0)
			pgf->dur_num = 1;

		if (pgf->dur_div == 0)
			pgf->dur_div = 1;

		acc_time += (((uint32_t)pgf->dur_num * GPRO_HZ) / (uint32_t)pgf->dur_div);
	}

	memset(pgf->capo, 0, sizeof(pgf->capo));
	memset(pgf->tuning, 0, sizeof(pgf->tuning));
	memset(pgf->string_max, 0, sizeof(pgf->string_max));

	for (x = 0; (x != ntrack) && (gpro_eof(pgf) == 0); x++) {

		pgf->track = x;

		gpro_get_track(pgf);
	}

	for (y = 0; y != nmeas; y++) {

		struct gpro_event *pev;

		pgf->imeas = y;

		for (x = 0; (x != ntrack) && (gpro_eof(pgf) == 0); x++) {

			pgf->track = x;
			pgf->ticks_sub = 0;

			nbeat = gpro_get_4(pgf);

			DPRINTF("nbeat = %d\n", nbeat);

			for (z = 0; (z != nbeat) && (gpro_eof(pgf) == 0); z++)
				gpro_get_beat(pgf);

			/* fixup all ringing strings */
			TAILQ_FOREACH(pev, &pgf->pmeas[pgf->imeas].head, entry) {
				if ((pev->dur == (uint32_t)-1) && (pev->chan == x)) {
					pev->dur = pgf->ticks_sub - 1;
				}
			}
		}
	}

	/* resolve repeat */

	acc_time = 0;

	for (y_repeat = y = 0; y < nmeas; y++) {

		if (pgf->pmeas[y].start_repeat || pgf->pmeas[y].end_repeat) {

			uint32_t n_repeat;

			if (pgf->pmeas[y].end_repeat) {
				/* end is inclusive */
				n_repeat = pgf->pmeas[y].end_repeat + 1U;
				y++;
			} else {
				/* start is inclusive */
				n_repeat = 1U;
			}

			value = (pgf->pmeas[y].start_time - pgf->pmeas[y_repeat].start_time);

			for (x = 0; x != n_repeat; x++) {
				for (z = y_repeat; z != y; z++) {
					gpro_copy_merge_head(&pgf->temp, &pgf->pmeas[z].head,
					    (x * value) + pgf->pmeas[z].start_time + acc_time);
				}
			}

			gpro_copy_merge_head(&pgf->head, &pgf->temp, 0);
			gpro_clean_events(&pgf->temp);

			acc_time += value * (n_repeat - 1);
			y_repeat = y;
		}
	}

	/* resolve the remainder of the measures */

	for (z = y_repeat; z < nmeas; z++) {
		gpro_copy_merge_head(&pgf->head, &pgf->pmeas[z].head,
		    pgf->pmeas[z].start_time + acc_time);
	}

	if (gpro_eof(pgf) == 0) {
		DPRINTF("Extra data at end of file\n");
	}
}

static void
gpro_cleanup(struct gpro_file *pgf)
{
	uint32_t x;

	gpro_clean_events(&pgf->head);
	gpro_clean_events(&pgf->temp);

	for (x = 0; x != pgf->nmeas; x++)
		gpro_clean_events(&pgf->pmeas[x].head);

	free(pgf->pmeas);

	for (x = 0; x != GPRO_MAX_TRACKS; x++) {
		free(pgf->track_str[x]);
		pgf->track_str[x] = NULL;
	}
}

MppGPro :: MppGPro(const uint8_t *ptr, uint32_t len)
	: QDialog()
{
	struct gpro_file gpf;
	char line_buf[64];
	uint32_t x;
	uint32_t y;
	uint32_t z;
	uint32_t t;
	uint32_t u;

	gpf.ptr = ptr;
	gpf.rem = len;

	gl = new QGridLayout(this);

	setWindowTitle(tr("GuitarPro v3 and v4 import"));
	setWindowIcon(QIcon(QString(MPP_ICON_FILE)));

	lbl_import[0] = new QLabel(tr("Select tracks\nto import"));
	lbl_import[0]->setAlignment(Qt::AlignCenter);

	lbl_import[1] = new QLabel(tr("Select tracks\nto import"));
	lbl_import[1]->setAlignment(Qt::AlignCenter);

	but_done = new QPushButton(tr("Done"));
	connect(but_done, SIGNAL(released()), this, SLOT(handle_done()));

	but_set_all = new QPushButton(tr("All tracks"));
	connect(but_set_all, SIGNAL(released()), this, SLOT(handle_set_all_track()));

	but_clear_all = new QPushButton(tr("No tracks"));
	connect(but_clear_all, SIGNAL(released()), this, SLOT(handle_clear_all_track()));

	y = 0;
	chan_mask = 0;

	gl->addWidget(lbl_import[0],y,1,1,1);
	gl->addWidget(lbl_import[1],y,3,1,1);

	y++;

	gpro_parse(&gpf, &output);

	for (z = x = 0; x != GPRO_MAX_TRACKS; x++) {
		if (gpf.track_str[x] != 0) {
			chan_mask |= (1 << x);
			z++;
		}
	}

	for (t = u = x = 0; x != GPRO_MAX_TRACKS; x++) {
		if (chan_mask & (1 << x)) {

			if (t >= ((z + 1) / 2)) {
				t = 0;
				u = 2;
			}

			snprintf(line_buf, sizeof(line_buf),
			    "Track%d: %s", (int)x, gpf.track_str[x]);

			cbx_import[x] = new MppCheckBox();

			lbl_info[x] = new QLabel(tr(line_buf));

			gl->addWidget(cbx_import[x],t+y,u+1,1,1,Qt::AlignHCenter|Qt::AlignVCenter);
			gl->addWidget(lbl_info[x],t+y,u+0,1,1,Qt::AlignLeft|Qt::AlignVCenter);

			t++;
		}
	}

	y += ((z + 1) / 2);

	cbx_single_track = new MppCheckBox();
	lbl_single_track = new QLabel(tr("Output like a single track"));

	gl->addWidget(cbx_single_track,y,3,1,1,Qt::AlignHCenter|Qt::AlignVCenter);
	gl->addWidget(lbl_single_track,y,0,1,3,Qt::AlignRight|Qt::AlignVCenter);

	y++;

	gl->addWidget(but_done,y,3,1,1);
	gl->addWidget(but_set_all,y,0,1,1);
	gl->addWidget(but_clear_all,y,1,1,1);

	exec();

	for (x = 0; x != GPRO_MAX_TRACKS; x++) {
		if (chan_mask & (1 << x)) {
			if (cbx_import[x]->isChecked() == 0) {
				chan_mask &= ~(1 << x);
			}
		}
	}

	/* only dump events if one or more tracks are selected */
	if ((gpf.chan_mask = chan_mask) != 0)
		gpro_dump_events(&gpf, output, cbx_single_track->isChecked());

	gpro_cleanup(&gpf);
}


MppGPro :: ~MppGPro()
{


}

void
MppGPro :: handle_done()
{
	accept();
}

void
MppGPro :: handle_set_all_track()
{
	uint32_t x;

	for (x = 0; x != GPRO_MAX_TRACKS; x++) {
		if (chan_mask & (1 << x)) {
			cbx_import[x]->setChecked(1);
		}
	}
}

void
MppGPro :: handle_clear_all_track()
{
	uint32_t x;

	for (x = 0; x != GPRO_MAX_TRACKS; x++) {
		if (chan_mask & (1 << x)) {
			cbx_import[x]->setChecked(0);
		}
	}
}
