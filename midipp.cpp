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

#include <sys/stdint.h>
#include <sys/param.h>

#include <QApplication>

#include <midipp.h>

#include <err.h>

QColor color_black   (0x00, 0x00, 0x00, 0xff);
QColor color_white   (0xff, 0xff, 0xff, 0xff);
QColor color_logo    (0xc4, 0x40, 0x20, 0xff);
QColor color_green   (0x40, 0xc4, 0x20, 0xff);

static void MppTimerCallback(void *arg);

static QString
MppBaseName(QString fname)
{
	QFileInfo fi(fname);
	return (fi.fileName());
}

static char *
MppQStringToAscii(QString s)
{
        QChar *ch;
	char *ptr;
	int len;
	int c;

	ch = s.data();
	if (ch == NULL)
		return (NULL);

	len = 1;

	while (1) {
		c = ch->toAscii();
		if (c == 0)
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
		ptr[len] = c;
		if (c == 0)
			break;
		len++;
		ch++;
	}
	return (ptr);
}

static QString
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

static void
MppWriteFile(QString fname, QString text, QString *perr)
{
	QFile file(fname);

	if (!file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate))
		goto failure;

	file.write(text.toAscii());

	if (file.error()) {
		file.close();
		goto failure;
	}

	file.close();

	if (perr != NULL)
		*perr = MppBaseName(fname) + QString(": Scores written to disk");
	return;

failure:

	if (perr)
		*perr = MppBaseName(fname) + QString(": Could not write scores to disk");
	return;
}

static void
MppParseMax(uint16_t *pmax, float value)
{
	value += MPP_VISUAL_MARGIN;

	if (value > 2048.0)
		value = 2048.0;
	if (value < 0.0)
		value = 0.0;
	if ((uint16_t)value > *pmax)
		*pmax = value;
}

static uint8_t
MppNoise(struct MppSoftc *sc, uint8_t factor)
{
	uint32_t temp;
	const uint32_t prime = 0xFFFF1D;

	if (factor == 0)
		return (0);

	if (sc->ScNoiseRem & 1)
		sc->ScNoiseRem += prime;

	sc->ScNoiseRem /= 2;

	temp = sc->ScNoiseRem * factor;

	return (temp >> 24);
}

static void
MppParseVisualEntries(struct MppSoftc *sc, QPrinter *pd,
    QPoint orig, float scale_f)
{
	QPainter paint;
	QPicture *pic;
	uint16_t x;
	uint16_t y;
	uint16_t z;
	uint16_t y_max;
	uint16_t x_max;
	char *ptr;
	uint8_t draw_chord;
	uint8_t last_dot;
	uint8_t last_jump = 0;

	float chord_x;
	float text_x;
	float adj_x;
	float scale_min;

	QFont fnt_a;
	QFont fnt_b;

	pic = NULL;

	fnt_a = *(sc->ScFont);
	fnt_a.setPixelSize(20);

	fnt_b = *(sc->ScFont);
	fnt_b.setPixelSize(24);

	sc->ScMaxScoresWidth = 0;

	for (x = 0; x != MPP_MAX_LINES; x++) {
		sc->ScVisualScores[x].x_off = 0;
	}

	if (pd != NULL) {

		/* count number of lines */
		for (x = y_max = y = 0; x != MPP_MAX_LINES; x++) {

			if (sc->ScPageNext[x] != 0) {
				if (y > y_max)
					y_max = y;
				y = 0;
				last_jump = 0;
			} else if (sc->ScJumpNext[x] != 0) {
				y++;
				last_jump = 1;
			}

			ptr = sc->ScVisualScores[x].pstr;
			if (ptr == NULL)
				continue;

			y++;
			last_jump = 0;
		}

		y -= last_jump;

		if (y > y_max)
			y_max = y;

		if (y_max != 0) {
			scale_min = ((float)(pd->height() - (2*orig.y()))) / 
			    (((float)y_max) * MPP_VISUAL_Y_MAX);

			if (scale_min < 0)
				scale_f = 0.5;	/* dummy */
			else if (scale_min < scale_f)
				scale_f = scale_min;
		}

		paint.begin(pd);
		paint.translate(orig);
		paint.scale(scale_f, scale_f);
		paint.translate(QPoint(0,-MPP_VISUAL_Y_MAX));
	}

	y_max = 0;

	for (x = 0; x != MPP_MAX_LINES; x++) {

		if (pd == NULL) {

			if (sc->ScVisualScores[x].pic != NULL) {
				delete (sc->ScVisualScores[x].pic);
				sc->ScVisualScores[x].pic = NULL;
			}
		} else {

			if (sc->ScPageNext[x] != 0) {
				pd->newPage();
				while (y_max--)
					paint.translate(QPoint(0, -MPP_VISUAL_Y_MAX));
				y_max = 0;
			} else if (sc->ScJumpNext[x] != 0) {
				paint.translate(QPoint(0,MPP_VISUAL_Y_MAX));
				y_max++;
			}
		}

		ptr = sc->ScVisualScores[x].pstr;
		if (ptr == NULL)
			continue;

		if (pd == NULL) {
			pic = new QPicture();
			paint.begin(pic);
		} else {
			paint.translate(QPoint(0,MPP_VISUAL_Y_MAX));
			y_max++;
		}

		draw_chord = 0;
		last_dot = 0;
		chord_x = MPP_VISUAL_MARGIN;
		text_x = MPP_VISUAL_MARGIN;
		z = x;
		x_max = 0;

		for (y = 0; ptr[y] != 0; y++) {

			if (draw_chord)
				paint.setFont(fnt_b);
			else
				paint.setFont(fnt_a);

			QString temp(ptr[y]);
			QRectF temp_size = 
			    paint.boundingRect(QRectF(0,0,0,0), temp);

			if (temp_size.width() == 0.0) {
				temp_size = 
				    paint.boundingRect(
				    QRectF(0,0,0,0), QString("-"));
			}

			if (ptr[y] == '(') {
				draw_chord = 1;
				continue;
			} else if (ptr[y] == ')') {
				draw_chord = 0;
				continue;
			} else if (ptr[y] == '.') {
				paint.setPen(QPen(color_black, 1));
				paint.setBrush(QColor(color_black));

				if (last_dot) {
					text_x += MPP_VISUAL_R_MAX;
				}

				if (ptr[y+1] != 0) {
					adj_x = paint.boundingRect(QRectF(0,0,0,0),
					    QString(ptr[y+1])).width();
					if (adj_x == 0) {
						paint.boundingRect(QRectF(0,0,0,0),
						   QString("-")).width();
					}
					adj_x = (adj_x - MPP_VISUAL_R_MAX) / 2.0;
				} else {
					adj_x = 0.0;
				}

				if (z < MPP_MAX_LINES) {

					sc->ScVisualScores[z].x_off = text_x + adj_x;
					sc->ScVisualScores[z].y_off = (MPP_VISUAL_Y_MAX/3);

					MppParseMax(&sc->ScMaxScoresWidth, 
					    sc->ScVisualScores[z].x_off + MPP_VISUAL_R_MAX);

					MppParseMax(&x_max, 
					    sc->ScVisualScores[z].x_off + MPP_VISUAL_R_MAX);

					paint.drawEllipse(QRect(sc->ScVisualScores[z].x_off,
						sc->ScVisualScores[z].y_off,
						MPP_VISUAL_R_MAX, MPP_VISUAL_R_MAX));
				}

				last_dot = 1;
				z++;
				continue;
			} else if (ptr[y] == '\n' || ptr[y] == '\r') {
				continue;
			}

			paint.setPen(QPen(color_black, 1));
			paint.setBrush(QColor(color_black));

			if (draw_chord) {
				paint.drawText(QPointF(chord_x, MPP_VISUAL_MARGIN + 
				    (MPP_VISUAL_Y_MAX/6)), temp);

				chord_x += temp_size.width();

				MppParseMax(&sc->ScMaxScoresWidth, chord_x);

				MppParseMax(&x_max, chord_x);
			} else {
				paint.drawText(QPointF(text_x, MPP_VISUAL_Y_MAX -
				    (MPP_VISUAL_Y_MAX/4) - MPP_VISUAL_MARGIN), temp);

				text_x += temp_size.width();
				chord_x = text_x;
				last_dot = 0;

				MppParseMax(&sc->ScMaxScoresWidth, text_x);

				MppParseMax(&x_max, text_x);
			}
		}

		if (pd == NULL) {
			int w;
			int h;

			w = x_max;
			h = MPP_VISUAL_Y_MAX;

			paint.end();

			if (w && h) {
				sc->ScVisualScores[x].pic = new QPixmap(w, h);

				paint.begin(sc->ScVisualScores[x].pic);
				paint.setRenderHints(QPainter::Antialiasing, 1);
				paint.setPen(QPen(color_white, 0));
				paint.setBrush(QColor(color_white));
				paint.drawRect(QRect(0,0, w, h));
				paint.drawPicture(0,0, *pic);
				paint.end();
			}

			delete (pic);

			pic = NULL;
		}
	}

	if (pd != NULL)
		paint.end();
}

MppVisualScores :: MppVisualScores(struct MppSoftc *sc_init)
{
	sc = sc_init;
}

void
MppVisualScores :: paintEvent(QPaintEvent *event)
{
	QPainter paint(this);
	uint16_t max;
	uint16_t pos;
	uint16_t opos;
	uint16_t y_blocks;
	uint16_t y_div;
	uint16_t y_rem;
	uint16_t yo_div;
	uint16_t yo_rem;
	uint16_t x;
	uint16_t y;

	paint.fillRect(event->rect(), color_white);

	pthread_mutex_lock(&sc->mtx);
	pos = sc->ScCurrPos;
	opos = sc->ScLastPos;
	max = sc->ScLinesMax;
	pthread_mutex_unlock(&sc->mtx);

	y_blocks = (this->height() / MPP_VISUAL_Y_MAX);
	if (y_blocks == 0)
		y_blocks = 1;
	y_div = 0;
	y_rem = 0;
	yo_div = 0;
	yo_rem = 0;

	/* locate */

	for (x = y = 0; x != max; x++) {

		if (sc->ScVisualScores[x].pic != NULL) {
			y++;
		}

		if (x >= pos) {
			if (y != 0) {
				y_div = (y-1) / y_blocks;
				y_rem = (y-1) % y_blocks;
				break;
			}
		}
	}

	for (x = y = 0; x != max; x++) {

		if (sc->ScVisualScores[x].pic != NULL) {
			y++;
		}

		if (x >= opos) {
			if (y != 0) {
				yo_div = (y-1) / y_blocks;
				yo_rem = (y-1) % y_blocks;
				break;
			}
		}
	}

	/* paint */

	for (x = y = 0; x != max; x++) {

		if (sc->ScVisualScores[x].pic != NULL) {
			if (y_div == (y / y_blocks)) {
				paint.drawPixmap(
				    QPoint(0, (y % y_blocks) * MPP_VISUAL_Y_MAX),
				    *(sc->ScVisualScores[x].pic));
			}
			y++;
		}
	}

	/* overlay (last) */

	if ((pos != opos) && (yo_div == y_div) &&
	    (sc->ScVisualScores[opos].x_off != 0)) {

		paint.setPen(QPen(color_green, 4));
		paint.setBrush(QColor(color_green));
		paint.drawEllipse(QRect(sc->ScVisualScores[opos].x_off,
			sc->ScVisualScores[opos].y_off + (yo_rem * MPP_VISUAL_Y_MAX),
			MPP_VISUAL_R_MAX, MPP_VISUAL_R_MAX));

	}

	/* overlay (current) */

	if (sc->ScVisualScores[pos].x_off != 0) {
		paint.setPen(QPen(color_logo, 4));
		paint.setBrush(QColor(color_logo));
		paint.drawEllipse(QRect(sc->ScVisualScores[pos].x_off,
			sc->ScVisualScores[pos].y_off + (y_rem * MPP_VISUAL_Y_MAX),
			MPP_VISUAL_R_MAX, MPP_VISUAL_R_MAX));
	}
}

static void
MppParseNewLine(struct MppSoftc *sc)
{
	if (sc->line < MPP_MAX_LINES) {
		sc->line++;
		sc->index = 0;
	}
}

static void
MppParseNewVisual(struct MppSoftc *sc)
{
	if (sc->buf_line < MPP_MAX_LINES) {

		sc->buf[sc->buf_index] = 0;

		if (sc->ScVisualScores[sc->buf_line].pstr != NULL)
			free(sc->ScVisualScores[sc->buf_line].pstr);
		sc->ScVisualScores[sc->buf_line].pstr = strdup(sc->buf);
		sc->buf_index = 0;
		sc->buf_line = sc->line;
	}
}

