#!/bin/bash

{
  echo '==='
  echo -n 'date: ';  date '+%Y/%m/%d %H:%M:%S'
  echo -n 'uname: '; uname -a
  echo -n 'rev: ';   git rev-parse HEAD
  echo '+++'
  export JD_BM_TIME=10
  for test in "$@"; do
    ./$test
  done
  echo '---'
} | tee -a bm.log

# vim:ts=2:sw=2:sts=2:et:ft=sh

