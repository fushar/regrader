#!/usr/bin/perl

use strict;
use warnings;

BEGIN {
	defined $ENV{"MO_ROOT"} or die "Please set MO_ROOT to the contest root directory first.\n";
}
use lib $ENV{"MO_ROOT"} . "/lib/perl5";
use lib $ENV{"MO_ROOT"} . "/submit/lib/perl5";

use MO::Submit;
use Sherlock::Object;
use POSIX;

@ARGV == 0 or die "Usage: remote-status\n";

my $conn = new MO::Submit;
$conn->connect or die $conn->{"error"} . "\n";

sub or_die($) {
	my $r = shift @_;
	if (!defined $r) { die $conn->{"error"} . "\n"; }
	my $err = $r->get("-");
	if ($err) { die "$err\n"; }
}

my $r = new Sherlock::Object("!" => "STATUS");
$r = $conn->request($r);
or_die($r);
#$r->write_indented(*STDOUT);

foreach my $t ($r->getarray("(T")) {
	my $task = $t->get("T");
	foreach my $p ($t->getarray("(P")) {
		my $part = $p->get("P");
		my $name = $task;
		$part eq $task or $name .= "/$part";
		printf "%-16s", $name;

		my $current_ver = $p->get("V");
		my $printed = 0;
		foreach my $v ($p->getarray("(V")) {
			if ($v->get("V") == $current_ver) {
				my $time = strftime("%H:%M:%S", localtime $v->get("T"));
				print "OK (",
					"$part.", $v->get("X"), ", ",
					$v->get("L"), " bytes, ",
					$v->get("S"), " $time)\n";
				$printed = 1;
			}
		}
		$printed or print "---\n";
	}
}

$conn->disconnect;