static void
MppParse(struct MppSoftc *sc, const QString &ps)
{
	int c;
	int d;
	int x;
	int y;
	int label;
	int channel;
	int base_key;
	int duration;
	int flag;

	/* cleanup all scores */

	for (x = 0; x != MPP_MAX_LINES; x++) {
		if (sc->ScVisualScores[x].pstr != NULL) {
			free(sc->ScVisualScores[x].pstr);
			sc->ScVisualScores[x].pstr = NULL;
		}
	}

	x = -1;
	sc->line = 0;
	sc->index = 0;
	sc->buf_index = 0;
	sc->buf_line = 0;
	memset(sc->ScScores, 0, sizeof(sc->ScScores));
	memset(sc->ScJumpNext, 0, sizeof(sc->ScJumpNext));
	memset(sc->ScJumpTable, 0, sizeof(sc->ScJumpTable));
	memset(sc->ScPageNext, 0, sizeof(sc->ScPageNext));

	if (ps.isNull() || ps.isEmpty())
		goto done;

next_line:
	if (sc->index != 0) {
		MppParseNewLine(sc);
	}

	y = -1;
	channel = 0;
	duration = 1;

next_char:
	x++;
	y++;

	c = ps[x].toUpper().toAscii();

	switch (c) {
	case 'C':
		if (y == 0) {
			base_key = C0;
			goto parse_score;
		}
		goto next_char;
	case 'D':
		if (y == 0) {
			base_key = D0;
			goto parse_score;
		}
		goto next_char;
	case 'E':
		if (y == 0) {
			base_key = E0;
			goto parse_score;
		}
		goto next_char;
	case 'F':
		if (y == 0) {
			base_key = F0;
			goto parse_score;
		}
		goto next_char;
	case 'G':
		if (y == 0) {
			base_key = G0;
			goto parse_score;
		}
		goto next_char;
	case 'A':
		if (y == 0) {
			base_key = A0;
			goto parse_score;
		}
		goto next_char;
	case 'B':
	case 'H':
		if (y == 0) {
			base_key = H0;
			goto parse_score;
		}
		goto next_char;
	case 'T':
		if (y == 0) {
			goto parse_channel;
		}
		goto next_char;
	case 'L':
		if (y == 0) {
			goto parse_label;
		}
		goto next_char;
	case 'J':
		if (y == 0) {
			goto parse_jump;
		}
		goto next_char;
	case 'U':
		if (y == 0) {
			goto parse_duration;
		}
		goto next_char;
	case 'S':
		if (y == 0) {
			goto parse_string;
		}
		goto next_char;
	case 0:
		goto done;
	case '\n':
		goto next_line;
	case '/':
		if (y != 0)
			goto next_char;

		/* check for comment */
		c = ps[x+1].toUpper().toAscii();
		if (c == '*') {
			while (1) {
				c = ps[x+2].toUpper().toAscii();
				if (c == '*') {
					c = ps[x+3].toUpper().toAscii();
					if (c == '/') {
						/* end of comment */
						x += 3;
						break;
					}
				}
				if (c == 0)
					goto done;
				x++;
			}
			y = -1;
		}
		goto next_char;

	case ' ':
	case '\t':
		y = -1;
	default:
		goto next_char;
	}

parse_score:
	c = ps[x+1].toAscii();
	if (c >= '0' && c <= '9') {
		d = ps[x+2].toAscii();
		if (d >= '0' && d <= '9') {
			base_key += 120 * (c - '0');
			base_key += 12 * (d - '0');
			x += 2;
		} else {
			base_key += 12 * (c - '0');
			x += 1;
		}
		c = ps[x+1].toUpper().toAscii();
		if (c == 'B') {
			base_key -= 1;
			x += 1;
		}
		if ((sc->line < MPP_MAX_LINES) && (sc->index < MPP_MAX_SCORES)) {
			sc->ScScores[sc->line][sc->index].key = base_key & 127;
			sc->ScScores[sc->line][sc->index].dur = duration & 255;
			sc->ScScores[sc->line][sc->index].channel = channel & 15;
			sc->index++;
		}

	}
	goto next_char;

parse_duration:
	c = ps[x+1].toAscii();
	if (c >= '0' && c <= '9') {
		d = ps[x+2].toAscii();
		if (d >= '0' && d <= '9') {
			duration = (10 * (c - '0')) + (d - '0');
			x += 2;
		} else {
			duration = (c - '0');
			x += 1;
		}
	} else {
		duration = 0;
	}
	goto next_char;

parse_channel:
	c = ps[x+1].toAscii();
	if (c >= '0' && c <= '9') {
		d = ps[x+2].toAscii();
		if (d >= '0' && d <= '9') {
			channel = (10 * (c - '0')) + (d - '0');
			x += 2;
		} else {
			channel = (c - '0');
			x += 1;
		}
	} else {
		channel = 0;
	}
	goto next_char;

parse_label:
	c = ps[x+1].toAscii();
	if (c >= '0' && c <= '9') {
		d = ps[x+2].toAscii();
		if (d >= '0' && d <= '9') {
			label = (10 * (c - '0')) + (d - '0');
			x += 2;
		} else {
			label = (c - '0');
			x += 1;
		}
	} else {
		label = 0;
	}
	if ((label >= 0) && (label < MPP_MAX_LABELS)) {
		sc->ScJumpTable[label] = sc->line + 1;
	}
	goto next_char;


parse_string:
	c = ps[x+1].toAscii();
	if (c != '\"')
		goto next_char;

	if (sc->buf_index == 0)
		sc->buf_line = sc->line;

	while ((c = ps[x+2].toAscii()) != 0) {
		if (c == '\"')
			break;
		else if (c == '\n' || c == ';')
			MppParseNewVisual(sc);
		else if (sc->buf_index == (sizeof(sc->buf) - 1))
			break;
		else {
			sc->buf[sc->buf_index] = c;
			sc->buf_index++;
		}
		x++;
	}
	x += 1;
	goto next_char;

parse_jump:
	c = ps[x+1].toAscii();

	flag = 0;

	if (c == 'P') {
		c = ps[x+2].toAscii();
		x++;
		flag |= 1;
	}

	if (c >= '0' && c <= '9') {
		d = ps[x+2].toAscii();
		if (d >= '0' && d <= '9') {
			label = (10 * (c - '0')) + (d - '0');
			x += 2;
		} else {
			label = (c - '0');
			x += 1;
		}
	} else {
		label = 0;
	}

	if (sc->index != 0) {
		MppParseNewLine(sc);
	}

	if (flag & 1) {
		if (sc->line < MPP_MAX_LINES)
			sc->ScPageNext[sc->line] = 1;
	}

	if ((label >= 0) && 
	    (label < MPP_MAX_LABELS) && 
	    (sc->line < MPP_MAX_LINES)) {
		sc->ScJumpNext[sc->line] = label + 1;
		sc->line++;
	}
	goto next_char;

done:
	if (sc->index != 0) {
		MppParseNewLine(sc);
	}

	if (sc->buf_index != 0) {
		MppParseNewVisual(sc);
	}

	/* resolve all jumps */
	for (x = 0; x != sc->line; x++) {
		if (sc->ScJumpNext[x] != 0)
			sc->ScJumpNext[x] = sc->ScJumpTable[sc->ScJumpNext[x] - 1];
	}

	sc->ScLinesMax = sc->line;
	sc->ScCurrPos = 0;
	sc->ScLastPos = 0;

	MppParseVisualEntries(sc, NULL, QPoint(0,0), 1.0);
}

