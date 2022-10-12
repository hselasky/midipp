/*-
 * Copyright (c) 2017-2022 Hans Petter Selasky
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

#include "midipp_musicxml.h"
#include "midipp_checkbox.h"
#include "midipp_chords.h"
#include "midipp_decode.h"

#define	MXML_MAX_TAGS 8

static const QString
MppReadStrFilter(const QString &str)
{
	QString retval = str;

	retval.replace(QChar('\n'), QChar(' '));
	retval.replace(QChar('.'), QChar(' '));
	retval.replace(QChar('('), QChar('['));
	retval.replace(QChar(')'), QChar(']'));
	retval = retval.trimmed();
	return (retval);
}

static const char *MppGetNoteString[12] = {
	"C",
	"Db",
	"D",
	"Eb",
	"E",
	"F",
	"Gb",
	"G",
	"Ab",
	"A",
	"Hb",
	"H",
};

static int
MppGetNoteNumber(const QString &step, const QString &alter, const QString &octave)
{
	int retval = 0;

	if (step == "C")
		retval += C0;
	else if (step == "D")
		retval += D0;
	else if (step == "E")
		retval += E0;
	else if (step == "F")
		retval += F0;
	else if (step == "G")
		retval += G0;
	else if (step == "A")
		retval += A0;
	else if (step == "H" || step == "B")
		retval += H0;

	if (alter == "-1")
		retval -= 1;
	else if (alter == "1" || alter == "+1")
		retval += 1;

	retval += octave.toInt() * 12;

	/* range check */
	if (retval < 0) {
		retval = (12 + (retval % 12)) % 12;
	} else if (retval > 127) {
		retval = 120 + (retval % 12);
		if (retval > 127)
			retval -= 12;
	}
	return (retval);
}

