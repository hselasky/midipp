/*-
 * Copyright (c) 2013 Hans Petter Selasky. All rights reserved.
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

#ifndef _MIDIPP_DATABASE_H_
#define	_MIDIPP_DATABASE_H_

#include "midipp.h"

union record;

class MppDataBase : public QWidget
{
	Q_OBJECT;

public:
	MppDataBase(MppMainWindow *);
	~MppDataBase();

	uint8_t tar_record_foreach(union record **);

	void handle_open(union record *, MppScoreMain *);
	void handle_download_finished_sub();

	void update_list_view();

	QByteArray input_data;
	void *input_ptr;
	uint64_t input_len;

	union record **record_ptr;
	uint64_t record_count;

	MppMainWindow *parent;

	QGridLayout *gl;

	QLineEdit *location;
	QLineEdit *search;
	QListWidget *result;
	MppGroupBox *gb_result;

	QPushButton *download;
	QPushButton *open_a;
	QPushButton *open_b;
	QPushButton *reset;
	QPushButton *clear_url;
	QPushButton *clear_search;

	QNetworkAccessManager net;

public slots:
	void handle_open_a();
	void handle_open_b();
	void handle_reset();
	void handle_clear_url();
	void handle_clear_search();

	void handle_search_changed(const QString &);
	void handle_download();
	void handle_download_progress(qint64, qint64);
	void handle_download_finished(QNetworkReply *);
};

#endif	/* _MIDIPP_DATABASE_H_ */
