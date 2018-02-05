/*-
 * Copyright (c) 2011-2018 Hans Petter Selasky. All rights reserved.
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
#include "midipp_element.h"

static void
MppValueFromText(const QString &str, int &value, int &valid)
{
	MppElement *ptr;
	MppHead temp;

	temp += str;
        temp.flush();

	value = 0;
	valid = 0;

	TAILQ_FOREACH(ptr, &temp.head, entry) {
		if (ptr->type == MPP_T_SCORE_SUBDIV) {
			value = ptr->value[0];
			valid++;
		} else {
			valid = 0;
			break;
		}
	}
}

MppSpinBox :: MppSpinBox(QWidget *parent, int allow_neg)
  : QSpinBox(parent)
{
	if (allow_neg)
		setRange(0x8000, 0x7fff);
	else
		setRange(0, 0x7fff);
	setSingleStep(MPP_BAND_STEP_12);
}

int
MppSpinBox :: valueFromText(const QString &str) const
{
	int value;
	int valid;

	MppValueFromText(str, value, valid);
	if (valid)
		return (value);
	return (0);
}

QString
MppSpinBox :: textFromValue(int n) const
{
	return (MppKeyStr(n));
}

QValidator::State
MppSpinBox :: validate(QString &input, int &pos) const
{
	int value;
	int valid;

	if (input.isEmpty() == 0) {
		MppValueFromText(input, value, valid);

		if (valid == 0 || value < minimum() || value > maximum())
			return (QValidator::Invalid);
	}
	return (QValidator::Acceptable);
}
