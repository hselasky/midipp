/*-
 * Copyright (c) 2011-2017 Hans Petter Selasky. All rights reserved.
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

#include "midipp_spinbox.h"
#include "midipp_decode.h"

MppSpinBox :: MppSpinBox(QWidget *parent, int step, int allow_neg)
  : QSpinBox(parent)
{
#ifdef HAVE_QUARTERTONE
	if (step < 1)
		step = 1;
#else
	if (step < 2)
		step = 2;
#endif
	if (allow_neg)
		setRange(-256 + step, 256 - step);
	else
		setRange(0, 256 - step);
	setSingleStep(step);
}

int
MppSpinBox :: valueFromText(const QString &n) const
{
	const QChar *ch = n.data();
	int temp;
	int rem;
	int c;
	int neg = 0;
	int error = minimum() - 1;
	int step = singleStep();

	c = ch->toLatin1();

	switch (c) {
	case '-':
		neg = 1;
		ch++;
		break;
	default:
		break;
	}

	c = ch->toLatin1();

	switch (c) {
	case 'C':
		rem = MPP_C0;
		ch++;
		break;
	case 'D':
		rem = MPP_D0;
		ch++;
		break;
	case 'E':
		rem = MPP_E0;
		ch++;
		break;
	case 'F':
		rem = MPP_F0;
		ch++;
		break;
	case 'G':
		rem = MPP_G0;
		ch++;
		break;
	case 'A':
		rem = MPP_A0;
		ch++;
		break;
	case 'H':
	case 'B':
		rem = MPP_H0;
		ch++;
		break;
	case 0:
		if (ch->isNull()) {
			rem = 0;
			break;
		}
	default:
		return (error);
	}

	c = ch->toLatin1();

	switch (c) {
	case '0':
	case '1':
	case '2':
	case '3':
	case '4':
	case '5':
	case '6':
	case '7':
	case '8':
	case '9':
		temp = (c - '0') * MPP_MAX_BANDS;
		ch++;
		break;
	default:
		temp = 0;
		break;
	}

	c = ch->toLatin1();

	switch (c) {
	case '0':
	case '1':
	case '2':
	case '3':
	case '4':
	case '5':
	case '6':
	case '7':
	case '8':
	case '9':
		temp *= 10;
		temp += (c - '0') * MPP_MAX_BANDS;
		ch++;
		break;
	case 'b':
	case 'B':
		rem -= MPP_BAND_STEP_12;
		ch++;
		break;
#ifdef HAVE_QUARTERTONE
	case 'q':
	case 'Q':
		rem -= 3;
		ch++;
		break;
	case 'c':
	case 'C':
		rem -= 1;
		ch++;
		break;
#endif
	case '#':
		rem += MPP_BAND_STEP_12;
		ch++;
		break;
	case 0:
		if (ch->isNull())
			break;
	default:
		return (error);
	}

	c = ch->toLatin1();

	if (c != 0)
		return (error);

	rem += temp;

	/* check alignment  */
	if (rem % step)
		return (error);

	/* check if negative */
	if (neg)
		rem = -rem;

	return (rem);
}

QString
MppSpinBox :: textFromValue(int n) const
{
	QString temp;

	if (n < 0) {
		n = -n;
		temp = "-";
	}
	if (n < 256)
		return (temp + MppKeyStr[n]);
	return ("");
}

QValidator::State
MppSpinBox :: validate(QString &input, int &pos) const
{
	int temp = valueFromText(input);

	if (temp < minimum())
		return (QValidator::Invalid);

	if (temp > maximum())
		return (QValidator::Invalid);

	return (QValidator::Acceptable);
}
