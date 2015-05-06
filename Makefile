#
# Copyright (c) 2011-2013 Hans Petter Selasky. All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
# 1. Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer.
# 2. Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer in the
#    documentation and/or other materials provided with the distribution.
#
# THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
# ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
# ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
# FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
# DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
# OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
# HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
# LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
# OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
# SUCH DAMAGE.
#
#
# Makefile for MIDI Player Pro
#

VERSION=1.2.18

DESTDIR?=
PREFIX?=/usr/local
HAVE_SCREENSHOT?=
HAVE_STATIC?=

all: Makefile.unix
	make -f Makefile.unix -j5 all

Makefile.unix: midipp.pro
	qmake-qt4 HAVE_SCREENSHOT=${HAVE_SCREENSHOT} PREFIX=${PREFIX} \
		DESTDIR=${DESTDIR} HAVE_STATIC=${HAVE_STATIC} \
		-o Makefile.unix midipp.pro
help:
	@echo "Targets are: all, install, clean, package, help"

install: Makefile.unix
	make -f Makefile.unix install

clean: Makefile.unix
	make -f Makefile.unix clean
	rm -f Makefile.unix

package: clean

	tar -cvf temp.tar --exclude="*~" --exclude="*#" \
		--exclude=".svn" --exclude="*.orig" --exclude="*.rej" \
		Makefile midipp*.pro midipp*.qrc midipp*.plist \
		MidiPlayerPro.icns HISTORY.TXT \
		MidiPlayerPro*.entitlements \
		midipp*.cpp midipp*.h *.png midipp*.desktop \
		midipp*.spec

	rm -rf midipp-${VERSION}

	mkdir midipp-${VERSION}

	tar -xvf temp.tar -C midipp-${VERSION}

	rm -rf temp.tar

	tar --uid=0 --gid=0 -jcvf midipp-${VERSION}.tar.bz2 midipp-${VERSION}
