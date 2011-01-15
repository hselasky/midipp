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

#include <midipp_mainwindow.h>
#include <midipp_scores.h>
#include <midipp_looptab.h>
#include <midipp_import.h>

static void MppTimerCallback(void *arg);
static void MppPllCallback(void *arg);

MppScoreView :: MppScoreView(MppScoreMain *parent)
{
	pScores = parent;
}

void
MppScoreView :: mousePressEvent(QMouseEvent *e)
{
	pScores->viewMousePressEvent(e);
}

void
MppScoreView :: paintEvent(QPaintEvent *event)
{
	pScores->viewPaintEvent(event);
}

MppScoreMain :: MppScoreMain(MppMainWindow *parent)
{
	QString defaultText = tr(
	    "/*\n"
	    " * Copyright (c) 2009-2010 Hans Petter Selasky. All rights reserved.\n"
	    " */\n"

	    "\n"
	    "/*\n"
	    " * Command syntax:\n"
	    " * U<number>[.] - specifies the duration of the following scores (0..255)\n"
	    " * T<number> - specifies the track number of the following scores (0..31)\n"
	    " * K<number> - defines a command (0..99)\n"
	    " * K0 - no operation\n"
	    " * K1 - lock play key until next label jump\n"
	    " * K2 - unlock play key\n"
	    " * L<number> - defines a label (0..31)\n"
	    " * M<bpm 0..60000>[.<duty 0..99>] - configure auto PLL\n"
	    " * J<number> - jumps to the given label (0..31)\n"
	    " * JP<number> - jumps to the given label (0..31) and starts a new page\n"
	    " * S\"<string>\" - creates a visual string\n"
	    " * CDEFGAH<number><B> - defines a score in the given octave (0..10)\n"
	    " * { - start PLL program\n"
	    " * } - stop PLL program\n"
	    " */\n"
	    "\n"
	    "S\"(L0:) .Welcom.e!\"\n"
	    "\nC3"
	    "\nC3\n");

	/* set memory default */

	memset(auto_zero_start, 0, auto_zero_end - auto_zero_start);

	/* Set parent */

	mainWindow = parent;

	/* Buttons */

	butScoreFileNew = new QPushButton(tr("New"));
	butScoreFileOpen = new QPushButton(tr("Open"));
	butScoreFileSave = new QPushButton(tr("Save"));
	butScoreFileSaveAs = new QPushButton(tr("Save As"));
	butScoreFilePrint = new QPushButton(tr("Print"));
	butScoreFileImport = new QPushButton(tr("Import"));

	connect(butScoreFileNew, SIGNAL(pressed()), this, SLOT(handleScoreFileNew()));
	connect(butScoreFileOpen, SIGNAL(pressed()), this, SLOT(handleScoreFileOpen()));
	connect(butScoreFileSave, SIGNAL(pressed()), this, SLOT(handleScoreFileSave()));
	connect(butScoreFileSaveAs, SIGNAL(pressed()), this, SLOT(handleScoreFileSaveAs()));
	connect(butScoreFilePrint, SIGNAL(pressed()), this, SLOT(handleScorePrint()));
	connect(butScoreFileImport, SIGNAL(pressed()), this, SLOT(handleScoreImport()));

	/* Editor */

	editWidget = new QTextEdit();

	editWidget->setText(defaultText);
	editWidget->setCursorWidth(4);
	editWidget->setLineWrapMode(QTextEdit::NoWrap);

	/* Visual */

	viewWidget = new MppScoreView(this);

	/* Initial compile */

	handleCompile();
}

MppScoreMain :: ~MppScoreMain()
{
	handleScoreFileNew();

	umidi20_unset_timer(&MppPllCallback, this);

	umidi20_unset_timer(&MppTimerCallback, this);
}

void
MppScoreMain :: parseMax(uint16_t *pmax, float value)
{
	value += MPP_VISUAL_MARGIN;

	if (value > 2048.0)
		value = 2048.0;

	if (value < 0.0)
		value = 0.0;

	if ((uint16_t)value > *pmax)
		*pmax = value;
}

