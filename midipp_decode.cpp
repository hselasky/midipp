/*-
 * Copyright (c) 2010-2019 Hans Petter Selasky. All rights reserved.
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

#include "midipp_buttonmap.h"
#include "midipp_chords.h"
#include "midipp_mainwindow.h"
#include "midipp_scores.h"
#include "midipp_button.h"
#include "midipp_element.h"
#include "midipp_groupbox.h"
#include "midipp_instrument.h"

Q_DECL_EXPORT const QString
MppKeyStr(int key)
{
	int rem = MPP_BAND_REM(key, MPP_MAX_BANDS);
	int off;
	int sub;
	int oct;

	off = rem / MPP_BAND_STEP_12;
	sub = rem % MPP_BAND_STEP_12;
	oct = (key - rem) / MPP_MAX_BANDS;

	if (sub != 0) {
		const char *off_map[12] = {
			"C%1.%2", "D%1B.%2", "D%1.%2", "E%1B.%2",
			"E%1.%2", "F%1.%2", "G%1B.%2", "G%1.%2",
			"A%1B.%2", "A%1.%2", "H%1B.%2", "H%1.%2"
		};
		sub = MPP_SUBDIV_REM_BITREV(sub);
		return (QString(off_map[off]).arg(oct).arg(sub));
	} else {
	  	const char *off_map[12] = {
			"C%1", "D%1B", "D%1", "E%1B",
			"E%1", "F%1", "G%1B", "G%1",
			"A%1B", "A%1", "H%1B", "H%1"
		};
		return (QString(off_map[off]).arg(oct));
	}
}

Q_DECL_EXPORT const QString
MppKeyStrNoOctave(int key)
{
	int rem = MPP_BAND_REM(key, MPP_MAX_BANDS);
	int off;
	int sub;
	int oct;

	off = rem / MPP_BAND_STEP_12;
	sub = rem % MPP_BAND_STEP_12;
	oct = (key - rem) / MPP_MAX_BANDS;

	if (sub != 0) {
		const char *off_map[12] = {
			"C.%1", "Db.%1", "D.%1", "Eb.%1",
			"E.%1", "F.%1", "Gb.%1", "G.%1",
			"Ab.%1", "A.%1", "Hb.%1", "H.%1"
		};
		sub = MPP_SUBDIV_REM_BITREV(sub);
		return (QString(off_map[off]).arg(sub));
	} else {
	  	const char *off_map[12] = {
			"C", "Db", "D", "Eb",
			"E", "F", "Gb", "G",
			"Ab", "A", "Hb", "H"
		};
		return (QString(off_map[off]));
	}
}

Q_DECL_EXPORT const QString
MppBitsToString(const MppChord_t &mask, int off)
{
	QString temp;
	int x;

	for (x = 0; x != MPP_MAX_CHORD_BANDS; x++) {
		if (mask.test(x) == 0)
			continue;
		temp += MppKeyStrNoOctave((x * MPP_BAND_STEP_CHORD) + off);
		temp += " ";
	}
	return (temp);
}

static void
MppFindOptimalMajorViaBinarySearch(int &adjust0, int &adjust1)
{
	int step = MPP_BAND_STEP_12;
	int freq = 7 * MPP_BAND_STEP_12;

	while (step != 0) {
		int p_nor;
		int p_add;
		int p_sub;

		(void) MppFreqAdjust(freq / (double)MPP_MAX_BANDS, &p_nor);
		(void) MppFreqAdjust((freq + step) / (double)MPP_MAX_BANDS, &p_add);
		(void) MppFreqAdjust((freq - step) / (double)MPP_MAX_BANDS, &p_sub);

		int d_nor = p_nor % MPP_MAX_BANDS;
		int d_add = p_add % MPP_MAX_BANDS;
		int d_sub = p_sub % MPP_MAX_BANDS;

		int adjust;

		if (d_add < -(MPP_MAX_BANDS / 2))
			d_add += MPP_MAX_BANDS;
		else if (d_add > (MPP_MAX_BANDS / 2))
			d_add -= MPP_MAX_BANDS;

		if (d_add < 0)
			d_add = -d_add;

		if (d_sub < -(MPP_MAX_BANDS / 2))
			d_sub += MPP_MAX_BANDS;
		else if (d_sub > (MPP_MAX_BANDS / 2))
			d_sub -= MPP_MAX_BANDS;

		if (d_sub < 0)
			d_sub = -d_sub;
	  
		if (d_nor < -(MPP_MAX_BANDS / 2))
			d_nor += MPP_MAX_BANDS;
		else if (d_nor > (MPP_MAX_BANDS / 2))
			d_nor -= MPP_MAX_BANDS;

		if (d_nor < 0)
			d_nor = -d_nor;

		adjust = 0;
		if (d_add < d_nor) {
			adjust = step;
			d_nor = d_add;
		}
		if (d_sub < d_nor) {
			adjust = -step;
			d_nor = d_sub;
		}
		freq += adjust;
		step /= 2;
	}

	adjust0 = MppFreqAdjust(freq / (double)MPP_MAX_BANDS, 0) - 4 * MPP_BAND_STEP_12;
	adjust1 = freq - 7 * MPP_BAND_STEP_12;
}

void
MppScoreVariantInit(void)
{
	const int rk = 0;
	MppChord_t mask;
	QString str;
	uint32_t x;
	uint32_t y;
	uint32_t z;

	Mpp.VariantList += QString("\n/* Unique chords having two keys */\n\n");

	for (z = 0, x = MPP_BAND_STEP_12; x != MPP_MAX_BANDS; x += MPP_BAND_STEP_12) {
		mask.zero();
		mask.set(0);
		mask.set(x / MPP_BAND_STEP_CHORD);

		if (MppFindChordRoot(mask) != mask)
			continue;

		MppChordToStringGeneric(mask, rk, rk, 0, MPP_BAND_STEP_12, str);
		if (str.isEmpty())
			str = "  ";
		else
			z++;

		Mpp.VariantList += str;
		Mpp.VariantList += " = ";
		Mpp.VariantList += MppBitsToString(mask, rk);
		Mpp.VariantList += "\n";
	}

	Mpp.VariantList += QString("\n/* Total number of variants is %1 */\n\n").arg(z);
	Mpp.VariantList += QString("/* Unique chords having three keys */\n\n");

	for (z = 0, x = MPP_BAND_STEP_12; x != MPP_MAX_BANDS; x += MPP_BAND_STEP_12) {
		for (y = x + MPP_BAND_STEP_12; y != MPP_MAX_BANDS; y += MPP_BAND_STEP_12) {
			mask.zero();
			mask.set(0);
			mask.set(x / MPP_BAND_STEP_CHORD);
			mask.set(y / MPP_BAND_STEP_CHORD);

			if (MppFindChordRoot(mask) != mask)
				continue;

			MppChordToStringGeneric(mask, rk, rk, 0, MPP_BAND_STEP_12, str);
			if (str.isEmpty())
				str = "   ";
			else
				z++;
			Mpp.VariantList += str;
			Mpp.VariantList += " = ";
			Mpp.VariantList += MppBitsToString(mask, rk);
			Mpp.VariantList += "\n";
		}
	}

	Mpp.VariantList += QString("\n/* Total number of variants is %1 */\n\n").arg(z);

	Mpp.VariantList += QString("/* List of supported keys */\n\n");
	
	for (x = 0; x != MPP_MAX_CHORD_BANDS; x++) {
		Mpp.VariantList += MppKeyStr(x * MPP_BAND_STEP_CHORD);
		Mpp.VariantList += QString(" /* %1 of %2 */\n").arg(x + 1)
		    .arg(MPP_MAX_CHORD_BANDS);
	}

	Mpp.VariantList += "\n/* List of harmonic triads with phase key */\n\n";

	for (x = 2; x != MPP_MAX_CHORD_BANDS; x++) {
		int phase;
		int u = MppFreqAdjust((double)x / (double)MPP_MAX_CHORD_BANDS, &phase);
		int v = u;

		v += MPP_BAND_STEP_CHORD / 2;
		v -= v % MPP_BAND_STEP_CHORD;

		phase += MPP_BAND_STEP_CHORD / 2;
		phase -= phase % MPP_BAND_STEP_CHORD;

		Mpp.VariantList += MppKeyStr(
		    MPP_MAX_BANDS * 5 + 0 * MPP_BAND_STEP_CHORD);
		Mpp.VariantList += " ";
		Mpp.VariantList += MppKeyStr(
		    MPP_MAX_BANDS * 5 + v);
		Mpp.VariantList += " ";
		Mpp.VariantList += MppKeyStr(
		    MPP_MAX_BANDS * 5 + x * MPP_BAND_STEP_CHORD);
		Mpp.VariantList += " ";
		Mpp.VariantList += MppKeyStr(
		    MPP_MAX_BANDS * 5 + phase);
		Mpp.VariantList += "\n";
	}

	Mpp.VariantList += "\n/* Harmonic major with key C */\n\n";

	MppFindOptimalMajorViaBinarySearch(Mpp.MajorAdjust[0], Mpp.MajorAdjust[1]);

	Mpp.VariantList += MppKeyStr(MPP_MAX_BANDS * 5);
	Mpp.VariantList += " ";
	Mpp.VariantList += MppKeyStr(MPP_MAX_BANDS * 5 + Mpp.MajorAdjust[0] + MPP_BAND_STEP_12 * 4);
	Mpp.VariantList += " ";
	Mpp.VariantList += MppKeyStr(MPP_MAX_BANDS * 5 + Mpp.MajorAdjust[1] + MPP_BAND_STEP_12 * 7);
	Mpp.VariantList += "\n";
}

