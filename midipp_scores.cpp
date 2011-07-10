/*-
 * Copyright (c) 2009-2011 Hans Petter Selasky. All rights reserved.
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
#include <midipp_pattern.h>

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
	    " * Copyright (c) 2009-2011 Hans Petter Selasky. All rights reserved.\n"
	    " */\n"

	    "\n"
	    "/*\n"
	    " * Command syntax:\n"
	    " * U<number>[.] - specifies the duration of the following scores (0..255).\n"
	    " * T<number> - specifies the track number of the following scores (0..31).\n"
	    " * K<number> - defines a command (0..99).\n"
	    " * W<number>.<number> - defines an autoplay timeout (1..9999ms).\n"
	    " * K0 - no operation.\n"
	    " * K1 - lock play key until next label jump.\n"
	    " * K2 - unlock play key.\n"
	    " * L<number> - defines a label (0..31).\n"
	    " * J<R><P><number> - jumps to the given label (0..31) or \n"
	    " *     Relative(R) line (0..31) and starts a new page(P).\n"
	    " * S\"<string>\" - creates a visual string.\n"
	    " * CDEFGAH<number><B> - defines a score in the given octave (0..10).\n"
	    " * X[+/-]<number> - defines the transpose level of the following scores in half-steps.\n"
	    " */\n"
	    "\n"
	    "S\"(L0:) .Welcome .to .MIDI .Player .Pro!\"\n"
	    "\nC3"
	    "\nC3"
	    "\nD3"
	    "\nE3"
	    "\nC3"
	    "\n");

	/* set memory default */

	memset(auto_zero_start, 0, auto_zero_end - auto_zero_start);

	/* all devices are input */

	devInputMask = 0;
	baseKey = C4;
	cmdKey = C3;
	delayNoise = 25;

	/* Set parent */

	mainWindow = parent;

	/* Buttons */

	butScoreFileNew = new QPushButton(tr("New"));
	butScoreFileOpen = new QPushButton(tr("Open"));
	butScoreFileSave = new QPushButton(tr("Save"));
	butScoreFileSaveAs = new QPushButton(tr("Save As"));
	butScoreFilePrint = new QPushButton(tr("Print"));

	connect(butScoreFileNew, SIGNAL(pressed()), this, SLOT(handleScoreFileNew()));
	connect(butScoreFileOpen, SIGNAL(pressed()), this, SLOT(handleScoreFileOpen()));
	connect(butScoreFileSave, SIGNAL(pressed()), this, SLOT(handleScoreFileSave()));
	connect(butScoreFileSaveAs, SIGNAL(pressed()), this, SLOT(handleScoreFileSaveAs()));
	connect(butScoreFilePrint, SIGNAL(pressed()), this, SLOT(handleScorePrint()));

	/* Widget */

	viewWidget.setContentsMargins(0,0,0,0);

	/* Editor */

	editWidget = new QPlainTextEdit();

	editWidget->setFont(font_fixed);
	editWidget->setPlainText(defaultText);
	editWidget->setCursorWidth(4);
	editWidget->setLineWrapMode(QPlainTextEdit::NoWrap);

	/* GridLayout */

	viewGrid = new QGridLayout(&viewWidget);
	viewGrid->setSpacing(0);
	viewGrid->setContentsMargins(1,1,1,1);

	viewScroll = new QScrollBar(Qt::Vertical);
	viewScroll->setValue(0);
	viewScroll->setMinimum(0);
	viewScroll->setMaximum(0);
	viewScroll->setPageStep(1);

	connect(viewScroll, SIGNAL(valueChanged(int)), this, SLOT(handleScrollChanged(int)));

	/* Visual */

	viewWidgetSub = new MppScoreView(this);

	viewGrid->addWidget(viewWidgetSub, 0, 0, 1, 1);
	viewGrid->addWidget(viewScroll, 0, 1, 1, 1);

	/* Initial compile */

	handleCompile();
}

