#!/usr/bin/env perl

use strict;
use warnings;

use Path::Class;
use Data::Dumper;

use constant PUBLIC => ('jsondata.h');
use constant PRIVATE => ( 'jd_private.h', 'jd_path.h', 'jd_utf8.h' );

my @bad_public  = my @good_public  = map read_header($_), PUBLIC;
my @bad_private = my @good_private = map read_header($_), PRIVATE;

s/^jd_+/jd__/ for @bad_public,  @good_private;
s/^jd_+/jd_/  for @good_public, @bad_private;

my %seen = map { $_ => 1 } @good_public;
$seen{$_}++ for @bad_private;
my @dup = grep { $seen{$_} > 1 } keys %seen;
if (@dup) {
  print
   "The following symbols appear in both public and private headers:\n";
  print "  $_\n" for sort @dup;
  print "\nPlease fix and try again.\n";
  exit 1;
}

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

