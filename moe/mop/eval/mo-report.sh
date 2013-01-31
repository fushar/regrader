#!/usr/bin/perl
# A simple generator of evaluation reports
# (c) 2007 Martin Mares <mj@ucw.cz>

use strict;
use warnings;
use POSIX;

foreach my $user (`cd testing && ls`) {
	chomp $user;
	print "$user:";
	open REP, ">testing/$user/REPORT" or die;
	print REP "Evaluation report for CEOI 2007 Day 2\n\n";
	my $total = 0;
	foreach my $task (@ARGV) {
		print REP "### Task $task ###\n\n";
		if (open PTS, "testing/$user/$task/points") {
			my %merged_pts = ();
			my %split_comm = ();
			while (<PTS>) {
				chomp;
				my ($test, $pts, $comm) = /(\S+)\s+(\S+)\s+(.*)/;
				my $merged = $test;
				$merged =~ s/[^0-9]//;
				if (!defined($merged_pts{$merged}) || $merged_pts{$merged} > $pts) {
					$merged_pts{$merged} = $pts;
				}
				$split_comm{$merged}{$test} = $comm;
			}
			close PTS;
			my $tt = 0;
			foreach my $merged (sort { $a <=> $b } keys %merged_pts) {
				printf REP "Test %2s: %2d points", $merged, $merged_pts{$merged};
				$tt += $merged_pts{$merged};
				my @k = sort keys %{$split_comm{$merged}};
				if (@k == 1) {
					print REP " -- ", $split_comm{$merged}{$k[0]}, "\n";
				} else {
					print REP " -- ";
					my $cc = 0;
					foreach my $t (@k) {
						$cc++ and print REP ", ";
						print REP "$t: $split_comm{$merged}{$t}";
					}
					print REP "\n";
				}
			}
			print REP "\nTOTAL: $tt points\n\n";
			$total += $tt;
			print " $tt";
		} else {
			print REP "No solution submitted.\n\n";
			print " -";
		}
	}
	print REP "### TOTAL FOR DAY 2 ###\n\n";
	print REP "$total points\n";
	print REP "\n\n(generated on ", strftime("%Y-%m-%d %H:%M:%S", localtime), ")\n";
	close REP;
	print " -> $total\n";
}
