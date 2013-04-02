#!/bin/bash

{
  echo '==='
  echo -n 'date: ';  date '+%Y/%m/%d %H:%M:%S'
  echo -n 'uname: '; uname -a
  echo -n 'rev: ';   git rev-parse HEAD
  echo '+++'
  for test in "$@"; do
    ./$test 1 >/dev/null 2>&1 && ./$test 10 || echo "$test failed: $?"
  done
  echo '---'
} | tee -a bm.log

# vim:ts=2:sw=2:sts=2:et:ft=sh

