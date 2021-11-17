/*-
 * Copyright (c) 2012-2020 Hans Petter Selasky. All rights reserved.
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

#include "midipp_mainwindow.h"
#include "midipp_scores.h"
#include "midipp_settings.h"
#include "midipp_volume.h"
#include "midipp_mode.h"
#include "midipp_import.h"
#include "midipp_database.h"
#include "midipp_groupbox.h"
#include "midipp_checkbox.h"
#include "midipp_button.h"
#include "midipp_buttonmap.h"
#include "midipp_custom.h"
#include "midipp_shortcut.h"
#include "midipp_show.h"
#include "midipp_sheet.h"
#include "midipp_instrument.h"
#include "midipp_devsel.h"

MppSettingsSub :: MppSettingsSub(MppMainWindow *_parent, const QString &fname) :
    QSettings(fname), mw(_parent)
{
	save.instruments = -1;
	save.viewmode = -1;
	save.devices = -1;
	save.font = -1;
	save.database_url = -1;
	save.database_data = -1;
	save.custom = -1;
	save.shortcut = -1;
	save.hpsjam_server = -1;
}

MppSettings :: MppSettings(MppMainWindow *_parent)
{
	MppButton *pb;

	mw = _parent;

	gb_save_preset = new MppGroupBox(tr("Save selected preset"));
	gb_load_preset = new MppGroupBox(tr("Load selected preset"));

	for (unsigned x = 0; x != MPP_MAX_SETTINGS; x++) {
		QString name;
		QString fname;

		if (x == 0) {
			name = tr("DEFAULT");
			fname = "midipp";
		} else {
			name = tr("PRESET::%1").arg(x);
			fname = QString("midipp%1").arg(x);
		}

		pb = new MppButton(name, x);
		connect(pb, SIGNAL(released(int)), this, SLOT(handle_save(int)));
		gb_save_preset->addWidget(pb, 0, x, 1, 1);

		pb = new MppButton(name, x);
		connect(pb, SIGNAL(released(int)), this, SLOT(handle_load(int)));
		gb_load_preset->addWidget(pb, 0, x, 1, 1);

		setting[x] = new MppSettingsSub(mw, fname);
	}

	but_config_clean = new QPushButton(tr("Preset Clean"));
	but_config_what = new QPushButton(tr("Preset What"));

	connect(but_config_clean, SIGNAL(released()), this, SLOT(handle_clean()));
	connect(but_config_what, SIGNAL(released()), this, SLOT(handle_what()));

	mpp_what = new MppSettingsWhat(this);
}

MppSettings :: ~MppSettings(void)
{
	delete mpp_what;

	for (unsigned x = 0; x != MPP_MAX_SETTINGS; x++)
		delete setting[x];
}

int
MppSettingsSub :: valueDefault(const QString &str, int n)
{
	int val;

	if (contains(str))
		val = value(str).toInt();
	else
		val = n;

	return (val);
}

QString
MppSettingsSub :: stringDefault(const QString &str, const QString &n)
{
	QString val;

	if (contains(str))
		val = value(str).toString();
	else
		val = n;

	return (val);
}

QByteArray
MppSettingsSub :: byteArrayDefault(const QString &str, const QByteArray &n)
{
	QByteArray val;

	if (contains(str))
		val = value(str).toByteArray();
	else
		val = n;

	return (val);
}

QString
MppSettingsSub :: concat(const char *fmt, int num, int sub)
{
	char buf[80];
	snprintf(buf, sizeof(buf), fmt, num, sub);
	return (QString(buf));
}

void
MppSettingsSub :: doSave(void)
{
	int x;
	int y;

	clear();

	beginGroup("global");
	setValue("save_instruments", save.instruments ? 1 : 0);
	setValue("save_viewmode", save.viewmode ? 1 : 0);
	setValue("save_devices", save.devices ? 1 : 0);
	setValue("save_font", save.font ? 1 : 0);
	setValue("save_database_url", save.database_url ? 1 : 0);
	setValue("save_database_data", save.database_data ? 1 : 0);
	setValue("save_custom", save.custom ? 1 : 0);
	setValue("save_shortcut", save.shortcut ? 1 : 0);
	setValue("save_hpsjam_server", save.hpsjam_server ? 1 : 0);
	endGroup();

	if (save.viewmode) {
		for (x = 0; x != MPP_MAX_VIEWS; x++) {
			beginGroup(concat("view%d", x));
			setValue("basekey192", mw->scores_main[x]->baseKey / MPP_BAND_STEP_192);
			setValue("delay", mw->scores_main[x]->delayNoise);
			switch (mw->scores_main[x]->keyMode) {
			case MM_PASS_NONE_FIXED:
				setValue("keymode", 2);
				break;
			case MM_PASS_NONE_TRANS:
				setValue("keymode", 3);
				break;
			case MM_PASS_NONE_CHORD_PIANO:
				setValue("keymode", 4);
				break;
			case MM_PASS_NONE_CHORD_AUX:
				setValue("keymode", 5);
				break;
			case MM_PASS_NONE_CHORD_TRANS:
				setValue("keymode", 6);
				break;
			default:
				setValue("keymode", 0);
				break;
			}
			switch (mw->scores_main[x]->noteMode) {
			case MM_NOTEMODE_SYSEX:
				setValue("notemode", 1);
				break;
			default:
				setValue("notemode", 0);
				break;
			}
			setValue("inputchannel", mw->scores_main[x]->inputChannel);
			setValue("synthchannel", mw->scores_main[x]->synthChannel);
			setValue("synthchannelbase", mw->scores_main[x]->synthChannelBase);
			setValue("synthchanneltreb", mw->scores_main[x]->synthChannelTreb);
			setValue("auxchannel", mw->scores_main[x]->auxChannel);
			setValue("auxchannelbase", mw->scores_main[x]->auxChannelBase);
			setValue("auxchanneltreb", mw->scores_main[x]->auxChannelTreb);
			setValue("synthdevice", mw->scores_main[x]->synthDevice);
			setValue("synthdevicebase", mw->scores_main[x]->synthDeviceBase);
			setValue("synthdevicetreb", mw->scores_main[x]->synthDeviceTreb);
			setValue("synthvolume", mw->trackVolume[MPP_DEFAULT_TRACK(x)]);
			setValue("synthvolumebase", mw->trackVolume[MPP_BASS_TRACK(x)]);
			setValue("synthvolumetreb", mw->trackVolume[MPP_TREBLE_TRACK(x)]);
			setValue("chordcontrast", mw->scores_main[x]->chordContrast);
			setValue("chordnormalize", mw->scores_main[x]->chordNormalize);
			setValue("songevents", mw->scores_main[x]->songEventsOn);
			endGroup();
		}
	}

	if (save.instruments) {
		beginGroup("instruments");
		for (x = 0; x != 16; x++) {
			setValue(concat("bank%d", x), mw->tab_instrument->spn_instr_bank[x]->value());
			setValue(concat("prog%d", x), mw->tab_instrument->spn_instr_prog[x]->value());
			setValue(concat("mute%d", x), (int)mw->tab_instrument->cbx_instr_mute[x]->isChecked());
		}
		endGroup();
	}

	if (save.devices) {
		for (y = 0; y != MPP_MAX_DEVS; y++) {
			beginGroup(concat("device%d", y));
			setValue("group", mw->but_config_sel[y]->value());
			setValue("device", mw->led_config_dev[y]->text());
			setValue("play", (int)mw->cbx_config_dev[y][0]->isChecked());
			setValue("muteprog", mw->muteProgram[y]);
			setValue("mutepedal", mw->mutePedal[y]);
			setValue("enablelocalkeys", mw->enableLocalKeys[y]);
			setValue("disablelocalkeys", mw->disableLocalKeys[y]);
			setValue("mutenonchannel", mw->muteAllNonChannel[y]);
			setValue("muteallcontrol", mw->muteAllControl[y]);
			for (x = 0; x != MPP_MAX_VIEWS; x++)
				setValue(concat("view%d", x), (int)mw->cbx_config_dev[y][1 + x]->isChecked());
			for (x = 0; x != 16; x++)
				setValue(concat("mute%d", x), mw->muteMap[y][x]);
			endGroup();
		}
	}
	if (save.font) {
		beginGroup("font");
		setValue("default", mw->defaultFont.toString());
		setValue("editor", mw->editFont.toString());
		setValue("print", mw->printFont.toString());
#ifndef HAVE_NO_SHOW
		setValue("show", mw->tab_show_control->showFont.toString());
#endif
		endGroup();
	}
	if (save.database_url) {
		beginGroup("database_url");
		setValue("location", mw->tab_database->location->text());
		endGroup();
	}
	if (save.database_data) {
		beginGroup("database_data");
		setValue("array", mw->tab_database->input_data);
		endGroup();
	}
	if (save.custom) {
		beginGroup("custom_data");
		for (x = 0; x != MPP_CUSTOM_MAX; x++)
			setValue(concat("command%d", x), mw->tab_custom->custom[x].led_send->text());
		endGroup();
	}
	if (save.shortcut) {
		beginGroup("shortcut_data");
		for (x = 0; x != MPP_SHORTCUT_MAX; x++)
			setValue(QString(mw->tab_shortcut->shortcut_desc[x]), mw->tab_shortcut->led_cmd[x]->text());
		endGroup();
	}
	if (save.hpsjam_server) {
		beginGroup("hpsjam_server");
#ifndef HAVE_NO_SHOW
		setValue("location", mw->tab_show_control->editHpsJamServer->text());
#endif
		endGroup();
	}
	sync();
}

void
MppSettingsSub :: doLoad(void)
{
	int x;
	int y;

	save.instruments = valueDefault("global/save_instruments", -1);
	save.viewmode = valueDefault("global/save_viewmode", -1);
	save.devices = valueDefault("global/save_devices", -1);
	save.font = valueDefault("global/save_font", -1);
	save.database_url = valueDefault("global/save_database_url", -1);
	save.database_data = valueDefault("global/save_database_data", -1);
	save.custom = valueDefault("global/save_custom", -1);
	save.shortcut = valueDefault("global/save_shortcut", -1);
	save.hpsjam_server = valueDefault("global/save_hpsjam_server", -1);

	if (save.viewmode > 0) {
		for (x = 0; x != MPP_MAX_VIEWS; x++) {
			int baseKey = valueDefault(concat("view%d/basekey192", x), MPP_DEFAULT_BASE_KEY / MPP_BAND_STEP_192);
			int delayNoise = valueDefault(concat("view%d/delay", x), 25);
			int keyMode = valueDefault(concat("view%d/keymode", x), 0);
			int noteMode = valueDefault(concat("view%d/notemode", x), 0);
			int inputChannel = valueDefault(concat("view%d/inputchannel", x), -1);
			int synthChannel = valueDefault(concat("view%d/synthchannel", x), (x == 1) ? 9 : 0);
			int synthChannelBase = valueDefault(concat("view%d/synthchannelbase", x), -1);
			int synthChannelTreb = valueDefault(concat("view%d/synthchanneltreb", x), -1);
			int auxChannel = valueDefault(concat("view%d/auxchannel", x), -1);
			int auxChannelBase = valueDefault(concat("view%d/auxchannelbase", x), -1);
			int auxChannelTreb = valueDefault(concat("view%d/auxchanneltreb", x), -1);
			int synthDevice = valueDefault(concat("view%d/synthdevice", x), -1);
			int synthDeviceBase = valueDefault(concat("view%d/synthdevicebase", x), -1);
			int synthDeviceTreb = valueDefault(concat("view%d/synthdevicetreb", x), -1);
			int synthVolume = valueDefault(concat("view%d/synthvolume", x), -1);
			int synthVolumeBase = valueDefault(concat("view%d/synthvolumebase", x), -1);
			int synthVolumeTreb = valueDefault(concat("view%d/synthvolumetreb", x), -1);
			int chordContrast = valueDefault(concat("view%d/chordcontrast", x), 128);
			int chordNormalize = valueDefault(concat("view%d/chordnormalize", x), 128);
			int songEvents = valueDefault(concat("view%d/songevents", x), 0);

			if (delayNoise < 0 || delayNoise > 255)
				delayNoise = 0;
			if (inputChannel < 0 || inputChannel > 15) {
				switch (inputChannel) {
				case MPP_CHAN_ANY:
				case MPP_CHAN_MPE:
					break;
				default:
					inputChannel = MPP_CHAN_ANY;
					break;
				}
			}
			if (synthChannel < 0 || synthChannel > 15)
				synthChannel = 0;
			if (synthChannelBase < 0 || synthChannelBase > 15)
				synthChannelBase = MPP_CHAN_NONE;
			if (synthChannelTreb < 0 || synthChannelTreb > 15)
				synthChannelTreb = MPP_CHAN_NONE;
			if (auxChannel < 0 || auxChannel > 15)
				auxChannel = MPP_CHAN_NONE;
			if (auxChannelBase < 0 || auxChannelBase > 15)
				auxChannelBase = MPP_CHAN_NONE;
			if (auxChannelTreb < 0 || auxChannelTreb > 15)
				auxChannelTreb = MPP_CHAN_NONE;
			if (synthDevice < 0 || synthDevice >= MPP_MAX_DEVS)
				synthDevice = -1;
			if (synthDeviceBase < 0 || synthDeviceBase >= MPP_MAX_DEVS)
				synthDeviceBase = -1;
			if (synthDeviceTreb < 0 || synthDeviceTreb >= MPP_MAX_DEVS)
				synthDeviceTreb = -1;
			if (synthVolume < 0 || synthVolume > MPP_VOLUME_MAX)
				synthVolume = MPP_VOLUME_UNIT;
			if (synthVolumeBase < 0 || synthVolumeBase > MPP_VOLUME_MAX)
				synthVolumeBase = MPP_VOLUME_UNIT;
			if (synthVolumeTreb < 0 || synthVolumeTreb > MPP_VOLUME_MAX)
				synthVolumeTreb = MPP_VOLUME_UNIT;
			if (chordContrast < 0 || chordContrast > 255)
				chordContrast = 128;
			if (chordNormalize < 0 || chordNormalize > 1)
				chordNormalize = 1;
			if (songEvents < 0 || songEvents > 1)
				songEvents = 0;

			mw->atomic_lock();
			mw->scores_main[x]->baseKey = baseKey * MPP_BAND_STEP_192;
			mw->scores_main[x]->delayNoise = delayNoise;
			switch (keyMode) {
			case 2:
				mw->scores_main[x]->keyMode = MM_PASS_NONE_FIXED;
				break;
			case 3:
				mw->scores_main[x]->keyMode = MM_PASS_NONE_TRANS;
				break;
			case 4:
				mw->scores_main[x]->keyMode = MM_PASS_NONE_CHORD_PIANO;
				break;
			case 5:
				mw->scores_main[x]->keyMode = MM_PASS_NONE_CHORD_AUX;
				break;
			case 6:
				mw->scores_main[x]->keyMode = MM_PASS_NONE_CHORD_TRANS;
				break;
			default:
				mw->scores_main[x]->keyMode = MM_PASS_ALL;
				break;
			}
			switch (noteMode) {
			case 1:
				mw->scores_main[x]->noteMode = MM_NOTEMODE_SYSEX;
				break;
			default:
				mw->scores_main[x]->noteMode = MM_NOTEMODE_NORMAL;
				break;
			}

			mw->scores_main[x]->inputChannel = inputChannel;
			mw->scores_main[x]->synthChannel = synthChannel;
			mw->scores_main[x]->synthChannelBase = synthChannelBase;
			mw->scores_main[x]->synthChannelTreb = synthChannelTreb;
			mw->scores_main[x]->auxChannel = auxChannel;
			mw->scores_main[x]->auxChannelBase = auxChannelBase;
			mw->scores_main[x]->auxChannelTreb = auxChannelTreb;
			mw->scores_main[x]->synthDevice = synthDevice;
			mw->scores_main[x]->synthDeviceBase = synthDeviceBase;
			mw->scores_main[x]->synthDeviceTreb = synthDeviceTreb;
			mw->trackVolume[MPP_DEFAULT_TRACK(x)] = synthVolume;
			mw->trackVolume[MPP_BASS_TRACK(x)] = synthVolumeBase;
			mw->trackVolume[MPP_TREBLE_TRACK(x)] = synthVolumeTreb;
			mw->scores_main[x]->chordContrast = chordContrast;
			mw->scores_main[x]->chordNormalize = chordNormalize;
			mw->scores_main[x]->songEventsOn = songEvents;
			mw->atomic_unlock();

			mw->dlg_mode[x]->update_all();
		}

		int value[2];

		mw->atomic_lock();
		value[0] = mw->scores_main[0]->keyMode;
		value[1] = mw->scores_main[1]->keyMode;
		mw->atomic_unlock();

		mw->mbm_key_mode_a->setSelection(value[0]);
		mw->mbm_key_mode_b->setSelection(value[1]);
	}

	if (save.instruments > 0) {
		for (x = 0; x != 16; x++) {
			mw->tab_instrument->spn_instr_bank[x]->setValue(valueDefault(concat("instruments/bank%d", x), 0) & 16383);
			mw->tab_instrument->spn_instr_prog[x]->setValue(valueDefault(concat("instruments/prog%d", x), 0) & 127);
			mw->tab_instrument->cbx_instr_mute[x]->setChecked(valueDefault(concat("instruments/mute%d", x), 0) ? 1 : 0);
		}
	}

	if (save.devices > 0) {
		for (y = 0; y != MPP_MAX_DEVS; y++) {
			mw->but_config_sel[y]->setValue(valueDefault(concat("device%d/group", y), y));
			mw->led_config_dev[y]->setText(stringDefault(concat("device%d/device", y), ""));
			mw->cbx_config_dev[y][0]->setChecked(valueDefault(concat("device%d/play", y), 0) ? 1 : 0);
			for (x = 0; x != MPP_MAX_VIEWS; x++) {
				mw->cbx_config_dev[y][1 + x]->setChecked(
				    valueDefault(concat("device%d/view%d", y, x), (x == 0)));
			}
			int muteProgram = valueDefault(concat("device%d/muteprog", y), 0) ? 1 : 0;
			int mutePedal = valueDefault(concat("device%d/mutepedal", y), 0) ? 1 : 0;
			int enableLocalKeys = valueDefault(concat("device%d/enablelocalkeys", y), 0) ? 1 : 0;
			int disableLocalKeys = valueDefault(concat("device%d/disablelocalkeys", y), 0) ? 1 : 0;
			int muteAllControl = valueDefault(concat("device%d/muteallcontrol", y), 0) ? 1 : 0;
			int muteAllNonChannel = valueDefault(concat("device%d/mutenonchannel", y), 0) ? 1 : 0;

			int mute[16];

			for (x = 0; x != 16; x++)
				mute[x] = valueDefault(concat("device%d/mute%d", y, x), 0) ? 1 : 0;

			mw->atomic_lock();
			mw->muteProgram[y] = muteProgram; 
			mw->mutePedal[y] = mutePedal; 
			mw->enableLocalKeys[y] = enableLocalKeys; 
			mw->disableLocalKeys[y] = disableLocalKeys; 
			mw->muteAllControl[y] = muteAllControl;
			mw->muteAllNonChannel[y] = muteAllNonChannel; 

			for (x = 0; x != 16; x++)
				mw->muteMap[y][x] = mute[x];
			mw->atomic_unlock();
		}
	}
	if (save.font > 0) {
		/* default font */
		mw->defaultFont.fromString(
		    stringDefault("font/default",
		    "Sans Serif,-1,20,5,75,0,0,0,0,0"));
		if (mw->defaultFont.pixelSize() < 1)
			mw->defaultFont.setPixelSize(20);