static void
MppInitArray(const uint8_t *src, MppChord_t *dst, uint8_t n)
{
	while (n--) {
		dst[n].zero();
		for (uint8_t x = 0; x != 12; x++) {
			if ((src[n] >> x) & 1)
				dst[n].set((MPP_BAND_STEP_12 / MPP_BAND_STEP_CHORD) * x);
		}
	}
}

static int
MppCommonKeys(const MppChord_t & pa, const MppChord_t & pb, int a_key, int b_key)
{
	MppChord_t temp = pb;
	int key = ((12 + a_key - b_key) % 12) *
	    (MPP_BAND_STEP_12 / MPP_BAND_STEP_CHORD);

	while (key--) {
		if (temp.test(0)) {
			temp.tog(0);
			temp.tog(MPP_MAX_CHORD_BANDS);
		}
		temp.shr();
	}

	temp &= pa;
	return (temp.order());
}

MppDecodeCircle :: MppDecodeCircle(MppDecodeTab *_ptab)
{
	static const uint8_t circle[NMAX] = { 0x91, 0x89 };
	ptab = _ptab;

	MppInitArray(circle, mask, NMAX);

	memset(r_press, 0, sizeof(r_press));
	memset(r_key, 0, sizeof(r_key));
	memset(r_mask, 0, sizeof(r_mask));

	setMinimumSize(128,128);
}

