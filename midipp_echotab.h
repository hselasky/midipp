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

#ifndef _MIDIPP_ECHOTAB_H_
#define	_MIDIPP_ECHOTAB_H_

#include <midipp.h>

#define	MPP_ECHO_MAX	8000U	/* ms */
#define	MPP_ECHO_UNIT	65536U
#define	MPP_ECHO_AMP	128U

enum {
	ME_MODE_DEFAULT,
	ME_MODE_BASE_ONLY,
	ME_MODE_SLIDE,
	ME_MODE_MAX,
};

struct MppEchoValues {
	uint32_t ival_init;
	uint32_t ival_repeat;
	uint32_t ival_rand;
	uint32_t amp_init;
	uint32_t amp_fact;
	uint32_t amp_rand;
	uint32_t num_echo;
	uint8_t in_channel;
	uint8_t out_channel;
	uint8_t last_key[2];
	uint8_t mode;
	int8_t transpose;
};

class MppEchoTab : public QWidget
{
	Q_OBJECT;

public:
	MppEchoTab(QWidget *parent, MppMainWindow *);
	~MppEchoTab();

	void watchdog();

	uint8_t auto_zero_start[0];

	MppMainWindow *mw;

	QGridLayout *gl;

	QSpinBox *spn_echo_ival_init;
	QSpinBox *spn_echo_ival_repeat;
	QSpinBox *spn_echo_ival_rand;
	QSpinBox *spn_echo_amp_init;
	QSpinBox *spn_echo_amp_fact;
	QSpinBox *spn_echo_amp_rand;
	QSpinBox *spn_echo_num;
	QSpinBox *spn_echo_transpose;
	QSpinBox *spn_echo_in_channel;
	QSpinBox *spn_echo_out_channel;

	QLabel *lbl_echo_mode;
	QLabel *lbl_echo_ival_init;
	QLabel *lbl_echo_ival_repeat;
	QLabel *lbl_echo_ival_rand;
	QLabel *lbl_echo_amp_init;
	QLabel *lbl_echo_amp_fact;
	QLabel *lbl_echo_amp_rand;
	QLabel *lbl_echo_num;
	QLabel *lbl_echo_transpose;
	QLabel *lbl_echo_in_channel;
	QLabel *lbl_echo_out_channel;

	QLabel *lbl_echo_title;
	QLabel *lbl_echo_status;

	QPushButton *but_echo_mode;
	QPushButton *but_echo_enable;
	QPushButton *but_echo_reset;

	struct MppEchoValues echo_val;

	uint8_t echo_enabled;
	uint8_t echo_dirty;
	uint8_t echo_mode;

	uint8_t auto_zero_end[0];

public slots:

	void handle_echo_reset();
	void handle_echo_enable();
	void handle_echo_generate(int);
	void handle_echo_mode();
};

#endif	/* _MIDIPP_ECHO_H_ */