#ifndef HAVE_NO_SHOW
		/* show font */
		mw->tab_show_control->showFont.fromString(
		    stringDefault("font/show",
		    "Sans Serif,-1,24,5,75,0,0,0,0,0"));
		if (mw->tab_show_control->showFont.pixelSize() < 1)
			mw->tab_show_control->showFont.setPixelSize(20);
#endif

		mw->handle_compile(1);

		/* editor font */
		mw->editFont.fromString(
		    stringDefault("font/editor",
#ifdef __APPLE__
		    "Courier New,-1,14,5,50,0,0,0,0,0"
#else
		    "Monospace,-1,14,5,50,0,0,0,0,0"
#endif
		    ));
		if (mw->editFont.pixelSize() < 1)
			mw->editFont.setPixelSize(14);

		for (x = 0; x != MPP_MAX_VIEWS; x++) {
			mw->scores_main[x]->editWidget->setFont(mw->editFont);
			mw->scores_main[x]->sheet->update();
		}
		mw->tab_help->setFont(mw->editFont);
		mw->tab_import->editWidget->setFont(mw->editFont);

		/* print font */
		mw->printFont.fromString(
		    stringDefault("font/print",
		    "Sans Serif,-1,18,5,75,0,0,0,0,0"));
	}
	if (save.database_url > 0) {
		mw->tab_database->location->setText(
		    stringDefault("database_url/location", MPP_DEFAULT_URL));
	}
	if (save.database_data > 0) {
		mw->tab_database->input_data =
		  byteArrayDefault("database_data/array", QByteArray());
		mw->tab_database->handle_download_finished_sub();
	}
	if (save.custom > 0) {
		for (x = 0; x != MPP_CUSTOM_MAX; x++) {
			mw->tab_custom->custom[x].led_send->setText(
			   stringDefault(concat("custom_data/command%d", x), ""));
		}
	}
	if (save.shortcut > 0) {
		for (x = 0; x != MPP_SHORTCUT_MAX; x++) {
			mw->tab_shortcut->led_cmd[x]->setText(
			    stringDefault(QString("shortcut_data/") +
			    QString(mw->tab_shortcut->shortcut_desc[x]), ""));
		}
	}
	if (save.hpsjam_server > 0) {
#ifndef HAVE_NO_SHOW
		mw->tab_show_control->butHpsJamOnOff->setSelection(0);
		mw->tab_show_control->editHpsJamServer->setText(
		    stringDefault("hpsjam_server/location", MPP_DEFAULT_HPSJAM));
#endif
	}
}

