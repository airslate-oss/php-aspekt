#!/usr/bin/env bash
#
# This file is part of the Aspekt.
#
# (c) airSlate Inc. <support@airslate.com>
#
# For the full copyright and license information, please view
# the LICENSE file that was distributed with this source code.

command -v autoconf && autoconf --version
echo "-------------------------------------------------------------------------"

command -v cc && cc --version
echo "-------------------------------------------------------------------------"

command -v make && make --version
echo "-------------------------------------------------------------------------"

command -v re2c && re2c --version
echo "-------------------------------------------------------------------------"

command -v php && (php -v; php -m)
echo "-------------------------------------------------------------------------"

command -v php-config && (php-config || true) # php-config returns 1
echo "-------------------------------------------------------------------------"

command -v phpize && phpize --version
echo "-------------------------------------------------------------------------"

ls -al "$(php-config --extension-dir)"
echo "-------------------------------------------------------------------------"

shopt -s nullglob

export LC_ALL=C

count=0
while IFS= read -r -d '' file
do
  (( count++ ))
  (>&1 printf ">>> START (%d)\\n%s\\n<<< END (%d)\\n\\n" ${count} "$(cat "$file")" ${count})
done < <(find ./tests -type f \( -name '*.out' -o -name '*.mem' \) -print0)

ls /tmp/

for i in /tmp/core.php.*; do
  (>&1 printf "Found core dump file: %s\\n\\n" "$i")
  gdb -q "$(phpenv which php)" "$i" <<EOF
set pagination 0
backtrace full
info registers
x/16i \$pc
thread apply all backtrace
quit
EOF
done
