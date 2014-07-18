/*-
 * Copyright (c) 2014 Hans Petter Selasky. All rights reserved.
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

#include "midipp_editor.h"

MppEditor :: MppEditor(QWidget * parent) :
    QPlainTextEdit(parent)
{
	timer.setSingleShot(1);
	connect(&timer, SIGNAL(timeout()), this, SLOT(handle_timeout()));
}

MppEditor :: MppEditor(const QString &str, QWidget * parent) :
    QPlainTextEdit(str, parent)
{
	timer.setSingleShot(1);
	connect(&timer, SIGNAL(timeout()), this, SLOT(handle_timeout()));
}

MppEditor :: ~MppEditor()
{
	timer.stop();
}

void
MppEditor :: mousePressEvent(QMouseEvent *event)
{
	if (event->buttons() & Qt::LeftButton)
		timer.start(MPP_POPUP_DELAY);

	QPlainTextEdit::mousePressEvent(event);
}

void
MppEditor :: mouseMoveEvent(QMouseEvent *event)
{
	if (event->buttons() & Qt::LeftButton)
		timer.start(MPP_POPUP_DELAY);

	QPlainTextEdit::mouseMoveEvent(event);
}

void
MppEditor :: mouseReleaseEvent(QMouseEvent *event)
{
	QPlainTextEdit::mouseReleaseEvent(event);

	if (event->button() == Qt::LeftButton)
		timer.stop();
}

void
MppEditor :: handle_timeout()
{
	QMenu *menu = createStandardContextMenu();
	menu->exec(mapToGlobal(pos()));
	delete menu;
}