void
MppSettings :: handle_save(int id)
{
	setting[id]->doSave();
}

void
MppSettings :: handle_clean(void)
{
	QMessageBox mbox;

	mbox.setText("Do you want to clear the default preset?");
	mbox.setInformativeText("This step cannot be undone!");
	mbox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
	mbox.setDefaultButton(QMessageBox::No);
	mbox.setIcon(QMessageBox::Critical);
	mbox.setWindowIcon(QIcon(MppIconFile));
	mbox.setWindowTitle(MppVersion);

	int ret = mbox.exec();

	if (ret != QMessageBox::Yes)
		return;

	setting[0]->doClear();
	setting[0]->doLoad();
}

void
MppSettingsSub :: doClear()
{
	clear();

	beginGroup("global");
	setValue("save_instruments", 1);
	setValue("save_viewmode", 1);
	setValue("save_devices", 1);
	setValue("save_font", 1);
	setValue("save_database_url", 1);
	setValue("save_database_data", 1);
	setValue("save_custom", 1);
	setValue("save_shortcut", 1);
	setValue("save_hpsjam_server", 1);
	endGroup();

	sync();
}

void
MppSettings :: handle_what(void)
{
	mpp_what->doShow(setting[0]->save);
}

void
MppSettings :: handle_load(int id)
{
	setting[id]->doLoad();

	mw->handle_config_apply();
}

