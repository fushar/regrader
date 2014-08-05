#!/usr/bin/perl
use strict;
use warnings;
use List::Util qw(min);

@ARGV or die "Usage: mo-score-mop task1 task2 ...";

print STDERR "Scanning contestants... ";
open (CT, "bin/mo-get-users --full |") || die "Cannot get list of contestants";
my %users = ();
while (<CT>) {
	chomp;
	my ($u, $f) = split /\t/;
	$u =~ /^mo/ or next;
	$users{$u}=$f;
}
close CT;
print STDERR 0+keys %users, "\n";

print STDERR "Scanning task results... ";
my %tasks = ();
for my $u (keys %users) {
	for my $t (@ARGV) {
		my $tt = "testing/$u/$t/points";
		-f $tt || next;
		print STDERR "$u/$t ";
		open (X, $tt) || die "Unable to open $tt";
		my %groups = ();
		while (<X>) {
			chomp;
			my ($test, $pts) = /^(\S+) (-?\d+)/ or die "Parse error: $_";
			my $group = $test;
			$group =~ s{\D}{}g;
			if (defined $groups{$group}) {
				$groups{$group} = min($groups{$group}, $pts);
			} else {
				$groups{$group} = $pts;
			}
		}
		close X;

		for my $g (keys %groups) {
			$tasks{$u}{$t} += $groups{$g};
		}
	}
}
print STDERR "OK\n";

print STDERR "Generating output... ";
for my $u (sort keys %users) {
	print join("\t", $u, $users{$u}, map { $tasks{$u}{$_} // '-' } @ARGV), "\n";
}
print STDERR "OK\n";