MppDecodeCircle :: ~MppDecodeCircle()
{
}

void
MppDecodeCircle :: mousePressEvent(QMouseEvent *event)
{
	QPoint p = event->pos();

	for (uint8_t x = 0; x != NMAX; x++) {
		for (uint8_t y = 0; y != 12; y++) {
			if (r_press[x][y].contains(p) == 0)
				continue;
			ptab->chord_key -= ptab->chord_key % MPP_MAX_BANDS;
			ptab->chord_key += MPP_BAND_STEP_12 * r_key[x][y];
			ptab->chord_bass = ptab->chord_key % MPP_MAX_BANDS;
			ptab->chord_mask = r_mask[x][y];
			ptab->chord_step = MPP_BAND_STEP_12;
			ptab->handle_refresh();
			ptab->handle_play_press();
			return;
		}
	}
}

void
MppDecodeCircle :: mouseReleaseEvent(QMouseEvent *event)
{
	QPoint p = event->pos();

	for (uint8_t x = 0; x != NMAX; x++) {
		for (uint8_t y = 0; y != 12; y++) {
			if (r_press[x][y].contains(p) == 0)
				continue;
			ptab->handle_play_release();
		}
	}
}

void
MppDecodeCircle :: paintEvent(QPaintEvent *event)
{
	QPainter paint(this);
	int w = width();	
	int h = height();
	int r;
	qreal step;
	uint8_t x;
	uint8_t y;
	uint8_t z;
	int found_key;
	uint32_t rols;
	MppChord_t footprint;

	paint.fillRect(QRectF(0,0,w,h), Mpp.ColorWhite);
	paint.setRenderHints(QPainter::Antialiasing, 1);

	memset(r_press, 0, sizeof(r_press));
	memset(r_key, 0, sizeof(r_key));
	memset(r_mask, 0, sizeof(r_mask));

	footprint = MppFindChordRoot(ptab->chord_mask, &rols);
	found_key = (ptab->chord_key +
	    (rols * MPP_BAND_STEP_CHORD)) % MPP_MAX_BANDS;
	found_key /= MPP_BAND_STEP_12;

	r = (w > h) ? h : w;
	step = r / (2 * (NMAX + 2));

	QFont fnt[2];

	fnt[0].setPixelSize(step / 1.5);
	fnt[1].setPixelSize(step / 2.5);

	for (x = 0; x != NMAX; x++) {
		qreal radius = (NMAX + 2 - x) * step - step / 2.0;

		paint.setPen(QPen(Mpp.ColorBlack, 2));
		paint.setBrush(QBrush());

		paint.drawEllipse(QRectF(w / 2.0 - radius,
					 h / 2.0 - radius,
					 2.0*radius, 2.0*radius));

		for (y = z = 0; y != 12; y++) {
			double phase = 2.0 * M_PI * (double)y / 12.0;
			qreal xp = radius * cos(phase) + w / 2.0;
			qreal yp = radius * sin(phase) + h / 2.0;

			r_press[x][y] = QRect(xp - step / 2.0,
					      yp - step / 2.0,
					      step, step);
			r_key[x][y] = (x == 1) ? (z + 9) % 12 : z;
			r_mask[x][y] = mask[x];

			z += 7;
			z %= 12;
		}
	}

	for (x = 0; x != NMAX; x++) {
		paint.setFont(fnt[x == 1]);

		for (y = 0; y != 12; y++) {
			QColor bg;
			QColor fg;

			switch (MppCommonKeys(r_mask[x][y], footprint,
					      r_key[x][y], found_key)) {
			case 3:
				bg = Mpp.ColorWhite;
				fg = Mpp.ColorBlack;
				break;
			case 2:
				bg = Mpp.ColorGrey;
				fg = Mpp.ColorWhite;
				break;
			case 1:
				bg = Mpp.ColorLight;
				fg = Mpp.ColorWhite;
				break;
			default:
				bg = Mpp.ColorBlack;
				fg = Mpp.ColorWhite;
				break;
			}
			paint.setPen(QPen(bg, 0));
			paint.setBrush(bg);

			paint.drawEllipse(r_press[x][y]);

			paint.setPen(QPen(fg, 0));
			paint.setBrush(fg);

			paint.drawText(r_press[x][y],
			    Qt::AlignCenter | Qt::TextSingleLine,
			    MppKeyStrNoOctave(r_key[x][y] * MPP_BAND_STEP_12) +
			    ((x == 1) ? "m" : ""));
		}
	}
}

