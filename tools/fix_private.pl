#!/usr/bin/env perl

use strict;
use warnings;

use Path::Class;
use Data::Dumper;

use constant PUBLIC  => 'jsondata.h';
use constant PRIVATE => 'jd_private.h';

my @bad_public  = my @good_public  = read_header(PUBLIC);
my @bad_private = my @good_private = read_header(PRIVATE);

s/jd_+/jd__/ for @bad_public;
s/jd_+/jd_/  for @good_public;
s/jd_+/jd_/  for @bad_private;
s/jd_+/jd__/ for @good_private;

my %fixup = ();
@fixup{ @bad_public, @bad_private } = ( @good_public, @good_private );
my $match = join '|', sort keys %fixup;
my $fixre = qr{\b($match)\b};

open my $ack, '-|', ack => '-f' or die "Can't run ack: $!\n";
while (<$ack>) {
  chomp;
  fixup( $_, \%fixup );
}

sub fixup {
  my $fn  = shift;
  my $src = file($fn)->slurp;
  ( my $dst = $src ) =~ s/$fixre/$fixup{$1}||$1/mseg;
  if ( $dst ne $src ) {
    print "Updating $fn\n";
    my $tmp = "$fn.tmp";
    print { file($tmp)->openw } $dst;
    rename $tmp, $fn or die "Can't rename $tmp as $fn: $!\n";
  }
}

sub read_header {
  my $hdr = shift;

  my %exp = ();
  for my $ln ( file($hdr)->slurp ) {
    $exp{$1}++ if $ln =~ /\b(jd_\w+)\(/;
  }

  return sort keys %exp;
}

# vim:ts=2:sw=2:sts=2:et:ft=perl

