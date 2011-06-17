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

/* GuitarPro format documentation: http://dguitar.sourceforge.net */

#include <midipp_gpro.h>

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <sys/queue.h>

struct gpro_event;
typedef TAILQ_ENTRY(gpro_event) gpro_event_entry_t;
typedef TAILQ_HEAD(,gpro_event) gpro_event_head_t;

#if 0
#define	DPRINTF(...) do { } while (0)
#else
#define	DPRINTF(fmt, ...) do { \
	printf("%s:%d: " fmt, __FUNCTION__, __LINE__,## __VA_ARGS__); \
} while (0)
#endif

struct gpro_file {
	const uint8_t *ptr;
	char *track_str[GPRO_MAX_TRACKS];
	uint32_t rem;
	uint32_t str_len;
	uint32_t ticks[GPRO_MAX_TRACKS];
	uint32_t capo[GPRO_MAX_TRACKS];
	uint32_t tuning[GPRO_MAX_TRACKS][8];
	gpro_event_head_t head;
	uint8_t track;
	uint8_t is_v4;
	uint8_t track_dump[GPRO_MAX_TRACKS];
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

	if (dur <= 10)
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

	TAILQ_FOREACH(pin, phead, entry) {
		if (pin->time > time)
			break;
	}

	if (pin == NULL)
		TAILQ_INSERT_TAIL(phead, pev, entry);
	else
		TAILQ_INSERT_BEFORE(pin, pev, entry);
}