uint8_t
MppDecodeTab :: parseScoreChord(MppChordElement *pinfo)
{
	QString out;
	uint32_t is_sharp;
	uint32_t any = 0;
	uint32_t key = 0;
	uint32_t bass;
	MppChord_t footprint;

	if (pinfo->chord != 0)
		is_sharp = (pinfo->chord->txt.indexOf('#') > -1);
	else
		is_sharp = 0;

	footprint.zero();

	for (int x = 0; x != MPP_MAX_CHORD_BANDS; x++) {
		if (pinfo->stats[x] == 0)
			continue;
		footprint.set(x);
		any = 1;
	}
	if (!any)
		return (1);	/* not found */

	while (footprint.test(0) == 0) {
		footprint.shr();
		key += MPP_BAND_STEP_CHORD;
	}

	bass = MPP_BAND_REM(pinfo->key_base, MPP_MAX_BANDS);

	MppChordToStringGeneric(footprint, key, bass, is_sharp, MPP_BAND_STEP_CHORD, out);

	if (out.isEmpty())
		return (1);	/* not found */

	chord_key = key;
	chord_bass = bass;
	chord_sharp = is_sharp;
	chord_mask = footprint;

	lin_edit->blockSignals(1);
	lin_edit->setText(out);
	lin_edit->blockSignals(0);

	handle_align(pinfo->key_max + 1);
	handle_refresh();

	return (0);
}

