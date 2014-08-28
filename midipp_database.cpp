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

#include <zlib.h>
#include <string.h>

#include "midipp_tar.h"
#include "midipp_database.h"
#include "midipp_groupbox.h"
#include "midipp_mainwindow.h"
#include "midipp_scores.h"
#include "midipp_show.h"

#define	MIDIPP_FILTER_MAX 32

struct filter {
	const char *match_word[MIDIPP_FILTER_MAX];
	uint8_t match_count;
};

static uint64_t
tar_decode_value(const char *where, int digs)
{
	uint64_t retval = 0;

	if (digs == 0)
		return (0);

	if (*where & 0x80) {
		retval = (uint8_t)(*where & 0x7F);
		digs--;
		where++;
		while (digs != 0) {
			retval <<= 8;
			retval |= (uint8_t)*where;
			digs--;
			where++;
		}
		return (retval);
	}
	while (digs && *where != 0 && *where != ' ') {
		if (*where >= '0' && *where <= '7')
			break;
		digs--;
		where++;
	}

	while (digs && *where != 0) {
		if (*where < '0' || *where > '7')
			break;
		retval <<= 3;
		retval += *where - '0';
		digs--;
		where++;
	}
	return (retval);
}

static uint32_t
tar_record_size(const union record *prec, uint64_t align)
{
	uint64_t size;

	size = tar_decode_value(prec->header.size, sizeof(prec->header.size));

	size += (-size & (align - 1ULL));

	/* safety limit record size to 16MBytes */
	if (size >= (1ULL << 24))
		size = (1ULL << 24) - 1ULL;

	return (size);
}

static int
is_alpha_numerical(char c)
{
	return ((c >= 'a' && c <= 'z') ||
		(c >= 'A' && c <= 'Z') ||
		(c >= '0' && c <= '9'));
}

static void
tar_filter_name(char *name, uint32_t size)
{
	uint32_t x;

	if (size == 0)
		return;
	name[size - 1] = 0;

	for (x = 0; x != size; x++) {
		if (is_alpha_numerical(name[x])) {
			/* pass */
		} else if (name[x] != 0) {
			name[x] = ' ';
		}
	}
}

static int
tar_match(void *arg, const char *str)
{
	const struct filter *filter = (struct filter *)arg;
	uint8_t x;

	for (x = 0; x != filter->match_count; x++) {
		if (strcasestr(str, filter->match_word[x]) == NULL)
			return (0);
	}
	return (1);
}

#ifdef __linux__
static int
tar_compare_r(const void *_pa, const void *_pb, void *arg)
#else
static int
tar_compare_r(void *arg, const void *_pa, const void *_pb)
#endif
{
	const union record *const *pa = (const union record *const *)_pa;
	const union record *const *pb = (const union record *const *)_pb;
	int va;
	int vb;

	va = tar_match(arg, (*pa)->header.name);
	vb = tar_match(arg, (*pb)->header.name);

	if (va != vb) {
		if (va < vb)
			return (1);
		else
			return (-1);
	}
	return (strcmp((*pa)->header.name, (*pb)->header.name));
}

/* Source: http://stackoverflow.com/questions/2690328/qt-quncompress-gzip-data */

#define	CHUNK_SIZE 1024

static QByteArray
do_gunzip(const QByteArray &data)
{
	QByteArray result;
	int ret;
	z_stream strm;
	uint8_t out[CHUNK_SIZE];

	if (data.size() <= 4)
		return (result);
 
	/* allocate inflate state */
	strm.zalloc = Z_NULL;
	strm.zfree = Z_NULL;
	strm.opaque = Z_NULL;
	strm.avail_in = data.size();
	strm.next_in = (Bytef * )(data.data());
 
	/* decode GZIP */
	ret = inflateInit2(&strm, 15 |	32);
	if (ret != Z_OK)
		return (result);
 
	/* decompress */
	do {
		strm.avail_out = CHUNK_SIZE;
		strm.next_out = (Bytef *)(out);
 
		ret = inflate(&strm, Z_NO_FLUSH);
		if (ret != Z_STREAM_END && ret != Z_OK)
			break;

		result.append((char *)out, CHUNK_SIZE - strm.avail_out);
	} while (strm.avail_out == 0);

	/* clean up and return */ 
	inflateEnd(&strm);
	return (result);
}

