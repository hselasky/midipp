/*-
 * Copyright (c) 2010 Hans Petter Selasky. All rights reserved.
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

#include <midipp_decode.h>
#include <midipp_mainwindow.h>
#include <midipp_scores.h>
#include <midipp_button.h>

/* The list is sorted by priority. C5 is base. */

static const struct score_variant score_variant[] = {
  { "dim7", {C5, E5B, G5B, A5} },
  { "m7b5", {C5, E5B, G5B, H5B} },
  { "m7", {C5, E5B, G5, H5B} },
  { "maj7", {C5, E5, G5, H5} },
  { "maj9", {C5, E5, G5, A5, C6} },
  { "", {C5, E5, G5} },
  { "M", {C5, E5, G5} },
  { "maj", {C5, E5, G5} },
  { "major", {C5, E5, G5} },
  { "7b5", {C5, E5, G5B, H5B} },
  { "7#5", {C5, E5, A5B, H5B} },
  { "7b9", {C5, E5, G5, H5B, D6B} },
  { "69", {C5, E5, G5, A5, D6} },
  { "m6", {C5, E5B, G5, A5} },
  { "m9", {C5, E5B, G5, H5B, D6} },
  { "m11", {C5, E5B, G5, H5B, D6, F6} },
  { "madd6", {C5, E5B, G5, A5} },
  { "madd9", {C5, E5B, G5, H5B, D6} },
  { "madd11", {C5, E5B, G5, H5B, D6, F6} },
  { "add3", {C5, E5, G5, H5 - 3} },
  { "add5", {C5, E5, G5, H5 - 5} },
  { "add9", {C5, E5, G5, H5 - 9} },
  { "sus2", {C5, D5, G5} },
  { "sus",  {C5, F5, G5} },
  { "sus4", {C5, F5, G5} },
  { "sus7", {C5, F5, G5, H5B} },
  { "dim", {C5, E5B, G5B} },
  { "aug", {C5, E5, A5B} },
  { "m", {C5, E5B, G5} },
  { "min", {C5, E5B, G5} },
  { "minor", {C5, E5B, G5} },
  { "2", {C5, D5, E5, G5} },
  { "6", {C5, E5, G5, A5} },
  { "7", {C5, E5, G5, H5B} },
  { "9", {C5, E5, G5, H5B, D6} },
  { "11", {C5, E5, G5, H5B, D6, F6} },
  { "13", {C5, H5B, D6, F6, A6} },
};

static uint8_t
mpp_get_key(const char *ptr)
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
	if (ptr[0])
		ptr++;

	switch (ptr[0]) {
	case '#':
		key ++;
		break;
	case 'b':
		key --;
		break;
	default:
		break;
	}

	return (key);
}

uint8_t
mpp_parse_score(const char *input, uint8_t base,
    int8_t rol, uint8_t pout[MPP_MAX_VAR_OFF])
{
	char *ptr;
	char buffer[16];
	uint8_t error = 0;
	uint8_t key;
	uint8_t x;
	uint8_t y;

	switch (*input) {
	case 'C':
		key = C5;
		input++;
		break;
	case 'D':
		key = D5;
		input++;
		break;
	case 'E':
		key = E5;
		input++;
		break;
	case 'F':
		key = F5;
		input++;
		break;
	case 'G':
		key = G5;
		input++;
		break;
	case 'A':
		key = A5;
		input++;
		break;
	case 'B':
	case 'H':
		key = H5;
		input++;
		break;
	default:
		key = C5;
		error = 1;
		break;
	}

	switch (*input) {
	case '#':
		key ++;
		input++;
		break;
	case 'b':
		key --;
		input++;
		break;
	default:
		break;
	}

	strlcpy(buffer, input, sizeof(buffer));

	ptr = strstr(buffer, "/");
	if (ptr)
		*ptr = 0;

	for (x = 0; x != (sizeof(score_variant)/sizeof(score_variant[0])); x++) {

		if (strcmp(buffer, score_variant[x].keyword) == 0) {

			for (y = 0; y != (MPP_MAX_VAR_OFF-1); y++) {
				uint8_t z;

				z = score_variant[x].offset[y];
				if (z == 0)
					break;
				pout[y] = (z - C5) + (key - C5) + base;
			}

			pout[y] = 0;
			mid_trans(pout, y, rol);
			return (error);
		}
	}
	pout[0] = 0;
	return (1);		/* failed */
}

MppDecode :: MppDecode(QWidget *parent, MppMainWindow *_mw)
  : QDialog(parent)
{
	int n;

	mw = _mw;

	gl = new QGridLayout(this);

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
MppDecode :: handle_parse()
{
	QString out;

	char *ptr;
	char *pba;

	int error;
	int base;
	int b_auto;
	int rol;
	int x;

	ptr = MppQStringToAscii(lin_edit->text().trimmed());
	if (ptr == NULL)
		return;

	rol = spn_rol->value();
	base = spn_base->value();

	error = mpp_parse_score(ptr, base, rol, current_score);

	if (error == 0)
		lbl_status->setText(tr("OK"));
	else
		lbl_status->setText(tr("ERROR"));

	out += QString("U1 ");

	pba = strstr(ptr, "/");
	if (pba != NULL)
		b_auto = mpp_get_key(pba + 1);
	else
		b_auto = mpp_get_key(ptr);

	if (b_auto != 0 && cbx_auto_base->isChecked()) {

		b_auto = b_auto - C5 + base;

		while (b_auto >= (int)(current_score[0] & 0x7F))
			b_auto -= 12;

		memset(auto_base, 0, sizeof(auto_base));

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

	for (x = 0; current_score[x]; x++) {
		out += QString(mid_key_str[current_score[x] & 0x7F]) +
		  QString(" ");
	}

	out += QString("/* ") + lin_edit->text().trimmed() 
	  + QString(" */");

	lin_out->setText(out);
	lbl_base->setText(tr("Base (") + tr(mid_key_str[base]) + tr("):"));

	free(ptr);
}

QString
MppDecode :: getText()
{
	return (lin_out->text());
}

void
MppDecode :: setText(const char *ptr)
{
	lin_edit->setText(QString(ptr));

	handle_parse();
}
