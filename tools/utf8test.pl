#!/usr/bin/env perl

use strict;
use warnings;

use Encode;

=for reference

   7 00000000 0000007F 1 0xxxxxxx
  11 00000080 000007FF 2 110xxxxx  10xxxxxx
  16 00000800 0000FFFF 3 1110xxxx  10xxxxxx  10xxxxxx
  21 00010000 001FFFFF 4 11110xxx  10xxxxxx  10xxxxxx  10xxxxxx
  26 00200000 03FFFFFF 5 111110xx  10xxxxxx  10xxxxxx  10xxxxxx  10xxxxxx
  31 04000000 7FFFFFFF 6 1111110x  10xxxxxx  10xxxxxx  10xxxxxx  10xxxxxx  10xxxxxx

=cut

my @cp = (
  0x00000000, 0x0000007F, 0x00000080, 0x000007FF, 0x00000800, 0x0000FFFF,
  0x00010000, 0x001FFFFF, 0x00200000, 0x03FFFFFF, 0x04000000, 0x7FFFFFFF
);

my @cp16 = (
  0x00000000, 0x0000007F, 0x00000080, 0x000007FF, 0x00000800, 0x0000FFFF
);

show(@cp);
show(@cp16);

sub utf8bytes { unpack 'C*', encode( 'utf8', join '', map chr, @_ ) }

sub show {
  my @oct = utf8bytes(@_);
  print join( ', ', map { sprintf '0x%02x', $_ } @oct ), "\n";
}

# vim:ts=2:sw=2:sts=2:et:ft=perl

