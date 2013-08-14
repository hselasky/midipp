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

#ifndef _MIDIPP_GPRO_H_
#define	_MIDIPP_GPRO_H_

#include "midipp.h"

#define	GPRO_LHZ 10
#define	GPRO_HZ (1U << GPRO_LHZ)
#define	GPRO_MAX_DURATION 255
#define	GPRO_MAX_TRACKS 16

class MppGPro : public QDialog
{
	Q_OBJECT;

public:
	MppGPro(const uint8_t *, uint32_t);
	~MppGPro();

	QString output;

private:
	uint32_t chan_mask;

	QGridLayout *gl;

	QLabel *lbl_import[2];
	QLabel *lbl_info[GPRO_MAX_TRACKS];
	QLabel *lbl_single_track;

	MppCheckBox *cbx_import[GPRO_MAX_TRACKS];
	MppCheckBox *cbx_single_track;

	QPushButton *but_done;
	QPushButton *but_set_all;
	QPushButton *but_clear_all;

public slots:

	void handle_done();
	void handle_set_all_track();
	void handle_clear_all_track();
};

#endif		/* _MIDIPP_GPRO_H_ */
