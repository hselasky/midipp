/*-
 * Copyright (c) 2010,2012 Hans Petter Selasky. All rights reserved.
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

/* The list is sorted by priority. C5 is base. */

#define MASK(x) (1U << ((x) % 12))

static const struct score_variant score_internal[] = {
  { "", MASK(C5)| MASK(E5)| MASK(G5), 0 },
  { "M", MASK(C5)| MASK(E5)| MASK(G5), 0 },
  { "m", MASK(C5)| MASK(E5B)| MASK(G5), 0 },

  { "+3", MASK(C5)| MASK(E5)| MASK(G5)| MASK(H5 - 3), 0 },
  { "+5", MASK(C5)| MASK(E5)| MASK(G5)| MASK(H5 - 5), 0 },
  { "+9", MASK(C5)| MASK(E5)| MASK(G5)| MASK(H5 - 9), 0 },

  { "2", MASK(C5)| MASK(D5)| MASK(E5)| MASK(G5), 0 },
  { "4", MASK(C5)| MASK(E5)| MASK(G5), 0 },
  { "5", MASK(C5)| MASK(G5), 0 },
  { "6", MASK(C5)| MASK(E5)| MASK(G5)| MASK(A5), 0 },
  { "7", MASK(C5)| MASK(E5)| MASK(G5)| MASK(H5B), 0 },
  { "9", MASK(C5)| MASK(D5)| MASK(E5)| MASK(G5)| MASK(H5B), 0 },
  { "11", MASK(C5)| MASK(D5)| MASK(E5)| MASK(F5)| MASK(G5)| MASK(H5B), 0 },
  { "13", MASK(C5)| MASK(D5)| MASK(F5)| MASK(A5)| MASK(H5B), 0 },

  { "69", MASK(C5)| MASK(D5)| MASK(E5)| MASK(G5)| MASK(A5), 0 },

  { "7b5", MASK(C5)| MASK(E5)| MASK(G5B)| MASK(H5B), 0 },
  { "7#5", MASK(C5)| MASK(E5)| MASK(A5B)| MASK(H5B), 0 },
  { "7b9", MASK(C5)| MASK(D5B)| MASK(E5)| MASK(G5)| MASK(H5B), 0 },
  { "7#9", MASK(C5)| MASK(E5B)| MASK(E5)| MASK(G5)| MASK(H5B), 0 },

  { "add3", MASK(C5)| MASK(E5)| MASK(G5)| MASK(H5 - 3), 0 },
  { "add5", MASK(C5)| MASK(E5)| MASK(G5)| MASK(H5 - 5), 0 },
  { "add9", MASK(C5)| MASK(E5)| MASK(G5)| MASK(H5 - 9), 0 },

  { "maj", MASK(C5)| MASK(E5)| MASK(G5), 0 },
  { "major", MASK(C5)| MASK(E5)| MASK(G5), 0 },

  { "min", MASK(C5)| MASK(E5B)| MASK(G5), 0 },
  { "minor", MASK(C5)| MASK(E5B)| MASK(G5), 0 },

  { "maj7", MASK(C5)| MASK(E5)| MASK(G5)| MASK(H5), 0 },
  { "maj8", MASK(C5)| MASK(E5)| MASK(G5)| MASK(H5B), 0 },
  { "maj9", MASK(C5)| MASK(E5)| MASK(G5)| MASK(A5), 0 },
  { "maj10", MASK(C5)| MASK(E5)| MASK(G5)| MASK(A5B), 0 },
};

#define	MAX_VAR (sizeof(score_internal)/sizeof(score_internal[0]))
#define	MAX_TYPE 5

static struct score_variant mpp_score_variant[MAX_VAR * MAX_TYPE];

static int mpp_max_variant;

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