uint8_t
MppDataBase :: tar_record_foreach(union record **pp)
{
	union record *curr;

	curr = *pp;

	if (curr == NULL) {
		curr = (union record *)input_ptr;
	} else {
		curr = (union record *)(
		    ((uint8_t *)curr) +
		    tar_record_size(curr, RECORDSIZE) + RECORDSIZE);
	}

	/* check that there is space for record header */
	if (((uint8_t *)curr) + (RECORDSIZE - 1) >=
	    (((uint8_t *)input_ptr) + input_len)) {
		*pp = NULL;
		return (0);
	}

	/* check that there is space for record data */
	if ((((uint8_t *)curr) + tar_record_size(curr, RECORDSIZE) +
	     (RECORDSIZE - 1)) >= (((uint8_t *)input_ptr) + input_len)) {
		*pp = NULL;
		return (0);
	}
	if (curr->header.magic[0] != 'u' ||
	    curr->header.magic[1] != 's' ||
	    curr->header.magic[2] != 't' ||
	    curr->header.magic[3] != 'a' ||
	    curr->header.magic[4] != 'r') {
		*pp = NULL;
		return (0);
	}
	*pp = curr;
	return (1);
}

MppDataBase :: MppDataBase(MppMainWindow *mw)
{
	parent = mw;

	input_ptr = 0;
	input_len = 0;

	record_ptr = 0;
	record_count = 0;

	gl = new QGridLayout(this);

	location = new QLineEdit(QString(MPP_DEFAULT_URL));
	location->setMaxLength(1024);

	search = new QLineEdit();
	search->setMaxLength(256);

	result = new QListWidget();

	gb_result = new MppGroupBox(tr("Search results"));
	gb_result->addWidget(result, 0,0,1,1);

	download = new QPushButton(tr("Download"));

	connect(download, SIGNAL(released()), this, SLOT(handle_download()));
	connect(search, SIGNAL(textChanged(const QString &)), this, SLOT(handle_search_changed(const QString &)));

	open_a = new QPushButton(tr("Open In A-Scores"));
	open_b = new QPushButton(tr("Open In B-Scores"));

	connect(open_a, SIGNAL(released()), this, SLOT(handle_open_a()));
	connect(open_b, SIGNAL(released()), this, SLOT(handle_open_b()));

	clear_url = new QPushButton(tr("Clear"));
	clear_search = new QPushButton(tr("Clear"));

	connect(clear_url, SIGNAL(released()), this, SLOT(handle_clear_url()));
	connect(clear_search, SIGNAL(released()), this, SLOT(handle_clear_search()));

	reset = new QPushButton(tr("Reset"));

	connect(reset, SIGNAL(released()), this, SLOT(handle_reset()));

	gl->addWidget(new QLabel(tr("Location:")), 0, 0, 1, 1);
	gl->addWidget(location, 0, 1, 1, 3);
	gl->addWidget(clear_url, 0, 4, 1, 1);
	gl->addWidget(download, 0, 5, 1, 1);

	gl->addWidget(new QLabel(tr("Search:")), 1, 0, 1, 1);
	gl->addWidget(search, 1, 1, 1, 4);
	gl->addWidget(clear_search, 1, 5, 1, 1);

	gl->addWidget(gb_result, 2, 0, 1, 6);

	gl->addWidget(open_a, 3, 2, 1, 1);
	gl->addWidget(open_b, 3, 3, 1, 1);
	gl->addWidget(reset, 3, 5, 1, 1);

	gl->setColumnStretch(1, 1);

	connect(&net, SIGNAL(finished(QNetworkReply *)),
	    this, SLOT(handle_download_finished(QNetworkReply *)));
}

MppDataBase :: ~MppDataBase()
{
}

void
MppDataBase :: update_list_view()
{
	struct filter filter;
	char *filter_str;
	char titlebuf[64];
	uint64_t x;
	uint64_t y;

	memset(&filter, 0, sizeof(filter));

	filter_str = MppQStringToAscii(search->text());

	for (x = y = 0; filter_str[x] != 0 && y < MIDIPP_FILTER_MAX; y++) {
		for (; filter_str[x] != 0 &&
		    is_alpha_numerical(filter_str[x]) == 0; x++)
			;
		filter.match_word[y] = &filter_str[x];
		filter.match_count = y + 1;

		for (; filter_str[x] != 0 &&
		    is_alpha_numerical(filter_str[x]) != 0; x++)
			;
		if (filter_str[x] != 0)
			filter_str[x++] = 0;
		else
			break;
	}

	if (record_count != 0) {
#ifdef __linux__
		qsort_r(record_ptr, record_count, sizeof(void *),
		    &tar_compare_r, &filter);
#else
		qsort_r(record_ptr, record_count, sizeof(void *),
		    &filter, &tar_compare_r);
#endif
	}

	result->clear();

	for (x = 0; x != record_count; x++) {
		if (tar_match(&filter, record_ptr[x]->header.name) == 0)
			break;
		new QListWidgetItem(QString(record_ptr[x]->header.name), result);
	}

	result->setCurrentRow(0);

	free(filter_str);

	snprintf(titlebuf, sizeof(titlebuf), "Search results: %u match%s",
	    (unsigned)x, (x == 1) ? "" : "es");

	gb_result->setTitle(tr(titlebuf));
}

