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

#include "midipp.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <err.h>
#include <getopt.h>

#include "midipp_mainwindow.h"
#include "midipp_scores.h"

Mpp :: Mpp() :
  ColorBlack(0x00,0x00,0x00,0xff),
  ColorWhite(0xff,0xff,0xff,0xff),
  ColorGrey(0xc0,0xc0,0xc0,0xff),
  ColorLogo(0xc4,0x40,0x20,0xff),
  ColorGreen(0x40,0xc4,0x20,0xff)
{

}

Mpp :: ~Mpp()
{

}

Q_DECL_EXPORT class Mpp Mpp;

Q_DECL_EXPORT const QString
MppChanName(int channel, int other)
{
	if (channel == 9)
		return (QString("DRUMS"));

	if (channel >= 0 && channel <= 15)
		return (QString("CH::%1").arg(channel + 1));

	switch (other) {
	case MPP_CHAN_ANY:
		return (QString("CH::ANY"));
	case MPP_CHAN_NONE:
		return (QString("CH::NONE"));
	default:
		return (QString("CH::UNDEF"));
	}
}

Q_DECL_EXPORT QString
MppBaseName(const QString & fname)
{
	QFileInfo fi(fname);
	return (fi.fileName());
}

Q_DECL_EXPORT char *
MppQStringToAscii(const QString &s)
{
	QByteArray temp = s.toUtf8();
	int len = temp.size();
	char *data = temp.data();
	char *ptr;

	ptr = (char *)malloc(len + 1);
	if (ptr == NULL)
		return (NULL);

	memcpy(ptr, data, len);
	ptr[len] = 0;		/* NUL terminate */
	return (ptr);
}

Q_DECL_EXPORT const char *
MppBaseKeyToString(int key, int sharp)
{
	switch (key) {
	case A0:
		return ("A");
	case H0:
		return ("H");
	case C0:
		return ("C");
	case D0:
		return ("D");
	case E0:
		return ("E");
	case F0:
		return ("F");
	case G0:
		return ("G");
	case H0B:
		if (sharp)
			return ("A#");
		else
			return ("Hb");
	case A0B:
		if (sharp)
			return ("G#");
		else
			return ("Ab");
	case G0B:
		if (sharp)
			return ("F#");
		else
			return ("Gb");
	case E0B:
		if (sharp)
			return ("D#");
		else
			return ("Eb");
	case D0B:
		if (sharp)
			return ("C#");
		else
			return ("Db");
	default:
		return ("??");
	}
}

Q_DECL_EXPORT QString
MppReadFile(const QString &fname)
{
	QFile file(fname);
	QString retval;
	QMessageBox box;

	if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
		goto error;

	retval = QString::fromUtf8(file.readAll());
	if (file.error()) {
		file.close();
		goto error;
	}

	file.close();
	return (retval);

error:
	box.setText(QObject::tr("Could not read from file!"));
	box.setStandardButtons(QMessageBox::Ok);
	box.setIcon(QMessageBox::Critical);
	box.setWindowIcon(QIcon(QString(MPP_ICON_FILE)));
	box.exec();
	return (QString());
}

Q_DECL_EXPORT void
MppWriteFile(const QString &fname, QString text)
{
	QFile file(fname);
	QMessageBox box;

	if (!file.open(QIODevice::WriteOnly | QIODevice::Text |
	    QIODevice::Truncate)) {
		goto error;
	}

	file.write(text.toUtf8());

	if (file.error()) {
		file.close();
		goto error;
	}

	file.close();
	return;

error:
	box.setText(QObject::tr("Could not write to file!"));
	box.setStandardButtons(QMessageBox::Ok);
	box.setIcon(QMessageBox::Critical);
	box.setWindowIcon(QIcon(QString(MPP_ICON_FILE)));
	box.exec();
}

Q_DECL_EXPORT uint8_t
MppReadRawFile(const QString &fname, QByteArray *pdata)
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

Q_DECL_EXPORT uint8_t
MppWriteRawFile(const QString &fname, QByteArray *pdata)
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

#ifdef HAVE_SCREENSHOT
Q_DECL_EXPORT void
MppScreenShot(QWidget *widget, QApplication &app)
{
	static uint32_t counter;

	/* make sure drawing and resizing is complete */
	app.processEvents();

	QString fname;
	fname.sprintf("screenshot%03d.png", counter++);
	QPixmap pixmap(widget->size());
	widget->render(&pixmap);
	pixmap.save(fname, "PNG");
}
#endif

static const char *mpp_input_file;
static int mpp_pdf_print;

static void
usage(void)
{
	fprintf(stderr, "midipp [-f <score_file.txt>] [-p show_print]\n");
	exit(1);
}

static const struct option midipp_opts[] = {
	{ "NSDocumentRevisionsDebugMode", required_argument, NULL, ' ' },
	{ NULL, 0, NULL, 0 }
};

Q_DECL_EXPORT int
main(int argc, char **argv)
{
	int c;

	/* must be first, before any threads are created */
	signal(SIGPIPE, SIG_IGN);

	QApplication app(argc, argv);

	/* set consistent double click interval */
	app.setDoubleClickInterval(250);

	Mpp.HomeDirMid = QDir::homePath();
	Mpp.HomeDirTxt = QDir::homePath();
	Mpp.HomeDirGp3 = QDir::homePath();
	Mpp.HomeDirBackground = QDir::homePath();

	while ((c = getopt_long_only(argc, argv, "f:ph", midipp_opts, NULL)) != -1) {
		switch (c) {
		case 'f':
			mpp_input_file = optarg;
			break;
		case 'p':
			mpp_pdf_print = 1;
			break;
		case ' ':
			/* ignore */
			break;
		default:
			usage();
			break;
		}
	}

	umidi20_init();

	c = umidi20_jack_init("midipp");

	if (c != 0 && c != -2 && mpp_pdf_print == 0) {
		QMessageBox box;

		box.setText(QObject::tr("Could not connect to "
		    "the JACK subsystem!"));
		box.setStandardButtons(QMessageBox::Ok);
		box.setIcon(QMessageBox::Critical);
		box.setWindowIcon(QIcon(QString(MPP_ICON_FILE)));
		box.exec();
	}

	c = umidi20_coremidi_init("midipp");

	if (c != 0 && c != -2 && mpp_pdf_print == 0) {
		QMessageBox box;

		box.setText(QObject::tr("Could not connect to "
		    "the COREMIDI subsystem!"));
		box.setStandardButtons(QMessageBox::Ok);
		box.setIcon(QMessageBox::Critical);
		box.setWindowIcon(QIcon(QString(MPP_ICON_FILE)));
		box.exec();
	}

	MppScoreVariantInit();

	MppMainWindow *pmain = new MppMainWindow();

	if (mpp_input_file != NULL) {
		if (pmain->scores_main[0]->handleScoreFileOpenSub(
		    QString(mpp_input_file)) != 0) {
			errx(1, "Could not open file '%s'", mpp_input_file);
		}
	}

	if (mpp_pdf_print) {
		pmain->scores_main[0]->handleScorePrint();
		exit(0);
	} else {
		/* show configuration window by default */
		if (strcmp(pmain->deviceName[0], "X:") == 0) {
			if (pmain->handle_config_dev(0, 1) == QDialog::Accepted) {
				pmain->handle_config_apply(0);
			}
		}
		pmain->show();
#ifdef HAVE_SCREENSHOT
		pmain->ScreenShot(app);
#endif
	}
	return (app.exec());
}
