#!/bin/bash

set -x
find . \( -name '*.gcno' -o -name '*.gcda' -o -name '*.gcov' \
  -o -name '*.info' \) -print0 | xargs -0r rm
rm -rf cover
CFLAGS="-fprofile-arcs -ftest-coverage" LDFLAGS="-lgcov" ./configure && \
  make clean && make

infos=""

for test in t/*.c; do
  [ $test = 't/jd_test.c' -o $test = 't/tap.c' -o $test = 't/framework.c' ] && continue
  bin=${test%.*}
  target=$( basename $bin )
  info="$target.info"
  result="cover/$target"
  #  make -C t $target
  find . -name '*.gcda' -print0 | xargs -0r rm
  pushd t >/dev/null 2>&1
  ./$target 
  popd >/dev/null 2>&1
  lcov -t "$bin" -o "$info" -c -d .
  infos="$infos -a $info"
  genhtml -o "$result" "$info"
done
lcov -o "all.info" $infos
genhtml -o "cover/all" "all.info"
find . \( -name '*.gcno' -o -name '*.gcda' -o -name '*.gcov' \
  -o -name '*.info' \) -print0 | xargs -0r rm
make clean

# vim:ts=2:sw=2:sts=2:et:ft=sh