static const QString
MppReadMusicXML(const QByteArray &data, uint32_t flags, uint32_t ipart, uint32_t nmeasure)
{
	QXmlStreamReader::TokenType token = QXmlStreamReader::NoToken;
	QXmlStreamReader xml(data);
	QString output;
	QString output_string;
	QString output_scores;
	QString title;
	QString composer;
	QString lyricist;
	QString arranger;
	QString text;
	QString scores;
	QString pitch_step;
	QString pitch_alter;
	QString pitch_octave;
	QString note_duration;
	QString bass_step;
	QString bass_alter;
	QString divisions;
	QString root_step;
	QString root_alter;
	QString duration;
	QString syllabic;
	QString kind;
	QString tags[MXML_MAX_TAGS];
	uint32_t imeasure = 0;
	uint8_t do_new_line = 0;
	uint8_t is_chord = 0;
	size_t si = 0;

	while (!xml.atEnd()) {
		if (token == QXmlStreamReader::NoToken)
			token = xml.readNext();

		switch (token) {
		case QXmlStreamReader::Invalid:
			goto done;
		case QXmlStreamReader::StartElement:
			if (si < MXML_MAX_TAGS)
				tags[si] = xml.name().toString();
			si++;

			if (tags[0] == "score-partwise") {
				if (si == 1) {
					title = QString();
					composer = QString();
					lyricist = QString();
					arranger = QString();
				} else if (tags[1] == "work") {
					if (si == 3 && tags[2] == "work-title") {
						token = xml.readNext();
						if (token != QXmlStreamReader::Characters)
							continue;
						title = MppReadStrFilter(xml.text().toString());
					}
				} else if (tags[1] == "identification") {
					if (si == 3 && tags[2] == "creator") {
						QString type = xml.attributes().value("type").toString();
						if (type == "composer") {
							token = xml.readNext();
							if (token != QXmlStreamReader::Characters)
								continue;
							composer = MppReadStrFilter(xml.text().toString());
						} else if (type == "lyricist") {
							token = xml.readNext();
							if (token != QXmlStreamReader::Characters)
								continue;
							lyricist = MppReadStrFilter(xml.text().toString());
						} else if (type == "arranger") {
							token = xml.readNext();
							if (token != QXmlStreamReader::Characters)
								continue;
							arranger = MppReadStrFilter(xml.text().toString());
						}
					}
				} else if (tags[1] == "part") {
					if (ipart != 0) {
						/* wrong part number */
					} else if (si == 2) {
						divisions = QString();
						imeasure = 0;
					} else if (tags[2] == "measure") {
						if (si == 3) {

						} else if (tags[3] == "attributes" && tags[4] == "divisions") {
							if (si == 5) {
								token = xml.readNext();
								if (token != QXmlStreamReader::Characters)
									continue;
								divisions = xml.text().toString().trimmed();
							}
						} else if (tags[3] == "print") {
							if (si == 4) {
								if (xml.attributes().value("new-page").toString() == "yes") {
									if (!output.isEmpty())
										output += "\nJP\n\n";
								}
							}
						} else if (tags[3] == "harmony") {
							if (si == 4) {
								root_step = QString();
								root_alter = QString();
								kind = QString();
								bass_step = QString();
								bass_alter = QString();
							} else if (si == 6 && tags[4] == "root" && tags[5] == "root-step") {
								token = xml.readNext();
								if (token != QXmlStreamReader::Characters)
									continue;
								root_step = xml.text().toString().trimmed();
							} else if (si == 6 && tags[4] == "root" && tags[5] == "root-alter") {
								token = xml.readNext();
								if (token != QXmlStreamReader::Characters)
									continue;
								root_alter = xml.text().toString().trimmed();
							} else if (si == 5 && tags[4] == "kind") {
								kind = xml.attributes().value("text").toString();
								if (kind.isEmpty()) {
									token = xml.readNext();
									if (token != QXmlStreamReader::Characters)
										continue;
									/*
									 * Try to translate kind into something which
									 * MidiPlayerPro understands:
									 */
									kind = xml.text().toString().trimmed();
									if (kind == "major")
										kind = "";
									else if (kind == "minor")
										kind = "m";
									else if (kind == "augmented")
										kind = "+";
									else if (kind == "diminished")
										kind = "dim";
									else if (kind == "dominant")
										kind = "7";
									else if (kind == "major-seventh")
										kind = "M7";
									else if (kind == "minor-seventh")
										kind = "m7";
									else if (kind == "diminished-seventh")
										kind = "o7";
									else if (kind == "augmented-seventh")
										kind = "+7";
									else if (kind == "half-diminished")
										kind = QString::fromUtf8("ø");
									else if (kind == "major-minor")
										kind = "mM7";
									else if (kind == "major-sixth")
										kind = "M6";
									else if (kind == "minor-sixth")
										kind = "m6";
									else if (kind == "dominant-ninth")
										kind = "dom9";
									else if (kind == "major-ninth")
										kind = "M9";
									else if (kind == "minor-ninth")
										kind = "m9";
									else if (kind == "dominant-11th")
										kind = "dom11";
									else if (kind == "major-11th")
										kind = "M11";
									else if (kind == "minor-11th")
										kind = "m11";
									else if (kind == "dominant-13th")
										kind = "dom13";
									else if (kind == "major-13th")
										kind = "M13";
									else if (kind == "minor-13th")
										kind = "m13";
									else if (kind == "suspended-second")
										kind = "sus2";
									else if (kind == "suspended-fourth")
										kind = "sus4";
									else if (kind == "power")
										kind = "5";
									else
										kind = "";	/* major */
								}
							} else if (si == 6 && tags[4] == "bass" && tags[5] == "bass-step") {
								token = xml.readNext();
								if (token != QXmlStreamReader::Characters)
									continue;
								bass_step = xml.text().toString().trimmed();
							} else if (si == 6 && tags[4] == "bass" && tags[5] == "bass-alter") {
								token = xml.readNext();
								if (token != QXmlStreamReader::Characters)
									continue;
								bass_alter = xml.text().toString().trimmed();
							}

						} else if (tags[3] == "note") {
							if (si == 4) {
								pitch_step = QString();
								pitch_alter = QString();
								pitch_octave = QString();
								note_duration = QString();
								is_chord = 0;
								text = QString();
								syllabic = QString();
								duration = QString();
							} else if (tags[4] == "chord") {
								if (si == 5) {
									is_chord = 1;
								}
							} else if (tags[4] == "pitch") {
								if (si == 5) {

								} else if (tags[5] == "step") {
									if (si == 6) {
										token = xml.readNext();
										if (token != QXmlStreamReader::Characters)
											continue;
										pitch_step = xml.text().toString().trimmed();
									}
								} else if (tags[5] == "alter") {
									if (si == 6) {
										token = xml.readNext();
										if (token != QXmlStreamReader::Characters)
											continue;
										pitch_alter = xml.text().toString().trimmed();
									}
								} else if (tags[5] == "octave") {
									if (si == 6) {
										token = xml.readNext();
										if (token != QXmlStreamReader::Characters)
											continue;
										pitch_octave = xml.text().toString().trimmed();
									}
								}
							} else if (tags[4] == "duration") {
								if (si == 5) {
									token = xml.readNext();
									if (token != QXmlStreamReader::Characters)
										continue;
									duration = xml.text().toString().trimmed();
								}
							} else if (tags[4] == "lyric") {
								if (si == 5) {

								} else if (tags[5] == "syllabic") {
									if (si == 6) {
										token = xml.readNext();
										if (token != QXmlStreamReader::Characters)
											continue;
										syllabic = xml.text().toString().trimmed();
									}
								} else if (tags[5] == "text") {
									if (si == 6) {
										token = xml.readNext();
										if (token != QXmlStreamReader::Characters)
											continue;
										text = MppReadStrFilter(xml.text().toString());
									}
								}
							}
						}
					}
				}
			}
			break;
		case QXmlStreamReader::EndElement:
			if (tags[0] == "score-partwise") {
				if (si == 1) {
					QString creator = composer;
					if (!lyricist.isEmpty()) {
						creator += " // ";
						creator += lyricist;
					}
					if (!arranger.isEmpty()) {
						creator += " || ";
						creator += arranger;
					}
					output = QString("S\"(") + title + QString(")") +
					    creator + QString("\"\n\nL0:\n") + output;
				} else if (tags[1] == "part") {
					if (si == 2) {
						/* end of part */
						imeasure = 0;
						ipart--;

						if (output_string.isEmpty() == 0 ||
						    output_scores.isEmpty() == 0) {
							output += "\nS\"";
							output += output_string;
							output += "\"\n\n";
							output += output_scores;
							output_string = QString();
							output_scores = QString();
						}
					} else if (ipart != 0) {
						/* wrong part number */
					} else if (tags[2] == "measure") {
						if (si == 3) {
							/* end of measure */
							if (do_new_line != 0) {
								do_new_line = 0;
								output_scores += "\n";
							}
							imeasure++;

							if ((imeasure % nmeasure) == (nmeasure - 1)) {
								if (output_string.isEmpty() == 0 ||
								    output_scores.isEmpty() == 0) {
									/* check if the last syllabic is split */
									if (flags & MXML_FLAG_KEEP_TEXT) {
										if (syllabic == "begin" || syllabic == "middle")
											output_string += "-";
									}
									output += "\nS\"";
									output += output_string;
									output += "\"\n\n";
									output += output_scores;
									output_string = QString();
									output_scores = QString();
								}
							}
						} else if (tags[3] == "harmony") {
							if (si == 4) {
								/* end of harmony */
								QString harmony[2];
								uint8_t x;
								uint8_t which;
								uint32_t bass;
								uint32_t root;
								MppChord_t mask;

								root = MppGetNoteNumber(root_step, root_alter, "5");

								if (bass_step.isEmpty())
									bass = root;
								else
									bass = MppGetNoteNumber(bass_step, bass_alter, "5");

								harmony[0] = QString(MppGetNoteString[root % 12]) + kind;
								if (root != bass) {
									harmony[0] += QString("/") +
									    QString(MppGetNoteString[bass % 12]);
								}

								/* fallback to major */
								harmony[1] = QString(MppGetNoteString[root % 12]);
								if (root != bass) {
									harmony[1] += QString("/") +
									    QString(MppGetNoteString[bass % 12]);
								}

								for (which = 0; which != 2; which++) {
									MppStringToChordGeneric(mask, root, bass,
									    MPP_BAND_STEP_12, harmony[which]);
									if (mask.test(0))
										break;
								}
								if (which == 2)
									goto skip_harmony;

								if (do_new_line != 0) {
									do_new_line = 0;
									output_scores += "\n";
								}

								if (flags & MXML_FLAG_CONV_CHORDS)
									output_string += ".";

								if (flags & MXML_FLAG_KEEP_CHORDS) {
									output_string += "(";
									output_string += harmony[which];
									output_string += ")";
								}

								if (flags & MXML_FLAG_CONV_CHORDS) {
									output_scores += "U1 ";
									output_scores += MppKeyStr((3 * 12 + (bass % 12)) * MPP_BAND_STEP_12);
									output_scores += " ";
									output_scores += MppKeyStr((4 * 12 + (bass % 12)) * MPP_BAND_STEP_12);
									output_scores += " ";

									for (x = 0; x != MPP_MAX_CHORD_BANDS; x++) {
										if (mask.test(x) == 0)
											continue;
										output_scores += MppKeyStr(
										    ((5 * 12 + (root % 12)) * MPP_BAND_STEP_12) +
										    (x * MPP_BAND_STEP_CHORD));
										output_scores += " ";
									}
									output_scores += QString("/* ") + harmony[which] + QString(" */\n");
								}
							skip_harmony:;
							}
						} else if (tags[3] == "note") {
							if (si == 4) {
								/* end of note */

								if (flags & MXML_FLAG_KEEP_SCORES) {
									if (do_new_line == 0) {
										if (!pitch_step.isEmpty()) {
											output_scores += "U1 ";
											output_string += ".";
										}
									} else if (is_chord == 0) {
										output_scores += "\n";
										if (!pitch_step.isEmpty()) {
											output_scores += "U1 ";
											output_string += ".";
										} else {
											do_new_line = 0;
										}
									}
								}
								if (flags & MXML_FLAG_KEEP_TEXT) {
									output_string += text;
									if (syllabic == "single" || syllabic == "end")
										output_string += " ";
								}
								if (flags & MXML_FLAG_KEEP_SCORES) {
									if (!pitch_step.isEmpty()) {
										uint8_t key;

										key = MppGetNoteNumber(pitch_step, pitch_alter,
										    pitch_octave);
										output_scores += mid_key_str[key];
										output_scores += " ";
										do_new_line = 1;
									}
								}
							}
						}
					}
				}
			}
			if (si == 0)
				break;
			si--;
			if (si < MXML_MAX_TAGS)
				tags[si] = QString();
			break;
		default:
			break;
		}
		token = QXmlStreamReader::NoToken;
	}
done:
	return (output);
}

