VERSION := $(shell node -e "console.log(require('./package.json').version)")
AUTH_FILE := blynk-auth-token
AUTH := $(shell test -e $(AUTH_FILE) && egrep -v '(^[[:space:]]*$|^\#)' $(AUTH_FILE))
FAKE_AUTH := deadbeefdeadbeef0123456789012345
BLYNK_SRC := garage-relays-blynk.ino
SRC_AUTHED := garage-relays-blynk.auth.ino
OUTFILE := firmware.bin


.PHONY: default ver changeincludes compile auth clean

# Add a default task so we don't release just because someone ran 'make'
default: ver compile

ver:
	@echo "Version $(VERSION)."

# I can't figure out how to avoid putting library files in the same directory
# as the source files when using the particle cli cloud compiler.  I would like
# to use the same source locally and also in the web IDE, but the #include path
# seems like it needs to differ.
compile: clean auth changeincludes
	@mkdir -p build
	@cp -f blynk-library-spark/firmware/*.{cpp,h} build
	@ln -sf ../particle.include build/
	@particle compile photon build/ --saveTo target/$(OUTFILE)
	@echo
	@echo "Warning: This firmware binary looks nothing like the result one gets when using the WebIDE compiler."
	@echo "It's untested, seems too small, and almost certainly broken.  FIXME!"
	@echo "Instead, copy/paste $(BLYNK_SRC) into the web IDE at http://build.particle.io"

auth:
	@mkdir -p build
	@sed -e 's/$(FAKE_AUTH)/$(AUTH)/' src/$(BLYNK_SRC) > build/$(SRC_AUTHED)
	@echo Created build/$(SRC_AUTHED)
	@grep 'auth.*=' build/$(SRC_AUTHED)

changeincludes:
	@sed -i '' -e 's/\(#include "\)[^\/]*\/\(.*\)/\1\2/' build/$(SRC_AUTHED)

clean:
	@rm -rf build
	@rm -f target/$(OUTFILE)