void
MppScoreMain :: handleParseSub(QPrinter *pd, QPoint orig, float scale_f)
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
	uint16_t duration;

	float chord_x;
	float text_x;
	float adj_x;
	float scale_min;

	QFont fnt_a;
	QFont fnt_b;

	pic = NULL;

	fnt_a = mainWindow->defaultFont;
	fnt_a.setPixelSize(20);

	fnt_b = mainWindow->defaultFont;
	fnt_b.setPixelSize(24);

	maxScoresWidth = 0;

	for (x = 0; x != MPP_MAX_LINES; x++) {
		visual[x].x_off = 0;
	}

	if (pd != NULL) {

		/* count number of lines */
		for (x = y_max = y = 0; x != MPP_MAX_LINES; x++) {

			if (pageNext[x] != 0) {
				if (y > y_max)
					y_max = y;
				y = 0;
				last_jump = 0;
			} else if (jumpNext[x] != 0) {
				y++;
				last_jump = 1;
			}

			ptr = visual[x].pstr;
			if (ptr == NULL)
				continue;

			y++;
			last_jump = 0;
		}

		y -= last_jump;

		if (y > y_max)
			y_max = y;

		if (y_max != 0) {
			scale_min = ((float)(pd->height() - (2 * orig.y()))) / 
			    (((float)y_max) * MPP_VISUAL_Y_MAX);

			if (scale_min < 0)
				scale_f = 0.5;	/* dummy */
			else if (scale_min < scale_f)
				scale_f = scale_min;
		}

		paint.begin(pd);
		paint.translate(orig);
		paint.scale(scale_f, scale_f);
		paint.translate(QPoint(0, -MPP_VISUAL_Y_MAX));
	}

	y_max = 0;

	for (x = 0; x != MPP_MAX_LINES; x++) {

		if (pd == NULL) {

			if (visual[x].pic != NULL) {
				delete (visual[x].pic);
				visual[x].pic = NULL;
			}
		} else {

			if (pageNext[x] != 0) {
				pd->newPage();
				while (y_max--)
					paint.translate(QPoint(0, -MPP_VISUAL_Y_MAX));
				y_max = 0;
			} else if (jumpNext[x] != 0) {
				paint.translate(QPoint(0,MPP_VISUAL_Y_MAX));
				y_max++;
			}
		}

		ptr = visual[x].pstr;
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

				if (ptr[y+1] == '[') {
					if ((ptr[y+2] >= '0') && (ptr[y+2] <= '9')) {
						if ((ptr[y+3] >= '0') && (ptr[y+3] <= '9')) {
							duration = 9;
						} else {
							duration = ptr[y+2] - '0';
						}
					} else {
						duration = 0;
					}
					while ((ptr[y+1] != ']') && (ptr[y+1] != 0))
						y++;

					if (ptr[y+1] != 0)
						y++;
				} else {
					duration = 0;
				}

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

					uint32_t foff;

					visual[z].x_off = text_x + adj_x;
					visual[z].y_off = (MPP_VISUAL_Y_MAX/3);

					parseMax(&maxScoresWidth, 
					    visual[z].x_off + MPP_VISUAL_R_MAX);

					parseMax(&x_max, 
					    visual[z].x_off + MPP_VISUAL_R_MAX);

					paint.drawEllipse(QRect(visual[z].x_off,
						visual[z].y_off,
						MPP_VISUAL_R_MAX, MPP_VISUAL_R_MAX));

					if (duration != 0) {

					  foff = 0;

					  if (duration > 5)
						duration = 5;

					  paint.drawLine(
					      visual[z].x_off + MPP_VISUAL_R_MAX,
					      visual[z].y_off + (MPP_VISUAL_R_MAX/2),
					      visual[z].x_off + MPP_VISUAL_R_MAX,
					      visual[z].y_off + (MPP_VISUAL_R_MAX/2) -
					      (3*MPP_VISUAL_R_MAX));

					  while (duration--) {
						paint.drawLine(
						    visual[z].x_off + MPP_VISUAL_R_MAX,
						    visual[z].y_off + (MPP_VISUAL_R_MAX/2) -
						    (3*MPP_VISUAL_R_MAX) + foff,
						    visual[z].x_off,
						    visual[z].y_off + MPP_VISUAL_R_MAX -
						    (3*MPP_VISUAL_R_MAX) + foff);

						foff += (MPP_VISUAL_R_MAX/2);
					  }
					}
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

				parseMax(&maxScoresWidth, chord_x);

				parseMax(&x_max, chord_x);
			} else {
				paint.drawText(QPointF(text_x, MPP_VISUAL_Y_MAX -
				    (MPP_VISUAL_Y_MAX/4) - MPP_VISUAL_MARGIN), temp);

				text_x += temp_size.width();
				chord_x = text_x;
				last_dot = 0;

				parseMax(&maxScoresWidth, text_x);

				parseMax(&x_max, text_x);
			}
		}

		if (pd == NULL) {
			int w;
			int h;

			w = x_max;
			h = MPP_VISUAL_Y_MAX;

			paint.end();

			if (w && h) {
				visual[x].pic = new QPixmap(w, h);

				paint.begin(visual[x].pic);
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

void
MppScoreMain :: viewMousePressEvent(QMouseEvent *e)
{
	int yi;
	uint16_t max;

	yi = e->y() / MPP_VISUAL_Y_MAX;

	if ((yi < 0) || (yi >= MPP_MAX_LINES))
		return;

	pthread_mutex_lock(&mainWindow->mtx);

	max = linesMax;

	if (mousePressPos[yi] < max) {
		pllPos = 0;
		pllDuration = 0;
		currPos = mousePressPos[yi];
		lastPos = mousePressPos[yi];
		isPlayKeyLocked = 0;
	}

	mainWindow->handle_stop();

	pthread_mutex_unlock(&mainWindow->mtx);
}

void
MppScoreMain :: viewPaintEvent(QPaintEvent *event)
{
	QPainter paint(viewWidget);
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
	uint16_t z;

	paint.fillRect(event->rect(), color_white);

	pthread_mutex_lock(&mainWindow->mtx);
	pos = currPos;
	opos = lastPos;
	max = linesMax;
	pthread_mutex_unlock(&mainWindow->mtx);

	y_blocks = (viewWidget->height() / MPP_VISUAL_Y_MAX);
	if (y_blocks == 0)
		y_blocks = 1;
	y_div = 0;
	y_rem = 0;
	yo_div = 0;
	yo_rem = 0;

	/* locate */

	for (x = y = 0; x != max; x++) {

		if (visual[x].pic != NULL) {
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

		if (visual[x].pic != NULL) {
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

	for (x = y = z = 0; x != max; x++) {

		if (visual[x].pic != NULL) {
			if (y_div == (y / y_blocks)) {
				/* compute mouse press jump positions */
				if (z < MPP_MAX_LINES) {
					mousePressPos[z] = x;
					z++;
				}
				paint.drawPixmap(
				    QPoint(0, (y % y_blocks) * MPP_VISUAL_Y_MAX),
				    *(visual[x].pic));
			}
			y++;
		}
	}

	/* fill out rest of mousePressPos[] */
	while ((z < MPP_MAX_LINES) && (z <= y_blocks)) {
			mousePressPos[z] = 65535;
			z++;
	}

	/* overlay (last) */

	if ((pos != opos) && (yo_div == y_div) &&
	    (visual[opos].x_off != 0)) {

		paint.setPen(QPen(color_green, 4));
		paint.setBrush(QColor(color_green));
		paint.drawEllipse(QRect(visual[opos].x_off,
		    visual[opos].y_off + (yo_rem * MPP_VISUAL_Y_MAX),
		    MPP_VISUAL_R_MAX, MPP_VISUAL_R_MAX));

	}

	/* overlay (current) */

	if (visual[pos].x_off != 0) {
		paint.setPen(QPen(color_logo, 4));
		paint.setBrush(QColor(color_logo));
		paint.drawEllipse(QRect(visual[pos].x_off,
		    visual[pos].y_off + (y_rem * MPP_VISUAL_Y_MAX),
		    MPP_VISUAL_R_MAX, MPP_VISUAL_R_MAX));
	}
}

void
MppScoreMain :: newLine()
{
	if (ps.pll_active) {
		if (ps.pll_line >= MPP_MAX_LINES)
			return;
		if (ps.pll_index == 0)
			return;

		ps.pll_line++;
		ps.pll_index = 0;
	} else {
		if (ps.line >= MPP_MAX_LINES)
			return;
		if (ps.index == 0)
			return;

		realLine[ps.line] = ps.realLine;
		ps.line++;
		ps.index = 0;
	}
}

void
MppScoreMain :: newVisual()
{
	if (ps.line >= MPP_MAX_LINES)
		goto done;
	if (ps.pll_active)
		goto done;

	bufData[ps.bufIndex] = 0;

	if (visual[ps.line].pstr != NULL)
		free(visual[ps.line].pstr);

	visual[ps.line].pstr = strdup(bufData);
done:
	ps.bufIndex = 0;
}

void
MppScoreMain :: parseAdv(uint8_t delta)
{
	while (delta--) {
		if (ps.x >= 0) {
			int c;
			c = (*(ps.ps))[ps.x].toUpper().toAscii();
			if (c == '\n')
				ps.realLine++;
		}
		ps.x++;
	}
}

void
MppScoreMain :: handleParse(const QString &pstr)
{
	int z;
	int c;
	int d;
	int y;
	int label;
	int channel;
	int command;
	int base_key;
	int duration;
	int flag;

	/* cleanup all scores */

	for (z = 0; z != MPP_MAX_LINES; z++) {
		if (visual[z].pstr != NULL) {
			free(visual[z].pstr);
			visual[z].pstr = NULL;
		}
	}

	ps.x = -1;
	ps.line = 0;
	ps.realLine = 0;
	ps.index = 0;
	ps.bufIndex = 0;
	ps.ps = &pstr;
	ps.pll_active = 0;
	ps.pll_bpm = 120;
	ps.pll_duty = 50;	/* per cent */
	ps.pll_line = 0;
	ps.pll_index = 0;

	memset(scores, 0, sizeof(scores));
	memset(jumpNext, 0, sizeof(jumpNext));
	memset(jumpTable, 0, sizeof(jumpTable));
	memset(pageNext, 0, sizeof(pageNext));
	memset(realLine, 0, sizeof(realLine));
	memset(playCommand, 0, sizeof(playCommand));
	memset(pll_scores, 0, sizeof(pll_scores));
	memset(pll_bpm, 0, sizeof(pll_bpm));
	memset(pll_duty, 0, sizeof(pll_duty));
	memset(pll_start, 0, sizeof(pll_start));
	memset(pll_duration, 0, sizeof(pll_duration));
	memset(pll_jump, 0, sizeof(pll_jump));
	active_channels = 1;

	if (pstr.isNull() || pstr.isEmpty())
		goto done;

next_line:
	newLine();

	y = -1;
	channel = 0;
	duration = 1;

next_char:
	parseAdv(1);
	y++;

	c = pstr[ps.x].toUpper().toAscii();

	switch (c) {
	case 'M':
		if (y == 0) {
			goto parse_pll_bpm;
		}
		goto next_char;
	case '{':
		if (y == 0) {
			goto pll_start;
		}
		goto next_char;
	case '}':
		if (y == 0) {
			goto pll_stop;
		}
		goto next_char;
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
	case 'K':
		if (y == 0) {
			goto parse_command;
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
	case ';':
		if (y == 0) {
			goto next_line;
		}
		goto next_char;
	case '/':
		if (y != 0)
			goto next_char;

		/* check for comment */
		c = pstr[ps.x+1].toUpper().toAscii();
		if (c == '*') {
			while (1) {
				c = pstr[ps.x+2].toUpper().toAscii();
				if (c == '*') {
					c = pstr[ps.x+3].toUpper().toAscii();
					if (c == '/') {
						/* end of comment */
						parseAdv(3);
						break;
					}
				}
				if (c == 0)
					goto done;
				parseAdv(1);
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

parse_pll_bpm:
	ps.pll_bpm = 0;
	ps.pll_duty = 0;
	while (1) {
		c = pstr[ps.x+1].toAscii();
		if (c >= '0' && c <= '9') {
			ps.pll_bpm *= 10;
			ps.pll_bpm += c - '0';
			parseAdv(1);
		} else {
			break;
		}
	}
	if (c == '.') {
		parseAdv(1);

		while (1) {
			c = pstr[ps.x+1].toAscii();
			if (c >= '0' && c <= '9') {
				ps.pll_duty *= 10;
				ps.pll_duty += c - '0';
				parseAdv(1);
			} else {
				break;
			}
		}
	}

	if (ps.pll_bpm < 0)
		ps.pll_bpm = 0;
	else if (ps.pll_bpm > 60000)
		ps.pll_bpm = 60000;

	if (ps.pll_duty < 0)
		ps.pll_duty = 0;
	else if (ps.pll_duty > 99)
		ps.pll_duty = 99;

	goto next_char;

pll_start:
	if (ps.pll_active == 0) {
		newLine();
		if ((ps.line > 0) && (ps.line < MPP_MAX_LINES) &&
		    (ps.pll_line < MPP_MAX_LINES)) {
			pll_start[ps.line - 1] = ps.pll_line + 1;
			pll_duration[ps.line - 1] = duration & 255;
			pll_bpm[ps.line - 1] = ps.pll_bpm;
			pll_duty[ps.line - 1] = ps.pll_duty;
		}
		ps.pll_active = 1;
	}
	goto next_char;

pll_stop:
	if (ps.pll_active) {
		newLine();
		if ((ps.line > 0) && (ps.line < MPP_MAX_LINES) &&
		    (ps.pll_line < MPP_MAX_LINES)) {
			pll_jump[ps.pll_line] = ps.line;
			ps.pll_line++;
		}
		ps.pll_active = 0;
	}
	goto next_char;

parse_score:
	c = pstr[ps.x+1].toAscii();
	if (c >= '0' && c <= '9') {
		d = pstr[ps.x+2].toAscii();
		if (d >= '0' && d <= '9') {
			base_key += 120 * (c - '0');
			base_key += 12 * (d - '0');
			parseAdv(2);
		} else {
			base_key += 12 * (c - '0');
			parseAdv(1);
		}
		c = pstr[ps.x+1].toUpper().toAscii();
		if (c == 'B') {
			base_key -= 1;
			parseAdv(1);
		}
		if (ps.pll_active) {
			if ((ps.pll_line < MPP_MAX_LINES) && (ps.pll_index < MPP_MAX_SCORES)) {
				pll_scores[ps.pll_line][ps.pll_index].key = base_key & 127;
				pll_scores[ps.pll_line][ps.pll_index].dur = duration & 255;
				pll_scores[ps.pll_line][ps.pll_index].channel = channel & 15;
				ps.pll_index++;
			}
		} else {
			if ((ps.line < MPP_MAX_LINES) && (ps.index < MPP_MAX_SCORES)) {
				scores[ps.line][ps.index].key = base_key & 127;
				scores[ps.line][ps.index].dur = duration & 255;
				scores[ps.line][ps.index].channel = channel & 15;
				ps.index++;
			}
		}
	}
	goto next_char;

parse_duration:
	c = pstr[ps.x+1].toAscii();
	if (c >= '0' && c <= '9') {
		d = pstr[ps.x+2].toAscii();
		if (d >= '0' && d <= '9') {
			duration = (10 * (c - '0')) + (d - '0');
			parseAdv(2);
		} else {
			duration = (c - '0');
			parseAdv(1);
		}
	} else {
		duration = 0;
	}

	duration *= 2;

	c = pstr[ps.x+1].toAscii();
	if (c == '.') {
		parseAdv(1);
	} else 	if (duration != 0)
		duration --;

	goto next_char;

parse_channel:
	c = pstr[ps.x+1].toAscii();
	if (c >= '0' && c <= '9') {
		d = pstr[ps.x+2].toAscii();
		if (d >= '0' && d <= '9') {
			channel = (10 * (c - '0')) + (d - '0');
			parseAdv(2);
		} else {
			channel = (c - '0');
			parseAdv(1);
		}
	} else {
		channel = 0;
	}
	if (channel < 16)
		active_channels |= (1 << channel);
	goto next_char;

parse_command:
	c = pstr[ps.x+1].toAscii();
	if (c >= '0' && c <= '9') {
		d = pstr[ps.x+2].toAscii();
		if (d >= '0' && d <= '9') {
			command = (10 * (c - '0')) + (d - '0');
			parseAdv(2);
		} else {
			command = (c - '0');
			parseAdv(1);
		}
	} else {
		command = MPP_CMD_NOP;		/* NOP */
	}
	if (ps.pll_active == 0 && ps.line < MPP_MAX_LINES)
		playCommand[ps.line] = command;

	goto next_char;

parse_label:
	if (ps.pll_active)
		goto next_char;

	c = pstr[ps.x+1].toAscii();
	if (c >= '0' && c <= '9') {
		d = pstr[ps.x+2].toAscii();
		if (d >= '0' && d <= '9') {
			label = (10 * (c - '0')) + (d - '0');
			parseAdv(2);
		} else {
			label = (c - '0');
			parseAdv(1);
		}
	} else {
		label = 0;
	}
	if ((label >= 0) && (label < MPP_MAX_LABELS))
		jumpTable[label] = ps.line + 1;

	goto next_char;

parse_string:
	if (ps.pll_active)
		goto next_char;

	c = pstr[ps.x+1].toAscii();
	if (c != '\"')
		goto next_char;

	/* check if the current line already has a string */
	if (ps.line < MPP_MAX_LINES && visual[ps.line].pstr != NULL) {
		jumpNext[ps.line] = MPP_JUMP_NOP;
		realLine[ps.line] = ps.realLine;
		ps.line++;
	}

	while ((c = pstr[ps.x+2].toAscii()) != 0) {
		if (c == '\"') {
			newVisual();
			break;
		}
		if (c == '\r' || c == '\n') {
			/* drop character */
			parseAdv(1);
			continue;
		}
		if (c == '\n') {
			/* new line */
			parseAdv(1);
			continue;
		}
		if (ps.bufIndex == (sizeof(bufData) - 1)) {
			/* wrap long line */
			newVisual();
		}
		bufData[ps.bufIndex] = c;
		ps.bufIndex++;
		parseAdv(1);
	}
	parseAdv(1);
	goto next_char;

parse_jump:
	if (ps.pll_active)
		goto next_char;

	c = pstr[ps.x+1].toAscii();

	flag = 0;

	if (c == 'P') {
		c = pstr[ps.x+2].toAscii();
		parseAdv(1);
		flag |= 1;
	}

	if (c >= '0' && c <= '9') {
		d = pstr[ps.x+2].toAscii();
		if (d >= '0' && d <= '9') {
			label = (10 * (c - '0')) + (d - '0');
			parseAdv(2);
		} else {
			label = (c - '0');
			parseAdv(1);
		}
	} else {
		label = 0;

		flag |= 2;
	}

	newLine();

	if (flag & 1) {
		if (ps.line < MPP_MAX_LINES)
			pageNext[ps.line] = 1;
	}

	/* no jump number - ignore */

	if (flag & 2)
		goto next_char;

	if ((label >= 0) && 
	    (label < MPP_MAX_LABELS) && 
	    (ps.line < MPP_MAX_LINES)) {
		jumpNext[ps.line] = label + 1;
		realLine[ps.line] = ps.realLine;
		ps.line++;
	}
	goto next_char;

done:
	if (ps.pll_active) {
		newLine();
		if ((ps.line > 0) &&
		    (ps.line < MPP_MAX_LINES) && (ps.pll_line < MPP_MAX_LINES)) {
			pll_jump[ps.pll_line] = ps.line;
			ps.pll_line++;
		}
		ps.pll_active = 0;
	}

	newLine();

	if (ps.bufIndex != 0)
		newVisual();

	/* resolve all jumps */
	for (z = 0; z != ps.line; z++) {
		if (jumpNext[z] != 0) {
			if (jumpNext[z] == MPP_JUMP_NOP)
				jumpNext[z] = z + 2;
			else
				jumpNext[z] = jumpTable[jumpNext[z] - 1];

			/* clip */
			if (jumpNext[z] > ps.line)
				jumpNext[z] = 1;
		}
	}

	linesMax = ps.line;
	currPos = 0;
	pllPos = 0;
	pllDuration = 0;
	lastPos = 0;
	isPlayKeyLocked = 0;

	handleParseSub(NULL, QPoint(0,0), 1.0);
}

static void
MppTimerCallback(void *arg)
{
	MppScoreMain *sm = (MppScoreMain *)arg;
	MppMainWindow *mw = sm->mainWindow;
	int key;

	pthread_mutex_lock(&mw->mtx);
	if (mw->midiTriggered) {
		key = mw->playKey;
		sm->handleKeyPress(key, 127);
		sm->handleKeyRelease(key);
	}
	pthread_mutex_unlock(&mw->mtx);
}

static void
MppPllCallback(void *arg)
{
	MppScoreMain *sm = (MppScoreMain *)arg;
	MppMainWindow *mw = sm->mainWindow;

	pthread_mutex_lock(&mw->mtx);
	if (mw->midiTriggered) {
		sm->handlePllPress();
	}
	pthread_mutex_unlock(&mw->mtx);
}

/* must be called locked */
void
MppScoreMain :: updateTimer()
{
	int bpm = bpmAutoPlay;
	int i;

	if (bpm > 0) {
		i = 60000 / bpm;
		if (i == 0)
			i = 1;
	} else {
		i = 0;
	}

	umidi20_set_timer(&MppTimerCallback, this, i);
}

void
MppScoreMain :: handleAutoPlay(int bpm)
{
	pthread_mutex_lock(&mainWindow->mtx);

	if (bpmAutoPlay != (uint32_t)bpm) {
		bpmAutoPlay = bpm;
		updateTimer();
	}

	pthread_mutex_unlock(&mainWindow->mtx);
}

void
MppScoreMain :: handleChannelChanged(int chan)
{
	pthread_mutex_lock(&mainWindow->mtx);
	synthChannel = chan;
	pthread_mutex_unlock(&mainWindow->mtx);
}

void
MppScoreMain :: handleScoreFileNew()
{
	editWidget->setText(QString());
	mainWindow->lbl_file_status->setText(QString());

	handleCompile();

	if (currScoreFileName != NULL) {
		delete (currScoreFileName);
		currScoreFileName = NULL;
	}
}

void
MppScoreMain :: handleScoreFileOpen()
{
	QFileDialog *diag = 
	  new QFileDialog(mainWindow, tr("Select Score File"), 
		QString(), QString("Score File (*.txt; *.TXT)"));
	QString scores;
	QString status;

	diag->setAcceptMode(QFileDialog::AcceptOpen);
	diag->setFileMode(QFileDialog::ExistingFile);

	if (diag->exec()) {
		handleScoreFileNew();

		currScoreFileName = new QString(diag->selectedFiles()[0]);

		scores = MppReadFile(*currScoreFileName, &status);

		editWidget->setText(scores);

		handleCompile();

		mainWindow->lbl_file_status->setText(status);
	}

	delete diag;
}

void
MppScoreMain :: handleScoreFileSave()
{
	QString status;

	if (currScoreFileName != NULL) {
		MppWriteFile(*currScoreFileName, editWidget->toPlainText(), 
		    &status);

		mainWindow->lbl_file_status->setText(status);
	} else {
		handleScoreFileSaveAs();
	}
}

void
MppScoreMain :: handleScoreFileSaveAs()
{
	QFileDialog *diag = 
	  new QFileDialog(mainWindow, tr("Select Score File"), 
		QString(), QString("Score File (*.txt; *.TXT)"));

	diag->setAcceptMode(QFileDialog::AcceptSave);
	diag->setFileMode(QFileDialog::AnyFile);

	if (diag->exec()) {
		if (currScoreFileName != NULL)
			delete (currScoreFileName);

		currScoreFileName = new QString(diag->selectedFiles()[0]);

		if (currScoreFileName != NULL)
			handleScoreFileSave();
	}

	delete diag;
}

int
MppScoreMain :: checkLabelJump(int pos)
{
	if ((pos < 0) || (pos > 7) || (jumpTable[pos] == 0))
		return (0);

	return (1);
}

int
MppScoreMain :: checkHalfPassThru(int key)
{
	static const uint8_t is_black[12] = {0,1,0,1,0,0,1,0,1,0,1,0};

	return ((key >= mid_next_key(baseKey, -1)) &&
	    (key <= mid_next_key(baseKey, +1)) &&
	    (is_black[((uint8_t)key) % 12U] ==
	     is_black[((uint8_t)baseKey) % 12U]));
}

/* must be called locked */
void
MppScoreMain :: handleLabelJump(int pos)
{
	if (checkLabelJump(pos) == 0)
		return;

	lastPos = currPos = jumpTable[pos] - 1;
	pllDuration = 0;
	pllPos = 0;
	isPlayKeyLocked = 0;

	mainWindow->cursorUpdate = 1;

	mainWindow->handle_stop();
}

/* must be called locked */
void
MppScoreMain :: handleKeyPress(int in_key, int vel)
{
	struct MppScoreEntry *pn;
	uint16_t pos;
	uint8_t chan;
	uint8_t x;
	uint8_t out_key;
	uint8_t delay;

	for (x = 0; x != 128; x++) {
		pos = jumpNext[currPos];
		if (pos != 0) {
			/* check for real jump */
			if ((pos - 2) != currPos)
				isPlayKeyLocked = 0;

			/* set new position */
			currPos = pos - 1;
		} else {
			break;
		}
	}

	switch (playCommand[currPos]) {
	case MPP_CMD_LOCK:
		whatPlayKeyLocked = in_key;
		isPlayKeyLocked = 1;
		break;
	case MPP_CMD_UNLOCK:
		isPlayKeyLocked = 0;
		break;
	default:
		break;
	}

	if (isPlayKeyLocked != 0) {
		if (in_key != (int)whatPlayKeyLocked) {
			return;
		}
	}

	decrementDuration();

	last_key = in_key;
	last_vel = vel;

	if (pll_duration[currPos] != 0)
		handleStartPll();

	pn = &scores[currPos][0];

	for (x = 0; x != MPP_MAX_SCORES; x++, pn++) {

		if (pn->dur != 0) {
			out_key = pn->key + (in_key - baseKey);
			out_key &= 127;

			chan = (synthChannel + pn->channel) & 0xF;

			delay = mainWindow->noise8(delayNoise);

			if (setPressedKey(chan, out_key, pn->dur, delay))
				continue;

			mainWindow->output_key(chan, out_key, vel, delay, 0, x);
		}
	}

	lastPos = currPos;
	currPos++;

	if (currPos >= linesMax) {
		currPos = 0;
		isPlayKeyLocked = 0;
	}

	mainWindow->cursorUpdate = 1;

	if (mainWindow->currScoreMain == this)
		mainWindow->do_update_bpm();
}

/* must be called locked */
void
MppScoreMain :: handlePllPress()
{
	struct MppScoreEntry *pn;
	int in_key;
	int vel;
	uint16_t pos;
	uint8_t chan;
	uint8_t x;
	uint8_t out_key;
	uint8_t delay;

	pos = pll_jump[pllPos];
	if (pos != 0) {
		/* indirect lookup */
		pllPos = pos - 1;
		pllPos = pll_start[pllPos];
		if (pllPos != 0)
			pllPos --;
	}

	in_key = last_key;
	vel = last_vel;

	pn = &pll_scores[pllPos][0];

	for (x = 0; x != MPP_MAX_SCORES; x++, pn++) {

		if (pn->dur != 0) {
			out_key = pn->key + (in_key - baseKey);
			out_key &= 127;

			chan = (synthChannel + pn->channel) & 0xF;

			delay = mainWindow->noise8(delayNoise);

			pll_pressed[out_key] = chan + 1;

			mainWindow->output_key(chan, out_key, vel, delay,
			    (pllCycleMs * (pn->dur - 1)) + pllDutyMs, x);
		}
	}

	pllPos++;

	if (pllPos >= MPP_MAX_LINES)
		pllPos = 0;
}

/* must be called locked */
void
MppScoreMain :: decrementDuration()
{
	uint8_t out_key;
	uint8_t chan;
	uint8_t delay;
	uint8_t x;

	for (x = 0; x != MPP_PRESSED_MAX; x++) {
		if ((pressedKeys[x] & 0xFF) == 1) {

			out_key = (pressedKeys[x] >> 8) & 0xFF;
			chan = (pressedKeys[x] >> 16) & 0xFF;
			delay = (pressedKeys[x] >> 24) & 0xFF;

			/* clear entry */
			pressedKeys[x] = 0;

			mainWindow->output_key(chan, out_key, 0, delay, 0, x);
		}

		if (pressedKeys[x] != 0)
			pressedKeys[x] --;
	}
}

/* must be called locked */
void
MppScoreMain :: handleKeyRelease(int in_key)
{
	if (isPlayKeyLocked != 0) {
		if (in_key != (int)whatPlayKeyLocked) {
			return;
		}
	}

	decrementDuration();

	lastPos = currPos;

	if (pllDuration > 0) {
		pllDuration--;
		if (pllDuration == 0)
			handleStopPll();
	}
}

void
MppScoreMain :: handleScorePrint(void)
{
	QPrinter printer(QPrinter::HighResolution);
	QPrintDialog *dlg;
	QPoint orig;
	qreal scale_f;
	QString temp;

	/* make sure everything is up-to-date */

	handleCompile();

	printer.setOutputFormat(QPrinter::PdfFormat);
	printer.setFontEmbeddingEnabled(true);
	printer.setFullPage(true);
	printer.setResolution(600);

	if (currScoreFileName != NULL) {
		temp = *currScoreFileName;
		temp.replace(QString(".txt"), QString(".pdf"),
		     Qt::CaseInsensitive);

		printer.setOutputFileName(temp);
	} else {
		printer.setOutputFileName(QString("NewSong.pdf"));
	}

	printer.setColorMode(QPrinter::Color);

	dlg = new QPrintDialog(&printer, mainWindow);

	if(dlg->exec() == QDialog::Accepted) {

		orig = QPoint(printer.logicalDpiX() * 0.5,
			      printer.logicalDpiY() * 0.5);

		scale_f = ((qreal)printer.logicalDpiY()) / (qreal)MPP_VISUAL_Y_MAX;

		handleParseSub(&printer, orig, scale_f);
	}

	delete dlg;
}

int
MppScoreMain :: setPressedKey(int chan, int out_key, int dur, int delay)
{
	uint32_t temp;
	uint8_t y;

	dur &= 0xFF;
	out_key &= 0xFF;
	chan &= 0xFF;
	delay &= 0xFF;

	temp = dur | (out_key << 8) | (chan << 16) | (delay << 24);

	if (dur == 0) {
		/* release key */
		for (y = 0; y != MPP_PRESSED_MAX; y++) {
			if ((pressedKeys[y] & 0x00FFFF00U) == (temp & 0x00FFFF00U)) {
				/* key information matches */
				/* clear key */
				pressedKeys[y] = 0;
			}
		}
		return (0);
	} else {
		/* pre-press key */
		for (y = 0; y != MPP_PRESSED_MAX; y++) {
			if ((pressedKeys[y] & 0x00FFFF00U) == (temp & 0x00FFFF00U)) {
				/* key already set */
				return (1);
			}
		}

		/* press key */
		for (y = 0; y != MPP_PRESSED_MAX; y++) {
			if (pressedKeys[y] != 0)
				continue;	/* key in use */

			pressedKeys[y] = temp;	/* set key */

			return (0);
		}
		return (1);
	}
}

void
MppScoreMain :: handleCompile()
{
	QString temp;

	temp = editWidget->toPlainText();

	if (temp != editText) {
		editText = temp;

		pthread_mutex_lock(&mainWindow->mtx);

		handleParse(editText);

		pthread_mutex_unlock(&mainWindow->mtx);

		viewWidget->setMinimumWidth(maxScoresWidth);
	}
}

void
MppScoreMain :: watchdog()
{
	QTextCursor cursor(editWidget->textCursor());

	QTextEdit::ExtraSelection format;

	/* Highlight the next line to be played */

	cursor.movePosition(QTextCursor::EndOfLine, QTextCursor::KeepAnchor, 1);

	format.cursor = cursor;
	format.format.setForeground(color_black);
	format.format.setBackground(color_grey);

	QList<QTextEdit::ExtraSelection> extras;
	extras << format;

	editWidget->setExtraSelections(extras);
}

void
MppScoreMain :: handleStartPll()
{
	int bpm;
	int i;
	int duty;

	bpm = pll_bpm[currPos];
	duty = pll_duty[currPos];
	pllDuration = pll_duration[currPos];
	pllPos = pll_start[currPos];

	if (bpm > 0) {
		i = 60000 / bpm;
		if (i == 0)
			i = 1;
	} else {
		i = 0;
	}

	if (pllPos != 0)
		pllPos--;

	pllCycleMs = i;
	if (pllCycleMs == 0)
		pllCycleMs = 1;

	pllDutyMs = (duty * i) / 100;
	if (pllDutyMs == 0)
		pllDutyMs = 1;

	umidi20_set_timer(&MppPllCallback, this, i);
}

void
MppScoreMain :: handleStopPll()
{
	umidi20_unset_timer(&MppPllCallback, this);
	pllDuration = 0;
	pllPos = 0;
}

void
MppScoreMain :: handleScoreImport()
{
	QFileDialog *diag = 
	  new QFileDialog(mainWindow, tr("Select Chord Tabular File"), 
		QString(), QString("Chord Tabular File (*.txt; *.TXT)"));

	diag->setAcceptMode(QFileDialog::AcceptOpen);
	diag->setFileMode(QFileDialog::ExistingFile);

	if (diag->exec()) {
		struct midipp_import ps;
		const char *ptr;

		ptr = MppQStringToAscii(diag->selectedFiles()[0]);
		if (ptr != NULL) {
		  if (midipp_import(ptr, &ps, this)) {
			mainWindow->lbl_file_status->setText(tr("Import failed!\n"));
		  } else {
			mainWindow->lbl_file_status->setText(QString(ptr) + tr(": Import successful!\n"));
		  }
		} else {
			mainWindow->lbl_file_status->setText(tr("Out of memory"));
		}
	}

	delete diag;
}
