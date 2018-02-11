/*-
 * Copyright (c) 2009-2018 Hans Petter Selasky. All rights reserved.
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

#ifdef __ANDROID__
#include <qpa/qplatformnativeinterface.h>
#endif

Mpp :: Mpp() :
  ColorBlack(0x00,0x00,0x00,0xff),
  ColorWhite(0xff,0xff,0xff,0xff),
  ColorGrey(0xc0,0xc0,0xc0,0xff),
  ColorLogo(0xc4,0x40,0x20,0xff),
  ColorGreen(0x40,0xc4,0x20,0xff),
  ColorLight(0x80,0x80,0x80,0xff),
  ColorYellow(0xc0,0xc0,0x00,0xff)
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

Q_DECL_EXPORT const QString
MppDevName(int device, int other)
{
	if (device >= 0 && device < MPP_MAX_DEVS)
		return (QString("DEV::%1").arg(device + 1));

	switch (other) {
	case MPP_DEV_ALL:
		return (QString("DEV::ALL"));
	case MPP_DEV_NONE:
		return (QString("DEV::NONE"));
	default:
		return (QString("DEV::UNDEF"));
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

Q_DECL_EXPORT uint8_t
MppIsLabel(const QString &str)
{
	return ((str.size() > 1) && (str[0] == 'L') && str[1].isDigit());
}

Q_DECL_EXPORT void
MppSplitBaseTreble(const int *score, uint8_t num, int *base, uint8_t *nbase,
    int *treble, uint8_t *ntreble)
{
	uint8_t stats[MPP_MAX_BANDS] = {};
	uint8_t count[MPP_MAX_BANDS] = {};
	uint8_t x;
	int key;
	uint8_t nb = 0;
	uint8_t nt = 0;

	for (x = 0; x != num; x++)
		stats[MPP_BAND_REM(score[x])]++;

	/*
	 * Treble only: C0
	 * Bass only: C0 C1 or C0 C0
	 * Bass and Treble: C0 C1 C2 or C0 C0 C1
	 */
	for (x = 0; x != num; x++) {
		uint32_t rem;

		key = score[x];
		rem = MPP_BAND_REM(key);

		if (stats[rem] == 1) {
			treble[nt++] = key;
		} else if (count[rem]++ < 2) {
			base[nb++] = key;
		} else {
			treble[nt++] = key;
		}
	}
	*nbase = nb;
	*ntreble = nt;
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
	box.setWindowIcon(QIcon(MppIconFile));
	box.setWindowTitle(MppVersion);
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
	box.setWindowIcon(QIcon(MppIconFile));
	box.setWindowTitle(MppVersion);
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

static size_t
MppSortIndex(size_t t)
{
	t ^= t >> 1;
	t ^= t >> 2;
	t ^= t >> 4;
	t ^= t >> 8;
	t ^= t >> 16;
	if (sizeof(t) > 4)
		t ^= t >> ((sizeof(t) > 4) ? 32 : 0);
	return (t);
}

static int
MppSortXform(void **ptr, size_t n, size_t lim, MppCmp_t *fn, void *arg)
{
#define	MPP_XSORT_TABLE_MAX (1 << 4)
	size_t x, y, z;
	unsigned t, u, v;
	size_t p[MPP_XSORT_TABLE_MAX];
	int retval = 0;

	x = n;
	while (1) {
		/* optimise */
		if (x >= MPP_XSORT_TABLE_MAX)
			v = MPP_XSORT_TABLE_MAX;
		else if (x >= 2)
			v = x;
		else
			break;

		/* divide down */
		x /= v;

		/* generate ramp table */
		for (t = 0; t != v; t++)
			p[t] = MppSortIndex(x * (t ^ (t / 2)));

		/* bitonic sort */
		for (y = 0; y != n; y += (v * x)) {
			for (z = 0; z != x; z++) {
				size_t w = y + z;

				/* insertion sort */
				for (t = 1; t != v; t++) {
					/* check for arrays which are not power of two */
					if ((w ^ p[t]) >= lim)
						break;
					for (u = t; u != 0; u--) {
						void **pa = ptr + (w ^ p[u - 1]);
						void **pb = ptr + (w ^ p[u]);

						if (fn(arg, pa, pb) > 0) {
							void *temp;
							temp = *pa;
							*pa = *pb;
							*pb = temp;
							retval = 1;
						} else {
							break;
						}
					}
				}
			}
		}
	}
	return (retval);
}

Q_DECL_EXPORT void
MppSort(void **ptr, size_t n, MppCmp_t *fn, void *arg)
{
	size_t max;

	if (n <= 1)
		return;

	for (max = 1; max < n; max <<= 1)
		;

	while (MppSortXform(ptr, max, n, fn, arg))
		;
}

Q_DECL_EXPORT void
MppSort(int *ptr, size_t num)
{
	size_t i, j;

	for (i = 0; i != num; i++) {
		for (j = i + 1; j != num; j++) {
			if (ptr[i] > ptr[j]) {
				int temp = ptr[i];
				ptr[i] = ptr[j];
				ptr[j] = temp;
			}
		}
	}
}

Q_DECL_EXPORT void
MppTrans(int *ptr, size_t num, int ntrans)
{
	if (num == 0)
		return;

	MppSort(ptr, num);

	if (ntrans < 0) {
		while (ntrans++) {
			ptr[num - 1] -= MPP_MAX_BANDS;
			MppSort(ptr, num);
		}
	} else if (ntrans > 0) {
		while (ntrans--) {
			ptr[0] += MPP_MAX_BANDS;
			MppSort(ptr, num);
		}
	}
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

const QString MppVersion("MIDI Player Pro v2.0.0");
const QString MppIconFile(":/midipp.png");

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
	Mpp.HomeDirMXML = QDir::homePath();
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
		box.setWindowIcon(QIcon(MppIconFile));
		box.setWindowTitle(MppVersion);
		box.exec();
	}

	c = umidi20_coremidi_init("midipp");

	if (c != 0 && c != -2 && mpp_pdf_print == 0) {
		QMessageBox box;

		box.setText(QObject::tr("Could not connect to "
		    "the COREMIDI subsystem!"));
		box.setStandardButtons(QMessageBox::Ok);
		box.setIcon(QMessageBox::Critical);
		box.setWindowIcon(QIcon(MppIconFile));
		box.setWindowTitle(MppVersion);
		box.exec();
	}

#ifdef __ANDROID__
	QPlatformNativeInterface *interface = app.platformNativeInterface();

	c = umidi20_android_init("midipp",
	    interface->nativeResourceForIntegration("QtActivity"));

	if (c != 0 && c != -2 && mpp_pdf_print == 0) {
		QMessageBox box;

		box.setText(QObject::tr("Could not connect to "
		    "the Android MIDI subsystem!"));
		box.setStandardButtons(QMessageBox::Ok);
		box.setIcon(QMessageBox::Critical);
		box.setWindowIcon(QIcon(MppIconFile));
		box.setWindowTitle(MppVersion);
		box.exec();
	}
#endif

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
