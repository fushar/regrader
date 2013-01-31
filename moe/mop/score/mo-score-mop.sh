#!/usr/bin/perl

use List::Util qw(min);

$tex = 0;
$usage = "Usage: mo-score-mop [--tex] theoretical_tasks_nr praxis_tasks_nr task1 task2 ...";
while (($arg = $ARGV[0]) =~ /^--([a-z]+)$/) {
	shift @ARGV;
	$var = "\$$1";
	if (!eval "defined $var") { die $usage; }
	eval "$var = 1;";
}
@ARGV >=2 || die $usage;
$theory=shift @ARGV;
$praxis=shift @ARGV;
@ARGV >= $praxis || die $usage;
$pos_delim=$tex ? '--' : '-';

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

print STDERR "Scanning teoretical results... ";
if (open (EX, "teorie.txt")) {
	while (<EX>) {
		chomp;
		(/^$/ || /^#/) && next;
		@a = split /\ *\t\ */;
		$u = shift @a;
		defined $users{$u} || die "Unknown user $u";
		$names{$u} = shift @a;
		$forms{$u} = shift @a;
		$addresses{$u} = "{". (shift @a) ."}";
		$i=0;
		while (@a) { $tasks{$u}{$i} = shift @a;$i++; }
	}
	close EX;
	print STDERR "OK\n";
} else {die "none, cannot find file teorie.txt!\n";}

print STDERR "Scanning task results... ";
$need_tasks = join("|", @ARGV);
foreach $u (keys %users) {
	opendir (D, "testing/$u") or next;
	foreach $t (readdir(D)) {
		$t =~ /^\./ && next;
		$t =~ /$need_tasks/ || next;

		$t_num=$praxis;
		for (my $t_num2=0;$t_num2<@ARGV;$t_num2++) {if ($t eq $ARGV[$t_num2]) {$t_num=$t_num2;}}
		$t_num+=$theory;

		$tt = "testing/$u/$t/points";
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
			$tasks{$u}{$t_num} += $groups{$g};
		}
	}
	closedir D;
}
print STDERR "OK\n";

print STDERR "Creating table template... ";
@body = ('','$names{$u}','$forms{$u}','$addresses{$u}');
for ($a=0;$a<$theory+$praxis;$a++) {push @body,"\$tasks{\$u}{$a}";}
print STDERR "OK\n";

print STDERR "Filling in results... ";
@table = ();
foreach $u (keys %users) {
	next unless defined $names{$u}; # don't show any user not defined in teorie.txt
	$row = [];
	$row_index=0;
	$row_sum=0;
	foreach my $c (@body) {
		$c =~ s/\$(\d+)/\$\$row[$1]/g;
		$x = eval $c;
		push @$row, (defined $x ? $x : '-');
		if ($row_index>3) {
		    if ((defined $x) && ($x>0)) {$row_sum+=$x;}
		}
		$row_index++;
	}
	push @$row, $row_sum;
	push @table, $row;
}
print STDERR "OK\n";

print STDERR "Sorting... ";
$sortcol = @{$table[0]} - 1;
$namecol = 1;
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
		${table[$j]}[0] = $j+1 . '.' . $pos_delim . $i . ".";
		$j_old=$j;
		$j++;
		while ($j < $i) { ${table[$j++]}[0] = $j_old+1 . '.' . $pos_delim . $i . "."; };
	}
}
print STDERR "OK\n";

if ($tex) {
        open HDR,"listina.hdr" or die "Cannot open file listina.hdr with TeX template!";
	while (<HDR>) {print; }
	close HDR;
	
	foreach $r (@table) { print join('&',@$r), "\\cr\n";}

        open FTR,"listina.ftr" or die "Cannot open file listina.ftr with TeX template!";
	while (<FTR>) {print; }
	close FTR;
} else {
	foreach $r (@table) { print join("\t",@$r), "\n"; }
}
