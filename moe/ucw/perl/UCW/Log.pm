#
#	Perl module for Logging
#
#	(c) 2007 Pavel Charvat <pchar@ucw.cz>
#

package UCW::Log;

use lib 'lib/perl5';
use strict;
use warnings;
use POSIX;
use Exporter;

our $version = 1.0;
our @ISA = qw(Exporter);
our @EXPORT = ();
our %EXPORT_TAGS = ( all => [qw(&Log &Die)]);
our @EXPORT_OK = (@{$EXPORT_TAGS{'all'}});

my $Prog = (reverse split(/\//, $0))[0];

sub Log {
  my $level = shift;
  my $text = join(' ', @_);
  print STDERR $level, strftime(" %Y-%m-%d %H:%M:%S ", localtime()), "[$Prog] ", $text, "\n";
}

sub Die {
  Log('!', @_);
  exit 1;
}

1;
