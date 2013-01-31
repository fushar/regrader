#!/usr/bin/perl

use lib "lib/perl5";
use Sherlock::Object;

foreach my $user (split /\s+/,`cd solutions && echo *`) {
	print "$user:\t";
	if (open S, "solutions/$user/status") {
		my @status = ();
		my $s = new Sherlock::Object;
		$s->read(\*S) or die "Cannot load status";
		foreach my $t ($s->getarray("(T")) {
			my $task = $t->get("T");
			foreach my $p ($t->getarray("(P")) {
				my $part = $p->get("P");
				my $ver = $p->get("V");
				my $name = $task . ($part eq $task ? "" : "/$part");
				push @status, "$name(v$ver)";
			}
		}
		close S;
		print join(" ", @status), "\n";
	} else {
		print "---\n";
	}
}