MppMainWindow :: MppMainWindow(QWidget *parent)
  : QWidget(parent)
{
	int n;
	int x;

	/* set memory default */

	memset(&main_sc, 0, sizeof(main_sc));

	memset(&mid_data, 0, sizeof(mid_data));

	CurrScoreFileName = NULL;
	CurrMidiFileName = NULL;
	song = NULL;
	track = NULL;

	umidi20_mutex_init(&main_sc.mtx);

	main_sc.ScNoiseRem = 1;

	default_font.fromString(QString("Sans Serif,12,-1,5,75,0,0,0,0,0"));

	main_sc.ScFont = &default_font;

	/* Setup GUI */

	main_gl = new QGridLayout(this);
	main_tw = new QTabWidget();
	scores_tw = new QTabWidget();

	/* Watchdog */

	watchdog = new QTimer(this);
	connect(watchdog, SIGNAL(timeout()), this, SLOT(handle_watchdog()));

	/* Editor */

	main_edit = new QTextEdit();
	main_edit->setText(tr("/*\n"
		" * Copyright (c) 2009 Hans Petter Selasky. All rights reserved.\n"
		" */\n"

		"\n"
		"/*\n"
		" * Command syntax:\n"
		" * U<number> - specifies the duration of the following scores (0..255)\n"
		" * T<number> - specifies the track number of the following scores (0..31)\n"
		" * L<number> - defines a label (0..31)\n"
		" * J<number> - jumps to the given label (0..31)\n"
		" * JP<number> - jumps to the given label (0..31) and starts a new page\n"
		" * S\"<string>\" - creates a visual string\n"
		" * CDEFGAH<number><B> - defines a score in the given octave (0..10)\n"
		" */\n"
		"\n"
		"S\"(L0:) .Welcom.e!\"\n"
		"\nC3"
		"\nC3\n"));

	/* Tabs */

	main_gl->addWidget(scores_tw,0,0,1,1);
	main_gl->addWidget(main_tw,0,1,1,1);

	scores_wg = new MppVisualScores(&main_sc);

	tab_file_wg = new QWidget();
	tab_play_wg = new QWidget();
	tab_edit_wg = new QWidget();
	tab_config_wg = new QWidget();
	tab_instr_wg = new QWidget();
	tab_volume_wg = new QWidget();

	tab_file_gl = new QGridLayout(tab_file_wg);
	tab_play_gl = new QGridLayout(tab_play_wg);
	tab_edit_gl = new QGridLayout(tab_edit_wg);
	tab_config_gl = new QGridLayout(tab_config_wg);
	tab_instr_gl = new QGridLayout(tab_instr_wg);
	tab_volume_gl = new QGridLayout(tab_volume_wg);

	scores_tw->addTab(scores_wg, tr("View Scores"));
	scores_tw->addTab(main_edit, tr("Edit Scores"));

	main_tw->addTab(tab_file_wg, tr("File"));
	main_tw->addTab(tab_play_wg, tr("Play"));
#if 0
	main_tw->addTab(tab_edit_wg, tr("Edit"));
#endif
	main_tw->addTab(tab_config_wg, tr("Config"));
	main_tw->addTab(tab_instr_wg, tr("Instrument"));
	main_tw->addTab(tab_volume_wg, tr("Volume"));

	/* <File> Tab */

	lbl_score_file = new QLabel(tr("- Score File -"));
	lbl_score_file->setAlignment(Qt::AlignHCenter|Qt::AlignVCenter);

	lbl_midi_file = new QLabel(tr("- MIDI File -"));
	lbl_midi_file->setAlignment(Qt::AlignHCenter|Qt::AlignVCenter);

	lbl_file_status = new QLabel(QString());
	lbl_file_status->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);

	but_score_file_new = new QPushButton(tr("New"));
	but_score_file_open = new QPushButton(tr("Open"));
	but_score_file_save = new QPushButton(tr("Save"));
	but_score_file_save_as = new QPushButton(tr("Save As"));
	but_score_file_print = new QPushButton(tr("Print"));
	but_quit = new QPushButton(tr("Quit"));

	but_midi_file_new = new QPushButton(tr("New"));
	but_midi_file_open = new QPushButton(tr("Open"));
	but_midi_file_merge = new QPushButton(tr("Merge"));
	but_midi_file_save = new QPushButton(tr("Save"));
	but_midi_file_save_as = new QPushButton(tr("Save As"));

	n = 0;

	tab_file_gl->addWidget(lbl_score_file, n, 0, 1, 4);

	n++;

	tab_file_gl->addWidget(but_score_file_new, n, 0, 1, 4);

	n++;

	tab_file_gl->addWidget(but_score_file_open, n, 0, 1, 4);

	n++;

	tab_file_gl->addWidget(but_score_file_save, n, 0, 1, 4);

	n++;

	tab_file_gl->addWidget(but_score_file_save_as, n, 0, 1, 4);

	n++;

	tab_file_gl->addWidget(but_score_file_print, n, 0, 1, 4);

	n++;

	tab_file_gl->addWidget(lbl_file_status, n, 0, 1, 8);

	n++;

	tab_file_gl->setRowStretch(n, 3);

	n++;

	tab_file_gl->addWidget(but_quit, n, 0, 1, 8);

	n = 0;

	tab_file_gl->addWidget(lbl_midi_file, n, 4, 1, 4);

	n++;

	tab_file_gl->addWidget(but_midi_file_new, n, 4, 1, 4);

	n++;

	tab_file_gl->addWidget(but_midi_file_open, n, 4, 1, 4);

	n++;

	tab_file_gl->addWidget(but_midi_file_merge, n, 4, 1, 4);

	n++;

	tab_file_gl->addWidget(but_midi_file_save, n, 4, 1, 4);

	n++;

	tab_file_gl->addWidget(but_midi_file_save_as, n, 4, 1, 4);

	n++;

	/* <Play> Tab */

	lbl_bpm_max = new QLabel(tr("Max"));
	lbl_bpm_max->setAlignment(Qt::AlignHCenter|Qt::AlignVCenter);
	lbl_bpm_min = new QLabel(tr("Min"));
	lbl_bpm_min->setAlignment(Qt::AlignHCenter|Qt::AlignVCenter);
	lbl_bpm_avg = new QLabel(tr("Average BPM"));
	lbl_bpm_avg->setAlignment(Qt::AlignHCenter|Qt::AlignVCenter);

	lbl_curr_time_val = new QLCDNumber(8);
	lbl_curr_time_val->setMode(QLCDNumber::Dec);
	lbl_curr_time_val->setFrameShape(QLCDNumber::NoFrame);
	lbl_curr_time_val->setSegmentStyle(QLCDNumber::Flat);

	lbl_bpm_min_val = new QLCDNumber(3);
	lbl_bpm_min_val->setMode(QLCDNumber::Dec);
	lbl_bpm_min_val->setFrameShape(QLCDNumber::NoFrame);
	lbl_bpm_min_val->setSegmentStyle(QLCDNumber::Flat);

	lbl_bpm_avg_val = new QLCDNumber(3);
	lbl_bpm_avg_val->setMode(QLCDNumber::Dec);
	lbl_bpm_avg_val->setFrameShape(QLCDNumber::NoFrame);
	lbl_bpm_avg_val->setSegmentStyle(QLCDNumber::Flat);

	lbl_bpm_max_val = new QLCDNumber(3);
	lbl_bpm_max_val->setMode(QLCDNumber::Dec);
	lbl_bpm_max_val->setFrameShape(QLCDNumber::NoFrame);
	lbl_bpm_max_val->setSegmentStyle(QLCDNumber::Flat);

	lbl_score_record = new QLabel(QString());
	lbl_midi_record = new QLabel(QString());
	lbl_midi_pass_thru = new QLabel(QString());
	lbl_midi_play = new QLabel(QString());

	but_jump[0] = new QPushButton(tr("J0"));
	but_jump[1] = new QPushButton(tr("J1"));
	but_jump[2] = new QPushButton(tr("J2"));
	but_jump[3] = new QPushButton(tr("J3"));
	but_jump[4] = new QPushButton(tr("J4"));
	but_jump[5] = new QPushButton(tr("J5"));
	but_jump[6] = new QPushButton(tr("J6"));
	but_jump[7] = new QPushButton(tr("J7"));

	but_midi_pass_thru = new QPushButton(tr("Pass Thru"));
	but_compile = new QPushButton(tr("Compile"));
	but_score_record = new QPushButton(tr("Scores"));
	but_midi_record = new QPushButton(tr("MIDI"));

	but_midi_play = new QPushButton(tr("MIDI"));
	but_midi_pause = new QPushButton(tr("Pause"));
	but_midi_trigger = new QPushButton(tr("Trigger"));
	but_midi_rewind = new QPushButton(tr("Rewind"));

	but_play = new QPushButton(tr(" \nPlay\n "));

	lbl_volume = new QLabel(tr("Volume (0..127..511)"));
	spn_volume = new QSpinBox();
	connect(spn_volume, SIGNAL(valueChanged(int)), this, SLOT(handle_volume_changed(int)));
	spn_volume->setMaximum(511);
	spn_volume->setMinimum(0);

	lbl_play_key = new QLabel(QString());
	spn_play_key = new QSpinBox();
	connect(spn_play_key, SIGNAL(valueChanged(int)), this, SLOT(handle_play_key_changed(int)));
	spn_play_key->setMaximum(127);
	spn_play_key->setMinimum(0);
	spn_play_key->setValue(C4);

	lbl_cmd_key = new QLabel(QString());
	spn_cmd_key = new QSpinBox();
	connect(spn_cmd_key, SIGNAL(valueChanged(int)), this, SLOT(handle_cmd_key_changed(int)));
	spn_cmd_key->setMaximum(127);
	spn_cmd_key->setMinimum(0);
	spn_cmd_key->setValue(C3);

	lbl_base_key = new QLabel(QString());
	spn_base_key = new QSpinBox();
	connect(spn_base_key, SIGNAL(valueChanged(int)), this, SLOT(handle_base_key_changed(int)));
	spn_base_key->setMaximum(127);
	spn_base_key->setMinimum(0);
	spn_base_key->setValue(C4);

	lbl_time_counter = new QLabel(tr(" - Time Counter -"));
	lbl_time_counter->setAlignment(Qt::AlignHCenter|Qt::AlignVCenter);

	lbl_synth = new QLabel(tr("- Synth Play -"));
	lbl_synth->setAlignment(Qt::AlignHCenter|Qt::AlignVCenter);

	lbl_playback = new QLabel(tr("- Playback -"));
	lbl_playback->setAlignment(Qt::AlignHCenter|Qt::AlignVCenter);

	lbl_recording = new QLabel(tr("- Recording -"));
	lbl_recording->setAlignment(Qt::AlignHCenter|Qt::AlignVCenter);

	lbl_scores = new QLabel(tr("- Scores -"));
	lbl_scores->setAlignment(Qt::AlignHCenter|Qt::AlignVCenter);

	n = 0;

	tab_play_gl->addWidget(lbl_time_counter, n, 0, 1, 4);

	n++;

	tab_play_gl->addWidget(lbl_curr_time_val, n, 0, 1, 4);

	n++;

	tab_play_gl->addWidget(lbl_playback, n, 0, 1, 4);

	n++;

	tab_play_gl->addWidget(lbl_midi_play, n, 3, 1, 1);
	tab_play_gl->addWidget(but_midi_play, n, 0, 1, 3);

	n++;

	tab_play_gl->addWidget(but_midi_pause, n, 0, 1, 4);

	n++;

	tab_play_gl->addWidget(but_midi_trigger, n, 0, 1, 4);

	n++;

	tab_play_gl->addWidget(but_midi_rewind, n, 0, 1, 4);

	n++;

	tab_play_gl->addWidget(lbl_scores, n, 0, 1, 4);

	n++;

	tab_play_gl->addWidget(but_compile, n, 0, 1, 4);

	n++;

	tab_play_gl->addWidget(lbl_recording, n, 0, 1, 4);

	n++;

	tab_play_gl->addWidget(lbl_midi_pass_thru, n, 3, 1, 1);
	tab_play_gl->addWidget(but_midi_pass_thru, n, 0, 1, 3);

	n++;

	tab_play_gl->addWidget(lbl_score_record, n, 3, 1, 1);
	tab_play_gl->addWidget(but_score_record, n, 0, 1, 3);

	n++;

	tab_play_gl->addWidget(lbl_midi_record, n, 3, 1, 1);
	tab_play_gl->addWidget(but_midi_record, n, 0, 1, 3);

	n = 0;

	tab_play_gl->addWidget(lbl_synth, n, 4, 1, 4);

	n++;

	tab_play_gl->addWidget(lbl_volume, n, 4, 1, 3);
	tab_play_gl->addWidget(spn_volume, n, 7, 1, 1);

	n++;

	tab_play_gl->addWidget(lbl_play_key, n, 4, 1, 3);
	tab_play_gl->addWidget(spn_play_key, n, 7, 1, 1);

	n++;

	tab_play_gl->addWidget(lbl_cmd_key, n, 4, 1, 3);
	tab_play_gl->addWidget(spn_cmd_key, n, 7, 1, 1);

	n++;

	tab_play_gl->addWidget(lbl_base_key, n, 4, 1, 3);
	tab_play_gl->addWidget(spn_base_key, n, 7, 1, 1);

	n++;

	tab_play_gl->addWidget(but_jump[4], n, 5, 1, 1);
	tab_play_gl->addWidget(but_jump[0], n, 4, 1, 1);

	n++;

	tab_play_gl->addWidget(but_jump[5], n, 5, 1, 1);
	tab_play_gl->addWidget(but_jump[1], n, 4, 1, 1);

	n++;

	tab_play_gl->addWidget(but_jump[6], n, 5, 1, 1);
	tab_play_gl->addWidget(but_jump[2], n, 4, 1, 1);

	n++;

	tab_play_gl->addWidget(but_jump[7], n, 5, 1, 1);
	tab_play_gl->addWidget(but_jump[3], n, 4, 1, 1);

	n++;

	tab_play_gl->addWidget(lbl_bpm_max, n, 4, 1, 1);
	tab_play_gl->addWidget(lbl_bpm_avg, n, 5, 1, 2);
	tab_play_gl->addWidget(lbl_bpm_min, n, 7, 1, 1);

	n++;

	tab_play_gl->addWidget(lbl_bpm_max_val, n, 4, 1, 1);
	tab_play_gl->addWidget(lbl_bpm_avg_val, n, 5, 1, 2);
	tab_play_gl->addWidget(lbl_bpm_min_val, n, 7, 1, 1);

	n++;

	tab_play_gl->addWidget(but_play, n, 4, 3, 4);

	n += 3;

	tab_play_gl->setRowStretch(n, 4);

	/* <Configuration> Tab */

	lbl_config_title = new QLabel(tr("- Device configuration -"));
	lbl_config_title->setAlignment(Qt::AlignHCenter|Qt::AlignVCenter);
	lbl_config_play = new QLabel(tr("Play"));
	lbl_config_play->setAlignment(Qt::AlignHCenter|Qt::AlignVCenter);
	lbl_config_rec = new QLabel(tr("Rec."));
	lbl_config_rec->setAlignment(Qt::AlignHCenter|Qt::AlignVCenter);
	lbl_config_synth = new QLabel(tr("Synth"));
	lbl_config_synth->setAlignment(Qt::AlignHCenter|Qt::AlignVCenter);
	lbl_bpm_count = new QLabel(tr("BPM average length (0..32)"));
	lbl_auto_play = new QLabel(tr("Auto Play BPM (0..6000)"));

	lbl_key_delay = new QLabel(tr("Random Key Delay (0..255)"));
	spn_key_delay = new QSpinBox();

	connect(spn_key_delay, SIGNAL(valueChanged(int)), this, SLOT(handle_key_delay_changed(int)));
	spn_key_delay->setMaximum(255);
	spn_key_delay->setMinimum(0);
	spn_key_delay->setValue(0);
	spn_key_delay->setSuffix(tr(" ms"));

	spn_bpm_length = new QSpinBox();
	spn_bpm_length->setMaximum(MPP_MAX_BPM);
	spn_bpm_length->setMinimum(0);
	spn_bpm_length->setValue(0);

	spn_auto_play = new QSpinBox();
	spn_auto_play->setMaximum(6000);
	spn_auto_play->setMinimum(0);
	connect(spn_auto_play, SIGNAL(valueChanged(int)), this, SLOT(handle_auto_play(int)));
	spn_auto_play->setValue(0);

	lbl_config_local = new QLabel(tr("Enable local MIDI on synth\n"));
	cbx_config_local = new QCheckBox();
	connect(cbx_config_local, SIGNAL(stateChanged(int)), this, SLOT(handle_config_local_changed(int)));
	cbx_config_local->setCheckState(Qt::Checked);

	but_config_apply = new QPushButton(tr("Apply"));
	but_config_revert = new QPushButton(tr("Revert"));
	but_config_fontsel = new QPushButton(tr("Select Font"));

	x = 0;

	tab_config_gl->addWidget(lbl_config_title, x, 0, 1, 5);
	tab_config_gl->addWidget(lbl_config_play, x, 5, 1, 1);
	tab_config_gl->addWidget(lbl_config_rec, x, 6, 1, 1);
	tab_config_gl->addWidget(lbl_config_synth, x, 7, 1, 1);

	x++;

	for (n = 0; n != MPP_MAX_DEVS; n++) {
		char buf[16];

		led_config_dev[n] = new QLineEdit(QString());
		cbx_config_dev[(3*n)+0] = new QCheckBox();
		cbx_config_dev[(3*n)+1] = new QCheckBox();
		cbx_config_dev[(3*n)+2] = new QCheckBox();


		snprintf(buf, sizeof(buf), "Dev%d:", n);

		lbl_config_dev[n] = new QLabel(tr(buf));

		tab_config_gl->addWidget(lbl_config_dev[n], x, 0, 1, 1, Qt::AlignHCenter|Qt::AlignLeft);
		tab_config_gl->addWidget(led_config_dev[n], x, 1, 1, 4, Qt::AlignHCenter|Qt::AlignLeft);
		tab_config_gl->addWidget(cbx_config_dev[(3*n)+0], x, 5, 1, 1, Qt::AlignHCenter|Qt::AlignVCenter);
		tab_config_gl->addWidget(cbx_config_dev[(3*n)+1], x, 6, 1, 1, Qt::AlignHCenter|Qt::AlignVCenter);
		tab_config_gl->addWidget(cbx_config_dev[(3*n)+2], x, 7, 1, 1, Qt::AlignHCenter|Qt::AlignVCenter);

		x++;
	}

	tab_config_gl->addWidget(lbl_bpm_count, x, 0, 1, 6);
	tab_config_gl->addWidget(spn_bpm_length, x, 6, 1, 2);

	x++;

	tab_config_gl->addWidget(lbl_auto_play, x, 0, 1, 6);
	tab_config_gl->addWidget(spn_auto_play, x, 6, 1, 2);

	x++;

	tab_config_gl->addWidget(lbl_key_delay, x, 0, 1, 6);
	tab_config_gl->addWidget(spn_key_delay, x, 6, 1, 2);

	x++;

	tab_config_gl->addWidget(lbl_config_local, x, 0, 1, 7);
	tab_config_gl->addWidget(cbx_config_local, x, 7, 1, 1);

	x++;

	tab_config_gl->addWidget(but_config_fontsel, x, 0, 1, 2);

	x++;

	tab_config_gl->setRowStretch(x, 1);

	x++;

	tab_config_gl->addWidget(but_config_apply, x, 4, 1, 2);
	tab_config_gl->addWidget(but_config_revert, x, 6, 1, 2);

	/* <Edit> Tab */

	lbl_edit_title = new QLabel(tr("- MIDI File Edit -"));
	lbl_edit_title->setAlignment(Qt::AlignHCenter|Qt::AlignVCenter);

	lbl_edit_channel = new QLabel(tr("Selected Channel (0..15):"));
	lbl_edit_transpose = new QLabel(tr("Transpose Steps (-128..127):"));
	lbl_edit_volume = new QLabel(tr("Average Volume (1..127):"));

	spn_edit_channel = new QSpinBox();
	spn_edit_channel->setMaximum(15);
	spn_edit_channel->setMinimum(0);
	spn_edit_channel->setValue(0);

	spn_edit_transpose = new QSpinBox();
	spn_edit_transpose->setMaximum(127);
	spn_edit_transpose->setMinimum(-128);
	spn_edit_transpose->setValue(0);

	spn_edit_volume = new QSpinBox();
	spn_edit_volume->setMaximum(127);
	spn_edit_volume->setMinimum(1);
	spn_edit_volume->setValue(80);

	but_edit_apply_transpose = new QPushButton(tr("Apply Channel Key Transpose"));
	but_edit_change_volume = new QPushButton(tr("Change Channel Event Volume"));
	but_edit_remove_pedal = new QPushButton(tr("Remove Channel Pedal Events"));
	but_edit_remove_keys = new QPushButton(tr("Remove Channel Key Events"));
	but_edit_remove_all = new QPushButton(tr("Remove All Channel Events"));

	n = 0;

	tab_edit_gl->addWidget(lbl_edit_title, n, 0, 1, 8);

	n++;

	tab_edit_gl->addWidget(lbl_edit_channel, n, 0, 1, 7);
	tab_edit_gl->addWidget(spn_edit_channel, n, 7, 1, 1);

	n++;

	tab_edit_gl->addWidget(lbl_edit_transpose, n, 0, 1, 7);
	tab_edit_gl->addWidget(spn_edit_transpose, n, 7, 1, 1);

	n++;

	tab_edit_gl->addWidget(lbl_edit_volume, n, 0, 1, 7);
	tab_edit_gl->addWidget(spn_edit_volume, n, 7, 1, 1);

	n++;

	tab_edit_gl->addWidget(but_edit_apply_transpose, n, 0, 1, 8);
	n++;
	tab_edit_gl->addWidget(but_edit_change_volume, n, 0, 1, 8);
	n++;
	tab_edit_gl->addWidget(but_edit_remove_pedal, n, 0, 1, 8);
	n++;
	tab_edit_gl->addWidget(but_edit_remove_keys, n, 0, 1, 8);
	n++;
	tab_edit_gl->addWidget(but_edit_remove_all, n, 0, 1, 8);
	n++;

	tab_edit_gl->setRowStretch(n, 4);

	/* <Instrument> tab */

	lbl_instr_title[0] = new QLabel(tr("- Bank/Program/Mute -"));
	lbl_instr_title[0]->setAlignment(Qt::AlignHCenter|Qt::AlignVCenter);

	lbl_instr_title[1] = new QLabel(tr("- Bank/Program/Mute -"));
	lbl_instr_title[1]->setAlignment(Qt::AlignHCenter|Qt::AlignVCenter);

	lbl_instr_prog = new QLabel(tr("- Synth/Record Channel and Selected Bank/Program -"));
	lbl_instr_prog->setAlignment(Qt::AlignHCenter|Qt::AlignVCenter);

	but_instr_apply = new QPushButton(tr("Apply"));
	but_instr_revert = new QPushButton(tr("Revert"));
	but_instr_program = new QPushButton(tr("Program"));
	but_instr_reset = new QPushButton(tr("Reset"));

	spn_instr_curr_chan = new QSpinBox();
	connect(spn_instr_curr_chan, SIGNAL(valueChanged(int)), this, SLOT(handle_instr_channel_changed(int)));
	spn_instr_curr_chan->setMaximum(15);
	spn_instr_curr_chan->setMinimum(0);
	spn_instr_curr_chan->setValue(0);

	spn_instr_curr_bank = new QSpinBox();
	spn_instr_curr_bank->setMaximum(16383);
	spn_instr_curr_bank->setMinimum(0);
	spn_instr_curr_bank->setValue(0);

	spn_instr_curr_prog = new QSpinBox();
	spn_instr_curr_prog->setMaximum(127);
	spn_instr_curr_prog->setMinimum(0);
	spn_instr_curr_prog->setValue(0);

	x = 0;

	tab_instr_gl->addWidget(lbl_instr_prog, x, 0, 1, 8);

	x++;

	tab_instr_gl->addWidget(spn_instr_curr_chan, x, 0, 1, 1);
	tab_instr_gl->addWidget(spn_instr_curr_bank, x, 1, 1, 1);
	tab_instr_gl->addWidget(spn_instr_curr_prog, x, 2, 1, 1);
	tab_instr_gl->addWidget(but_instr_program, x, 4, 1, 4);

	x++;

	tab_instr_gl->addWidget(lbl_instr_title[0], x, 0, 1, 4);
	tab_instr_gl->addWidget(lbl_instr_title[1], x, 4, 1, 4);

	x++;

	for (n = 0; n != 16; n++) {
		int y_off = (n & 8) ? 4 : 0;

		char buf[16];

		snprintf(buf, sizeof(buf), "Ch%X", n);

		lbl_instr_desc[n] = new QLabel(tr(buf));
		lbl_instr_desc[n]->setAlignment(Qt::AlignVCenter|Qt::AlignRight);

		spn_instr_bank[n] = new QSpinBox();
		spn_instr_bank[n]->setMaximum(16383);
		spn_instr_bank[n]->setMinimum(0);
		spn_instr_bank[n]->setValue(0);

		spn_instr_prog[n] = new QSpinBox();
		spn_instr_prog[n]->setMaximum(127);
		spn_instr_prog[n]->setMinimum(0);
		spn_instr_prog[n]->setValue(0);

		cbx_instr_mute[n] = new QCheckBox();

		tab_instr_gl->addWidget(lbl_instr_desc[n], (n & 7) + x, 0 + y_off, 1, 1);
		tab_instr_gl->addWidget(spn_instr_bank[n], (n & 7) + x, 1 + y_off, 1, 1);
		tab_instr_gl->addWidget(spn_instr_prog[n], (n & 7) + x, 2 + y_off, 1, 1);
		tab_instr_gl->addWidget(cbx_instr_mute[n], (n & 7) + x, 3 + y_off, 1, 1);
	}

	x += 8;

	tab_instr_gl->setRowStretch(x, 4);

	x++;

	tab_instr_gl->addWidget(but_instr_reset, x, 2, 1, 2);
	tab_instr_gl->addWidget(but_instr_revert, x, 4, 1, 2);
	tab_instr_gl->addWidget(but_instr_apply, x, 6, 1, 2);

	/* <Volume> tab */

	lbl_volume_title[0] = new QLabel(tr("- Playback -"));
	lbl_volume_title[0]->setAlignment(Qt::AlignHCenter|Qt::AlignVCenter);

	lbl_volume_title[1] = new QLabel(tr("- Synth/Record -"));
	lbl_volume_title[1]->setAlignment(Qt::AlignHCenter|Qt::AlignVCenter);

	but_volume_apply = new QPushButton(tr("Apply"));
	but_volume_revert = new QPushButton(tr("Revert"));
	but_volume_reset = new QPushButton(tr("Reset"));

	x = 0;

	tab_volume_gl->addWidget(lbl_volume_title[0], x, 0, 1, 4);
	tab_volume_gl->addWidget(lbl_volume_title[1], x, 4, 1, 4);

	x++;

	for (n = 0; n != 16; n++) {
		int y_off = (n & 8) ? 2 : 0;

		char buf[16];

		snprintf(buf, sizeof(buf), "Ch%X", n);

		lbl_volume_play[n] = new QLabel(tr(buf));
		lbl_volume_play[n]->setAlignment(Qt::AlignVCenter|Qt::AlignRight);

		lbl_volume_synth[n] = new QLabel(tr(buf));
		lbl_volume_synth[n]->setAlignment(Qt::AlignVCenter|Qt::AlignRight);

		spn_volume_synth[n] = new QSpinBox();
		spn_volume_synth[n]->setMaximum(511);
		spn_volume_synth[n]->setMinimum(0);
		spn_volume_synth[n]->setValue(MPP_VOLUME_UNIT);

		spn_volume_play[n] = new QSpinBox();
		spn_volume_play[n]->setMaximum(511);
		spn_volume_play[n]->setMinimum(0);
		spn_volume_play[n]->setValue(MPP_VOLUME_UNIT);

		tab_volume_gl->addWidget(lbl_volume_play[n], (n & 7) + x, 0 + y_off, 1, 1);
		tab_volume_gl->addWidget(spn_volume_play[n], (n & 7) + x, 1 + y_off, 1, 1);
		tab_volume_gl->addWidget(lbl_volume_synth[n], (n & 7) + x, 4 + y_off, 1, 1);
		tab_volume_gl->addWidget(spn_volume_synth[n], (n & 7) + x, 5 + y_off, 1, 1);
	}

	x += 8;

	tab_volume_gl->setRowStretch(x, 4);

	x++;

	tab_volume_gl->addWidget(but_volume_reset, x, 2, 1, 2);
	tab_volume_gl->addWidget(but_volume_revert, x, 4, 1, 2);
	tab_volume_gl->addWidget(but_volume_apply, x, 6, 1, 2);

	/* Connect all */

	connect(but_jump[0], SIGNAL(pressed()), this, SLOT(handle_jump_0()));
	connect(but_jump[1], SIGNAL(pressed()), this, SLOT(handle_jump_1()));
	connect(but_jump[2], SIGNAL(pressed()), this, SLOT(handle_jump_2()));
	connect(but_jump[3], SIGNAL(pressed()), this, SLOT(handle_jump_3()));
	connect(but_jump[4], SIGNAL(pressed()), this, SLOT(handle_jump_4()));
	connect(but_jump[5], SIGNAL(pressed()), this, SLOT(handle_jump_5()));
	connect(but_jump[6], SIGNAL(pressed()), this, SLOT(handle_jump_6()));
	connect(but_jump[7], SIGNAL(pressed()), this, SLOT(handle_jump_7()));

	connect(but_midi_pass_thru, SIGNAL(pressed()), this, SLOT(handle_pass_thru()));
	connect(but_compile, SIGNAL(pressed()), this, SLOT(handle_compile()));
	connect(but_score_record, SIGNAL(pressed()), this, SLOT(handle_score_record()));
	connect(but_midi_record, SIGNAL(pressed()), this, SLOT(handle_midi_record()));
	connect(but_midi_play, SIGNAL(pressed()), this, SLOT(handle_midi_play()));
	connect(but_play, SIGNAL(pressed()), this, SLOT(handle_play_press()));
	connect(but_play, SIGNAL(released()), this, SLOT(handle_play_release()));
	connect(but_quit, SIGNAL(pressed()), this, SLOT(handle_quit()));

	connect(but_score_file_new, SIGNAL(pressed()), this, SLOT(handle_score_file_new()));
	connect(but_score_file_open, SIGNAL(pressed()), this, SLOT(handle_score_file_open()));
	connect(but_score_file_save, SIGNAL(pressed()), this, SLOT(handle_score_file_save()));
	connect(but_score_file_save_as, SIGNAL(pressed()), this, SLOT(handle_score_file_save_as()));
	connect(but_score_file_print, SIGNAL(pressed()), this, SLOT(handle_score_print()));

	connect(but_midi_file_new, SIGNAL(pressed()), this, SLOT(handle_midi_file_new()));
	connect(but_midi_file_open, SIGNAL(pressed()), this, SLOT(handle_midi_file_new_open()));
	connect(but_midi_file_merge, SIGNAL(pressed()), this, SLOT(handle_midi_file_merge_open()));
	connect(but_midi_file_save, SIGNAL(pressed()), this, SLOT(handle_midi_file_save()));
	connect(but_midi_file_save_as, SIGNAL(pressed()), this, SLOT(handle_midi_file_save_as()));

	connect(but_midi_trigger, SIGNAL(pressed()), this, SLOT(handle_midi_trigger()));
	connect(but_midi_rewind, SIGNAL(pressed()), this, SLOT(handle_rewind()));
	connect(but_config_apply, SIGNAL(pressed()), this, SLOT(handle_config_apply()));
	connect(but_config_revert, SIGNAL(pressed()), this, SLOT(handle_config_revert()));
	connect(but_config_fontsel, SIGNAL(pressed()), this, SLOT(handle_config_fontsel()));

	connect(but_instr_program, SIGNAL(pressed()), this, SLOT(handle_instr_program()));
	connect(but_instr_apply, SIGNAL(pressed()), this, SLOT(handle_instr_apply()));
	connect(but_instr_revert, SIGNAL(pressed()), this, SLOT(handle_instr_revert()));
	connect(but_instr_reset, SIGNAL(pressed()), this, SLOT(handle_instr_reset()));

	connect(but_volume_apply, SIGNAL(pressed()), this, SLOT(handle_volume_apply()));
	connect(but_volume_revert, SIGNAL(pressed()), this, SLOT(handle_volume_revert()));
	connect(but_volume_reset, SIGNAL(pressed()), this, SLOT(handle_volume_reset()));

	connect(but_midi_pause, SIGNAL(pressed()), this, SLOT(handle_midi_pause()));

	MidiInit();

	setWindowTitle(tr("MIDI Player Pro v1.0"));

	watchdog->start(250);
}

