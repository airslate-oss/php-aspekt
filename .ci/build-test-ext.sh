#!/bin/sh
#
# This file is part of the Aspekt.
#
# (c) airSlate Inc. <support@airslate.com>
#
# For the full copyright and license information, please view
# the LICENSE file that was distributed with this source code.

# -e	Exit immediately if a command exits with a non-zero status.
# -u	Treat unset variables as an error when substituting.
set -eu

cd "$(dirname "$0")/../"

if test -f Makefile
then
	make distclean
	phpize --clean
fi

phpize

./configure --enable-aspekt --enable-aspekt-debug --enable-coverage

make -j"$(getconf _NPROCESSORS_ONLN)"
make coverage-initial

(>&1 printf "\\n\\tThanks for compiling Aspekt!")
(>&1 printf "\\n\\tBuild succeed: Please restart your web server to complete the installation.\\n\\n")