static int
MppReadMusicXMLParts(const QByteArray &data)
{
	QXmlStreamReader::TokenType token =
	    QXmlStreamReader::NoToken;
	QXmlStreamReader xml(data);
	QString tags[MXML_MAX_TAGS];
	size_t si = 0;
	int parts = 0;

	while (!xml.atEnd()) {
		if (token == QXmlStreamReader::NoToken)
			token = xml.readNext();

		switch (token) {
		case QXmlStreamReader::Invalid:
			goto done;
		case QXmlStreamReader::StartElement:
			if (si < MXML_MAX_TAGS)
				tags[si] = xml.name().toString();
			si++;
			break;
		case QXmlStreamReader::EndElement:
			if (si == 2 && tags[0] == "score-partwise" && tags[1] == "part")
				parts++;
			if (si == 0 || parts < 0)
				goto error;
			si--;
			if (si < MXML_MAX_TAGS)
				tags[si] = QString();
			break;
		default:
			break;
		}
		token = QXmlStreamReader::NoToken;
	}
done:
	if (xml.hasError())
		goto error;
	return (parts);
error:
	return (0);
}

MppMusicXmlImport :: MppMusicXmlImport(MppMainWindow *_mw, const QByteArray &data) :
    MppDialog(_mw, QObject::tr("MusicXML import"))
{
	MppDialog *d = this;

	int nparts = MppReadMusicXMLParts(data);

	if (nparts == 0)
		return;

	QLabel *lbl;
	
	gl = new QGridLayout(this);

	lbl = new QLabel(tr("Keep melody scores"));
	gl->addWidget(lbl, 0,0,1,1, Qt::AlignRight|Qt::AlignVCenter);

	lbl = new QLabel(tr("Keep lyrics text"));
	gl->addWidget(lbl, 1,0,1,1, Qt::AlignRight|Qt::AlignVCenter);

	lbl = new QLabel(tr("Keep chords"));
	gl->addWidget(lbl, 2,0,1,1, Qt::AlignRight|Qt::AlignVCenter);

	lbl = new QLabel(tr("Convert chords to scores"));
	gl->addWidget(lbl, 3,0,1,1, Qt::AlignRight|Qt::AlignVCenter);

	lbl = new QLabel(tr("Erase destination view"));
	gl->addWidget(lbl, 4,0,1,1, Qt::AlignRight|Qt::AlignVCenter);

	lbl = new QLabel(tr("Measures per line"));
	gl->addWidget(lbl, 5,0,1,1, Qt::AlignRight|Qt::AlignVCenter);

	lbl = new QLabel(tr("Part number to import"));
	gl->addWidget(lbl, 6,0,1,1, Qt::AlignRight|Qt::AlignVCenter);

	cbx_melody = new MppCheckBox();
	cbx_melody->setChecked(true);
	gl->addWidget(cbx_melody, 0,1,1,1, Qt::AlignCenter);

	cbx_text = new MppCheckBox();
	cbx_text->setChecked(true);
	gl->addWidget(cbx_text, 1,1,1,1, Qt::AlignCenter);

	cbx_chords = new MppCheckBox();
	cbx_chords->setChecked(true);
	gl->addWidget(cbx_chords, 2,1,1,1, Qt::AlignCenter);

	cbx_convert = new MppCheckBox();
	cbx_convert->setChecked(true);
	gl->addWidget(cbx_convert, 3,1,1,1, Qt::AlignCenter);

	cbx_erase = new MppCheckBox();
	cbx_erase->setChecked(true);
	gl->addWidget(cbx_erase, 4,1,1,1, Qt::AlignCenter);

	spn_nmeasure = new QSpinBox();
	spn_nmeasure->setRange(1,99);
	spn_nmeasure->setValue(4);
	gl->addWidget(spn_nmeasure, 5,1,1,1, Qt::AlignCenter);

	spn_partnumber = new QSpinBox();
	spn_partnumber->setRange(1,nparts);
	gl->addWidget(spn_partnumber, 6,1,1,1, Qt::AlignCenter);

	btn_done = new QPushButton(tr("Done"));
	d->connect(btn_done, SIGNAL(released()), d, SLOT(accept()));
	gl->addWidget(btn_done, 7,1,1,1);

	exec();

	uint32_t flags = 0;

	if (cbx_melody->isChecked())
		flags |= MXML_FLAG_KEEP_SCORES;
	if (cbx_text->isChecked())
		flags |= MXML_FLAG_KEEP_TEXT;
	if (cbx_chords->isChecked())
		flags |= MXML_FLAG_KEEP_CHORDS;
	if (cbx_convert->isChecked())
		flags |= MXML_FLAG_CONV_CHORDS;

	output = MppReadMusicXML(data, flags,
	    spn_partnumber->value() - 1, spn_nmeasure->value());
}
