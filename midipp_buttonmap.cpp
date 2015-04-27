/*-
 * Copyright (c) 2012 Hans Petter Selasky. All rights reserved.
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

#include "midipp_button.h"
#include "midipp_buttonmap.h"

MppButtonMap :: MppButtonMap(const char *title, int max,
    int width) : QGroupBox()
{
	int x;
	int w;
	int h;

	if (max < 1)
		return;
	if (max > MPP_MAX_BUTTON_MAP)
		max = MPP_MAX_BUTTON_MAP;

	nButtons = max;
	currSelection = 0;

	for (x = 0; x != MPP_MAX_BUTTON_MAP; x++)
		but[x] = 0;

	setTitle(QString(title));

	grid = new QGridLayout(this);

	for (x = 0; x != nButtons; x++) {

		w = (x % width);
		h = (x / width);

		title = title + strlen(title) + 1;

		but[x] = new MppButton(QString(title), x);
		but[x]->setFlat(x != 0);

		connect(but[x], SIGNAL(pressed(int)), this, SLOT(handle_pressed(int)));
		connect(but[x], SIGNAL(released(int)), this, SLOT(handle_released(int)));

		grid->addWidget(but[x], h, w, 1, 1,
		    Qt::AlignHCenter | Qt::AlignVCenter);
	}
}

MppButtonMap :: ~MppButtonMap()
{

}

void
MppButtonMap :: setSelection(int id)
{
	if (id == currSelection)
		return;

	handle_pressed(id);
	handle_released(id);
}

void
MppButtonMap :: handle_released(int id)
{
	int x;

	if (id == currSelection)
		return;

	currSelection = id;

	for (x = 0; x != nButtons; x++)
		but[x]->setFlat(x != id);

	selectionChanged(id);
}

void
MppButtonMap :: handle_pressed(int id)
{

}
