#!/usr/bin/perl

#	Rotate Sherlock logs
#	(c) 2001--2002 Martin Mares <mj@ucw.cz>

use File::stat;

@ARGV >= 3 or die "Usage: rotate-log <days-to-compress> <date-to-delete> <logs...>";

$now = time;
$cps = shift @ARGV;
$del = shift @ARGV;

$compress_thr = $now - 86400 * $cps;
$delete_thr = $now - 86400 * $del;
foreach $f (@ARGV) {
	-f $f or next;
	$st = stat $f or next;
	if ($del > 0 && $st->mtime < $delete_thr) {
		print "Deleting $f\n";
		unlink $f || die "Delete FAILED: $!";
	} elsif ($cps > 0 && $st->mtime < $compress_thr && $f !~ /\.(gz|bz2)$/) {
		print "Compressing $f\n";
		`gzip -f $f`;
		$? && die "Compression FAILED: $!";
	}
}