MppMainWindow :: ~MppMainWindow()
{
	umidi20_unset_timer(&MppTimerCallback, this);

	MidiUnInit();
}

void
MppMainWindow :: handle_quit()
{
	exit(0);
}

void
MppMainWindow :: handle_jump_N(int index)
{
	pthread_mutex_lock(&main_sc.mtx);
	handle_jump(index, 1);
	pthread_mutex_unlock(&main_sc.mtx);
}

void
MppMainWindow :: handle_jump_0()
{
	handle_jump_N(0);
}

void
MppMainWindow :: handle_jump_1()
{
	handle_jump_N(1);
}

void
MppMainWindow :: handle_jump_2()
{
	handle_jump_N(2);
}

void
MppMainWindow :: handle_jump_3()
{
	handle_jump_N(3);
}

void
MppMainWindow :: handle_jump_4()
{
	handle_jump_N(4);
}

void
MppMainWindow :: handle_jump_5()
{
	handle_jump_N(5);
}

void
MppMainWindow :: handle_jump_6()
{
	handle_jump_N(6);
}

void
MppMainWindow :: handle_jump_7()
{
	handle_jump_N(7);
}

void
MppMainWindow :: handle_config_local_changed(int state)
{
	pthread_mutex_lock(&main_sc.mtx);
	main_sc.ScSynthIsLocal = (state == Qt::Checked);
	pthread_mutex_unlock(&main_sc.mtx);
}

