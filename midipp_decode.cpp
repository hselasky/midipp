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

/* The list is sorted by priority. C5 is base. */

static const struct score_variant score_internal[] = {
  { "", {C5, E5, G5} },
  { "M", {C5, E5, G5} },
  { "maj", {C5, E5, G5} },
  { "major", {C5, E5, G5} },

  { "m", {C5, E5B, G5} },
  { "min", {C5, E5B, G5} },
  { "minor", {C5, E5B, G5} },

  { "maj7", {C5, E5, G5, H5} },
  { "maj8", {C5, E5, G5, H5B} },
  { "maj9", {C5, E5, G5, A5} },
  { "maj10", {C5, E5, G5, A5B} },

  { "7b5", {C5, E5, G5B, H5B} },
  { "7#5", {C5, E5, A5B, H5B} },
  { "7b9", {C5, D5B, E5, G5, H5B} },
  { "7#9", {C5, E5B, E5, G5, H5B} },

  { "69", {C5, D5, E5, G5, A5} },

  { "add3", {C5, E5, G5, H5 - 3} },
  { "add5", {C5, E5, G5, H5 - 5} },
  { "add9", {C5, E5, G5, H5 - 9} },

  { "+3", {C5, E5, G5, H5 - 3} },
  { "+5", {C5, E5, G5, H5 - 5} },
  { "+9", {C5, E5, G5, H5 - 9} },

  { "2", {C5, D5, E5, G5} },
  { "4", {C5, E5, G5} },
  { "5", {C5, G5} },
  { "6", {C5, E5, G5, A5} },
  { "7", {C5, E5, G5, H5B} },
  { "9", {C5, D5, E5, G5, H5B} },
  { "11", {C5, D5, E5, F5, G5, H5B} },
  { "13", {C5, D5, F5, A5, H5B} },
};

#define	MAX_VAR (sizeof(score_internal)/sizeof(score_internal[0]))
#define	MAX_TYPE 5

static struct score_variant mpp_score_variant[MAX_VAR * MAX_TYPE];

static int mpp_max_variant;

QString MppVariantList;

static uint8_t
score_variant_init_sub(unsigned int x, struct score_variant *ps)
{
	unsigned int type;
	unsigned int what;
	unsigned int y;
	int e_off = -1;
	int g_off = -1;

	type = x / MAX_VAR;
	what = x % MAX_VAR;

	for (y = 0; y != MPP_MAX_VAR_OFF; y++) {
		if (score_internal[what].offset[y] == E5)
			e_off = y;
		if (score_internal[what].offset[y] == G5)
			g_off = y;
	}

	if (type != 0 && (e_off == -1 || g_off == -1 ||
	    (score_internal[what].keyword[0] == 'M') ||
	    (score_internal[what].keyword[0] == 'm')))
		return (1);

	switch (type) {
	case 1:
		/* minors */
		ps->keyword[0] = 'm';
		STRLCPY(ps->keyword + 1, score_internal[what].keyword,
		    sizeof(ps->keyword) - 1);
		memcpy(ps->offset, score_internal[what].offset,
		    sizeof(ps->offset));

		ps->offset[e_off]--;
		break;
	case 2:
		/* dim */
		ps->keyword[0] = 'd';
		ps->keyword[1] = 'i';
		ps->keyword[2] = 'm';

		STRLCPY(ps->keyword + 3, score_internal[what].keyword,
		    sizeof(ps->keyword) - 3);
		memcpy(ps->offset, score_internal[what].offset,
		    sizeof(ps->offset));

		ps->offset[e_off]--;
		ps->offset[g_off]--;
		break;
	case 3:
		/* aug */
		ps->keyword[0] = 'a';
		ps->keyword[1] = 'u';
		ps->keyword[2] = 'g';

		STRLCPY(ps->keyword + 3, score_internal[what].keyword,
		    sizeof(ps->keyword) - 3);
		memcpy(ps->offset, score_internal[what].offset,
		    sizeof(ps->offset));

		ps->offset[g_off]++;
		break;
	case 4:
		/* sus */
		ps->keyword[0] = 's';
		ps->keyword[1] = 'u';
		ps->keyword[2] = 's';

		STRLCPY(ps->keyword + 3, score_internal[what].keyword,
		    sizeof(ps->keyword) - 3);
		memcpy(ps->offset, score_internal[what].offset,
		    sizeof(ps->offset));

		ps->offset[e_off]++;
		break;
	default:
		/* others */
		STRLCPY(ps->keyword, score_internal[what].keyword,
		    sizeof(ps->keyword));
		memcpy(ps->offset, score_internal[what].offset,
		    sizeof(ps->offset));
		break;
	}
	return (0);
}

