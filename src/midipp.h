/*-
 * Copyright (c) 2009-2020 Hans Petter Selasky. All rights reserved.
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

#ifndef _MIDIPP_H_
#define	_MIDIPP_H_

#include <stdint.h>
#include <sys/param.h>

#include <err.h>

#include <QApplication>
#include <QDialog>
#include <QPushButton>
#include <QGridLayout>
#include <QTextEdit>
#include <QScrollBar>
#include <QPlainTextEdit>
#include <QTabBar>
#include <QStackedWidget>
#include <QLabel>
#include <QSpinBox>
#include <QTextCursor>
#include <QTimer>
#include <QKeyEvent>
#include <QWheelEvent>
#include <QFile>
#include <QFileDialog>
#include <QLineEdit>
#include <QSpacerItem>
#include <QLCDNumber>
#include <QPicture>
#include <QColor>
#include <QPaintEvent>
#include <QPainter>
#include <QFont>
#ifdef HAVE_PRINTER
#include <QPrintDialog>
#include <QPrinter>
#endif
#include <QFontDialog>
#include <QFontInfo>
#include <QFileInfo>
#include <QMouseEvent>
#include <QSizePolicy>
#include <QListWidget>
#include <QIcon>
#include <QCloseEvent>
#include <QMenu>
#include <QMessageBox>
#include <QDir>
#include <QGroupBox>
#include <QSettings>
#include <QStyle>
#include <QSlider>
#include <QtNetwork>
#include <QUrl>
#include <QWheelEvent>
#include <QColorDialog>
#include <QPoint>
#include <QSplitter>
#include <QThread>    
#include <QRegExp>
#include <QByteArray>
#include <QChar>
#include <QString>
#include <QClipboard>
#include <QXmlStreamReader>

#include <umidi20.h>

#ifdef __APPLE__
#include "TargetConditionals.h"
#endif

#define	MPP_MAX_KEYS (128 * 128 * 128 * 128)	/* maximum number of keys */
#define	MPP_MAX_BANDS (12 * 128 * 128 * 128)	/* number of divisions per octave */
#define	MPP_MAX_CHORD_BANDS	192
#define	MPP_MAX_CHORD_MAP	(3 * 12 + 2)
#define	MPP_MAX_CHORD_FUTURE	12
#define	MPP_MAX_BUTTON_MAP	16
#define	MPP_MAX_VIEWS	2
#define	MPP_MAX_TRACKS		(MPP_TRACKS_PER_VIEW * MPP_MAX_VIEWS)
#define	MPP_MAX_LINES	8192
#define	MPP_MAX_SCORES	32
#define	MPP_MAX_LABELS	32
#define	MPP_MAX_QUEUE	32
#define	MPP_MAX_DEVS	8
#define	MPP_MAX_BPM	32
#define	MPP_MAX_LBUTTON	16
#define	MPP_MIN_POS	4	/* ticks */
#define	MPP_WHEEL_STEP	(8 * 15)
#define	MPP_PRESSED_MAX	128
#define	MPP_MAX_DURATION 255	/* inclusive */
#define	MPP_MAGIC_DEVNO	(UMIDI20_N_DEVICES - MPP_MAX_TRACKS)
#define	MPP_DEFAULT_URL "http://home.selasky.org/midipp/database.tar.gz"
#define	MPP_DEFAULT_CMD_KEY C3
#define	MPP_DEFAULT_BASE_KEY ((MPP_C0 + 12 * 5) * MPP_BAND_STEP_12)
#define	MPP_VISUAL_MARGIN	8
#define	MPP_VISUAL_R_MAX	8
#define	MPP_VISUAL_C_MAX	20
#define	MPP_VOLUME_UNIT		127
#define	MPP_VOLUME_MAX		511	/* inclusivly */
#define	MPP_CUSTOM_MAX		10
#define	MPP_LOOP_MAX		16
#define	MPP_MAX_TABS		32
#define	MPP_MAX_WIDGETS		32
#define	MPP_PIANO_TAB_LABELS	10	/* hard coded */
#define	MPP_POPUP_DELAY		2000	/* ms */