MppDecodeTab :: MppDecodeTab(MppMainWindow *_mw)
  : QWidget()
{

	chord_key = 5 * 12 * MPP_BAND_STEP_12; /* C5 */
	chord_bass = 0;	/* C */
	chord_sharp = 0;
	chord_step = MPP_BAND_STEP_12;
	chord_mask.zero();
	chord_mask.set(0);
	chord_mask.set((MPP_BAND_STEP_12 / MPP_BAND_STEP_CHORD) * 4);
	chord_mask.set((MPP_BAND_STEP_12 / MPP_BAND_STEP_CHORD) * 7);
	delta_v = 0;

	mw = _mw;

	gl = new QGridLayout(this);

	gb = new MppGroupBox(tr("Chord Selector"));
	gl->addWidget(gb, 0,0,1,1);
	gl->setRowStretch(2,1);
	gl->setColumnStretch(1,1);

	lin_edit = new QLineEdit(QString("C"));
	lin_edit->setMaxLength(256);

	lin_out = new QLineEdit();
	lin_out->setMaxLength(256);

	but_map_volume = new MppButtonMap("Key volume\0"
					  "MAX\0"
					  "63\0"
					  "31\0"
					  "15\0"
					  "7\0", 5, 5);
	but_map_volume->setSelection(4);

	but_map_view = new MppButtonMap("View selection\0"
					"View-A\0"
					"View-B\0", 2, 2);

#if MPP_MAX_VIEWS != 2
#error "Please update code above"
#endif
	but_insert = new QPushButton(tr("&Insert"));
	but_rol_up = new QPushButton(tr("Rotate\n&up"));
	but_rol_down = new QPushButton(tr("Rotate\nd&own"));
	but_mod_up = new QPushButton(tr("&Next\nvariant"));
	but_mod_down = new QPushButton(tr("Pre&vious\nvariant"));
	but_step_up_half = new QPushButton(tr("Step up\n12 scale"));
	but_step_down_half = new QPushButton(tr("Step down\n12 scale"));
	but_step_up_one = new QPushButton(tr("Step up\n192 scale"));
	but_step_down_one = new QPushButton(tr("Step down\n192 scale"));
	but_round_12 = new QPushButton(tr("Round to 12 scale"));
	but_play = new QPushButton(tr("&Play"));

	connect(but_play, SIGNAL(pressed()), this, SLOT(handle_play_press()));
	connect(but_play, SIGNAL(released()), this, SLOT(handle_play_release()));

	connect(but_insert, SIGNAL(released()), this, SLOT(handle_insert()));

	connect(but_rol_up, SIGNAL(pressed()), this, SLOT(handle_rol_up()));
	connect(but_rol_up, SIGNAL(released()), this, SLOT(handle_play_release()));
	connect(but_rol_down, SIGNAL(pressed()), this, SLOT(handle_rol_down()));
	connect(but_rol_down, SIGNAL(released()), this, SLOT(handle_play_release()));

	connect(but_mod_up, SIGNAL(pressed()), this, SLOT(handle_mod_up()));
	connect(but_mod_up, SIGNAL(released()), this, SLOT(handle_play_release()));
	connect(but_mod_down, SIGNAL(pressed()), this, SLOT(handle_mod_down()));
	connect(but_mod_down, SIGNAL(released()), this, SLOT(handle_play_release()));

	connect(but_step_up_half, SIGNAL(pressed()), this, SLOT(handle_step_up_half()));
	connect(but_step_up_half, SIGNAL(released()), this, SLOT(handle_play_release()));
	connect(but_step_down_half, SIGNAL(pressed()), this, SLOT(handle_step_down_half()));
	connect(but_step_down_half, SIGNAL(released()), this, SLOT(handle_play_release()));

	connect(but_step_up_one, SIGNAL(pressed()), this, SLOT(handle_step_up_one()));
	connect(but_step_up_one, SIGNAL(released()), this, SLOT(handle_play_release()));
	connect(but_step_down_one, SIGNAL(pressed()), this, SLOT(handle_step_down_one()));
	connect(but_step_down_one, SIGNAL(released()), this, SLOT(handle_play_release()));
	connect(but_round_12, SIGNAL(released()), this, SLOT(handle_round_12()));

	connect(lin_edit, SIGNAL(textChanged(const QString &)), this, SLOT(handle_parse()));

	gb->addWidget(
	    new QLabel(tr("[CDEFGABH][#b][...][/CDEFGABH[#b]]")),
	    0,0,1,4, Qt::AlignHCenter|Qt::AlignVCenter);

	gb->addWidget(but_rol_down, 2,0,1,1);
	gb->addWidget(but_rol_up, 2,1,1,1);

	gb->addWidget(but_mod_down, 2,2,1,1);
	gb->addWidget(but_mod_up, 2,3,1,1);

	gb->addWidget(but_step_down_half, 3,0,1,1);
	gb->addWidget(but_step_up_half, 3,1,1,1);
	gb->addWidget(but_step_down_one, 3,2,1,1);
	gb->addWidget(but_step_up_one, 3,3,1,1);

	gb->addWidget(but_round_12, 4,0,1,2);

	gb->addWidget(lin_edit, 5,0,1,4);
	gb->addWidget(lin_out, 6,0,1,4);

	gb->addWidget(but_map_volume, 7,0,1,4);
	gb->addWidget(but_map_view, 8,0,1,4);

	gb->addWidget(but_insert, 9, 2, 1, 2);
	gb->addWidget(but_play, 9, 0, 1, 2);

	gb_gen = new MppGroupBox(tr("Chord Scratch Area"));
	gl->addWidget(gb_gen, 1,0,1,2);

	editor = new MppDecodeEditor(_mw);
	gb_gen->addWidget(editor, 0,0,1,1);

	gb_dc = new MppGroupBox(tr("Circle of fifths"));
	wi_dc = new MppDecodeCircle(this);
	gb_dc->addWidget(wi_dc, 0,0,1,1);

	gl->addWidget(gb_dc, 0,1,1,1);
	
	handle_parse();
    
	but_insert->setFocus();
}

