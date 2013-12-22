/*-
 * Copyright (c) 2012 Hans Petter Selasky. All rights reserved.
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

#ifndef _MIDIPP_SETTINGS_H_
#define	_MIDIPP_SETTINGS_H_

#include "midipp.h"

class MppSettings : public QSettings 
{
	Q_OBJECT;

public:
	MppSettings(MppMainWindow *_parent, const QString & fname);
	~MppSettings(void);

	int save_volume;
	int save_instruments;
	int save_viewmode;
	int save_devices;
	int save_font;
	int save_database_url;
	int save_database_data;
	int save_custom;
	int save_shortcut;

	QString stringDefault(const QString &, const QString &);
	QByteArray byteArrayDefault(const QString &, const QByteArray &);

	int valueDefault(const QString &, int);

	QString concat(const char *, int = 0, int = 0);

	void doSave(void);
	void doLoad(void);

	MppSettingsWhat *mpp_what;

	QPushButton *but_config_save;
	QPushButton *but_config_clean;
	QPushButton *but_config_what;
	QPushButton *but_config_load;

	MppMainWindow *mw;

public slots:

	void handle_save(void);
	void handle_clean(void);
	void handle_what(void);
	void handle_load(void);
};

class MppSettingsWhat : public QDialog
{
	Q_OBJECT;

public:
	MppSettingsWhat(MppSettings *_parent);
	~MppSettingsWhat(void);

	void doShow(void);

	MppSettings *ms;

	QGridLayout *gl;

	QPushButton *but_reset;
	QPushButton *but_ok;

	MppCheckBox *cbx_volume;
	MppCheckBox *cbx_instruments;
	MppCheckBox *cbx_viewmode;
	MppCheckBox *cbx_deviceconfig;
	MppCheckBox *cbx_font;
	MppCheckBox *cbx_database_url;
	MppCheckBox *cbx_database_data;
	MppCheckBox *cbx_custom;
	MppCheckBox *cbx_shortcut;

public slots:
	void handle_reset(void);
};

#endif			/* _MIDIPP_SETTINGS_H_ */