#define	MPP_CHAN_NONE_MASK	(1 << 1)
#define	MPP_CHAN_ANY_MASK	(1 << 2)
#define	MPP_CHAN_MPE_MASK	(1 << 3)
#define	MPP_CHAN_NONE		-1
#define	MPP_CHAN_ANY		-2
#define	MPP_CHAN_MPE 		-3	/* multi polyphonic expression */
#define	MPP_DEV_ALL		1
#define	MPP_DEV_NONE		2

/* list of tracks supported */
#define	MPP_DEFAULT_TRACK(n) ((n) * MPP_TRACKS_PER_VIEW + 0)
#define	MPP_TREBLE_TRACK(n) ((n) * MPP_TRACKS_PER_VIEW + 1)
#define	MPP_BASS_TRACK(n) ((n) * MPP_TRACKS_PER_VIEW + 2)

#define	MPP_BLOCKED(obj, what) do {		\
    (obj)->blockSignals(1);			\
    (obj)->what;				\
    (obj)->blockSignals(0);			\
} while (0)

#define	MPP_TRACKS_PER_VIEW 3

#if (MPP_MAGIC_DEVNO < MPP_MAX_DEVS)
#error "UMIDI20_N_DEVICES is too small."
#endif

/* list of supported band steps */
#define	MPP_BAND_STEP_12 (MPP_MAX_BANDS / 12)
#define	MPP_BAND_STEP_24 (MPP_MAX_BANDS / 24)
#define	MPP_BAND_STEP_48 (MPP_MAX_BANDS / 48)
#define	MPP_BAND_STEP_96 (MPP_MAX_BANDS / 96)
#define	MPP_BAND_STEP_192 (MPP_MAX_BANDS / 192)
#define	MPP_BAND_STEP_CHORD (MPP_MAX_BANDS / MPP_MAX_CHORD_BANDS)

static inline int
MPP_BAND_REM(int x, int y)
{
	x = (MPP_MAX_BANDS + (x % MPP_MAX_BANDS)) % MPP_MAX_BANDS;
	return (x / (MPP_MAX_BANDS / y));
}

static inline int
MPP_SUBDIV_REM_BITREV(int x)
{
	int x_mask = MPP_BAND_STEP_12 / 2;
	int r_mask = 1;
	int retval = 0;

	while (r_mask != MPP_BAND_STEP_12) {
		if (x & x_mask)
			retval |= r_mask;
		r_mask <<= 1;
		x_mask >>= 1;
	}
	return (retval);
}

static inline int
MPP_BAND_REM_TO_KEY(int x)
{
	int x_mask = 4 * MPP_BAND_STEP_12 / 2;
	int r_mask = 1;
	int retval = 0;

	while (r_mask != (4 * MPP_BAND_STEP_12)) {
		if (x & x_mask)
			retval |= r_mask;
		r_mask <<= 1;
		x_mask >>= 1;
	}

	retval *= 3;

	if (x & (4 * MPP_BAND_STEP_12))
		retval += 1;
	if (x & (8 * MPP_BAND_STEP_12))
		retval += 2;

	return (retval);
}

static inline int
MPP_KEY_TO_BAND_REM(int x)
{
	int x_mask = 4 * MPP_BAND_STEP_12 / 2;
	int r_mask = 1;
	int retval;

	retval = (x % 3) * (MPP_MAX_BANDS / 3);

	x /= 3;

	while (r_mask != (4 * MPP_BAND_STEP_12)) {
		if (x & x_mask)
			retval += r_mask;
		r_mask <<= 1;
		x_mask >>= 1;
	}
	return (retval);
}

#define	MPP_KEY_MIN (1 << 31)

/* list of basic 12-step score values */
#define	MPP_C0 0
#define	MPP_D0B 1
#define	MPP_D0 2
#define	MPP_E0B 3
#define	MPP_E0 4
#define	MPP_F0 5
#define	MPP_G0B 6
#define	MPP_G0 7
#define	MPP_A0B 8
#define	MPP_A0 9
#define	MPP_H0B 10
#define	MPP_H0 11

