GIT = git
TAR = /usr/bin/tar
UNIX2DOS = $(DO)unix2dos <$< >$@
GREP = @grep -H

# no user-configurable paths below this line

VERSION = 4.0.0

ifndef DO
$(error Use "Makefile" instead of "release.mk")
endif

dist: \
	release/asap-$(VERSION)-android.apk \
	release/asap-$(VERSION)-web.zip \
	release/asap-$(VERSION)-win32.msi \
	release/asap-$(VERSION)-win32.zip \
	release/asap-$(VERSION)-win64.msi \
	release/foo_asap-$(VERSION).fb2k-component \
	srcdist
.PHONY: dist

srcdist: $(srcdir)MANIFEST $(srcdir)asap.c $(srcdir)asap.h $(ASM6502_OBX)
	$(RM) release/asap-$(VERSION).tar.gz && $(TAR) -c --numeric-owner --owner=0 --group=0 --mode=644 -T MANIFEST --transform=s,,asap-$(VERSION)/, | $(SEVENZIP) -tgzip -si release/asap-$(VERSION).tar.gz
.PHONY: srcdist

$(srcdir)MANIFEST:
	$(DO)if test -e $(srcdir).git; then \
		($(GIT) ls-files | grep -vF .gitignore \
			&& echo MANIFEST && echo asap.c && echo asap.h \
			&& for obx in $(ASM6502_OBX); do echo $$obx; done) | /usr/bin/sort -u >$@; \
	fi
.PHONY: $(srcdir)MANIFEST

release/asap-$(VERSION)-web.zip: release/COPYING.txt \
	javascript/asap.js $(srcdir)javascript/asapweb.js $(srcdir)javascript/binaryHttpRequest.js
	$(MAKEZIP)

release/asap-$(VERSION)-win32.zip: release/COPYING.txt \
	$(addprefix win32/,asapconv.exe asapscan.exe wasap.exe in_asap.dll foo_asap.dll apokeysnd.dll xmp-asap.dll bass_asap.dll ASAPShellEx.dll libasap_plugin.dll)
	$(MAKEZIP)

release/foo_asap-$(VERSION).fb2k-component: win32/foo_asap.dll
	$(MAKEZIP)

release/asap-vlc-$(VERSION)-osx.dmg: release/osx/libasap_plugin.dylib release/osx/plugins
	$(DO)hdiutil create -volname asap-vlc-$(VERSION)-osx -srcfolder release/osx -format UDBZ -imagekey bzip2-level=3 -ov $@

release/osx/libasap_plugin.dylib: libasap_plugin.dylib
	$(DO)strip -o $@ -x $< && chmod 644 $@
CLEANDIR += release/osx

release/osx/plugins:
	$(DO)ln -s /Applications/VLC.app/Contents/MacOS/plugins $@

deb:
	debuild -b -us -uc
.PHONY: deb

release/COPYING.txt: $(srcdir)COPYING
	$(UNIX2DOS)
CLEAN += release/COPYING.txt

version:
	@echo ./release/release.mk: VERSION=$(VERSION)
	$(GREP) -m 1 ^ASAP $(srcdir)NEWS
	$(GREP) "<since>" $(srcdir)PORTS.xml | /usr/bin/sort -ru | head -1
	$(GREP) Version: $(srcdir)asap.spec
	$(GREP) -m 1 ^asap $(srcdir)debian/changelog
	$(GREP) "int Version" $(srcdir)asapinfo.ci
	$(GREP) "VERSION =" $(srcdir)chksap.pl
	$(GREP) android:versionName $(srcdir)java/android/AndroidManifest.xml
	$(GREP) ", v" $(srcdir)win32/rmt/apokeysnd_dll.c
.PHONY: version