static uint32_t
gpro_event_duration(struct gpro_file *pgf, struct gpro_event *pev)
{
  return (1);

	uint32_t tim = pev->time;
	uint32_t end = tim + pev->dur;
	uint32_t dur = 1;

	while ((pev = TAILQ_NEXT(pev, entry))) {

		if (pev->chan >= GPRO_MAX_TRACKS)
			continue;
		if (pgf->track_dump[pev->chan] == 0)
			continue;
		if (pev->time != tim) {
			if (pev->time >= end)
				break;

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
			gpro_get_4(pgf);

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
			gpro_get_4(pgf);

			for (x = 0; x != 6; x++) {
				/* read fret */
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

	DPRINTF("\n");

	hdr1 = gpro_get_1(pgf);
	hdr2 = gpro_get_1(pgf);

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

static void
gpro_get_note_effect_v3(struct gpro_file *pgf)
{
	uint8_t hdr;

	DPRINTF("\n");

	hdr = gpro_get_1(pgf);

	if (hdr & (1 << 0))
		gpro_get_bend(pgf);

	if (hdr & (1 << 4))
		gpro_get_grace(pgf);
}

static void
gpro_get_note_effect_v4(struct gpro_file *pgf)
{
	uint8_t hdr1;
	uint8_t hdr2;

	DPRINTF("\n");

	hdr1 = gpro_get_1(pgf);
	hdr2 = gpro_get_1(pgf);

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
}

static uint32_t
gpro_get_note(struct gpro_file *pgf)
{
	uint8_t hdr;
	uint8_t fret = 0;
	uint8_t dur = 0;

	hdr = gpro_get_1(pgf);

	DPRINTF("hdr = 0x%x\n", hdr);

	/* note type */
	if (hdr & (1 << 5))
		gpro_get_1(pgf);

	/* duration */
	if (hdr & (1 << 0)) {
		dur = gpro_get_1(pgf);
		DPRINTF("dur=%d\n", dur);
		gpro_get_1(pgf);
	}

	/* note dynamic */
	if (hdr & (1 << 4)) {
		gpro_get_1(pgf);
	}

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
			gpro_get_note_effect_v4(pgf);
		else
			gpro_get_note_effect_v3(pgf);
	}
	return (fret | (dur << 8));
}

static void
gpro_get_measure(struct gpro_file *pgf)
{
	uint8_t hdr;

	hdr = gpro_get_1(pgf);

	DPRINTF("hdr = 0x%x\n", hdr);

	/* numerator key signature */
	if (hdr & (1 << 0))
		gpro_get_1(pgf);

	/* denominator key signature */
	if (hdr & (1 << 1))
		gpro_get_1(pgf);

	/* end of repeat */
	if (hdr & (1 << 3))
		gpro_get_1(pgf);

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
	uint8_t hdr;
	uint8_t str;
	uint8_t n;
	uint8_t dur;

	hdr = gpro_get_1(pgf);	/* header */

	DPRINTF("hdr = 0x%x\n", hdr);

	if (hdr & 0x80)
		hdr &= 0x3f;

	/* beat status */
	if (hdr & (1 << 6))
		gpro_get_1(pgf);

	/* beat duration */
	dur = gpro_get_1(pgf);

	DPRINTF("dur = %d\n", dur);

	if (hdr & (1 << 5)) {
		/* N-tuplet */
		gpro_get_4(pgf);
	}

	if (hdr & (1 << 1)) {
		gpro_get_chord_diagram(pgf);
	}

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
	str = gpro_get_1(pgf);

	DPRINTF("str = 0x%x\n", str);

	for (n = 0; n != 7; n++) {

		uint32_t info;

		if (str & (1 << n)) {
			info = gpro_get_note(pgf);

			gpro_new_event(&pgf->head, pgf->ticks[pgf->track],
			    gpro_duration_to_ticks((info >> 8) & 0xFF),
			    (info & 0xFF) + pgf->capo[pgf->track] +
			    pgf->tuning[pgf->track][6 - n], pgf->track);
		}
	}

	pgf->ticks[pgf->track] += gpro_duration_to_ticks(dur);
}

static void
gpro_get_midi_info(struct gpro_file *pgf)
{
	gpro_get_4(pgf);	/* instrument */
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
gpro_dump_events(struct gpro_file *pgf)
{
	struct gpro_event *pev;
	uint32_t chan_last = 0;
	uint32_t dur_last = -1U;
	uint32_t dur;
	uint32_t time_last;

	pev = TAILQ_FIRST(&pgf->head);
	if (pev != 0)
		time_last = ~pev->time;
	else
		time_last = 0;

	TAILQ_FOREACH(pev, &pgf->head, entry) {

		if (pev->chan >= GPRO_MAX_TRACKS)
			continue;
		if (pgf->track_dump[pev->chan] == 0)
			continue;

		if (pev->time != time_last) {
			time_last = pev->time;
			chan_last = 0;
			dur_last = -1U;
			printf("\n");
		}

		if (pev->chan != chan_last) {
			chan_last = pev->chan;
			printf("T%u ", chan_last);
		}

		dur = gpro_event_duration(pgf, pev);

		if (dur != dur_last) {
			dur_last = dur;
			printf("U%u ", dur_last);
		}

		printf("%s ", mid_key_str[pev->key & 0x7F]);
	}

	printf("\n");
}

static void
gpro_parse(struct gpro_file *pgf)
{
	char *ptr;
	uint32_t nmeas;
	uint32_t ntrack;
	uint32_t value;
	uint32_t nbeat;
	uint32_t nscore;
	uint32_t x;
	uint32_t y;
	uint32_t z;

	TAILQ_INIT(&pgf->head);

	memset(pgf->track_str, 0, sizeof(pgf->track_str));
	memset(pgf->track_dump, 0, sizeof(pgf->track_dump));

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
	free(ptr);

	/* subtitle */
	ptr = gpro_get_string_4_wrap(pgf);
	free(ptr);

	/* interpret */
	ptr = gpro_get_string_4_wrap(pgf);
	free(ptr);

	/* album */
	ptr = gpro_get_string_4_wrap(pgf);
	free(ptr);

	/* author of the song */
	ptr = gpro_get_string_4_wrap(pgf);
	free(ptr);

	/* copyright */
	ptr = gpro_get_string_4_wrap(pgf);
	free(ptr);

	/* author of the tablature */
	ptr = gpro_get_string_4_wrap(pgf);
	free(ptr);

	/* instructional */
	ptr = gpro_get_string_4_wrap(pgf);
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
		gpro_get_4(pgf);

		for (x = 0; x != 5; x++) {
			gpro_get_4(pgf);
			ptr = gpro_get_string_4(pgf);
			free(ptr);
		}
	}

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

	for (x = 0; (x != nmeas) && (gpro_eof(pgf) == 0); x++) {
		gpro_get_measure(pgf);
	}

	memset(pgf->capo, 0, sizeof(pgf->capo));
	memset(pgf->tuning, 0, sizeof(pgf->tuning));

	for (x = 0; (x != ntrack) && (gpro_eof(pgf) == 0); x++) {

		pgf->track = x;

		gpro_get_track(pgf);
	}

	memset(pgf->ticks, 0, sizeof(pgf->ticks));
	
	for (y = 0; (y != nmeas) && (gpro_eof(pgf) == 0); y++) {

		for (x = 0; (x != ntrack) && (gpro_eof(pgf) == 0); x++) {

			pgf->track = x;

			nbeat = gpro_get_4(pgf);

			DPRINTF("nbeat = %d\n", nbeat);

			for (z = 0; (z != nbeat) && (gpro_eof(pgf) == 0); z++) {
				gpro_get_beat(pgf);
			}
		}
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

	gpf.ptr = ptr;
	gpf.rem = len;

	gl = new QGridLayout(this);

	setWindowTitle(tr("GuitarPro v3 and v4 import"));

	lbl_import = new QLabel(tr("Select tracks\nto import"));
	but_done = new QPushButton(tr("Done"));
	connect(but_done, SIGNAL(released()), this, SLOT(handle_done()));

	gl->addWidget(lbl_import,0,1,1,1);

	gpro_parse(&gpf);

	for (x = y = 0; x != GPRO_MAX_TRACKS; x++) {
		if (gpf.track_str[x] != 0) {
			snprintf(line_buf, sizeof(line_buf),
			    "Track%d: %s", (int)x, gpf.track_str[x]);

			cbx_import[x] = new QCheckBox();
			cbx_import[x]->setChecked(1);

			lbl_info[x] = new QLabel(tr(line_buf));

			gl->addWidget(cbx_import[x],y+1,1,1,1,Qt::AlignHCenter|Qt::AlignVCenter);
			gl->addWidget(lbl_info[x],y+1,0,1,1);

			y++;
		}
	}

	gl->addWidget(but_done,y+1,1,1,1);

	exec();

	for (x = y = 0; x != GPRO_MAX_TRACKS; x++) {
		if (gpf.track_str[x] != 0) {
			if (cbx_import[x]->isChecked()) {
				gpf.track_dump[x] = 1;
			}
		}
	}

	gpro_dump_events(&gpf);

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
