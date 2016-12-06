Name:           midipp
Version:        1.3.3
Release:        1%{?dist}
Summary:        MIDI Player Pro
Group:          Graphical Desktop/Applications/Multimedia
License:        BSD-2-Clause
URL:            http://www.selasky.org/hans_petter/midistudio
Source0:        http://www.selasky.org/hans_petter/distfiles/%{name}-%{version}.tar.bz2
Source1:        http://www.selasky.org/hans_petter/distfiles/libumidi-2.0.13.tar.bz2

BuildRequires:  make gcc libjack-devel phonon-devel qt-devel qt-settings qtwebkit-devel
Requires:       qt

%description
MIDI Player Pro allows you to play any kind of MIDI music in seconds
with your fingertips. List of supported features:

- Raw MIDI.
- Jack MIDI.
- Import from lyrics sites (chorded lyrics)
- Import from GuitarPro v3 and v4 format.
- Loading and saving from and to standard v1.0 MIDI files.
- Realtime MIDI processing.
- Simple sequence looping.
- 30000 BPM MIDI recording and playback.
- Undo/Redo support.
- Printing music like PDF.

%prep
%setup -q -a 1

%build
qmake-qt4 PREFIX=/usr DESTDIR=$RPM_BUILD_ROOT HAVE_STATIC=YES HAVE_JACK=YES LIBUMIDIPATH=libumidi-2.0.13 midipp.pro
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