MppScoreMain :: ~MppScoreMain()
{
	handleScoreFileNew();
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
			} else if (jumpLabel[x] != 0) {
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
			} else if (jumpLabel[x] != 0) {
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
	QPainter paint(viewWidgetSub);
	uint16_t max;
	uint16_t pos;
	uint16_t opos;
	uint16_t scroll;
	uint16_t y_blocks;
	uint16_t y_div;
	uint16_t y_rem;
	uint16_t yc_div;
	uint16_t yc_rem;
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
	scroll = picScroll;
	pthread_mutex_unlock(&mainWindow->mtx);

	y_blocks = (viewWidgetSub->height() / MPP_VISUAL_Y_MAX);
	if (y_blocks == 0)
		y_blocks = 1;
	y_div = 0;
	y_rem = 0;
	yc_div = 0;
	yc_rem = 0;
	yo_div = 0;
	yo_rem = 0;

	/* locate current scroll position */

	for (x = y = 0; x != max; x++) {

		if (visual[x].pic != NULL)
			y++;

		if ((y != 0) && ((y - 1) >= scroll)) {
			y_rem = (y - 1);
			break;
		}
	}

	/* locate current play position */

	for (x = y = 0; x != max; x++) {

		if (visual[x].pic != NULL)
			y++;

		if ((y != 0) && (x >= pos)) {
			yc_rem = (y - 1);
			break;
		}
	}

	/* locate last play position */

	for (x = y = 0; x != max; x++) {

		if (visual[x].pic != NULL)
			y++;

		if ((y != 0) && (x >= opos)) {
			yo_rem = (y - 1);
			break;
		}
	}

	/* compute scrollbar */

	y_div = y_rem / y_blocks;
	y_rem = y_rem % y_blocks;

	/* align current position with scroll bar */

	if (yc_rem < y_rem) {
		yc_rem = 0;
		yc_div = picMax;
	} else {
		yc_rem -= y_rem;
		yc_div = yc_rem / y_blocks;
		yc_rem = yc_rem % y_blocks;
	}

	/* align last position with scroll bar */

	if (yo_rem < y_rem) {
		yo_rem = 0;
		yo_div = picMax;
	} else {
		yo_rem -= y_rem;
		yo_div = yo_rem / y_blocks;
		yo_rem = yo_rem % y_blocks;
	}

	/* paint */

	for (x = y = z = 0; x != max; x++) {

		if (visual[x].pic != NULL) {
			if ((y >= y_rem) &&
			    (y_div == ((y - y_rem) / y_blocks))) {
				/* compute mouse press jump positions */
				if (z < MPP_MAX_LINES) {
					mousePressPos[z] = x;
					z++;
				}
				paint.drawPixmap(
				     QPoint(0, ((y - y_rem) % y_blocks) * MPP_VISUAL_Y_MAX),
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

	if ((yc_div == y_div) && (visual[pos].x_off != 0)) {
		paint.setPen(QPen(color_logo, 4));
		paint.setBrush(QColor(color_logo));
		paint.drawEllipse(QRect(visual[pos].x_off,
		    visual[pos].y_off + (yc_rem * MPP_VISUAL_Y_MAX),
		    MPP_VISUAL_R_MAX, MPP_VISUAL_R_MAX));
	}
}

void
MppScoreMain :: newLine()
{
	if (ps.line >= MPP_MAX_LINES)
		return;
	if (ps.index == 0 &&
	    timer_ticks_pre[ps.line] == 0 &&
	    timer_ticks_post[ps.line] == 0)
		return;

	realLine[ps.line] = ps.realLine;
	ps.line++;
	ps.index = 0;
}

void
MppScoreMain :: newVisual()
{
	if (ps.line >= MPP_MAX_LINES)
		goto done;

	bufData[ps.bufIndex] = 0;

	if (visual[ps.line].pstr != NULL)
		free(visual[ps.line].pstr);

	visual[ps.line].pstr = strdup(bufData);
done:
	ps.bufIndex = 0;
}

char
MppScoreMain :: getChar(uint32_t offset)
{
	char c;

	offset += ps.x;

	if (offset >= (uint32_t)ps.ps->size())
		return (0);

	c = (*(ps.ps))[offset].toAscii();
	/* convert non-printable characters into space */
	if (c == 0)
		return (' ');
	return (c);
}

void
MppScoreMain :: parseAdv(uint8_t delta)
{
	char c;
	while (delta--) {
		if (ps.x >= 0) {
			c = getChar(0);
			if (c == '\n')
				ps.realLine++;
		}
		ps.x++;
	}
}

uint32_t
MppScoreMain :: resolveJump(uint32_t line)
{
	uint32_t jump;

	if (line >= MPP_MAX_LINES) {
		jump = 1;	/* jump to next */
	} else {
		uint8_t lbl;

		lbl = jumpLabel[line];
		if (lbl >= MPP_JUMP_REL)
			jump = line + 2 + (lbl - MPP_JUMP_REL);
		else if (lbl > 0 && lbl <= MPP_MAX_LABELS)
			jump = jumpTable[lbl - 1];
		else
			jump = 0;
	}
	/* clip */
	if (jump != 0 && jump > (uint32_t)ps.line)
		jump = 1;
	return (jump);
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
	int timer;
	int negative;
	int transpose;

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

	memset(scores, 0, sizeof(scores));
	memset(jumpLabel, 0, sizeof(jumpLabel));
	memset(jumpTable, 0, sizeof(jumpTable));
	memset(pageNext, 0, sizeof(pageNext));
	memset(realLine, 0, sizeof(realLine));
	memset(playCommand, 0, sizeof(playCommand));
	memset(timer_ticks_pre, 0, sizeof(timer_ticks_pre));
	memset(timer_ticks_post, 0, sizeof(timer_ticks_post));
	active_channels = 1;

	if (pstr.isNull() || pstr.isEmpty())
		goto done;

next_line:
	newLine();

	y = -1;
	channel = 0;
	duration = 1;
	transpose = 0;

next_char:
	parseAdv(1);
	y++;

	c = getChar(0);
	c = toupper(c);

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
	case 'W':
		if (y == 0) {
			goto parse_timer;
		}
		goto next_char;
	case 'X':
		if (y == 0) {
			goto parse_transpose;
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
		c = getChar(1);
		if (c == '*') {
			while (1) {
				c = getChar(2);
				if (c == '*') {
					c = getChar(3);
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
		goto next_char;
	default:
		goto next_char;
	}

parse_score:

	c = getChar(1);
	if (c >= '0' && c <= '9') {
		d = getChar(2);
		if (d >= '0' && d <= '9') {
			base_key += 120 * (c - '0');
			base_key += 12 * (d - '0');
			parseAdv(2);
		} else {
			base_key += 12 * (c - '0');
			parseAdv(1);
		}
		c = getChar(1);
		if (c == 'B' || c == 'b') {
			base_key -= 1;
			parseAdv(1);
		}
		/* transpose, if any */
		base_key += transpose;

		if ((ps.line < MPP_MAX_LINES) && (ps.index < MPP_MAX_SCORES)) {
			scores[ps.line][ps.index].key = base_key & 127;
			scores[ps.line][ps.index].dur = duration & 255;
			scores[ps.line][ps.index].channel = channel & 15;
			ps.index++;
		}
	}
	goto next_char;

parse_duration:

	c = getChar(1);
	if (c >= '0' && c <= '9') {
		d = getChar(2);
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

	c = getChar(1);
	if (c == '.') {
		parseAdv(1);
	} else 	if (duration != 0)
		duration --;

	goto next_char;

parse_timer:

	c = getChar(1);
	if (c >= '0' && c <= '9') {
		timer = (c - '0');

		d = getChar(2);
		if (d >= '0' && d <= '9') {
			timer *= 10;
			timer += (d - '0');

			d = getChar(3);
			if (d >= '0' && d <= '9') {
				timer *= 10;
				timer += (d - '0');

				d = getChar(4);
				if (d >= '0' && d <= '9') {
					timer *= 10;
					timer += (d - '0');
					parseAdv(4);
				} else {
					parseAdv(3);
				}
			} else {
				parseAdv(2);
			}
		} else {
			parseAdv(1);
		}
	} else {
		timer = 0;
	}

	if (ps.line < MPP_MAX_LINES)
		timer_ticks_pre[ps.line] = timer;

	c = getChar(1);
	if (c == '.') {

		c = getChar(2);
		if (c >= '0' && c <= '9') {
			timer = (c - '0');

			d = getChar(3);
			if (d >= '0' && d <= '9') {
				timer *= 10;
				timer += (d - '0');

				d = getChar(4);
				if (d >= '0' && d <= '9') {
					timer *= 10;
					timer += (d - '0');

					d = getChar(5);
					if (d >= '0' && d <= '9') {
						timer *= 10;
						timer += (d - '0');
						parseAdv(5);
					} else {
						parseAdv(4);
					}
				} else {
					parseAdv(3);
				}
			} else {
				parseAdv(2);
			}
		} else {
			parseAdv(1);
			timer = 0;
		}
	} else {
		timer = 0;
	}

	if (ps.line < MPP_MAX_LINES)
		timer_ticks_post[ps.line] = timer;

	goto next_char;

parse_channel:

	/* check for channel number */
	c = getChar(1);
	if (c >= '0' && c <= '9') {
		d = getChar(2);
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

parse_transpose:

	negative = 0;

	c = getChar(1);

	/* check sign, if any */
	if (c == '-') {
		negative = 1;
		parseAdv(1);
	} else if (c == '+') {
		negative = 0;
		parseAdv(1);
	} else {
		negative = 0;
	}

	c = getChar(1);

	/* check for transpose value */
	if (c >= '0' && c <= '9') {
		d = getChar(2);
		if (d >= '0' && d <= '9') {
			transpose = (10 * (c - '0')) + (d - '0');
			parseAdv(2);
		} else {
			transpose = (c - '0');
			parseAdv(1);
		}
		if (negative)
			transpose = -transpose;
	} else {
		transpose = 0;
	}
	goto next_char;

parse_command:

	c = getChar(1);
	if (c >= '0' && c <= '9') {
		d = getChar(2);
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
	if (ps.line < MPP_MAX_LINES)
		playCommand[ps.line] = command;

	goto next_char;

parse_label:

	c = getChar(1);
	if (c >= '0' && c <= '9') {
		d = getChar(2);
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

	c = getChar(1);
	if (c != '\"')
		goto next_char;

	/* check if the current line already has a string */
	if (ps.line < MPP_MAX_LINES && visual[ps.line].pstr != NULL) {
		jumpLabel[ps.line] = MPP_JUMP_REL;
		realLine[ps.line] = ps.realLine;
		ps.line++;
	}

	while ((c = getChar(2)) != 0) {
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
	flag = 0;

parse_jump_sub:

	c = getChar(1);
	if (c == 'R' || c == 'r') {
		c = getChar(2);
		parseAdv(1);
		flag |= 2;
		goto parse_jump_sub;
	}

	if (c == 'P' || c == 'p') {
		c = getChar(2);
		parseAdv(1);
		flag |= 1;
		goto parse_jump_sub;
	}

	if (c >= '0' && c <= '9') {
		label = (c - '0');

		c = getChar(2);
		if (c >= '0' && c <= '9') {
			label *= 10;
			label += (c - '0');
			parseAdv(2);
		} else {
			parseAdv(1);
		}

		if (label < 0 || label >= MPP_MAX_LABELS)
			goto next_char;

		/* check for relative jump */

		if (flag & 2)
			label += MPP_JUMP_REL - 1;

	} else {
		/* no jump */

		label = MPP_JUMP_REL - 1;
	}

	newLine();

	if (ps.line < MPP_MAX_LINES) {
		if (flag & 1)
			pageNext[ps.line] = 1;

		jumpLabel[ps.line] = label + 1;
		realLine[ps.line] = ps.realLine;
		ps.line++;
	}
	goto next_char;

done:
	newLine();

	if (ps.bufIndex != 0)
		newVisual();

	linesMax = ps.line;
	currPos = 0;
	lastPos = 0;

	isPlayKeyLocked = 0;

	handleParseSub(NULL, QPoint(0,0), 1.0);

	picMax = 0;
	for (z = 0; z != linesMax; z++) {
		if (visual[z].pic != NULL)
			picMax++;
	}

	if (picMax == 0) {
		viewScroll->setValue(0);
		viewScroll->setMaximum(0);
	} else {
		viewScroll->setValue(0);
		viewScroll->setMaximum(picMax - 1);
	}
}

void
MppScoreMain :: handleScoreFileNew()
{
	editWidget->setPlainText(QString());
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
		QString(), QString("Score File (*.txt *.TXT)"));
	QString scores;
	QString status;

	diag->setAcceptMode(QFileDialog::AcceptOpen);
	diag->setFileMode(QFileDialog::ExistingFile);

	if (diag->exec()) {
		handleScoreFileNew();

		currScoreFileName = new QString(diag->selectedFiles()[0]);

		scores = MppReadFile(*currScoreFileName, &status);

		editWidget->setPlainText(scores);

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
		QString(), QString("Score File (*.txt *.TXT)"));

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
	if ((pos < 0) || (pos >= 12) || (pos >= MPP_MAX_LABELS) ||
	    (jumpTable[pos] == 0))
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
	isPlayKeyLocked = 0;

	mainWindow->cursorUpdate = 1;

	mainWindow->handle_stop(1);
}

/* must be called locked */
void
MppScoreMain :: handleKeyPress(int in_key, int vel, uint32_t key_delay)
{
	struct MppScoreEntry *pn;
	uint32_t timeout = 0;
	uint32_t t_pre;
	uint32_t t_post;
	uint16_t pos;
	uint8_t chan;
	uint8_t x;
	uint8_t out_key;
	uint8_t delay;

 repeat:
	for (x = 0; x != 128; x++) {
		pos = resolveJump(currPos);
		if (pos != 0) {
			/* avoid infinite loops */
			if (timeout != 0)
				goto done;
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

	t_pre = timer_ticks_pre[currPos];
	t_post = timer_ticks_pre[currPos];

	if (timer_was_active && t_pre == 0 && t_post == 0)
		goto done;

	timer_was_active = 0;

	decrementDuration();

	last_key = in_key;
	last_vel = vel;

	pn = &scores[currPos][0];

	for (x = 0; x != MPP_MAX_SCORES; x++, pn++) {

		if (pn->dur != 0) {
			out_key = pn->key + (in_key - baseKey);
			out_key &= 127;

			chan = (synthChannel + pn->channel) & 0xF;

			delay = mainWindow->noise8(delayNoise);

			if (setPressedKey(chan, out_key, pn->dur, delay))
				continue;

			mainWindow->output_key(chan, out_key, vel, key_delay + timeout + delay, 0, x);
		}
	}

	lastPos = currPos;
	currPos++;

	if (currPos >= linesMax) {
		currPos = 0;
		isPlayKeyLocked = 0;
	}

	if (t_pre || t_post) {
		timeout += t_pre;
		decrementDuration(timeout);
		timeout += t_post;
		timer_was_active = 1;

		/* avoid infinite loops */
		if (currPos != 0)
			goto repeat;
	}

done:
	mainWindow->cursorUpdate = 1;

	if (mainWindow->currScoreMain == this)
		mainWindow->do_update_bpm();
}

/* must be called locked */
void
MppScoreMain :: decrementDuration(uint32_t timeout)
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

			mainWindow->output_key(chan, out_key, 0, timeout + delay, 0, x);
		}

		if (pressedKeys[x] != 0)
			pressedKeys[x] --;
	}
}

/* must be called locked */
void
MppScoreMain :: handleKeyRelease(int in_key, uint32_t key_delay)
{
	if (isPlayKeyLocked != 0) {
		if (in_key != (int)whatPlayKeyLocked) {
			return;
		}
	}

	if (timer_was_active) {
		timer_was_active = 0;
		return;
	}

	decrementDuration(key_delay);

	lastPos = currPos;
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

		viewWidgetSub->setMinimumWidth(maxScoresWidth);
	}
}

void
MppScoreMain :: watchdog()
{
	uint16_t x;
	uint16_t y;
	uint16_t max;
	uint16_t pos;
	uint16_t y_blocks;

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

	/* Compute scrollbar */

	pthread_mutex_lock(&mainWindow->mtx);
	pos = currPos;
	max = linesMax;
	pthread_mutex_unlock(&mainWindow->mtx);

	/* Compute alignment factor */

	y_blocks = (viewWidgetSub->height() / MPP_VISUAL_Y_MAX);
	if (y_blocks == 0)
		y_blocks = 1;

	/* Compute picture index */

	for (x = y = 0; x != max; x++) {

		if (visual[x].pic != NULL)
			y++;

		if (x >= pos) {
			if (y != 0)
				break;
		}
	}
	if (y == 0 || y > picMax)
		viewScroll->setValue(0);
	else
		viewScroll->setValue((y - 1) - ((y - 1) % y_blocks));
}

void
MppScoreMain :: handleScrollChanged(int value)
{
	pthread_mutex_lock(&mainWindow->mtx);
	picScroll = value;
	pthread_mutex_unlock(&mainWindow->mtx);

	viewWidgetSub->repaint();
}
