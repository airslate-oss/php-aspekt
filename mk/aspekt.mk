# This file is part of the Aspekt.
#
# (c) airSlate Inc. <support@airslate.com>
#
# For the full copyright and license information, please view
# the LICENSE file that was distributed with this source code.

RE2C := $(shell command -v re2c 2>/dev/null)
ifndef RE2C
$(error No re2c found in the $$PATH. Consider install re2c or/and add re2c executable to the $$PATH)
endif

RE2C_FLAGS=
RE2C_VERSION=$(shell $(RE2C) --vernum 2>/dev/null)
ifeq ($(shell test "$(RE2C_VERSION)" -gt "9999"; echo $$?),0)
RE2C_FLAGS=-W
endif

test: export NO_INTERACTION=1
clean: build-clean tests-clean
distclean: custom-clean

.PHONY: custom-clean
custom-clean:
	$(RM) -r ./include
	$(RM) ./configure.ac
	$(RM) ./configure.info
	$(RM) ./coverage.info

.PHONY: build-clean
build-clean:
	find . -name '*.loT' -o -name '*.out' -type f -delete
	find . -name '*.deps' -type f -delete

.PHONY: tests-clean
tests-clean:
	find ./tests -name '*.php' -o -name '*.sh' -type f -delete
	find ./tests -name '*.diff' -o -name '*.exp' -o -name '*.log' -type f -delete
	find ./tests -name '*.tmp' -o -name '*.out' -type f -delete
	find ./tests/_output ! -iname '.gitignore' -type f -delete

.PHONY: maintainer-clean
maintainer-clean:
	@echo 'This command is intended for maintainers to use; it'
	@echo 'deletes files that may need special tools to rebuild.'
	@echo
	$(RM) $(srcdir)/aspekt/lexer.c

$(srcdir)/aspekt/lexer.c: $(srcdir)/aspekt/lexer.re
	$(RE2C) $(RE2C_FLAGS) -d --no-generation-date -o $@ $<