void
MppMainWindow :: handle_play_key_changed(int key)
{
	key &= 0x7F;

	lbl_play_key->setText(tr("Play Key (0..127) ") + QString(mid_key_str[key]));

	pthread_mutex_lock(&main_sc.mtx);
	main_sc.ScPlayKey = key;
	pthread_mutex_unlock(&main_sc.mtx);
}

void
MppMainWindow :: handle_cmd_key_changed(int key)
{
	key &= 0x7F;

	pthread_mutex_lock(&main_sc.mtx);
	main_sc.ScCmdKey = key;
	pthread_mutex_unlock(&main_sc.mtx);

	lbl_cmd_key->setText(tr("Cmd Key (0..127) ") + QString(mid_key_str[key]));
}

void
MppMainWindow :: handle_base_key_changed(int key)
{
	key &= 0x7F;

	pthread_mutex_lock(&main_sc.mtx);
	main_sc.ScBaseKey = key;
	pthread_mutex_unlock(&main_sc.mtx);

	lbl_base_key->setText(tr("Base Key (0..127) ") + QString(mid_key_str[key]));
}

void
MppMainWindow :: handle_key_delay_changed(int delay)
{
	delay &= 0xFF;

	pthread_mutex_lock(&main_sc.mtx);
	main_sc.ScDelayNoise = delay;
	pthread_mutex_unlock(&main_sc.mtx);
}

void
MppMainWindow :: handle_pass_thru()
{
	pthread_mutex_lock(&main_sc.mtx);
	main_sc.ScMidiPassThruOff = !main_sc.ScMidiPassThruOff;
	pthread_mutex_unlock(&main_sc.mtx);

	if (main_sc.ScMidiPassThruOff == 0)
		lbl_midi_pass_thru->setText(tr("ON"));
	else
		lbl_midi_pass_thru->setText(tr("OFF"));
}

void
MppMainWindow :: handle_compile()
{
	pthread_mutex_lock(&main_sc.mtx);
	handle_stop();
	MppParse(&main_sc, main_edit->toPlainText());
	pthread_mutex_unlock(&main_sc.mtx);

	scores_wg->setMinimumWidth(main_sc.ScMaxScoresWidth);
}

void
MppMainWindow :: handle_score_record()
{
	pthread_mutex_lock(&main_sc.mtx);
	main_sc.ScScoreRecordOff = !main_sc.ScScoreRecordOff;
	pthread_mutex_unlock(&main_sc.mtx);

	if (main_sc.ScScoreRecordOff == 0)
		lbl_score_record->setText(tr("ON"));
	else
		lbl_score_record->setText(tr("OFF"));
}

void
MppMainWindow :: handle_midi_pause()
{
	uint32_t pos;
	uint8_t triggered;
	uint8_t paused;

	pthread_mutex_lock(&main_sc.mtx);
	pos = (umidi20_get_curr_position() - main_sc.ScStartPosition) % 0x40000000UL;
	triggered = main_sc.ScMidiTriggered;
	paused = main_sc.ScMidiPaused;
	pthread_mutex_unlock(&main_sc.mtx);

	if (paused)
		return;		/* nothing to do */

	handle_rewind();

	pthread_mutex_lock(&main_sc.mtx);
	if (triggered != 0) {
		main_sc.ScMidiPaused = 1;
		main_sc.ScPausePosition = pos;
	}
	pthread_mutex_unlock(&main_sc.mtx);
}

void
MppMainWindow :: handle_midi_play()
{
	uint8_t triggered;

	pthread_mutex_lock(&main_sc.mtx);
	main_sc.ScMidiPlayOff = !main_sc.ScMidiPlayOff;
	triggered = main_sc.ScMidiTriggered;
	update_play_device_no();
	pthread_mutex_unlock(&main_sc.mtx);

	if (main_sc.ScMidiPlayOff == 0)
		lbl_midi_play->setText(tr("ON"));
	else
		lbl_midi_play->setText(tr("OFF"));

	handle_midi_pause();

	if (triggered)
		handle_midi_trigger();
}

void
MppMainWindow :: handle_midi_record()
{
	uint8_t triggered;

	pthread_mutex_lock(&main_sc.mtx);
	main_sc.ScMidiRecordOff = !main_sc.ScMidiRecordOff;
	triggered = main_sc.ScMidiTriggered;
	update_play_device_no();
	pthread_mutex_unlock(&main_sc.mtx);

	if (main_sc.ScMidiRecordOff == 0)
		lbl_midi_record->setText(tr("ON"));
	else
		lbl_midi_record->setText(tr("OFF"));

	handle_midi_pause();

	if (triggered)
		handle_midi_trigger();
}

static void
MppTimerCallback(void *arg)
{
	MppMainWindow *mw = (MppMainWindow *)arg;
	int x;

	pthread_mutex_lock(&mw->main_sc.mtx);
	x = mw->main_sc.ScMidiTriggered;
	pthread_mutex_unlock(&mw->main_sc.mtx);

	if (x) {
		mw->handle_play_press();
		mw->handle_play_release();
	}
}

/* must be called locked */
static void
MppSetTimer(MppMainWindow *mw)
{
	int bpm = mw->main_sc.ScBpmAutoPlay;
	int i;

	if (bpm > 0) {
		i = 60000 / bpm;
		if (i == 0)
			i = 1;
	} else {
		i = 0;
	}

	umidi20_set_timer(&MppTimerCallback, mw, i);
}

void
MppMainWindow :: handle_auto_play(int bpm)
{
	pthread_mutex_lock(&main_sc.mtx);

	main_sc.ScBpmAutoPlay = bpm;

	MppSetTimer(this);

	pthread_mutex_unlock(&main_sc.mtx);
}

void
MppMainWindow :: handle_play_press()
{
	pthread_mutex_lock(&main_sc.mtx);
	handle_key_press(main_sc.ScPlayKey, 90);
	pthread_mutex_unlock(&main_sc.mtx);
}

void
MppMainWindow :: handle_play_release()
{
	pthread_mutex_lock(&main_sc.mtx);
	handle_key_release(main_sc.ScPlayKey);
	pthread_mutex_unlock(&main_sc.mtx);
}

void
MppMainWindow :: handle_watchdog()
{
	QTextCursor cursor(main_edit->textCursor());
	uint32_t delta;
	char buf[32];
	uint8_t events_copy[MPP_MAX_QUEUE];
	uint8_t num_events;
	uint8_t x;
	uint8_t y;
	uint8_t z;
	uint8_t last_duration;
	uint8_t instr_update;

	pthread_mutex_lock(&main_sc.mtx);
	instr_update = main_sc.ScInstrUpdated;
	main_sc.ScInstrUpdated = 0;
	num_events = main_sc.ScNumInputEvents;
	if (num_events != 0) {
		delta =  umidi20_get_curr_position() - main_sc.ScLastInputEvent;
		if (delta >= ((UMIDI20_BPM + 45 -1) / 45)) {
			main_sc.ScNumInputEvents = 0;
			memcpy(events_copy, main_sc.ScInputEvents, num_events);
		} else {
			/* wait until 2 seconds have elapsed */
			num_events = 0;
		}
	}
	pthread_mutex_unlock(&main_sc.mtx);

	if (num_events != 0) {
		mid_sort(events_copy, num_events);

		last_duration = 0;

		cursor.movePosition(QTextCursor::StartOfLine, QTextCursor::MoveAnchor, 1);
		cursor.beginEditBlock();

		for (x = 0; x != num_events; x++) {
			for (y = x; y != num_events; y++) {
				if (events_copy[x] != events_copy[y])
					break;
			}

			z = y - 1;
			y = y - x;

			if (y != last_duration) {
				last_duration = y;
				snprintf(buf, sizeof(buf), "U%d %s ",
				    y, mid_key_str[events_copy[x] & 0x7F]);
			} else {
				snprintf(buf, sizeof(buf), "%s ",
				    mid_key_str[events_copy[x] & 0x7F]);
			}

			cursor.insertText(QString(buf));

			x = z;
		}

		cursor.insertText(QString("\n"));
		cursor.endEditBlock();
	}

	if (instr_update)
		handle_instr_revert();

	do_bpm_stats();

	do_clock_stats();

	scores_wg->repaint();
}

void
MppMainWindow :: handle_score_file_new()
{
	main_edit->setText(QString());
	lbl_file_status->setText(QString());

	handle_compile();

	if (CurrScoreFileName != NULL) {
		delete (CurrScoreFileName);
		CurrScoreFileName = NULL;
	}
}

void
MppMainWindow :: handle_score_file_open()
{
	QFileDialog *diag = 
	  new QFileDialog(this, tr("Select Score File"), 
		QString(), QString("Score File (*.txt; *.TXT)"));
	QString scores;
	QString status;

	diag->setAcceptMode(QFileDialog::AcceptOpen);
	diag->setFileMode(QFileDialog::ExistingFile);

	handle_score_file_new();

	if (diag->exec()) {
		CurrScoreFileName = new QString(diag->selectedFiles()[0]);
		scores = MppReadFile(*CurrScoreFileName, &status);
		main_edit->setText(scores);
		handle_compile();
		lbl_file_status->setText(status);
	}

	delete diag;
}

void
MppMainWindow :: handle_score_file_save()
{
	QString status;

	if (CurrScoreFileName != NULL) {
		MppWriteFile(*CurrScoreFileName, main_edit->toPlainText(), 
		    &status);

		lbl_file_status->setText(status);
	} else {
		handle_score_file_save_as();
	}
}

void
MppMainWindow :: handle_score_file_save_as()
{
	QFileDialog *diag = 
	  new QFileDialog(this, tr("Select Score File"), 
		QString(), QString("Score File (*.txt; *.TXT)"));

	diag->setAcceptMode(QFileDialog::AcceptSave);
	diag->setFileMode(QFileDialog::AnyFile);

	if (diag->exec()) {
		if (CurrScoreFileName != NULL)
			delete (CurrScoreFileName);

		CurrScoreFileName = new QString(diag->selectedFiles()[0]);

		if (CurrScoreFileName != NULL)
			handle_score_file_save();
	}

	delete diag;
}

void
MppMainWindow :: handle_midi_file_clear_name()
{
	if (CurrMidiFileName != NULL) {
		delete (CurrMidiFileName);
		CurrMidiFileName = NULL;
	}
	lbl_file_status->setText(QString());
}

void
MppMainWindow :: handle_midi_file_new()
{
	uint8_t x;

	handle_midi_file_clear_name();

	handle_rewind();

	if (track != NULL) {
		pthread_mutex_lock(&main_sc.mtx);
		umidi20_event_queue_drain(&(track->queue));
		for (x = 0; x != 16; x++) {
			main_sc.ScInstr[x].bank = 0;
			main_sc.ScInstr[x].prog = 0;
			main_sc.ScInstr[x].updated = 1;
			main_sc.ScInstr[x].muted = 0;
		}
		main_sc.ScInstrUpdated = 1;
		main_sc.ScChanUsageMask = 0;
		pthread_mutex_unlock(&main_sc.mtx);

		handle_instr_channel_changed(0);
	}
}

void
MppMainWindow :: update_play_device_no()
{
	uint8_t device_no = main_sc.ScPlayDevice;
	struct umidi20_event *event;
	uint8_t vel;

	if (track == NULL)
		return;

	UMIDI20_QUEUE_FOREACH(event, &(track->queue)) {
		event->device_no = device_no;

		vel = umidi20_event_get_velocity(event);

		if (vel != 0) {
			vel |= 1;	/* hint for "MidiEventTxCallback()" */
			umidi20_event_set_velocity(event, vel);
		}
	}
}

