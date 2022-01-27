Name:           midipp
Version:        2.1.2
Release:        1%{?dist}
Summary:        MIDI Player Pro
Group:          Graphical Desktop/Applications/Multimedia
License:        BSD-2-Clause
URL:            https://github.com/hselasky/midipp
Source0:        https://codeload.github.com/hselasky/midipp/tar.gz/v2.1.2?dummy=/hselasky-midipp-v2.1.2_GH0.tar.gz
Source1:        https://codeload.github.com/hselasky/libumidi/tar.gz/v2.1.3?dummy=/hselasky-libumidi-v2.1.3_GH0.tar.gz

BuildRequires:  make gcc libjack-devel phonon-devel qt-devel qt-settings qtwebkit-devel
Requires:       qt

%description
MIDI Player Pro allows you to play any kind of MIDI music in seconds
with your fingertips. List of supported features:

- Raw MIDI.
- ALSA MIDI.
- JACK MIDI.
- MPE support.
- Import from lyrics sites (chorded lyrics)
- Import from GuitarPro v3 and v4 format.
- Import from MusicXML format.
- Loading and saving from and to standard v1.0 MIDI files.
- Realtime MIDI processing.
- Simple sequence looping.
- 30000 BPM MIDI recording and playback.
- Undo/Redo support.
- Printing music like PDF.

%prep
%setup -q -a 1

%build
qmake PREFIX=/usr DESTDIR=$RPM_BUILD_ROOT HAVE_STATIC=YES HAVE_ALSA=YES HAVE_JACK=YES LIBUMIDIPATH=libumidi-2.1.3 midipp.pro
make %{?_smp_mflags}

%install
rm -rf $RPM_BUILD_ROOT
INSTALL_ROOT=$RPM_BUILD_ROOT
export INSTALL_ROOT

%make_install
unset INSTALL_ROOT

%files
/usr/bin/midipp
/usr/share/applications/midipp.desktop
/usr/share/pixmaps/midipp.png
%doc

%changelog
* Thu Jan 27 2022 HPS hps@selasky.org 2.1.3-1
* Tue Jan 27 2022 HPS hps@selasky.org 2.1.2-1
- Added support for ALSA MIDI.
* Tue Sep 2 2021 HPS hps@selasky.org 2.1.1-1
- Fix for crash using replay tab.
* Tue Aug 5 2021 HPS hps@selasky.org 2.1.0-1
- Added more sustained chords.
- Improve circle of fifths.
* Tue Jun 20 2021 HPS hps@selasky.org 2.0.9-1
* Improved chord encoding
* Tue Jun 20 2021 HPS hps@selasky.org 2.0.8-1
- Improved score recording
* Tue Nov 29 2020 HPS hps@selasky.org 2.0.7-1
- Fix for crash
* Tue Nov 19 2020 HPS hps@selasky.org 2.0.6-1
- Fixed excessive stack usage
- Added keyboard shortcuts for navigating views
* Tue Nov 11 2020 HPS hps@selasky.org 2.0.5-1
- Added support for HPS JAM (Lyrics and Chords).
- Fixed some chords.
* Tue Apr 18 2020 HPS hps@selasky.org 2.0.4-1
- Added support for MPE.
- Added support for multiple presets.
- Fixed some bugs.
* Tue Dec 19 2019 HPS hps@selasky.org 2.0.3-1
- Bugfixes and GUI updates.
* Tue Jul 16 2019 HPS hps@selasky.org 2.0.2-1
- Minor bugfix release.
* Sun Apr 21 2019 HPS hps@selasky.org 2.0.1-1
- Multiple minor fixes.
* Wed Oct 3 2018 HPS hps@selasky.org 2.0.0-1
- Added support for quartertones, octotones, hexatones and so on.
- Added support for bass offset effect.
- Added support for circle of fifths.
- Improved support for printing.
* Tue Aug 3 2017 HPS hps@selasky.org 1.3.4-1
- Added support for importing uncompressed MusicXML files 
* Tue Dec 6 2016 HPS hps@selasky.org 1.3.3-1
- Fixed support for more than 12 jump labels
- Fixed output MIDI channel when using the piano tab
* Thu May 22 2016 HPS hps@selasky.org 1.3.2-1
- Allow PianoTab to accept keyboard input
- Separated pedal mute from control events mute
- Improved chord decoding
* Thu Jan 11 2016 HPS hps@selasky.org 1.3.1-1
- Fixed send song events button.
- Improved lyrics parsing.
- Added new sheet tab feature.
- Fixed saving of song events setting.
- Updated play tab GUI a bit.
* Thu Nov 27 2015 HPS hps@selasky.org 1.3.0-1
- Improved detection of chords when transposing a song.
- Shrunk GUI a bit.
- Added button to select first picture in show mode.
* Thu Jul 20 2015 HPS hps@selasky.org 1.2.18-1
- Added support for setting key-mode when loading songs.
- Added support for song background selection changes.
- Reworked show mode configuration.
* Thu May 6 2015 HPS hps@selasky.org 1.2.17-1
- Added support for RePlay tab.
- Added support for Metronome.
- Fixed bug parsing all uppercased lyrics 
- Added detection of label marks in lyrics
- Reworked show tab and its animations
* Thu Mar 5 2015 HPS hps@selasky.org 1.2.16-1
- No changes.
* Mon Jan 12 2015 HPS hps@selasky.org 1.2.15-1
- Corrected and added support for many new chords.
  See help tab for a complete list.
* Sun Dec 14 2014 HPS hps@selasky.org 1.2.14-1
- Improved lyrics import and export functionality
- Added support for two more chords
* Thu Oct 16 2014 HPS hps@selasky.org 1.2.13-1
- Updated ViewMode selection to be more interactive
- Moved all device selection to configuration tab
* Fri Sep 23 2014 HPS hps@selasky.org 1.2.12-1
- Created RPM package.
- Some minor compile fixes.

