/*-
 * Copyright (c) 2022 Hans Petter Selasky
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

#include "midipp_onlinetabs.h"
#include "midipp_mainwindow.h"
#include "midipp_scores.h"
#include "midipp_import.h"
#include "midipp_button.h"

#include <QTextDocumentFragment>
#include <QSslConfiguration>

MppOnlineTabs :: MppOnlineTabs(MppMainWindow *mw)
{
	parent = mw;

	gl = new QGridLayout(this);

	location = new QLineEdit(QString(MPP_NORTABS_URL));
	location->setMaxLength(1024);
	connect(location, SIGNAL(returnPressed()), this, SLOT(handle_return_pressed()));

	result = new MppOnlineTabsBrowse();
	result->setReadOnly(true);
	result->setTabChangesFocus(true);
	connect(result, SIGNAL(linkChanged()), this, SLOT(handle_follow_ref()));

	download = new QPushButton(tr("Reload"));

	connect(download, SIGNAL(released()), this, SLOT(handle_download()));

	open_a = new QPushButton(tr("Import selection to view A"));
	open_b = new QPushButton(tr("Import selection to view B"));

	connect(open_a, SIGNAL(released()), this, SLOT(handle_open_a()));
	connect(open_b, SIGNAL(released()), this, SLOT(handle_open_b()));

	reset = new QPushButton(tr("Reset"));

	connect(reset, SIGNAL(released()), this, SLOT(handle_reset()));

	gl->addWidget(new QLabel(tr("Location:")), 0, 0, 1, 1,
	    Qt::AlignHCenter | Qt::AlignLeft);
	gl->addWidget(location, 0, 1, 1, 3);
	gl->addWidget(download, 0, 5, 1, 1);

	gl->addWidget(result, 2, 0, 1, 6);

	gl->addWidget(open_a, 3, 2, 1, 1);
	gl->addWidget(open_b, 3, 3, 1, 1);
	gl->addWidget(reset, 3, 5, 1, 1);

	gl->setColumnStretch(1, 1);

	connect(&net, SIGNAL(finished(QNetworkReply *)),
	    this, SLOT(handle_download_finished(QNetworkReply *)));
}

void
MppOnlineTabsBrowse :: mousePressEvent(QMouseEvent *event)
{
	if (event->button() & Qt::LeftButton) {
		QString str(anchorAt(event->pos()));
		if (!str.isEmpty()) {
			link = str;
			emit linkChanged();
		}
	}
	QTextEdit::mousePressEvent(event);
}

void
MppOnlineTabs :: handle_follow_ref()
{
	const QString &nstr = result->link;
	QString str(location->text().trimmed());

	if (nstr.startsWith("http://")) {
		location->setText(nstr);
		download->animateClick();
	} else if (nstr.startsWith("https://")) {
		location->setText(nstr);
		download->animateClick();
	} else if (nstr.contains("://")) {
		/* do nothing */
	} else if (str.startsWith("http://")) {
		  for (int i = 7; i != str.length(); i++) {
			if (str[i] == '/') {
				str = str.left(i);
				break;
			}
		  }
		  if (!nstr.startsWith("/"))
			str += '/';
		  str += nstr;
		  location->setText(str);
		  download->animateClick();
	} else if (str.startsWith("https://")) {
		for (int i = 8; i != str.length(); i++) {
			if (str[i] == '/') {
				str = str.left(i);
				break;
			}
		  }
		  if (!nstr.startsWith("/"))
			str += '/';
		  str += nstr;
		  location->setText(str);
		  download->animateClick();
	}
}

void
MppOnlineTabs :: handle_return_pressed()
{
	download->animateClick();
}

void
MppOnlineTabs :: handle_open(int view)
{
	QTextCursor cursor(result->textCursor());
	QString str;

	if (cursor.hasSelection())
		str = cursor.selection().toPlainText();
	else
		str = result->toPlainText();

	str = str.trimmed();
	if (str.isEmpty())
		return;

	parent->scores_main[view]->handleScoreFileNew();
	parent->tab_import->editWidget->setPlainText(str);
	parent->tab_import->handleImport(view);
}

void
MppOnlineTabs :: handle_open_a()
{
#if (MPP_MAX_VIEWS > 0)
	handle_open(0);
#endif
}

void
MppOnlineTabs :: handle_open_b()
{
#if (MPP_MAX_VIEWS > 1)
	handle_open(1);
#endif
}

void
MppOnlineTabs :: handle_reset()
{
	location->setText(QString(MPP_NORTABS_URL));
	result->setPlainText(QString());
	handle_download_finished_sub();
}

void
MppOnlineTabs :: handle_download()
{
	QString str = location->text().trimmed();

	if (str.isEmpty())
		return;

	if (!str.contains("://")) {
		str = QString(MPP_NORTABS_URL "/search/?q=") + str;
		location->setText(str);
	}

	download->setEnabled(0);

	QUrl url(str);
	QNetworkRequest request(url);

	QSslConfiguration conf = request.sslConfiguration();
	conf.setPeerVerifyMode(QSslSocket::VerifyNone);
	request.setSslConfiguration(conf);

	QNetworkReply *reply = net.get(request);

	connect(reply, SIGNAL(downloadProgress(qint64,qint64)),
	    this, SLOT(handle_download_progress(qint64,qint64)));
}

void
MppOnlineTabs :: handle_download_progress(qint64 curr, qint64 total)
{
	if (total < 1)
		total = 1;
	if (curr < 0)
		curr = 0;
	download->setText(tr("Reload %1kb").arg((curr + 1023) / 1024));
}

void
MppOnlineTabs :: handle_download_finished_sub()
{
}

void
MppOnlineTabs :: handle_download_finished(QNetworkReply *reply)
{
	download->setEnabled(1);

	if (reply->error()) {
		QMessageBox::information(this, tr("HTTP"),
			tr("Reload failed: %1.")
			.arg(reply->errorString()));
	} else {
		QString str(reply->readAll());
		/* strip all header tags */
		str = str.replace(QRegularExpression("<header[^>]*>"), "<!-- ");
		str = str.replace(QRegularExpression("</header[^>]*>"), " -->");

		/* parse HTML */
		result->setHtml(str);

		handle_download_finished_sub();
	}

	reply->deleteLater();
}