MppSettingsWhat :: MppSettingsWhat(MppSettings *_parent)
  : QDialog()
{
	ms = _parent;

	gl = new QGridLayout(this);

	cbx_instruments = new MppCheckBox();
	cbx_viewmode = new MppCheckBox();
	cbx_deviceconfig = new MppCheckBox();
	cbx_font = new MppCheckBox();
	cbx_database_url = new MppCheckBox();
	cbx_database_data = new MppCheckBox();
	cbx_custom = new MppCheckBox();
	cbx_shortcut = new MppCheckBox();
	cbx_hpsjam_server = new MppCheckBox();

	but_ok = new QPushButton(tr("Close"));
	but_reset = new QPushButton(tr("Reset"));

	connect(but_ok, SIGNAL(released()), this, SLOT(accept()));
	connect(but_reset, SIGNAL(released()), this, SLOT(handle_reset()));

	setWindowTitle(tr("Preset save and load selection"));
	setWindowIcon(QIcon(MppIconFile));

	gl->addWidget(new QLabel(tr("Save instrument settings")), 0, 0, 1, 1);
	gl->addWidget(new QLabel(tr("Save view mode settings")), 1, 0, 1, 1);
	gl->addWidget(new QLabel(tr("Save device configuration")), 2, 0, 1, 1);
	gl->addWidget(new QLabel(tr("Save font selection")), 3, 0, 1, 1);
	gl->addWidget(new QLabel(tr("Save database URL")), 4, 0, 1, 1);
	gl->addWidget(new QLabel(tr("Save database contents")), 5, 0, 1, 1);
	gl->addWidget(new QLabel(tr("Save custom MIDI commands")), 6, 0, 1, 1);
	gl->addWidget(new QLabel(tr("Save shortcut MIDI commands")), 7, 0, 1, 1);
	gl->addWidget(new QLabel(tr("Save HPSJAM server location")), 8, 0, 1, 1);

	gl->addWidget(cbx_instruments, 0, 2, 1, 2);
	gl->addWidget(cbx_viewmode, 1, 2, 1, 2);
	gl->addWidget(cbx_deviceconfig, 2, 2, 1, 2);
	gl->addWidget(cbx_font, 3, 2, 1, 2);
	gl->addWidget(cbx_database_url, 4, 2, 1, 2);
	gl->addWidget(cbx_database_data, 5, 2, 1, 2);
	gl->addWidget(cbx_custom, 6, 2, 1, 2);
	gl->addWidget(cbx_shortcut, 7, 2, 1, 2);
	gl->addWidget(cbx_hpsjam_server, 8, 2, 1, 2);

	gl->addWidget(but_reset, 9, 1, 1, 1);
	gl->addWidget(but_ok, 9, 2, 1, 1);
}

