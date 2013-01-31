#!/usr/bin/perl
# A generator of score sheets. More ugly than it deserves.

$debug = 0;
$detail = 0;
$html = 0;
$tex = 0;
$extras = 0;
$alt = 0;
$merged = 0;
$table = 0;
$usage = "Usage: mo-score [--detail] [--alt] [--extras] [--html] [--tex] [--table] [--merged] [<directory>/]<task> ...";
while (($arg = $ARGV[0]) =~ /^--([a-z]+)$/) {
	shift @ARGV;
	$var = "\$$1";
	if (!eval "defined $var") { die $usage; }
	eval "$var = 1;";
}
@ARGV || die $usage;

$print_key = 1;
if ($table) {
	open STDOUT, "|column -t -s'\t'" or die;
	$print_key = 0;
}

print STDERR "Scanning contestants... ";
open (CT, "bin/mo-get-users --full |") || die "Cannot get list of contestants";
while (<CT>) {
	chomp;
	($u,$f) = split /\t/;
	($u eq "somebody") && next;
	$users{$u}=$f;
}
close CT;
print STDERR 0+keys %users, "\n";

print STDERR "Scanning exceptions... ";
if ($extras && open (EX, "exceptions")) {
	while (<EX>) {
		chomp;
		(/^$/ || /^#/) && next;
		@a = split /\s+/;
		$u = shift @a;
		defined $users{$u} || die "Unknown user $u";
		while (@a) { $extra{$u} += shift @a; }
	}
	close EX;
	print STDERR "OK\n";
} else { print STDERR "none\n"; }

print STDERR "Scanning task results... ";
%messages = ();
%error_codes = ();
foreach $u (keys %users) {
	foreach $task (@ARGV) {
		my ($dir, $t) = ("testing", $task);
		if ($task =~ m@^(.*)/([^/]*)$@) {
			$dir = $1;
			$t = $2;
		}
		$known_tasks{$t} = 1;
		$tt = "$dir/$u/$t/points" . ($alt ? ".alt" : "");
		-f $tt || next;
		print STDERR "$u/$t ";
		open (X, $tt) || die "Unable to open $tt";
		while (<X>) {
			chomp;
			/^(\S+) (-?\d+)\s*(.*)/ || die "Parse error: $_";
			$ttest = $1;
			$tpts = $2;
			$trem = $3;
			$trem =~ s/\[.*//;
			($ttest_merged = $ttest) =~ s/[^0-9]//g;
			$ttest = $ttest_merged if $merged;
			$known_tests{$t}{$ttest} = 1;
			$cmt = $tpts;
			if ($tpts == 0 && $trem ne "OK") {
				if ($trem =~ /^Compile /) { $cmt = "CE"; }
				elsif ($trem =~ /^Time limit exceeded/) { $cmt = "TO"; }
				elsif ($trem =~ /^Exited with error /) { $cmt = "RE"; }
				elsif ($trem =~ /^Caught fatal signal /) { $cmt = "SG"; }
				elsif ($trem =~ /^([A-Za-z])\S*\s+([A-Za-z])/) {
					($cmt = "$1$2") =~ tr/a-z/A-Z/;
				} elsif ($trem =~ /^([A-Za-z]{2})/) {
					($cmt = $1) =~ tr/a-z/A-Z/;
				}
				if (!defined $messages{$trem}) {
					$messages{$trem} = $cmt;
					if (!defined $error_codes{$cmt}) {
						$error_codes{$cmt} = $trem;
					} else {
						$error_codes{$cmt} .= ", $trem";
					}
				}
			}
			if (!defined($results{$u}{$t}{$ttest}) || $results{$u}{$t}{$ttest} > $tpts) {
				$results{$u}{$t}{$ttest} = $tpts;
				$comment{$u}{$t}{$ttest} = $cmt;
			}
			if (!defined($results_merged{$u}{$t}{$ttest_merged}) || $results_merged{$u}{$t}{$ttest_merged} > $tpts) {
				$results_merged{$u}{$t}{$ttest_merged} = $tpts;
			}
		}
		close X;
	}
	foreach my $t (keys %known_tasks) {
		$total{$u}{$t} = 0;
		foreach my $pts (values %{$results_merged{$u}{$t}}) { $total{$u}{$t} += $pts; }
	}
}
print STDERR "OK\n";

print STDERR "Creating table template... ";
@header = ("Rank","User","Name");
@body = ('','$u','$users{$u}');
@bodysums = ();
@footer = ('"Total"','','');
if (keys %extra) {
	push @header, "Extra";
	push @body, '$extra{$u}';
	$col = 0+@footer;
	push @bodysums, $col;
	push @footer, "sum($col)";
}
@tasks = ();
foreach $task (@ARGV) {
	my $t = ($task =~ m@/([^/]*)$@) ? $1 : $task;
	defined $known_tasks{$t} || die "Unknown task $t";
	push @tasks, $t;
	push @header, substr($t, 0, 4);
	push @body, "(\$xx = \$total{\$u}{'$t'}) > 0 ? \$xx : 0";
	$col = 0+@footer;
	push @footer, "sum($col)";
	push @bodysums, $col;
	if ($detail) {
		foreach $s (sort { $a <=> $b } keys %{$known_tests{$t}}) {
			push @header, "$s";
			push @body, "\$comment{\$u}{'$t'}{'$s'}";
			$col = 0+@footer;
			push @footer, "sum($col)";
		}
	}
}
push @header, "Total";
push @body, join('+', map { $_ = "\$$_" } @bodysums);
$col = 0+@footer;
push @footer, "sum($col)";
print STDERR "OK\n";

print STDERR "h: ", join(':',@header), "\n" if $debug;
print STDERR "b: ", join(':',@body), "\n" if $debug;
print STDERR "f: ", join(':',@footer), "\n" if $debug;

print STDERR "Filling in results... ";
@table = ();
foreach $u (keys %users) {
	$row = [];
	foreach my $c (@body) {
		$c =~ s/\$(\d+)/\$\$row[$1]/g;
		$x = eval $c;
		push @$row, (defined $x ? $x : '-');
	}
	print STDERR "row: ", join(':',@$row), "\n" if $debug;
	push @table, $row;
}
print STDERR "OK\n";

print STDERR "Sorting... ";
$sortcol = @{$table[0]} - 1;
$namecol = 2;
@table = sort {
	my $p, $an, $bn;
	$p = $$b[$sortcol] <=> $$a[$sortcol];
	($an = $$a[$namecol]) =~ s/(\S+)\s+(\S+)/$2 $1/;
	($bn = $$b[$namecol]) =~ s/(\S+)\s+(\S+)/$2 $1/;
	$p ? $p : ($an cmp $bn);
} @table;
$i=0;
while ($i < @table) {
	$j = $i;
	while ($i < @table && ${$table[$i]}[$sortcol] == ${$table[$j]}[$sortcol]) {
		$i++;
	}
	if ($i == $j+1) {
		${table[$j]}[0] = "$i.";
	} else {
		${table[$j]}[0] = $j+1 . ".-" . $i . ".";
		$j++;
		while ($j < $i) { ${table[$j++]}[0] = " "; };
	}
}
print STDERR "OK\n";

print STDERR "Attaching headers and footers... ";
sub sum { my $col=shift @_; my $t=0; foreach my $z (0..@table-1) { $t += ${$table[$z]}[$col]; } $t; }
map { $_ = eval $_; } @footer;
push @table, \@footer;
unshift @table, \@header;
print STDERR "OK\n";

if ($debug) {
	foreach $r (@table) { print join(':',@$r), "\n"; }
} elsif ($html) {
	print '<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01//EN" "http://www.w3.org/TR/html40/strict.dtd">', "\n";
	print "<HTML><HEAD><TITLE>Rank list</TITLE></HEAD><BODY>\n";
	print "<H1>Rank list</H1>\n";

	my @perm;
 	&printHtmlHeader(\@perm);
	print "<tbody>";

	foreach $r (@table[1..($#table - 1)]) {
		&printHtmlRow(@{$r}[@perm]);
	}

	print "<tbody>";
	&printHtmlRow(@{$table[$#table]}[@perm]);

	print "</TABLE>\n";
	if ($detail) {
		print "<H2>Error codes</H2><UL>\n";
		foreach $r (sort keys %error_codes) { print "<LI>$r: $error_codes{$r}\n"; }
		print "</UL>\n";
	}
	print "</BODY></HTML>\n";
} elsif ($tex) {
	print "\\error{TeX output not supported yet!}\n";
} else {
	foreach $r (@table) { print join("\t",@$r), "\n"; }
	if ($print_key) {
		print "\n";
		foreach $r (sort keys %error_codes) { print "$r: $error_codes{$r}\n"; }
	}
}

sub printHtmlRow {
	print "<TR>", join('',map {
		if ($hdr) { $_ = "<TH>$_"; }
		else { $_ = "<TD align=" . (/^[0-9A-Z-]+$/ ? "right" : "left") . (length($_) > 14 ? " width=150" : "") . ">$_"; }
	} @_), "\n";
}

sub printHtmlHeader {

  my ($perm) = @_;

   my $colspec = "<colgroup span=3>";
   my $hdr1;
   my $hdr2;

   @$perm = (0, 1, 2);
   my $p = 3;

   if ($detail) {
     $hdr1 = "<th rowspan=2>Rank<th rowspan=2>User<th rowspan=2>Name";
     $extras and $p++ and push @$perm, 3 and $hdr1.="<th rowspan=2>Extra" and $colspec.="<colgroup span=1>";         ##Extra hack
     for my $task (@tasks) {

	my $nSub = scalar(keys %{$known_tests{$task}});

	$p++;
	map { push @$perm, $p++ } (1..$nSub);
	push @$perm, $p - $nSub - 1;

	$colspec .= "<colgroup span='" . $nSub . "'>\n";
	$colspec .= "<colgroup span='1'>\n";
	$hdr1 .= "<th colspan='" . ($nSub + 1) . "' style='border-bottom:1px solid black;'>$task";
	$hdr2 .= join("", map { "<th>$_" } sort {$a <=> $b} keys %{$known_tests{$task}});
	$hdr2 .= "<th>Total";
     }

     $hdr1 .= "<th rowspan='2'>Total";

   } else {  ## no detail

     $hdr1 = "<th>Rank<th>User<th>Name";
     $extras and $p++ and push @$perm, 3 and $hdr1.="<th>Extra" and $colspec.="<colgroup span=1>";                  ##Extra hack

     for my $task (@tasks) {
        push @$perm, $p++;
	$hdr1 .= "<th>$task";
     }
     $hdr1 .= "<th>Total";
     $colspec .= "<colgroup span='" . scalar (@tasks) . "'>";
   }

   push @$perm, $p++;

   print "<TABLE rules=groups frame=all border='1' cellpadding='2'>\n";
   print "$colspec<colgroup span='1'>\n";
   print "<tr>$hdr1</tr>\n";
   print "<tr>$hdr2</tr>\n" if $detail;

}

close STDOUT;
