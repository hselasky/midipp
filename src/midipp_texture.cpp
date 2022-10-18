/*-
 * Copyright (c) 2022 Hans Petter Selasky.
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

#include "midipp_texture.h"

#include <QPainter>

static void
round_corners(QImage *p, int r)
{
	const QSize s(p->size());

	r = qMin(r, qMin(s.width(),s.height()));
	if (r <= 1)
		return;

	const int zz = r * r;

	for (int x = 0; x != r; x++) {
		const int xx = x * x;

		for (int y = r; y--; ) {
			const int yy = y * y;

			const QPoint pos[4] = {
				QPoint(r - 1 - x, r - 1 - y),
				QPoint(s.width() - 1 - (r - 1 - x), s.height() - 1 - (r - 1 - y)),
				QPoint(r - 1 - x, s.height() - 1 - (r - 1 - y)),
				QPoint(s.width() - 1 - (r - 1 - x), r - 1 - y),
			};

			if (xx + yy > zz) {
				p->setPixel(pos[0],0);
				p->setPixel(pos[1],0);
				p->setPixel(pos[2],0);
				p->setPixel(pos[3],0);
			} else {
				QColor cc[4] = {
					p->pixelColor(pos[0]),
					p->pixelColor(pos[1]),
					p->pixelColor(pos[2]),
					p->pixelColor(pos[3]),
				};

				cc[0].setAlpha(cc[0].alpha() / 2);
				cc[1].setAlpha(cc[1].alpha() / 2);
				cc[2].setAlpha(cc[2].alpha() / 2);
				cc[3].setAlpha(cc[3].alpha() / 2);

				p->setPixelColor(pos[0],cc[0]);
				p->setPixelColor(pos[1],cc[1]);
				p->setPixelColor(pos[2],cc[2]);
				p->setPixelColor(pos[3],cc[3]);

				break;
			}
		}
	}
}

void
MppRounded :: paintEvent(QWidget *w, QPaintEvent *event)
{
	const QSize s = w->size();

	if (s.width() <= 0 || s.height() <= 0)
		return;

	QImage p(s.width(), s.height(), QImage::Format_ARGB32);
	QRect clip(event->region().boundingRect());

	if (clip.x() == 0 && clip.y() == 0 && clip.size() == s) {
		/* paint everything */
		p.fill(rgb);
	} else {
		/* only paint the part needed */
		for (int x = 0; x != clip.width(); x++) {
			for (int y = 0; y != clip.height(); y++) {
				p.setPixelColor(clip.x() + x, clip.y() + y, rgb);
			}
		}
	}

	round_corners(&p, r);

	QPainter paint(w);

	paint.setClipRegion(clip);
	paint.drawImage(0,0,p);
	paint.end();
};