void
MppDataBase :: handle_open(union record *prec, MppScoreMain *ps)
{
	ps->handleScoreFileOpenRaw((char *)(prec + 1), tar_record_size(prec, 1));
}

void
MppDataBase :: handle_open_a()
{
	int n = result->currentRow();
	MppScoreMain *sm = parent->scores_main[0];

	if (n > -1 && n < (int)record_count)
		handle_open(record_ptr[n], sm);
}

void
MppDataBase :: handle_open_b()
{
	int n = result->currentRow();
	MppScoreMain *sm = parent->scores_main[1];

	if (n > -1 && n < (int)record_count)
		handle_open(record_ptr[n], sm);
}

void
MppDataBase :: handle_reset()
{
	location->setText(QString(MPP_DEFAULT_URL));
	search->setText(QString());
	input_data = QByteArray();
	handle_download_finished_sub();
}

void
MppDataBase :: handle_search_changed(const QString &str)
{
	update_list_view();
}

void
MppDataBase :: handle_clear_url()
{
	location->setText(QString());
}

void
MppDataBase :: handle_clear_search()
{
	search->setText(QString());
}

void
MppDataBase :: handle_download()
{
	download->setEnabled(0);

	QNetworkReply *reply = net.get(QNetworkRequest(QUrl(location->text())));

	connect(reply, SIGNAL(downloadProgress(qint64,qint64)),
	    this, SLOT(handle_download_progress(qint64,qint64)));
}

void
MppDataBase :: handle_download_progress(qint64 curr, qint64 total)
{
	if (total < 1)
		total = 1;
	if (curr < 0)
		curr = 0;
	download->setText(tr("Download %1kb").arg((curr + 1023) / 1024));
}

void
MppDataBase :: handle_download_finished_sub()
{
	union record *prec;

	input_len = input_data.size();
	input_ptr = input_data.data();

	free(record_ptr);
	record_ptr = 0;
	record_count = 0;

	prec = NULL;
	while (tar_record_foreach(&prec)) {
		char name[sizeof(prec->header.name)];
		char magic[sizeof(prec->header.magic)];
		char size[sizeof(prec->header.size)];
		uint32_t off_start;
		uint32_t off_end;

		/* use space as fill character to save configuration space */
		memcpy(name, prec->header.name, sizeof(name));
		memcpy(magic, prec->header.magic, sizeof(magic));
		memcpy(size, prec->header.size, sizeof(size));
		memset(prec, ' ', RECORDSIZE);

		/* force terminating zero in name */
		name[sizeof(name)-1] = 0;
		strcpy(prec->header.name, name);

		/* copy other fields */
		memcpy(prec->header.size, size, sizeof(size));
		memcpy(prec->header.magic, magic, sizeof(magic));

		off_start = tar_record_size(prec, 1);
		off_end = tar_record_size(prec, RECORDSIZE);

		/* use space as fill character to save configuration space */
		memset(((char *)(prec + 1)) + off_start, ' ', off_end - off_start);

		record_count++;
	}

	if (record_count != 0) {
		record_ptr = (union record **)malloc(sizeof(void *) * record_count);
		if (record_ptr != NULL) {
			prec = NULL;
			record_count = 0;
			while (tar_record_foreach(&prec)) {
				tar_filter_name(prec->header.name, sizeof(prec->header.name));
				record_ptr[record_count++] = prec;
			}
		}
	}
	update_list_view();
}

void
MppDataBase :: handle_download_finished(QNetworkReply *reply)
{
	download->setEnabled(1);

	if (reply->error()) {
		QMessageBox::information(this, tr("HTTP"),
			tr("Download failed: %1.")
			.arg(reply->errorString()));
	} else {
		input_data = do_gunzip(reply->readAll());

		handle_download_finished_sub();

		if (record_count == 0) {
			QMessageBox::information(this, tr("TAR"),
			    tr("Database is empty."));
		}
	}

	reply->deleteLater();
}