void
MppMainWindow :: handle_midi_file_merge_open()
{
	handle_midi_file_open(1);
}

void
MppMainWindow :: handle_midi_file_new_open()
{
	handle_midi_file_open(0);
}

void
MppMainWindow :: handle_midi_file_open(int merge)
{
	QFileDialog *diag = 
	  new QFileDialog(this, tr("Select MIDI File"), 
		QString(), QString("MIDI File (*.mid; *.MID)"));
	struct umidi20_song *song_copy;
	struct umidi20_track *track_copy;
	struct umidi20_event *event;
	struct umidi20_event *event_copy;
	const char *filename;
	uint8_t chan;

	diag->setAcceptMode(QFileDialog::AcceptOpen);
	diag->setFileMode(QFileDialog::ExistingFile);

	if (merge) {
		handle_midi_file_clear_name();
		handle_rewind();
	} else {
		handle_midi_file_new();
	}

	if (diag->exec()) {
		CurrMidiFileName = new QString(diag->selectedFiles()[0]);

		filename = MppQStringToAscii(*CurrMidiFileName);

		if (filename != NULL) {
			pthread_mutex_lock(&main_sc.mtx);
			song_copy = umidi20_load_file(&main_sc.mtx, filename);
			pthread_mutex_unlock(&main_sc.mtx);

			free((void *)filename);

			if (song_copy != NULL) {
				lbl_file_status->setText(MppBaseName(*CurrMidiFileName) +
				    tr(": MIDI file opened"));
				goto load_file;
			} else {
				lbl_file_status->setText(MppBaseName(*CurrMidiFileName) +
				    tr(": Could not open MIDI file"));
			}
		}
	}

	goto done;

load_file:

	printf("format %d\n", song_copy->midi_file_format);
	printf("resolution %d\n", song_copy->midi_resolution);
	printf("division_type %d\n", song_copy->midi_division_type);

	pthread_mutex_lock(&main_sc.mtx);

	chan = main_sc.ScSynthChannel;

	UMIDI20_QUEUE_FOREACH(track_copy, &(song_copy->queue)) {

	    printf("track %p\n", track_copy);

	    UMIDI20_QUEUE_FOREACH(event, &(track_copy->queue)) {

	        if (umidi20_event_is_voice(event) ||
		    umidi20_event_is_sysex(event)) {

		    if (do_instr_check(event)) {
			event_copy = NULL;
		    } else {
			if (umidi20_event_get_what(event) & UMIDI20_WHAT_CHANNEL) {
				uint8_t chan;
				chan = umidi20_event_get_channel(event);
				main_sc.ScChanUsageMask |= (1 << chan);
			}
			event_copy = umidi20_event_copy(event, 0);
		    }

		    if (event_copy != NULL) {
			/* reserve low positions for channel program events */
			if (event_copy->position < MPP_MIN_POS)
				event_copy->position = MPP_MIN_POS;

			umidi20_event_queue_insert(&(track->queue),
			    event_copy, UMIDI20_CACHE_INPUT);
		    }
		}
	    }
	}

	umidi20_song_free(song_copy);

	/* restore synth channel */
	main_sc.ScSynthChannel = chan;

	pthread_mutex_unlock(&main_sc.mtx);

done:
	/* make sure we save into a new file */
	if (merge)
		handle_midi_file_clear_name();

	delete diag;
}

/* must be called locked */
void
MppMainWindow :: handle_midi_file_instr_prepend()
{
	struct mid_data *d = &mid_data;
	uint8_t x;

	for (x = 0; x != 16; x++) {

		/* 
		 * Don't insert instrument bank and program commands
		 * for channels without events!
		 */
		if (!(main_sc.ScChanUsageMask & (1 << x)))
			continue;

		mid_set_channel(d, x);
		mid_set_position(d, 0);
		mid_set_device_no(d, 0xFF);
		mid_set_bank_program(d, x,
		    main_sc.ScInstr[x].bank,
		    main_sc.ScInstr[x].prog);
	}
}

/* must be called locked */
void
MppMainWindow :: handle_midi_file_instr_delete()
{
	umidi20_event_queue_move(&(track->queue), NULL, 0,
	    MPP_MIN_POS, 0, 0-1, UMIDI20_CACHE_INPUT);
}

void
MppMainWindow :: handle_midi_file_save()
{
	const char *filename;
	uint8_t status;

	if (CurrMidiFileName != NULL) {

		filename = MppQStringToAscii(*CurrMidiFileName);

		if (filename != NULL) {
			pthread_mutex_lock(&main_sc.mtx);
			handle_midi_file_instr_prepend();
			status = umidi20_save_file(song, filename);
			handle_midi_file_instr_delete();
			pthread_mutex_unlock(&main_sc.mtx);
			free((void *)filename);

			if (status) {
				lbl_file_status->setText(MppBaseName(*CurrMidiFileName) + 
				    tr(": Could not save MIDI file"));
			} else {
				lbl_file_status->setText(MppBaseName(*CurrMidiFileName) + 
				    tr(": MIDI file saved"));
			}
		}
	} else {
		handle_midi_file_save_as();
	}
}

void
MppMainWindow :: handle_midi_file_save_as()
{
	QFileDialog *diag = 
	  new QFileDialog(this, tr("Select MIDI File"), 
		QString(), QString("MIDI File (*.mid; *.MID)"));

	diag->setAcceptMode(QFileDialog::AcceptSave);
	diag->setFileMode(QFileDialog::AnyFile);

	if (diag->exec()) {
		if (CurrMidiFileName != NULL)
			delete (CurrMidiFileName);

		CurrMidiFileName = new QString(diag->selectedFiles()[0]);

		if (CurrMidiFileName != NULL)
			handle_midi_file_save();
	}

	delete diag;
}

void
MppMainWindow :: handle_rewind()
{
	pthread_mutex_lock(&main_sc.mtx);

	main_sc.ScMidiTriggered = 0;
	main_sc.ScMidiPaused = 0;
	main_sc.ScPausePosition = 0;

	update_play_device_no();

	if (song != NULL) {
		umidi20_song_stop(song,
		    UMIDI20_FLAG_PLAY | UMIDI20_FLAG_RECORD);
		umidi20_song_start(song, 0x40000000, 0x80000000,
		    UMIDI20_FLAG_PLAY | UMIDI20_FLAG_RECORD);
		main_sc.ScStartPosition = umidi20_get_curr_position() - 0x40000000;
	}

	pthread_mutex_unlock(&main_sc.mtx);
}

void
MppMainWindow :: handle_midi_trigger()
{
	pthread_mutex_lock(&main_sc.mtx);

	if (main_sc.ScMidiTriggered == 0) {
		if (main_sc.ScMidiPlayOff == 0) {
			umidi20_song_stop(song,
			    UMIDI20_FLAG_PLAY | UMIDI20_FLAG_RECORD);
			umidi20_song_start(song, main_sc.ScPausePosition, 0x40000000,
			    UMIDI20_FLAG_PLAY | UMIDI20_FLAG_RECORD);
			main_sc.ScStartPosition = umidi20_get_curr_position() - main_sc.ScPausePosition;
		} else {
			umidi20_song_stop(song,
			    UMIDI20_FLAG_PLAY | UMIDI20_FLAG_RECORD);
			umidi20_song_start(song, 0x40000000 + main_sc.ScPausePosition, 0x80000000,
			    UMIDI20_FLAG_PLAY | UMIDI20_FLAG_RECORD);
			main_sc.ScStartPosition = umidi20_get_curr_position() - 0x40000000 - main_sc.ScPausePosition;
		}
		main_sc.ScMidiTriggered = 1;
		main_sc.ScMidiPaused = 0;
		main_sc.ScPausePosition = 0;
		MppSetTimer(this);
	}

	pthread_mutex_unlock(&main_sc.mtx);
}

void
MppMainWindow :: handle_config_reload()
{
	struct umidi20_config cfg;
	int n;
	int x;
	int y;

	/* setup the I/O devices */

	umidi20_config_export(&cfg);

	main_sc.ScPlayDevice = 0;

	for (n = 0; n != MPP_MAX_DEVS; n++) {

		if ((main_sc.ScDeviceBits & (MPP_DEV0_RECORD << (3 * n))) && 
		    (main_sc.ScDeviceName[n] != NULL) && 
		    (main_sc.ScDeviceName[n][0] != 0))  {
			strlcpy(cfg.cfg_dev[n].rec_fname, main_sc.ScDeviceName[n],
			    sizeof(cfg.cfg_dev[n].rec_fname));
			cfg.cfg_dev[n].rec_enabled_cfg = 1;
		} else {
			cfg.cfg_dev[n].rec_enabled_cfg = 0;
		}

		if ((main_sc.ScDeviceBits & ((MPP_DEV0_SYNTH | MPP_DEV0_PLAY) << (3 * n))) && 
		    (main_sc.ScDeviceName[n] != NULL) && 
		    (main_sc.ScDeviceName[n][0] != 0))  {
			strlcpy(cfg.cfg_dev[n].play_fname, main_sc.ScDeviceName[n],
			    sizeof(cfg.cfg_dev[n].play_fname));
			cfg.cfg_dev[n].play_enabled_cfg = 1;
		} else {
			cfg.cfg_dev[n].play_enabled_cfg = 0;
		}

		if (main_sc.ScDeviceBits & (MPP_DEV0_PLAY << (3 * n))) {
			main_sc.ScPlayDevice = n;
		}
	}

	umidi20_config_import(&cfg);

	main_sc.ScDeviceBits &= ~(MPP_DEV0_PLAY|MPP_DEV1_PLAY|MPP_DEV2_PLAY);
	main_sc.ScDeviceBits |= MPP_DEV0_PLAY << (3 * main_sc.ScPlayDevice);

	handle_compile();

	main_sc.ScBpmAutoPlayOld = main_sc.ScBpmAutoPlay;

	handle_config_revert();

	pthread_mutex_lock(&main_sc.mtx);
	for (y = 0; y != MPP_MAX_DEVS; y++) {
		if (check_synth(y)) {
			uint8_t buf[4];
			/* set local on all channels */
			for (x = 0; x != 16; x++) {
				buf[0] = 0xB0 | x;
				buf[1] = 0x7A;
				buf[2] = main_sc.ScSynthIsLocal ? 0x7F : 0x00;
				mid_add_raw(&mid_data, buf, 3, x);
			}
		}
	}
	pthread_mutex_unlock(&main_sc.mtx);
}

void
MppMainWindow :: handle_config_revert()
{
	int n;

	for (n = 0; n != MPP_MAX_DEVS; n++) {

		if (main_sc.ScDeviceName[n] != NULL)
			led_config_dev[n]->setText(QString(main_sc.ScDeviceName[n]));
		else
			led_config_dev[n]->setText(QString());

		cbx_config_dev[(3*n)+0]->setChecked(
		    (main_sc.ScDeviceBits & (1UL << ((3*n)+0))) ? 1 : 0);

		cbx_config_dev[(3*n)+1]->setChecked(
		    (main_sc.ScDeviceBits & (1UL << ((3*n)+1))) ? 1 : 0);

		cbx_config_dev[(3*n)+2]->setChecked(
		    (main_sc.ScDeviceBits & (1UL << ((3*n)+2))) ? 1 : 0);
	}

	spn_bpm_length->setValue(main_sc.ScBpmAvgLength);
	spn_auto_play->setValue(main_sc.ScBpmAutoPlayOld);
}

void
MppMainWindow :: handle_config_apply()
{
	int n;

	main_sc.ScDeviceBits = 0;

	for (n = 0; n != MPP_MAX_DEVS; n++) {

		if (main_sc.ScDeviceName[n] != NULL)
			free(main_sc.ScDeviceName[n]);

		main_sc.ScDeviceName[n] = MppQStringToAscii(led_config_dev[n]->text());

		if (cbx_config_dev[(3*n)+0]->isChecked())
			main_sc.ScDeviceBits |= 1UL << ((3*n)+0);
		if (cbx_config_dev[(3*n)+1]->isChecked())
			main_sc.ScDeviceBits |= 1UL << ((3*n)+1);
		if (cbx_config_dev[(3*n)+2]->isChecked())
			main_sc.ScDeviceBits |= 1UL << ((3*n)+2);
	}

	n = spn_bpm_length->value();

	pthread_mutex_lock(&main_sc.mtx);
	main_sc.ScBpmAvgLength = n;
	pthread_mutex_unlock(&main_sc.mtx);

	handle_config_reload();
}

void
MppMainWindow :: handle_config_fontsel()
{
	bool success;
	QFont font = QFontDialog::getFont(&success, default_font, this);
	if (success) {
		default_font = font;
		MppParseVisualEntries(&main_sc, NULL, QPoint(0,0), 1.0);

		/* update minimum width */
		scores_wg->setMinimumWidth(main_sc.ScMaxScoresWidth);
	}
}

uint8_t
MppMainWindow :: check_synth(uint8_t device_no)
{
	struct mid_data *d = &mid_data;
	uint32_t pos;

	if (device_no >= MPP_MAX_DEVS)
		return (0);

	if (main_sc.ScDeviceBits & (MPP_DEV0_SYNTH << (3 * device_no))) {

		handle_midi_trigger();

		pos = umidi20_get_curr_position() - main_sc.ScStartPosition + 1;

		mid_set_channel(d, main_sc.ScSynthChannel);
		mid_set_position(d, pos);
		mid_set_device_no(d, device_no);

		return (1);
	}
	return (0);
}

