VERSION := $(shell node -e "console.log(require('./package.json').version)")
AUTH_FILE := blynk-auth-token
AUTH := $(shell test -e $(AUTH_FILE) && egrep -v '(^[[:space:]]*$|^\#)' $(AUTH_FILE))
FAKE_AUTH := deadbeefdeadbeef0123456789012345
BLYNK_SRC := garage-relays-blynk.ino
SRC_AUTHED := garage-relays-blynk.auth.ino
OUTFILE := firmware.bin


.PHONY: default ver compile auth clean

# Add a default task so we don't release just because someone ran 'make'
default: ver compile

ver:
	@echo "Version $(VERSION)."

compile: clean auth
	@mkdir -p build
	@ln -sFf ../blynk-library-spark/firmware build/blynk
	@ln -sf ../particle.include build/
	@particle compile photon build/ --saveTo target/$(OUTFILE)

auth:
	@mkdir -p build
	@sed -e 's/$(FAKE_AUTH)/$(AUTH)/' src/$(BLYNK_SRC) > build/$(SRC_AUTHED)
	@echo Created build/$(SRC_AUTHED)
	@grep 'auth.*=' build/$(SRC_AUTHED)

clean:
	@rm -rf build
	@rm -f target/$(OUTFILE)

