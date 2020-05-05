# This file is part of the Aspekt.
#
# (c) airSlate Inc. <support@airslate.com>
#
# For the full copyright and license information, please view
# the LICENSE file that was distributed with this source code.

clean: lexer-clean

.PHONY: lexer-clean
lexer-clean:
	@for /f "delims==" %F in ('@dir /B /S *.loT *.out Makefile') do @if not %~nF == Makefile del /q /f %F

.PHONY: lexer-clean tests-clean
tests-clean:
	@for /f "delims==" %F in ('dir /B /S .\tests\*.php .\tests\*.sh .\tests\skipif.inc') do @if not %~nF == skipif del /q /f %F
	@for /f "delims==" %F in ('dir /B /S .\tests\*.diff .\tests\*.exp .\tests\skipif.inc') do @if not %~nF == skipif del /q /f %F
	@for /f "delims==" %F in ('dir /B /S .\tests\*.tmp .\tests\*.log .\tests\skipif.inc') do @if not %~nF == skipif del /q /f %F
	@for /f "delims==" %F in ('dir /B /S .\tests\*.out .\tests\skipif.inc') do @if not %~nF == skipif del /q /f %F

.PHONY: maintainer-clean
maintainer-clean:
	@echo This command is intended for maintainers to use; it
	@echo deletes files that may need special tools to rebuild.
	@if exist $(PHP_SRC_DIR)\aspekt\lexer.c ( del /q /f $(PHP_SRC_DIR)\aspekt\lexer.c )

$(PHP_SRC_DIR)\aspekt\lexer.c: $(PHP_SRC_DIR)\aspekt\lexer.re
	$(RE2C) -d -i --no-generation-date -o $(PHP_SRC_DIR)\aspekt\lexer.c $(PHP_SRC_DIR)\aspekt\lexer.re
