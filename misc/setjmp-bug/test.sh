#!/bin/bash

for cc in gcc clang; do
  ccbin=$( which $cc )
  if [ "$ccbin" ]; then
    echo -n "Testing with: "
    $cc --version | head -n 1
    for opt in g O{1..3}; do
      out="sjb-$cc-$opt"
      echo "  Testing $out"
      $cc -o $out -$opt setjmp-bug.c && ./$out && objdump -d $out > $out.s
    done
  fi
done



# vim:ts=2:sw=2:sts=2:et:ft=sh

