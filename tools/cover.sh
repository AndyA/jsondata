#!/bin/bash

set -x
find . \( -name '*.gcno' -o -name '*.gcda' -o -name '*.gcov' \
  -o -name '*.info' \) -print0 | xargs -0 rm
rm -rf cover
make clean && make PROFILE=yes

infos=""
for test in t/*.t; do
  bin=${test%.t}
  target=$( basename $bin )
  info="$target.info"
  result="cover/$target"
  make -C t $target PROFILE=yes
  find . -name '*.gcda' -print0 | xargs -0 rm
  cd t
  ./$target 
  cd ..
  ./$bin
  lcov -t "$bin" -o "$info" -c -d .
  infos="$infos -a $info"
  genhtml -o "$result" "$info"
done
lcov -o "all.info" $infos
genhtml -o "cover/all" "all.info"
find . \( -name '*.gcno' -o -name '*.gcda' -o -name '*.gcov' \
  -o -name '*.info' \) -print0 | xargs -0 rm
make clean

# vim:ts=2:sw=2:sts=2:et:ft=sh

