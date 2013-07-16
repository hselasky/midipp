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

#include "midipp_spinbox.h"

MppSpinBox :: MppSpinBox(QWidget *parent)
  : QSpinBox(parent)
{
}

MppSpinBox :: ~MppSpinBox()
{

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
		rem = C0;
		ch++;
		break;
	case 'D':
		rem = D0;
		ch++;
		break;
	case 'E':
		rem = E0;
		ch++;
		break;
	case 'F':
		rem = F0;
		ch++;
		break;
	case 'G':
		rem = G0;
		ch++;
		break;
	case 'A':
		rem = A0;
		ch++;
		break;
	case 'H':
	case 'B':
		rem = H0;
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
		temp = (c - '0') * 12;
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
		temp += (c - '0') * 12;
		ch++;
		break;
	case 'b':
	case 'B':
		rem -= 1;
		ch++;
		break;
	case '#':
		rem += 1;
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

	if (neg)
		rem = -rem;

	return (rem);
}

QString
MppSpinBox :: textFromValue(int n) const
{
	int rem = n;
	int odd = 0;
	QString temp;

	if (rem < 0) {
		rem = -rem;
		temp += QChar('-');
	}

	switch (rem % 12) {
	case 0:
		temp += QChar('C');
		break;
	case 1:
		temp += QChar('D');
		odd = 1;
		break;
	case 2:
		temp += QChar('D');
		break;
	case 3:
		temp += QChar('E');
		odd = 1;
		break;
	case 4:
		temp += QChar('E');
		break;
	case 5:
		temp += QChar('F');
		break;
	case 6:
		temp += QChar('G');
		odd = 1;
		break;
	case 7:
		temp += QChar('G');
		break;
	case 8:
		temp += QChar('A');
		odd = 1;
		break;
	case 9:
		temp += QChar('A');
		break;
	case 10:
		temp += QChar('B');
		odd = 1;
		break;
	case 11:
		temp += QChar('B');
		break;
	default:
		break;
	}

	if (rem >= (12 * 10))
		temp += QChar('0' + ((rem / (12 * 10)) % 10));

	temp += QChar('0' + ((rem / 12) % 10));

	if (odd)
		temp += QChar('B');

	return (temp);
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
