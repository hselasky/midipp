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

#ifndef _MIDIPP_MODE_H_
#define	_MIDIPP_MODE_H_

#include <midipp.h>

enum {
	MM_PASS_ALL,
	MM_PASS_ONE_MIXED,
	MM_PASS_NONE_FIXED,
	MM_PASS_NONE_TRANS,
	MM_PASS_NONE_CHORD,
	MM_PASS_MAX,
};

class MppMode : public QDialog
{
	Q_OBJECT;

public:
	MppMode(uint8_t _vi);
	~MppMode();

	/* input device mask */
	uint32_t input_mask;

	/* base key, if set */
	uint8_t base_key;

	/* command key, if set */
	uint8_t cmd_key;

	/* key delay */
	uint8_t key_delay;

	/* view number */
	uint8_t view_index;

	/* key mode */
	uint8_t key_mode;

	/* synth channel */
	uint8_t channel;

public:
	QGridLayout *gl;

	QLabel *lbl_mode;
	QLabel *lbl_chan;
	QLabel *lbl_input;
	QLabel *lbl_base;
	QLabel *lbl_cmd;
	QLabel *lbl_delay;
	QLabel *lbl_dev[MPP_MAX_DEVS];

	QCheckBox *cbx_dev[MPP_MAX_DEVS];

	QPushButton *but_mode;
	QPushButton *but_done;
	QPushButton *but_set_all;
	QPushButton *but_clear_all;

	MppSpinBox *spn_cmd;
	MppSpinBox *spn_base;

	QSpinBox *spn_chan;
	QSpinBox *spn_delay;

public slots:

	void handle_done();
	void handle_mode();
	void handle_set_all_devs();
	void handle_clear_all_devs();
};

#endif		/* _MIDIPP_MODE_H_ */