#define	STRLCPY(a,b,c) do { \
    strncpy(a,b,c); ((char *)(a))[(c)-1] = 0; \
} while (0)

class MppBpm;
class MppButton;
class MppButtonMap;
class MppChanSel;
class MppChanSelDiag;
class MppCheckBox;
class MppCustomTab;
class MppDataBase;
class MppDecodeTab;
class MppDevices;
class MppDevSel;
class MppDevSelDiag;
class MppElement;
class MppGPro;
class MppGridLayout;
class MppGroupBox;
class MppHead;
class MppImportTab;
class MppInstrumentTab;
class MppLoopTab;
class MppMainWindow;
class MppMidi;
class MppMetronome;
class MppMode;
class MppPianoTab;
class MppReplace;
class MppReplayTab;
class MppScoreHighLighter;
class MppScoreMain;
class MppScoreView;
class MppSettings;
class MppSettingsWhat;
class MppSheet;
class MppShortcutTab;
class MppShowControl;
class MppShowWidget;
class MppSpinBox;
class MppTabBar;
class MppVolume;

struct MppChord;
struct MppChordElement;

typedef struct MppChord MppChord_t;

class QPrinter;

struct MppScoreEntry {
	int key;
	int dur;
	int8_t track;
	int8_t trackSec;
	int8_t channel;
	int8_t channelSec;
};

struct MppInstr {
	uint16_t bank;
	uint8_t prog;
	uint8_t muted;
	uint8_t updated;
};

struct MppVisualDot {
	qreal x_off;
	qreal y_off;
};

struct MppVisualScore {
	QPicture *pic;
	QString *str;
	QString *str_chord;
	struct MppVisualDot *pdot;
	MppElement *start;
	MppElement *stop;
	int newpage;
	int ndot;
};

class Mpp {
public:
	Mpp();
	~Mpp();

	QColor ColorBlack;
	QColor ColorWhite;
	QColor ColorGrey;
	QColor ColorLogo;
	QColor ColorGreen;
	QColor ColorLight;
	QColor ColorYellow;

	QString VariantList;
	QString HomeDirTxt;
	QString HomeDirMid;
	QString HomeDirGp3;
	QString HomeDirMXML;
	QString HomeDirBackground;

	int KeyAdjust[12];
};

class MppSleep : public QThread
{
public:
    static void usleep(unsigned long usecs) { QThread::usleep(usecs); }
    static void msleep(unsigned long msecs) { QThread::msleep(msecs); }
    static void sleep(unsigned long secs) { QThread::sleep(secs); }
};

extern Mpp Mpp;

extern const QString MppChanName(int);
extern const QString MppDevName(int, int = 0);
extern QString MppBaseName(const QString &);
extern char *MppQStringToAscii(const QString &);
extern QString MppReadFile(const QString &);
extern void MppWriteFile(const QString &, QString text);
extern uint8_t MppReadRawFile(const QString &, QByteArray *pdata);
extern uint8_t MppWriteRawFile(const QString &, QByteArray *pdata);
extern const char *MppBaseKeyToString(int key, int sharp);
extern void MppScoreVariantInit(void);
extern uint8_t MppIsLabel(const QString &);
extern void MppSplitBaseTreble(const int *, uint8_t, int *, uint8_t *, int *, uint8_t *);
extern const QString MppChordModChars;
extern const QString MppVersion;
extern const QString MppIconFile;

typedef int (MppCmp_t)(void *, const void *, const void *);
extern void MppSort(void *, size_t, size_t, MppCmp_t *, void *);
extern void MppSort(int *, size_t);
extern void MppTrans(int *ptr, size_t num, int ntrans);

#ifdef HAVE_SCREENSHOT
extern void MppScreenShot(QWidget *, QApplication &);
#endif

#endif	/* _MIDIPP_H_ */