uint8_t
MppMainWindow :: check_record()
{
	struct mid_data *d = &mid_data;
	uint32_t pos;
	uint8_t chan;

	if (main_sc.ScMidiRecordOff)
		return (0);

	handle_midi_trigger();

	pos = (umidi20_get_curr_position() - main_sc.ScStartPosition) % 0x40000000UL;
	if (pos < MPP_MIN_POS)
		pos = MPP_MIN_POS;

	chan = main_sc.ScSynthChannel;
	main_sc.ScChanUsageMask |= (1 << chan);

	mid_set_channel(d, chan);
	mid_set_position(d, pos);
	mid_set_device_no(d, 0xFF);

	return (1);
}

void
MppMainWindow :: do_key_press(struct mid_data *d, int key, int vel, int dur)
{
	int ch = mid_get_channel(d);

	if (vel != 0) {
		vel = (main_sc.ScSynthVolume[ch] * vel) / MPP_VOLUME_UNIT;

		if (vel > 127)
			vel = 127;
		else if (vel < 0)
			vel = 0;
		else if (vel == 1)
			vel = 2;

		vel &= 0x7E;	/* hint for "MidiEventTxCallback()" */
	}

	if (key > 127)
		return;
	else if (key < 0)
		return;

	if (dur < 0)
		return;

	mid_key_press(d, key, vel, dur);
}

void
MppMainWindow :: handle_stop(void)
{
	struct mid_data *d = &mid_data;
	uint8_t ScMidiTriggered;
	uint8_t out_key;
	uint8_t old_chan;
	uint8_t chan;
	uint8_t x;
	uint8_t y;
	uint8_t delay;

	ScMidiTriggered = main_sc.ScMidiTriggered;
	main_sc.ScMidiTriggered = 1;

	for (x = 0; x != MPP_PRESSED_MAX; x++) {
		if (main_sc.ScPressed[x] != 0) {

			out_key = (main_sc.ScPressed[x] >> 8) & 0xFF;
			chan = (main_sc.ScPressed[x] >> 16) & 0xFF;
			delay = (main_sc.ScPressed[x] >> 24) & 0xFF;

			old_chan = main_sc.ScSynthChannel;
			main_sc.ScSynthChannel = chan;

			main_sc.ScPressed[x] = 0;

			for (y = 0; y != MPP_MAX_DEVS; y++) {
				if (check_synth(y)) {
					mid_delay(d, delay);
					do_key_press(d, out_key, 0, 0);
				}
			}

			if (check_record()) {
				mid_delay(d, delay);
				do_key_press(d, out_key, 0, 0);
			}

			/* restore synth channel */
			main_sc.ScSynthChannel = old_chan;
		}
	}

	main_sc.ScMidiTriggered = ScMidiTriggered;
}

uint8_t
MppMainWindow :: handle_jump(int pos, int do_jump)
{
	if ((pos < 0) || (pos > 7) || (main_sc.ScJumpTable[pos] == 0))
		return (0);

	if (do_jump == 0)
		return (1);

	main_sc.ScCurrPos = main_sc.ScJumpTable[pos] - 1;
	main_sc.ScLastPos = main_sc.ScCurrPos;

	handle_stop();

	return (1);
}

int
MppMainWindow :: set_pressed_key(int chan, int out_key, int dur, int delay)
{
	uint8_t y;

	for (y = 0; y != MPP_PRESSED_MAX; y++) {
		if (main_sc.ScPressed[y] != 0)
			continue;
		main_sc.ScPressed[y] =
		  (dur & 0xFF) | 
		  ((out_key & 0xFF) << 8) |
		  ((chan & 0xFF) << 16) |
		  ((delay & 0xFF) << 24);
		return (0);
	}
	return (1);
}

void
MppMainWindow :: handle_key_press(int in_key, int vel)
{
	struct mid_data *d = &mid_data;
	struct MppScore *pn;
	uint16_t pos;
	uint8_t old_chan;
	uint8_t chan;
	uint8_t x;
	uint8_t y;
	uint8_t out_key;
	uint8_t delay;

	pos = main_sc.ScJumpNext[main_sc.ScCurrPos];
	if (pos != 0)
		main_sc.ScCurrPos = pos - 1;

	pn = &main_sc.ScScores[main_sc.ScCurrPos][0];

	for (x = 0; x != MPP_MAX_SCORES; x++, pn++) {

		if (pn->dur != 0) {
			out_key = pn->key + (in_key - main_sc.ScBaseKey);
			out_key &= 127;

			chan = (main_sc.ScSynthChannel + pn->channel) & 15;

			delay = MppNoise(&main_sc, main_sc.ScDelayNoise);

			if (set_pressed_key(chan, out_key, pn->dur, delay))
				continue;

			old_chan = main_sc.ScSynthChannel;
			main_sc.ScSynthChannel = chan;

			for (y = 0; y != MPP_MAX_DEVS; y++) {
				if (check_synth(y)) {
					mid_delay(d, delay);
					do_key_press(d, out_key, vel, 0);
				}
			}

			if (check_record()) {
				mid_delay(d, delay);
				do_key_press(d, out_key, vel, 0);
			}

			/* restore synth channel */
			main_sc.ScSynthChannel= old_chan;
		}
	}

	main_sc.ScLastPos = main_sc.ScCurrPos;
	main_sc.ScCurrPos++;

	if (main_sc.ScCurrPos >= main_sc.ScLinesMax)
		main_sc.ScCurrPos = 0;

	do_update_bpm();
}

/* must be called locked */
void
MppMainWindow :: handle_key_release(int in_key)
{
	struct mid_data *d = &mid_data;
	uint8_t out_key;
	uint8_t old_chan;
	uint8_t chan;
	uint8_t delay;
	uint8_t x;
	uint8_t y;

	for (x = 0; x != MPP_PRESSED_MAX; x++) {
		if ((main_sc.ScPressed[x] & 0xFF) == 1) {

			out_key = (main_sc.ScPressed[x] >> 8) & 0xFF;
			chan = (main_sc.ScPressed[x] >> 16) & 0xFF;
			delay = (main_sc.ScPressed[x] >> 24) & 0xFF;

			/* clear entry */
			main_sc.ScPressed[x] = 0;

			old_chan = main_sc.ScSynthChannel;
			main_sc.ScSynthChannel = chan;

			for (y = 0; y != MPP_MAX_DEVS; y++) {
				if (check_synth(y)) {
					mid_delay(d, delay);
					do_key_press(d, out_key, 0, 0);
				}
			}

			if (check_record()) {
				mid_delay(d, delay);
				do_key_press(d, out_key, 0, 0);
			}

			/* restore synth channel */
			main_sc.ScSynthChannel = old_chan;
		}

		if (main_sc.ScPressed[x] != 0)
			main_sc.ScPressed[x] --;
	}

	main_sc.ScLastPos = main_sc.ScCurrPos;
}

/* This function must be called locked */

void
MppMainWindow :: do_update_bpm(void)
{
	uint32_t delta;
	uint32_t curr;

	curr = umidi20_get_curr_position();
	delta = curr - main_sc.ScLastKeyPress;
	main_sc.ScLastKeyPress = curr;

	/* too big delay */
	if (delta >= (UMIDI20_BPM / 15))
		return;

	/* store statistics */
	main_sc.ScBpmData[main_sc.ScBpmAvgPos] = delta;
	main_sc.ScBpmAvgPos++;

	if (main_sc.ScBpmAvgPos >= main_sc.ScBpmAvgLength)
		main_sc.ScBpmAvgPos = 0;
}

void
MppMainWindow :: do_clock_stats(void)
{
	uint32_t time_offset;
	char buf[32];

	pthread_mutex_lock(&main_sc.mtx);

	if (main_sc.ScMidiTriggered == 0) {
		if (main_sc.ScMidiPaused != 0)
			time_offset = main_sc.ScPausePosition;
		else
			time_offset = 0;
	} else {
		time_offset = (umidi20_get_curr_position() - main_sc.ScStartPosition) % 0x40000000UL;
	}

	time_offset %= 100000000UL;

	pthread_mutex_unlock(&main_sc.mtx);

	snprintf(buf, sizeof(buf), "%d", time_offset);
	lbl_curr_time_val->display(QString(buf));
}

void
MppMainWindow :: do_bpm_stats(void)
{
	uint32_t min = 0xFFFFFFFFUL;
	uint32_t max = 0;
	uint32_t sum = 0;
	uint32_t val;
	uint8_t x;
	uint8_t len;
	char buf[32];

	len = main_sc.ScBpmAvgLength;

	if (len == 0)
		return;

	pthread_mutex_lock(&main_sc.mtx);

	for (x = 0; x != len; x++) {
		val = main_sc.ScBpmData[x];

		sum += val;
		if (val > max)
			max = val;
		if (val < min)
			min = val;
	}

	pthread_mutex_unlock(&main_sc.mtx);

	if (sum == 0)
		sum = 1;
	if (max == 0)
		max = 1;
	if (min == 0)
		min = 1;

	sum = (len * UMIDI20_BPM) / sum;
	max = UMIDI20_BPM / max;
	min = UMIDI20_BPM / min;

	if (sum > 999)
		sum = 999;
	if (min > 999)
		min = 999;
	if (max > 999)
		max = 999;

	snprintf(buf, sizeof(buf), "%d", min);
	lbl_bpm_max_val->display(QString(buf));

	snprintf(buf, sizeof(buf), "%d", max);
	lbl_bpm_min_val->display(QString(buf));

	snprintf(buf, sizeof(buf), "%d", sum);
	lbl_bpm_avg_val->display(QString(buf));
}

/* is called locked */
static void
MidiEventRxCallback(uint8_t device_no, void *arg, struct umidi20_event *event, uint8_t *drop)
{
	MppMainWindow *mw = (MppMainWindow *)arg;
	struct mid_data *d = &mw->mid_data;
	uint8_t y;
	int key;
	int vel;

	*drop = 1;

	if (umidi20_event_get_control_address(event) == 0x40) {

		for (y = 0; y != MPP_MAX_DEVS; y++) {
			if (mw->check_synth(y)) {
				mid_pedal(d, umidi20_event_get_control_value(event));
			}
		}

		if (mw->check_record()) {
			mid_pedal(d, umidi20_event_get_control_value(event));
		}

	} else if (umidi20_event_is_key_start(event)) {

		key = umidi20_event_get_key(event) & 0x7F;
		vel = umidi20_event_get_velocity(event);

		if (mw->main_sc.ScScoreRecordOff == 0) {
			if (mw->main_sc.ScNumInputEvents < MPP_MAX_QUEUE) {
				mw->main_sc.ScInputEvents[mw->main_sc.ScNumInputEvents] = key;
				mw->main_sc.ScNumInputEvents++;
				mw->main_sc.ScLastInputEvent = umidi20_get_curr_position();
			}
		}

		if (mw->main_sc.ScMidiPassThruOff != 0) {
			if (mw->handle_jump(key - mw->main_sc.ScCmdKey, 1) == 0) {
				mw->handle_key_press(key, vel);
			}
		} else if (mw->set_pressed_key(mw->main_sc.ScSynthChannel, key, 255, 0) == 0) {
			for (y = 0; y != MPP_MAX_DEVS; y++) {
				if (mw->check_synth(y)) {
					mw->do_key_press(d, key, vel, 0);
				}
			}

			if (mw->check_record()) {
				mw->do_key_press(d, key, vel, 0);
			}

			mw->do_update_bpm();
		}
	} else if (umidi20_event_is_key_end(event)) {

		key = umidi20_event_get_key(event) & 0x7F;

		if (mw->main_sc.ScMidiPassThruOff != 0) {

			if (mw->handle_jump(key - mw->main_sc.ScCmdKey, 0) == 0) {
				mw->handle_key_release(key);
			}

		} else {

			for (y = 0; y != MPP_MAX_DEVS; y++) {
				if (mw->check_synth(y)) {
					mw->do_key_press(d, key, 0, 0);
				}
			}

			if (mw->check_record()) {
				mw->do_key_press(d, key, 0, 0);
			}

			mw->main_sc.ScPressed[key] = 0;
		}
	} else if (mw->do_instr_check(event)) {
		/* found instrument */
	}
}

/* is called locked */
static void
MidiEventTxCallback(uint8_t device_no, void *arg, struct umidi20_event *event, uint8_t *drop)
{
	MppMainWindow *mw = (MppMainWindow *)arg;
	int vel;

	if (umidi20_event_get_what(event) & UMIDI20_WHAT_CHANNEL) {
		uint8_t chan;

		chan = umidi20_event_get_channel(event) & 0xF;

		vel = umidi20_event_get_velocity(event);

		/* update playback velocity, if any */

		if (vel & 1) {
			vel |= 1;	/* hint for "MidiEventTxCallback()" */

			vel = (vel * mw->main_sc.ScPlayVolume[chan]) / MPP_VOLUME_UNIT;

			if (vel > 127)
				vel = 127;
			else if (vel < 0)
				vel = 0;

			umidi20_event_set_velocity(event, vel);
		}

		*drop = mw->main_sc.ScInstr[chan].muted;
	}
}

