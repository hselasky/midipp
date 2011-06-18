/*-
 * Copyright (c) 2009-2010 Hans Petter Selasky. All rights reserved.
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

#include <midipp.h>

#include <signal.h>

#include <midipp_mainwindow.h>

QColor color_black   (0x00, 0x00, 0x00, 0xff);
QColor color_white   (0xff, 0xff, 0xff, 0xff);
QColor color_grey    (0xc0, 0xc0, 0xc0, 0xff);
QColor color_logo    (0xc4, 0x40, 0x20, 0xff);
QColor color_green   (0x40, 0xc4, 0x20, 0xff);

QFont font_fixed (QString("Courier New"));

QString
MppBaseName(QString fname)
{
	QFileInfo fi(fname);
	return (fi.fileName());
}

char *
MppQStringToAscii(QString s)
{
        QChar *ch;
	char *ptr;
	int len;
	int c;

	ch = s.data();
	if (ch == NULL)
		return (NULL);

	len = 1;		/* reserve space for NUL character */

	while (1) {
		c = ch->toAscii();
		if (ch->isNull())
			break;
		len++;
		ch++;
	}

	ptr = (char *)malloc(len);
	if (ptr == NULL)
		return (NULL);

	ch = s.data();
	len = 0;

	while (1) {
		c = ch->toAscii();
		if (ch->isNull())
			break;
		if (c == 0)
			c = ' ';
		ptr[len] = c;
		len++;
		ch++;
	}
	ptr[len] = 0;		/* NUL terminate */
	return (ptr);
}

QString
MppReadFile(QString fname, QString *perr)
{
	QFile file(fname);
	QString retval;

	if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
		goto failure;

	retval = file.readAll();
	if (file.error()) {
		file.close();
		goto failure;
	}

	file.close();

	if (perr != NULL) {
		*perr = MppBaseName(fname) + 
		    QString(": Scores read from disk");
	}

	return (retval);

failure:
	if (perr != NULL) {
		*perr = MppBaseName(fname) + 
		    QString(": Could not read scores from disk");
	}

	return (QString());
}

void
MppWriteFile(QString fname, QString text, QString *perr)
{
	QFile file(fname);

	if (!file.open(QIODevice::WriteOnly | QIODevice::Text |
	    QIODevice::Truncate))
		goto failure;

	file.write(text.toAscii());

	if (file.error()) {
		file.close();
		goto failure;
	}

	file.close();

	if (perr != NULL) {
		*perr = MppBaseName(fname) +
		    QString(": Scores written to disk");
	}
	return;

failure:

	if (perr) {
		*perr = MppBaseName(fname) +
		    QString(": Could not write scores to disk");
	}
	return;
}

uint8_t
MppReadRawFile(QString fname, QByteArray *pdata)
{
	QFile file(fname);

	if (!file.open(QIODevice::ReadOnly))
		goto failure;

	*pdata = file.readAll();

	if (file.error()) {
		file.close();
		goto failure;
	}

	file.close();

	return (0);

failure:
	return (1);
}

uint8_t
MppWriteRawFile(QString fname, QByteArray *pdata)
{
	QFile file(fname);

	if (!file.open(QIODevice::WriteOnly |
	    QIODevice::Truncate))
		goto failure;

	file.write(*pdata);

	if (file.error()) {
		file.close();
		goto failure;
	}

	file.close();

	return (0);

failure:
	return (1);
}

static void
mask_signal(int sig)
{
}

int
main(int argc, char **argv)
{
	QApplication app(argc, argv);

	signal(SIGPIPE, mask_signal);

	umidi20_init();

	umidi20_jack_init("midipp");

	MppMainWindow main;

	main.show();

	return (app.exec());
}
