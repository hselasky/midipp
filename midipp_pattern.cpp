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

#include "midipp_pattern.h"

MppPattern :: MppPattern(QWidget *parent)
  : QLineEdit(parent)
{
	pthread_mutex_init(&mtx, NULL);

	setText(QString("1"));

	handleChanged(QString("1"));

	connect(this, SIGNAL(textChanged(const QString &)), this, SLOT(handleChanged(const QString &)));
}

MppPattern :: ~MppPattern()
{
	pthread_mutex_destroy(&mtx);
}

int
MppPattern :: matchPattern(uint32_t time)
{
	uint32_t temp;
	uint8_t n = 0;
	uint8_t i;

	time &= 0x7FFFFFFFU;

	pthread_mutex_lock(&mtx);

	for (i = 0; i != MPP_PATTERN_MAX; i++) {
		temp = pattern[i];

		if ((time & temp) == temp)
			n ^= 1;
	}

	pthread_mutex_unlock(&mtx);

	return (n);
}

void
MppPattern :: handleChanged(const QString &text)
{
	const QChar *ch;
	int c;
	uint32_t temp = 0;
	uint8_t i;
	uint8_t any;

	ch = text.data();

	pthread_mutex_lock(&mtx);

	for (i = 0; i != MPP_PATTERN_MAX; i++)
		pattern[i] = 0x80000000U;

	i = 0;
	any = 0;

	while (ch->isNull() == 0) {
		c = ch->toAscii();
		if (c >= 'a' && c <= 'z') {
			temp |= 1 << (c - 'a');
			any = 1;
		}
		if (c == '1') {
			any = 1;
		}
		if (c == '+') {
			if (i < MPP_PATTERN_MAX) {
				pattern[i] = temp;
				temp = 0;
				any = 0;
				i++;
			}
		}
		ch++;
	}

	if (any != 0 && i < MPP_PATTERN_MAX)
		pattern[i] = temp;

	pthread_mutex_unlock(&mtx);
}