/* must be called locked */
uint8_t
MppMainWindow :: do_instr_check(struct umidi20_event *event)
{
	if (umidi20_event_get_what(event) & UMIDI20_WHAT_CONTROL_VALUE) {
		uint8_t addr;
		uint8_t val;
		uint8_t chan;

		addr = umidi20_event_get_control_address(event);
		val = umidi20_event_get_control_value(event);
		chan = umidi20_event_get_channel(event) & 0xF;

		if (addr == 0x00) {
			main_sc.ScInstr[chan].bank &= 0x007F;
			main_sc.ScInstr[chan].bank |= (val << 7);
			main_sc.ScInstr[chan].updated = 1;
			main_sc.ScInstr[chan].muted = 0;
			main_sc.ScInstrUpdated = 1;
			main_sc.ScSynthChannel = chan;
			return (1);
		} else if (addr == 0x20) {
			main_sc.ScInstr[chan].bank &= 0xFF80;
			main_sc.ScInstr[chan].bank |= (val & 0x7F);
			main_sc.ScInstr[chan].updated = 1;
			main_sc.ScInstr[chan].muted = 0;
			main_sc.ScInstrUpdated = 1;
			main_sc.ScSynthChannel = chan;
			return (1);
		}
	} else if (umidi20_event_get_what(event) & UMIDI20_WHAT_PROGRAM_VALUE) {
		uint8_t val;
		uint8_t chan;

		val = umidi20_event_get_program_number(event);
		chan = umidi20_event_get_channel(event) & 0xF;

		main_sc.ScInstr[chan].prog = val;
		main_sc.ScInstr[chan].updated = 1;
		main_sc.ScInstr[chan].muted = 0;
		main_sc.ScInstrUpdated = 1;
		main_sc.ScSynthChannel = chan;
		return (1);
	}
	return (0);
}

void
MppMainWindow :: handle_instr_channel_changed(int chan)
{
	int temp[3];

	pthread_mutex_lock(&main_sc.mtx);
	main_sc.ScSynthChannel = chan;
	temp[0] = main_sc.ScInstr[chan].bank;
	temp[1] = main_sc.ScInstr[chan].prog;
	temp[2] = main_sc.ScSynthVolume[chan];
	pthread_mutex_unlock(&main_sc.mtx);

	spn_instr_curr_bank->setValue(temp[0]);
	spn_instr_curr_prog->setValue(temp[1]);
	spn_volume->setValue(temp[2]);

	if (spn_instr_curr_chan->value() != chan)
		spn_instr_curr_chan->setValue(chan);
}

void
MppMainWindow :: handle_instr_program()
{
	int chan = spn_instr_curr_chan->value();
	int bank = spn_instr_curr_bank->value();
	int prog = spn_instr_curr_prog->value();

	pthread_mutex_lock(&main_sc.mtx);
	main_sc.ScInstr[chan].bank = bank;
	main_sc.ScInstr[chan].prog = prog;
	main_sc.ScInstr[chan].muted = 0;
	main_sc.ScInstr[chan].updated = 1;
	pthread_mutex_unlock(&main_sc.mtx);

	handle_instr_revert();
}

void 
MppMainWindow :: handle_instr_apply()
{
	int temp[3];
	uint8_t x;
	uint8_t update_curr;

	for (x = 0; x != 16; x++) {

		temp[0] = spn_instr_bank[x]->value();
		temp[1] = spn_instr_prog[x]->value();
		temp[2] = cbx_instr_mute[x]->isChecked();

		pthread_mutex_lock(&main_sc.mtx);
		main_sc.ScInstr[x].bank = temp[0];
		main_sc.ScInstr[x].prog = temp[1];
		main_sc.ScInstr[x].muted = temp[2];
		main_sc.ScInstr[x].updated = 1;
		update_curr = (main_sc.ScSynthChannel == x);
		pthread_mutex_unlock(&main_sc.mtx);

		if (update_curr) {
			spn_instr_curr_chan->setValue(x);
			spn_instr_curr_bank->setValue(temp[0]);
			spn_instr_curr_prog->setValue(temp[1]);
		}
	}
	handle_instr_reload();
}

void 
MppMainWindow :: handle_instr_revert()
{
	int temp[3];
	uint8_t x;
	uint8_t update_curr;

	for (x = 0; x != 16; x++) {

		pthread_mutex_lock(&main_sc.mtx);
		temp[0] = main_sc.ScInstr[x].bank;
		temp[1] = main_sc.ScInstr[x].prog;
		temp[2] = main_sc.ScInstr[x].muted;
		update_curr = (main_sc.ScSynthChannel == x);
		pthread_mutex_unlock(&main_sc.mtx);

		spn_instr_bank[x]->setValue(temp[0]);
		spn_instr_prog[x]->setValue(temp[1]);
		cbx_instr_mute[x]->setChecked(temp[2]);

		if (update_curr) {
			spn_instr_curr_chan->setValue(x);
			spn_instr_curr_bank->setValue(temp[0]);
			spn_instr_curr_prog->setValue(temp[1]);
		}
	}
	handle_instr_reload();
}

void 
MppMainWindow :: handle_instr_reset()
{
	uint8_t x;

	for (x = 0; x != 16; x++) {
		spn_instr_bank[x]->setValue(0);
		spn_instr_prog[x]->setValue(0);
		cbx_instr_mute[x]->setChecked(0);
	}

	spn_instr_curr_chan->setValue(0);
	spn_instr_curr_bank->setValue(0);
	spn_instr_curr_prog->setValue(0);

	handle_instr_reload();
}

void
MppMainWindow :: handle_instr_reload()
{
	struct mid_data *d = &mid_data;
	uint8_t x;
	uint8_t y;
	uint8_t chan;
	uint8_t trig;

	pthread_mutex_lock(&main_sc.mtx);
	chan = main_sc.ScSynthChannel;
	trig = main_sc.ScMidiTriggered;
	main_sc.ScMidiTriggered = 1;

	for (x = 0; x != 16; x++) {
		if (main_sc.ScInstr[x].updated == 0)
			continue;

		main_sc.ScSynthChannel = x;
		main_sc.ScInstr[x].updated = 0;
		for (y = 0; y != MPP_MAX_DEVS; y++) {
			if (check_synth(y)) {
				mid_delay(d, (4 * x));
				mid_set_bank_program(d, x, 
				    main_sc.ScInstr[x].bank,
				    main_sc.ScInstr[x].prog);
			}
		}
	}

	/* restore synth channel */
	main_sc.ScSynthChannel = chan;
	main_sc.ScMidiTriggered = trig;
	pthread_mutex_unlock(&main_sc.mtx);
}

void
MppMainWindow :: handle_volume_changed(int vol)
{
	int x;

	pthread_mutex_lock(&main_sc.mtx);
	x = main_sc.ScSynthChannel;
	main_sc.ScSynthVolume[x] = vol;
	pthread_mutex_unlock(&main_sc.mtx);

	spn_volume_synth[x]->setValue(vol);
}

void 
MppMainWindow :: handle_volume_apply()
{
	int temp[2];

	uint8_t x;
	uint8_t update_curr;

	for (x = 0; x != 16; x++) {

		temp[0] = spn_volume_play[x]->value();
		temp[1] = spn_volume_synth[x]->value();

		pthread_mutex_lock(&main_sc.mtx);
		main_sc.ScPlayVolume[x] = temp[0];
		main_sc.ScSynthVolume[x] = temp[1];
		update_curr = (main_sc.ScSynthChannel == x);
		pthread_mutex_unlock(&main_sc.mtx);

		if (update_curr)
			spn_volume->setValue(temp[1]);
	}

	handle_volume_reload();
}

void 
MppMainWindow :: handle_volume_revert()
{
	int temp[2];

	uint8_t x;
	uint8_t update_curr;

	for (x = 0; x != 16; x++) {

		pthread_mutex_lock(&main_sc.mtx);
		temp[0] = main_sc.ScPlayVolume[x];
		temp[1] = main_sc.ScSynthVolume[x];
		update_curr = (main_sc.ScSynthChannel == x);
		pthread_mutex_unlock(&main_sc.mtx);

		spn_volume_play[x]->setValue(temp[0]);
		spn_volume_synth[x]->setValue(temp[1]);

		if (update_curr)
			spn_volume->setValue(temp[1]);
	}

	handle_volume_reload();
}

void 
MppMainWindow :: handle_volume_reset()
{
	uint8_t x;

	for (x = 0; x != 16; x++) {
		spn_volume_play[x]->setValue(MPP_VOLUME_UNIT);
		spn_volume_synth[x]->setValue(MPP_VOLUME_UNIT);
	}

	spn_volume->setValue(MPP_VOLUME_UNIT);

	handle_volume_reload();
}

void
MppMainWindow :: handle_volume_reload()
{

}

void
MppMainWindow :: handle_score_print(void)
{
	QPrinter printer(QPrinter::HighResolution);
	QPrintDialog *dlg;
	QPoint orig;
	qreal scale_f;

	printer.setOutputFormat(QPrinter::PdfFormat);
	printer.setFontEmbeddingEnabled(true);
	printer.setFullPage(true);
	printer.setResolution(600);

	if (CurrScoreFileName != NULL) {
		printer.setOutputFileName(CurrScoreFileName->
		    replace(QString(".txt"), QString(".pdf"),
		    Qt::CaseInsensitive));
	} else {
		printer.setOutputFileName(QString("NewSong.pdf"));
	}

	printer.setColorMode(QPrinter::Color);

	dlg = new QPrintDialog(&printer, this);

	if(dlg->exec() == QDialog::Accepted) {

		orig = QPoint(printer.logicalDpiX() * 0.5,
			      printer.logicalDpiY() * 0.5);

		scale_f = ((qreal)printer.logicalDpiY()) / (qreal)MPP_VISUAL_Y_MAX;

		MppParseVisualEntries(&main_sc, &printer, orig, scale_f);
	}

	delete dlg;
}

void
MppMainWindow :: MidiInit(void)
{
	int n;

	main_sc.ScDeviceBits = MPP_DEV0_SYNTH | MPP_DEV0_PLAY | 
	  MPP_DEV1_RECORD | MPP_DEV2_RECORD | MPP_DEV3_RECORD;
	main_sc.ScDeviceName[0] = strdup("/midi");
	main_sc.ScDeviceName[1] = strdup("/dev/umidi0.0");
	main_sc.ScDeviceName[2] = strdup("/dev/umidi1.0");
	main_sc.ScDeviceName[3] = strdup("/dev/umidi2.0");
	main_sc.ScBpmAvgLength = 4;

	handle_midi_record();
	handle_midi_play();
	handle_score_record();
	handle_pass_thru();
	handle_instr_apply();
	handle_volume_apply();

	for (n = 0; n != UMIDI20_N_DEVICES; n++) {
		umidi20_set_record_event_callback(n, &MidiEventRxCallback, this);
		umidi20_set_play_event_callback(n, &MidiEventTxCallback, this);
	}

	pthread_mutex_lock(&main_sc.mtx);

	song = umidi20_song_alloc(&main_sc.mtx, UMIDI20_FILE_FORMAT_TYPE_0, 500,
	    UMIDI20_FILE_DIVISION_TYPE_PPQ);

	track = umidi20_track_alloc();

	if (song == NULL || track == NULL) {
		pthread_mutex_unlock(&main_sc.mtx);
		err(1, "Could not allocate new song or track\n");
	}
	umidi20_song_track_add(song, NULL, track, 0);

	umidi20_song_set_record_track(song, track);

	/* get the MIDI up! */

	mid_init(&mid_data, track);

	umidi20_song_start(song, 0x40000000, 0x80000000,
	    UMIDI20_FLAG_PLAY | UMIDI20_FLAG_RECORD);

	main_sc.ScStartPosition = umidi20_get_curr_position() - 0x40000000;

	pthread_mutex_unlock(&main_sc.mtx);

	/* reload the configuration */

	handle_config_reload();
}

void
MppMainWindow :: MidiUnInit(void)
{
	int n;

	handle_rewind();

	pthread_mutex_lock(&main_sc.mtx);

	umidi20_song_free(song);

	song = NULL;

	umidi20_song_stop(song, UMIDI20_FLAG_PLAY | UMIDI20_FLAG_RECORD);

	pthread_mutex_unlock(&main_sc.mtx);

	for (n = 0; n != MPP_MAX_DEVS; n++) {
		if (main_sc.ScDeviceName[n] != NULL) {
			free (main_sc.ScDeviceName[n]);
			main_sc.ScDeviceName[n] = NULL;
		}
	}
}

void
MppMainWindow :: keyPressEvent(QKeyEvent *event)
{
#if 0
	if (event->isAutoRepeat())
		return;

	if (event->key() == Qt::Key_PageUp || 
	    event->key() == Qt::Key_PageDown) {
		handle_play_press();
	}
#endif
}

void
MppMainWindow :: keyReleaseEvent(QKeyEvent *event)
{
#if 0
	if (event->isAutoRepeat())
		return;

	if (event->key() == Qt::Key_PageUp || 
	    event->key() == Qt::Key_PageDown) {
		handle_play_release();
	}
#endif
}

int
main(int argc, char **argv)
{
	QApplication app(argc, argv);

	umidi20_init();

	MppMainWindow main;

	main.show();

	return (app.exec());
}