void
MppScoreVariantInit(void)
{
	unsigned int x;
	unsigned int y;

	for (x = y = 0; x != (MAX_VAR * MAX_TYPE); x++) {
		if (score_variant_init_sub(x, &mpp_score_variant[y]) == 0) {
			MppVariantList += QString("/* C") +
			    QString(mpp_score_variant[y].keyword) +
			    QString(" */\n");
			y++;
		}
	}

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
MppDecode :: parseScoreChord(struct MppScoreEntry *ps,
    const char *chord, uint8_t allow_var)
{
	QString out;

	uint8_t foot_print[12];
	uint8_t foot_max[12];
	uint8_t foot_len = 0;
	uint8_t max;
	uint8_t min;
	uint8_t var_min;
	uint8_t var_max;
	uint8_t fkey;
	uint8_t match_curr;
	uint8_t match_max;
	uint8_t match_var;
	uint8_t match_trans;

	int x;
	int y;
	int z;
	int is_sharp;
	int key;

	memset(foot_print, 0, sizeof(foot_print));
	memset(foot_max, 0, sizeof(foot_max));
	max = 0;

	is_sharp = (strstr(chord, "#") != NULL);

	if (mpp_find_chord(chord, &min, &fkey, &var_min)) {
		min = 255;
		var_min = 0;
		var_max = mpp_max_variant;
	} else {
		min %= 12;
		var_max = var_min + 1;
	}

	for (x = 0; x != MPP_MAX_SCORES; x++) {
		if (ps[x].dur != 0) {
			key = ps[x].key % 12;
			/*
			 * When the base score is unknown, we look for
			 * a score that occurs more than once:
			 */
			if (foot_print[key] != 0 && min == 255)
				min = key;
			foot_print[key] = 1;
			if (foot_max[key] < ps[x].key)
				foot_max[key] = ps[x].key;
			if (max < ps[x].key)
				max = ps[x].key;
		}
	}
	for (x = 0; x != 12; x++) {
		if (foot_print[x])
			foot_len++;
		else
			foot_max[x] = max;
	}

	if (foot_len == 0)
		return (1);

	match_max = 255;	/* smaller is better */
	match_var = var_min;
	match_trans = 0;

	for (x = 0; x != 12; x++) {
		for (y = var_min; y != var_max; y++) {
			uint8_t p = 0;
			uint8_t q = 0;

			for (z = 0; z != MPP_MAX_VAR_OFF; z++) {
				key = mpp_score_variant[y].offset[z];

				if (key != 0) {
					p++;

					if (foot_print[(key + x) % 12])
						q++;
				}
			}

			if (p < foot_len)
				p = foot_len;

			if (p > q)
				match_curr = p - q;
			else
				match_curr = q - p;

			if (match_curr < match_max) {
				match_max = match_curr;
				match_var = y;
				match_trans = x;
			}
		}
	}

	if (match_max != 0 && allow_var == 0) {
		if (min == 255)
			return (1);
		z = MPP_MAX_VAR_OFF;
		y = var_min;
		key = fkey % 12;
		spn_base->setValue(C5);
		spn_rol->setValue(0);
	} else {
		y = match_var;
		x = match_trans;

		for (z = 0; z != MPP_MAX_VAR_OFF; z++) {
			key = mpp_score_variant[y].offset[z];

			if ((key % 12) == ((max - foot_max[x]) % 12))
				break;
		}
		key = (foot_max[x] % 12);
		spn_base->setValue(foot_max[x] - key);
		z = z - foot_len + 1;
		spn_rol->setValue(z);
	}

	out += MppBaseKeyToString(key, is_sharp);
	out += mpp_score_variant[y].keyword;

	if (min != 255 && min != key) {
		out += "/";
		out += MppBaseKeyToString(min, is_sharp);
	}

	lin_edit->setText(out);

	return (0);
}

uint8_t
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

uint8_t
mpp_parse_chord(const char *input, uint8_t trans,
    int8_t rol, uint8_t *pout, uint8_t *pn,
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
		while (change_var > 0) {
			x++;
			x %= mpp_max_variant;
			if (mpp_score_variant[x].offset[0] != 0)
				change_var--;
		}
		while (change_var < 0) {
			x--;
			x += mpp_max_variant;
			x %= mpp_max_variant;
			if (mpp_score_variant[x].offset[0] != 0)
				change_var++;
		}
	}

	n = 0;

	pout[n++] = (base - C5) + trans;

	for (y = 0; y != MPP_MAX_VAR_OFF; y++) {
		uint8_t z;

		z = mpp_score_variant[x].offset[y];
		if (z == 0)
			break;
		if (n < *pn)
			pout[n++] = (z - C5) + (key - C5) + trans;
	}

	mid_trans(pout + 1, n - 1, rol);

	*pn = n;
	*pvar = x;

	return (0);
}

