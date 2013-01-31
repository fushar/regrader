#!/usr/bin/perl

use strict;
use warnings;

BEGIN {
	defined $ENV{"MO_ROOT"} or die "Please set MO_ROOT to the contest root directory first.\n";
}
use lib $ENV{"MO_ROOT"} . "/submit";
use lib $ENV{"MO_ROOT"} . "/lib/perl5";

use MO::Submit;
use Sherlock::Object;
use File::stat;

@ARGV == 2 || @ARGV == 3 or die "Usage: remote-submit <task> [<part>] <filename>\n";
my $task = $ARGV[0];
my $part = $task;
if (@ARGV == 3) {
	$part = $ARGV[1];
	shift @ARGV;
}
my $file = $ARGV[1];
my ($ext) = ($file =~ /\.([^.]+)$/) or die "Unable to determine filename extension\n";

open F, $file or die "Unable to open $file: $!\n";
my $s = stat(*F) or die;
my $size = $s->size;

my $conn = new MO::Submit;

my $he = $conn->write_history($task, $part, $ext, $file);
if (defined $he) { die "$he\n"; }

$conn->connect or die $conn->{"error"} . "\n";

sub or_die($) {
	my $r = shift @_;
	if (!defined $r) { die $conn->{"error"} . "\n"; }
	my $err = $r->get("-");
	if ($err) { die "$err\n"; }
}

my $r = new Sherlock::Object("!" => "SUBMIT", "T" => $task, "P" => $part, "X" => $ext, "S" => $size);
$r = $conn->request($r);
or_die($r);

$r = $conn->send_file(\*F, $size);
or_die($r);

print "Submitted OK.\n";

$conn->disconnect;
close F;