MppDecodeTab :: ~MppDecodeTab()
{

}

void
MppDecodeTab :: handle_play_press()
{
	MppScoreMain *sm = mw->scores_main[but_map_view->currSelection];
	uint8_t vel = (127 >> but_map_volume->currSelection);
	int key_bass;

	mw->atomic_lock();
	key_bass = chord_key - MPP_BAND_REM(chord_key, MPP_MAX_BANDS) + chord_bass;
	if (key_bass >= chord_key)
		key_bass -= 2 * MPP_MAX_BANDS;
	else
		key_bass -= 1 * MPP_MAX_BANDS;

	mw->output_key(MPP_DEFAULT_TRACK(sm->unit), sm->synthChannel, key_bass, vel, 0, 0);
	mw->output_key(MPP_DEFAULT_TRACK(sm->unit), sm->synthChannel, key_bass + MPP_MAX_BANDS, vel, 0, 0);

	for (int x = 0; x != MPP_MAX_CHORD_BANDS; x++) {
		if (chord_mask.test(x)) {
			mw->output_key(MPP_DEFAULT_TRACK(sm->unit),
			sm->synthChannel, chord_key + (x * MPP_BAND_STEP_CHORD), vel, 0, 0);
		}
	}
	mw->atomic_unlock();
}

void
MppDecodeTab :: handle_play_release()
{
  	MppScoreMain *sm = mw->scores_main[but_map_view->currSelection];
	int key_bass;

	mw->atomic_lock();
	key_bass = chord_key - MPP_BAND_REM(chord_key, MPP_MAX_BANDS) + chord_bass;
	if (key_bass >= chord_key)
		key_bass -= 2 * MPP_MAX_BANDS;
	else
		key_bass -= 1 * MPP_MAX_BANDS;

	mw->output_key(MPP_DEFAULT_TRACK(sm->unit), sm->synthChannel, key_bass, 0, 0, 0);
	mw->output_key(MPP_DEFAULT_TRACK(sm->unit), sm->synthChannel, key_bass + MPP_MAX_BANDS, 0, 0, 0);

	for (int x = 0; x != MPP_MAX_CHORD_BANDS; x++) {
		if (chord_mask.test(x)) {
			mw->output_key(MPP_DEFAULT_TRACK(sm->unit),
			    sm->synthChannel, chord_key + (x * MPP_BAND_STEP_CHORD), 0, 0, 0);
		}
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
				ptr->type = MPP_T_UNKNOWN;
				ptr->txt = "";
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
MppDecodeTab :: handle_align(int key)
{
	if (key < (2 * MPP_MAX_BANDS) ||
	    key > (128 * MPP_BAND_STEP_12 - MPP_MAX_BANDS))
		return;

	chord_key = key - MPP_BAND_REM(key, MPP_MAX_BANDS) +
	    MPP_MAX_BANDS + MPP_BAND_REM(chord_key, MPP_MAX_BANDS);

	for (int x = MPP_MAX_CHORD_BANDS; x--; ) {
		if (chord_mask.test(x) == 0)
			continue;
		if ((chord_key + (x * MPP_BAND_STEP_CHORD)) < key)
			break;
		int rols = 0;
		MppRolUpChord(chord_mask, rols);
		chord_key -= rols * MPP_BAND_STEP_CHORD;
		x = MPP_MAX_CHORD_BANDS;
	}
}

void
MppDecodeTab :: handle_rol_up()
{
	int rols;

	if (chord_key > (128 * MPP_BAND_STEP_12 - MPP_MAX_BANDS))
		return;
	handle_play_release();
	rols = 0;
	MppRolDownChord(chord_mask, rols);
	chord_key += rols * MPP_BAND_STEP_CHORD;
	handle_refresh();
	handle_play_press();
}

void
MppDecodeTab :: handle_rol_down()
{
	int rols;

	if (chord_key < 2 * MPP_MAX_BANDS)
		return;
	handle_play_release();
	rols = 0;
	MppRolUpChord(chord_mask, rols);
	chord_key -= rols * MPP_BAND_STEP_CHORD;
	handle_refresh();
	handle_play_press();
}

void
MppDecodeTab :: handle_mod_up()
{
	uint32_t rols;
	handle_play_release();
	chord_mask = MppFindChordRoot(chord_mask, &rols);
	chord_key += rols * MPP_BAND_STEP_CHORD;
	MppNextChordRoot(chord_mask,
	    chord_step / MPP_BAND_STEP_CHORD);
	handle_refresh();
	handle_play_press();
}

void
MppDecodeTab :: handle_mod_down()
{
  	uint32_t rols;
	handle_play_release();
	chord_mask = MppFindChordRoot(chord_mask, &rols);
	chord_key += rols * MPP_BAND_STEP_CHORD;
	MppPrevChordRoot(chord_mask,
	    chord_step / MPP_BAND_STEP_CHORD);
	handle_refresh();
	handle_play_press();
}

void
MppDecodeTab :: handle_step_up_half()
{
	handle_play_release();
	if (chord_key < (128 * MPP_BAND_STEP_12)) {
		chord_key += MPP_BAND_STEP_12;
		chord_bass += MPP_BAND_STEP_12;
		chord_bass %= MPP_MAX_BANDS;
		handle_refresh();
	}
	handle_play_press();
}

void
MppDecodeTab :: handle_step_down_half()
{
	handle_play_release();
	if (chord_key >= (25 * MPP_BAND_STEP_12)) {
		chord_key -= MPP_BAND_STEP_12;
		chord_bass += 11 * MPP_BAND_STEP_12;
		chord_bass %= MPP_MAX_BANDS;
		handle_refresh();
	}
	handle_play_press();
}

void
MppDecodeTab :: handle_step_up_one()
{
	handle_play_release();
	if (chord_key < (128 * MPP_BAND_STEP_12)) {
		chord_key += MPP_BAND_STEP_CHORD;
		chord_bass += MPP_BAND_STEP_CHORD;
		chord_bass %= MPP_MAX_BANDS;
		handle_refresh();
	}
	handle_play_press();
}

void
MppDecodeTab :: handle_step_down_one()
{
	handle_play_release();
	if (chord_key >= (24 * MPP_BAND_STEP_12 + 1)) {
		chord_key -= MPP_BAND_STEP_CHORD;
		chord_bass += MPP_MAX_BANDS - MPP_BAND_STEP_CHORD;
		chord_bass %= MPP_MAX_BANDS;
		handle_refresh();
	}
	handle_play_press();
}

void
MppDecodeTab :: handle_round_12()
{
	round(MPP_BAND_STEP_12);
}

void
MppDecodeTab :: round(int value)
{
	int x,y,mod;

	if (value < 0)
		return;

	mod = (value / MPP_BAND_STEP_CHORD);

	chord_step = value;

	chord_key += value / 2;
	chord_key -= chord_key % value;

	chord_bass += value / 2;
	chord_bass -= chord_bass % value;
	chord_bass %= MPP_MAX_BANDS;

	if (mod > 0) {
		for (x = 0; x != MPP_MAX_CHORD_BANDS; x++) {
			if (chord_mask.test(x) == 0 || (x % mod) == 0)
				continue;

			y = x + (mod / 2);
			y -= (y % mod);
			y %= MPP_MAX_CHORD_BANDS;

			chord_mask.clr(x);
			chord_mask.set(y);
		}
	}
	handle_refresh();
}

void
MppDecodeTab :: handle_refresh()
{
	QString out_key;
	QString str;
	int key_bass;

	key_bass = chord_key -
	    MPP_BAND_REM(chord_key, MPP_MAX_BANDS) +
	    chord_bass;

	if (key_bass >= chord_key)
		key_bass -= 2 * MPP_MAX_BANDS;
	else
		key_bass -= 1 * MPP_MAX_BANDS;

	MppChordToStringGeneric(chord_mask, chord_key, key_bass,
	    chord_sharp, MPP_BAND_STEP_CHORD, str);

	out_key += MppKeyStr(key_bass);
	out_key += " ";
	out_key += MppKeyStr(key_bass + MPP_MAX_BANDS);
	out_key += " ";
	
	for (int x = 0; x != MPP_MAX_CHORD_BANDS; x++) {
		if (chord_mask.test(x) == 0)
			continue;
		out_key += MppKeyStr((x * MPP_BAND_STEP_CHORD) + chord_key);
		out_key += " ";
	}

	if (str.isEmpty() == 0) {
		out_key += "/* ";
		out_key += str;
		out_key += " */";
	}

	lin_edit->blockSignals(1);
	lin_edit->setText(str);
	lin_edit->blockSignals(0);
	lin_out->blockSignals(1);
	lin_out->setText(out_key);
	lin_out->blockSignals(0);

	wi_dc->update();
}

void
MppDecodeTab :: handle_parse()
{
	QString out_key;
	MppChord_t mask;
	int key_bass;
	uint32_t rem;
	uint32_t bass;

	QString chord = lin_edit->text().trimmed();
	if (chord.isEmpty())
		goto error;

	MppStringToChordGeneric(mask, rem, bass, 1, chord);
	chord_sharp = (chord.indexOf('#') > -1);

	if (mask.test(0) == 0)
		goto error;

	chord_bass = MPP_BAND_REM(bass, MPP_MAX_BANDS);
	chord_key = chord_key -
	    MPP_BAND_REM(chord_key, MPP_MAX_BANDS) +
	    rem;
	chord_mask = mask;

	key_bass = chord_key -
	    MPP_BAND_REM(chord_key, MPP_MAX_BANDS) +
	    chord_bass;

	if (key_bass >= chord_key)
		key_bass -= 2 * MPP_MAX_BANDS;
	else
		key_bass -= 1 * MPP_MAX_BANDS;

	out_key += MppKeyStr(key_bass);
	out_key += " ";
	out_key += MppKeyStr(key_bass + MPP_MAX_BANDS);
	out_key += " ";
	
	for (int x = 0; x != MPP_MAX_CHORD_BANDS; x++) {
		if (chord_mask.test(x) == 0)
			continue;
		out_key += MppKeyStr((x * MPP_BAND_STEP_CHORD) + chord_key);
		out_key += " ";
	}

	out_key += "/* ";
	out_key += chord;
	out_key += " */";

	lin_out->setText(out_key);

	goto done;
error:
	chord_mask.zero();
	chord_mask.set(0);
	lin_out->setText("/* ERROR */");
done:
	wi_dc->update();
}

QString
MppDecodeTab :: getText()
{
	return (lin_out->text());
}

void
MppDecodeTab :: setText(QString str)
{
  	lin_edit->blockSignals(1);
	lin_edit->setText(str);
	lin_edit->blockSignals(0);

	handle_parse();
}

void
MppDecodeTab :: keyPressEvent(QKeyEvent *event)
{
	switch (event->key()) {
	case Qt::Key_Up:
		handle_mod_up();
		break;
	case Qt::Key_Down:
		handle_mod_down();
		break;
	default:
		break;
	}
}

void
MppDecodeEditor :: mouseDoubleClickEvent(QMouseEvent *e)
{
	MppHead temp;
	MppChordElement info;
	QTextCursor cursor(textCursor());
	int row;

	cursor.movePosition(QTextCursor::EndOfLine, QTextCursor::MoveAnchor, 1);
	cursor.movePosition(QTextCursor::StartOfLine, QTextCursor::KeepAnchor, 1);

	/* check if the line is empty */
	if (cursor.selectedText().simplified().size() == 0)
		return;

	row = cursor.blockNumber();

	temp += toPlainText();
	temp.flush();

	/* check if the chord is valid */
	if (temp.getChord(row, &info) != 0)
		mw->tab_chord_gl->parseScoreChord(&info);

	setTextCursor(cursor);
}
