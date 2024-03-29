/*-
 * Copyright (c) 2012-2022 Hans Petter Selasky
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

#ifndef _MIDIPP_BUTTONMAP_H_
#define	_MIDIPP_BUTTONMAP_H_

#include "midipp_groupbox.h"

class MppButtonMap : public MppGroupBox
{
	Q_OBJECT

public:
	MppButtonMap(const char *, int, int);

	void setSelection(int);

	MppButton *but[MPP_MAX_BUTTON_MAP];

	int currSelection;
	int nButtons;

signals:
	void selectionChanged(int);

public slots:
	void handle_pressed(int);
	void handle_released(int);
};

#define	MppKeyModeButtonMap(title)	\
	MppButtonMap(title "\0"		\
	    "ALL\0"			\
	    "FIXED\0"			\
	    "TRANSP\0"			\
	    "CHORD-PIANO\0"		\
	    "CHORD-AUX\0"		\
	    "CHORD-TRANS\0", 6, 3)

#endif		/* _MIDIPP_BUTTONMAP_H_ */