static uint8_t
score_variant_init_sub(unsigned int x, struct score_variant *ps)
{
	unsigned int type;
	unsigned int what;
	uint32_t e_mask = 0;
	uint32_t g_mask = 0;

	type = x / MAX_VAR;
	what = x % MAX_VAR;

	e_mask = score_internal[what].footprint & MASK(E5);
	g_mask = score_internal[what].footprint & MASK(G5);

	if (type != 0 && (e_mask == 0 || g_mask == 0))
		return (1);

	switch (type) {
	case 1:
		/* minors */
		ps->keyword[0] = 'm';
		STRLCPY(ps->keyword + 1, score_internal[what].keyword,
		    sizeof(ps->keyword) - 1);
		ps->footprint = (score_internal[what].footprint ^ e_mask) | (e_mask / 2);
		break;
	case 2:
		/* dim */
		ps->keyword[0] = 'd';
		ps->keyword[1] = 'i';
		ps->keyword[2] = 'm';

		STRLCPY(ps->keyword + 3, score_internal[what].keyword,
		    sizeof(ps->keyword) - 3);
		ps->footprint = (score_internal[what].footprint ^
		    e_mask ^ g_mask) | (e_mask / 2) | (g_mask / 2);
		break;
	case 3:
		/* aug */
		ps->keyword[0] = 'a';
		ps->keyword[1] = 'u';
		ps->keyword[2] = 'g';

		STRLCPY(ps->keyword + 3, score_internal[what].keyword,
		    sizeof(ps->keyword) - 3);

		ps->footprint = (score_internal[what].footprint ^
		    g_mask) | (g_mask / 2);
		break;
	case 4:
		/* sus */
		ps->keyword[0] = 's';
		ps->keyword[1] = 'u';
		ps->keyword[2] = 's';

		STRLCPY(ps->keyword + 3, score_internal[what].keyword,
		    sizeof(ps->keyword) - 3);
		ps->footprint = (score_internal[what].footprint ^
		    e_mask) | (e_mask * 2);
		break;
	default:
		/* others */
		STRLCPY(ps->keyword, score_internal[what].keyword,
		    sizeof(ps->keyword));
		ps->footprint = score_internal[what].footprint;
		break;
	}
	return (0);
}

