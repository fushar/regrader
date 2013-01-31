#!/usr/bin/perl
# Time limit calibrator: Simulation
# (c) 2010 Martin Mares <mj@ucw.cz>

use strict;
use warnings;
use List::Util qw(max);

my $t = $ARGV[0] or die;
my $thresh = $ARGV[1] || 999;
my $debug = 0;

my %want = ();
my %points = ();
my %times = ();

for my $a (<cal/$t/*.points>) {
	my $sol = $a;
	$sol =~ s/^.*\///;
	$sol =~ s/\.points$//;
	my ($want) = ($sol =~ m{(\d+)}) or die "Cannot parse $sol\n";
	print "$sol (want $want):" if $debug;
	$want{$sol} = $want;

	open PTS, $a or die;
	while (<PTS>) {
		chomp;
		my ($test, $pts, $comment) = split /\s+/;
		$points{$sol}{$test} = $pts;
	}
	close PTS;

	open LOG, "cal/$t/$sol.log" or die;
	while (<LOG>) {
		chomp;
		my ($test, $time) = m{/(\d+[a-z]*)\.log:.*\(([0-9.]+) } or die "Log parse error: $_";
		$times{$sol}{$test} = $time;
	}
	close LOG;

	for my $t (sort keys %{$points{$sol}}) {
		defined $times{$sol}{$t} or $times{$sol}{$t}=0;
		print " $t:", $points{$sol}{$t}, "/", $times{$sol}{$t} if $debug;
	}

	print "\n" if $debug;
}

print "\n## Threshold=$thresh\n\n";

for my $s (sort keys %want) {
	my %groups = ();
	my %oks = ();
	my %pts = ();
	my $maxtime = 0;
	my @acc;
	my $lastg = '';
	for my $t (sort keys %{$points{$s}}) {
		my $g = $t;
		$g =~ s{\D+}{};
		if ($g ne $lastg && $lastg ne '') {
			push @acc, "|";
		}
		$lastg = $g;
		$groups{$g}++;
		$oks{$g} += 0;
		if ($times{$s}{$t} <= $thresh) {
			push @acc, ($points{$s}{$t} ? "+" : 0);
			$pts{$g} = $points{$s}{$t};
			$oks{$g}++ if $points{$s}{$t};
			$maxtime = max($maxtime, $times{$s}{$t});
		} else {
			push @acc, "T";
		}
	}

	my $sum = 0;
	for my $g (keys %groups) {
		if ($groups{$g} == $oks{$g}) {
			$sum += $pts{$g};
		}
	}
	printf "%-40s %2d/%2d  %2.3f  %s\n", $s, $sum, $want{$s}, $maxtime, join("", @acc);
}