MppSettingsWhat :: ~MppSettingsWhat(void)
{

}

void
MppSettingsWhat :: doShow(const MppSettingsSave &save_)
{
	MppSettingsSave save = save_;

	cbx_instruments->setChecked(save.instruments ? 1 : 0);
	cbx_viewmode->setChecked(save.viewmode ? 1 : 0);
	cbx_deviceconfig->setChecked(save.devices ? 1 : 0);
	cbx_font->setChecked(save.font ? 1 : 0);
	cbx_database_url->setChecked(save.database_url ? 1 : 0);
	cbx_database_data->setChecked(save.database_data ? 1 : 0);
	cbx_custom->setChecked(save.custom ? 1 : 0);
	cbx_shortcut->setChecked(save.shortcut ? 1 : 0);
	cbx_hpsjam_server->setChecked(save.hpsjam_server ? 1 : 0);

	exec();

	save.instruments = cbx_instruments->isChecked() ? 1 : 0;
	save.viewmode = cbx_viewmode->isChecked() ? 1 : 0;
	save.devices = cbx_deviceconfig->isChecked() ? 1 : 0;
	save.font = cbx_font->isChecked() ? 1 : 0;
	save.database_url = cbx_database_url->isChecked() ? 1 : 0;
	save.database_data = cbx_database_data->isChecked() ? 1 : 0;
	save.custom = cbx_custom->isChecked() ? 1 : 0;
	save.shortcut = cbx_shortcut->isChecked() ? 1 : 0;
	save.hpsjam_server = cbx_hpsjam_server->isChecked() ? 1 : 0;

	/* update what to save */
	for (unsigned x = 0; x != MPP_MAX_SETTINGS; x++)
		ms->setting[x]->save = save;
}

void
MppSettingsWhat :: handle_reset(void)
{
	cbx_instruments->setChecked(1);
	cbx_viewmode->setChecked(1);
	cbx_deviceconfig->setChecked(1);
	cbx_font->setChecked(1);
	cbx_database_url->setChecked(1);
	cbx_database_data->setChecked(1);
	cbx_custom->setChecked(1);
	cbx_shortcut->setChecked(1);
	cbx_hpsjam_server->setChecked(1);
}