void
MppScoreVariantInit(void)
{
	unsigned int x;
	unsigned int y;
	unsigned int z;
	unsigned int t;

	for (x = y = 0; x != (MAX_VAR * MAX_TYPE); x++) {
		if (score_variant_init_sub(x, &mpp_score_variant[y]) != 0)
			continue;
		y++;
	}

	for (x = 0; x != y; x ++) {
		int dup = mpp_score_variant[x].duplicate;

		Mpp.VariantList += QString("/* C") +
		    QString(mpp_score_variant[x].keyword) +
		    (dup ? QString(" = C") + QString(mpp_score_variant[dup - 1].keyword) : QString()) +
		    QString(" */\n");

		if (dup)
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

	Mpp.VariantList += QString("\n/* Number of supported chords is %1 */\n").arg(y);

	mpp_max_variant = y;
}

static uint8_t
mpp_get_key(char *ptr, char **pp)
{
	uint8_t key;

	switch (ptr[0]) {
	case 'C':
		key = C5;
		break;
	case 'D':
		key = D5;
		break;
	case 'E':
		key = E5;
		break;
	case 'F':
		key = F5;
		break;
	case 'G':
		key = G5;
		break;
	case 'A':
		key = A5;
		break;
	case 'B':
	case 'H':
		key = H5;
		break;
	default:
		key = 0;
		break;
	}
	if (key)
		ptr++;

	switch (ptr[0]) {
	case '#':
		key ++;
		ptr++;
		break;
	case 'b':
		key --;
		ptr++;
		break;
	default:
		break;
	}

	if (pp != NULL)
		*pp = ptr;

	return (key);
}

uint8_t
MppDecode :: parseScoreChord(MppChordElement *pinfo)
{
	QString out;
	uint8_t temp[12];
	uint32_t footprint = 0;
	int x;
	int y;
	int z;
	int n;
	int is_sharp;
	int rol;
	int flags;
	int best_x;
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

	while (flags != 3 &&
	    rol < (spn_rol->maximum() - 1) &&
	    rol > (spn_rol->minimum() + 1)) {

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

	spn_rol->setValue(rol);

	out += MppBaseKeyToString(y, is_sharp);
	out += mpp_score_variant[x].keyword;

	if (y != pinfo->key_base) {
		out += "/";
		out += MppBaseKeyToString(pinfo->key_base, is_sharp);
	}

	lin_edit->setText(out);

	return (0);
}

Q_DECL_EXPORT uint8_t
mpp_find_chord(const char *input, uint8_t *pbase,
    uint8_t *pkey, uint8_t *pvar)
{
	char *ptr;
	char *pb2;
	char buffer[16];
	uint8_t key;
	uint8_t base;
	uint8_t x;

	if (pbase != NULL)
		*pbase = 0;
	if (pkey != NULL)
		*pkey = 0;
	if (pvar != NULL)
		*pvar = 0;

	STRLCPY(buffer, input, sizeof(buffer));

	key = base = mpp_get_key(buffer, &ptr);

	if (key == 0)
		return (1);

	pb2 = strstr(ptr, "/");
	if (pb2 != NULL) {
		*pb2 = 0;
		base = mpp_get_key(pb2 + 1, NULL);
		if (base == 0)
			base = key;
	}

	for (x = 0; x != mpp_max_variant; x++) {
		if (strcmp(ptr, mpp_score_variant[x].keyword) == 0)
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
mpp_parse_chord(const char *input, int8_t rol,
    uint8_t *pout, uint8_t *pn,
    uint8_t *pvar, int change_var)
{
	uint8_t error = 0;
	uint8_t base;
	uint8_t key;
	uint8_t x;
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

MppDecode :: MppDecode(MppMainWindow *_mw, MppScoreMain *_sm, int is_edit)
  : QDialog(0)
{
	int n;

	mw = _mw;
	sm = _sm;

	gl = new QGridLayout(this);

	if (is_edit)
		setWindowTitle(tr("Editing a chord"));
	else
		setWindowTitle(tr("Inserting a chord"));

	setWindowIcon(QIcon(QString(MPP_ICON_FILE)));

	lin_edit = new QLineEdit(QString('C'));
	lin_edit->setMaxLength(256);

	lin_out = new QLineEdit();
	lin_out->setMaxLength(256);

	lbl_format = new QLabel(tr("[CDEFGABH][#b][...][/CDEFGABH[#b]]"));
	lbl_status = new QLabel(tr(""));
	lbl_rol = new QLabel(tr("Rotate:"));
	lbl_auto_base = new QLabel(tr("Add auto base:"));

	spn_rol = new QSpinBox();
	spn_rol->setMaximum(127);
	spn_rol->setMinimum(-127);
	spn_rol->setValue(0);

	cbx_auto_base = new MppCheckBox();
	cbx_auto_base->setChecked(1);

	but_ok = new QPushButton(tr("Ok"));
	but_cancel = new QPushButton(tr("Cancel"));
	but_play[0] = new MppButton(tr("Play &90"), 90);
	but_play[1] = new MppButton(tr("Play &60"), 60);
	but_play[2] = new MppButton(tr("Play &30"), 30);

	connect(but_ok, SIGNAL(released()), this, SLOT(handle_ok()));
	connect(but_cancel, SIGNAL(released()), this, SLOT(handle_cancel()));
	for (n = 0; n != 3; n++) {
		connect(but_play[n], SIGNAL(pressed(int)), this, SLOT(handle_play_press(int)));
		connect(but_play[n], SIGNAL(released(int)), this, SLOT(handle_play_release(int)));
	}
	connect(lin_edit, SIGNAL(textChanged(const QString &)), this, SLOT(handle_parse_text(const QString &)));
	connect(spn_rol, SIGNAL(valueChanged(int)), this, SLOT(handle_parse_int(int)));
	connect(cbx_auto_base, SIGNAL(stateChanged(int,int)), this, SLOT(handle_parse_int(int)));

	memset(current_score, 0, sizeof(current_score));
	memset(auto_base, 0, sizeof(auto_base));

	gl->addWidget(lbl_format, 0,0,1,5, Qt::AlignHCenter|Qt::AlignVCenter);

	gl->addWidget(lbl_rol, 2,0,1,1, Qt::AlignHCenter|Qt::AlignVCenter);
	gl->addWidget(spn_rol, 2,1,1,2);

	gl->addWidget(lin_edit, 3,0,1,5);
	gl->addWidget(lbl_status, 2,4,1,1, Qt::AlignHCenter|Qt::AlignVCenter);

	gl->addWidget(lin_out, 4,0,1,5);

	gl->addWidget(lbl_auto_base, 5,0,1,4, Qt::AlignRight|Qt::AlignVCenter);
	gl->addWidget(cbx_auto_base, 5,4,1,1, Qt::AlignHCenter|Qt::AlignVCenter);

	for (n = 0; n != 3; n++)
		gl->addWidget(but_play[n], 6, n, 1, 1, Qt::AlignHCenter|Qt::AlignVCenter);

	gl->addWidget(but_ok, 6, 3, 1, 1, Qt::AlignHCenter|Qt::AlignVCenter);
	gl->addWidget(but_cancel, 6, 4, 1, 1, Qt::AlignHCenter|Qt::AlignVCenter);

	handle_parse();
}

MppDecode :: ~MppDecode()
{

}

void
MppDecode :: handle_play_press(int vel)
{
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
MppDecode :: handle_play_release(int vel)
{
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
MppDecode :: handle_ok()
{
	this->accept();
}

void
MppDecode :: handle_cancel()
{
	this->reject();
}

void
MppDecode :: handle_parse_int(int x)
{
	handle_parse();
}

void
MppDecode :: handle_parse_text(const QString &x)
{
	handle_parse();
}

void
MppDecode :: handle_parse(int change_var)
{
	QString out;

	char *ptr;

	int error;
	int b_auto;
	int rol;
	int x;

	uint8_t n;
	uint8_t var;

	ptr = MppQStringToAscii(lin_edit->text().trimmed());
	if (ptr == NULL)
		return;

	out += QString("U1 ");

	rol = spn_rol->value();

	memset(current_score, 0, sizeof(current_score));
	memset(auto_base, 0, sizeof(auto_base));

	n = sizeof(current_score) / sizeof(current_score[0]);
	error = mpp_parse_chord(ptr, rol, current_score,
	    &n, &var, change_var);

	if (error == 0) {
		if (change_var && ptr[0] != 0) {
			const char *pslash = strstr(ptr, "/");

			if (pslash == NULL)
				pslash = "";

			switch (ptr[1]) {
			case 'b':
			case 'B':
				lin_edit->setText(QString(ptr[0]) + QString("b") + 
				    mpp_score_variant[var].keyword + QString(pslash));
				break;
			case '#':
				lin_edit->setText(QString(ptr[0]) + QString("#") + 
				    mpp_score_variant[var].keyword + QString(pslash));
				break;
			default:
				lin_edit->setText(QString(ptr[0]) +
				    mpp_score_variant[var].keyword + QString(pslash));
				break;
			}
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

	free(ptr);
}

QString
MppDecode :: getText()
{
	return (lin_out->text());
}

void
MppDecode :: setText(QString str)
{
	lin_edit->setText(str);

	handle_parse();
}

void
MppDecode :: keyPressEvent(QKeyEvent *event)
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
MppDecode :: wheelEvent(QWheelEvent *event)
{
	int num = event->delta();

	if (event->orientation() == Qt::Vertical && num != 0) {
		handle_parse(num / (8 * 15));
		event->accept();
	} else {
		event->ignore();
	}
}