MppDecode :: MppDecode(MppMainWindow *_mw, int is_edit)
  : QDialog(0)
{
	int n;

	mw = _mw;

	gl = new QGridLayout(this);

	if (is_edit)
		setWindowTitle(tr("Edit chord"));
	else
		setWindowTitle(tr("Chord decoder"));

	setWindowIcon(QIcon(QString(MPP_ICON_FILE)));

	lin_edit = new QLineEdit();
	lin_edit->setText(QString("C"));

	lin_out = new QLineEdit();
	lbl_format = new QLabel(tr("[CDEFGABH][#b][...][/CDEFGABH[#b]]"));
	lbl_status = new QLabel(tr(""));
	lbl_rol = new QLabel(tr("Rotate:"));
	lbl_base = new QLabel(tr("Base (C5):"));
	lbl_auto_base = new QLabel(tr("Add auto base:"));

	spn_rol = new QSpinBox();
	spn_rol->setMaximum(127);
	spn_rol->setMinimum(-127);
	spn_rol->setValue(0);

	spn_base = new QSpinBox();
	spn_base->setMaximum(127);
	spn_base->setMinimum(0);
	spn_base->setValue(C5);

	cbx_auto_base = new QCheckBox();
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
	connect(spn_base, SIGNAL(valueChanged(int)), this, SLOT(handle_parse_int(int)));
	connect(cbx_auto_base, SIGNAL(stateChanged(int)), this, SLOT(handle_parse_int(int)));

	memset(current_score, 0, sizeof(current_score));
	memset(auto_base, 0, sizeof(auto_base));

	gl->addWidget(lbl_format, 0,0,1,5, Qt::AlignHCenter|Qt::AlignVCenter);

	gl->addWidget(lbl_rol, 2,0,1,1, Qt::AlignHCenter|Qt::AlignVCenter);
	gl->addWidget(spn_rol, 2,1,1,1, Qt::AlignHCenter|Qt::AlignVCenter);

	gl->addWidget(lbl_base, 2,2,1,1, Qt::AlignHCenter|Qt::AlignVCenter);
	gl->addWidget(spn_base, 2,3,1,1, Qt::AlignHCenter|Qt::AlignVCenter);

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
			mw->output_key(mw->currScoreMain()->synthChannel, auto_base[x], vel, 0, 0);
	}
	for (x = 0; current_score[x]; x++) {
		mw->output_key(mw->currScoreMain()->synthChannel, current_score[x], vel, 0, 0);
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
			mw->output_key(mw->currScoreMain()->synthChannel, auto_base[x], 0, 0, 0);
	}
	for (x = 0; current_score[x]; x++) {
		mw->output_key(mw->currScoreMain()->synthChannel, current_score[x], 0, 0, 0);
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
	int base;
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
	base = spn_base->value();

	memset(current_score, 0, sizeof(current_score));
	memset(auto_base, 0, sizeof(auto_base));

	n = sizeof(current_score) / sizeof(current_score[0]);
	error = mpp_parse_chord(ptr, base, rol, current_score,
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
	lbl_base->setText(tr("Base (") + QString(mid_key_str[base]) + tr("):"));

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
