#	Perl module for UCW Configure Scripts
#
#	(c) 2005--2008 Martin Mares <mj@ucw.cz>
#
#	This software may be freely distributed and used according to the terms
#	of the GNU Lesser General Public License.

package UCW::Configure;

use strict;
use warnings;

BEGIN {
	# The somewhat hairy Perl export mechanism
	use Exporter();
	our ($VERSION, @ISA, @EXPORT, @EXPORT_OK, %EXPORT_TAGS);
	$VERSION = 1.0;
	@ISA = qw(Exporter);
	@EXPORT = qw(&Init &Log &Notice &Warn &Fail &IsSet &IsGiven &Set &UnSet &Append &Override &Get &Test &Include &Finish &FindFile &TryFindFile &DebugDump &PostConfig &AtWrite);
	@EXPORT_OK = qw();
	%EXPORT_TAGS = ();
}

our %vars;
our %overriden;
our @postconfigs;
our @atwrites;

sub DebugDump() {
	print "VARS:\n";
	print "$_: $vars{$_}\n" foreach( keys %vars );
}

sub Log($) {
	print @_;
}

sub Notice($) {
	print @_ if $vars{"VERBOSE"};
}

sub Warn($) {
	print "WARNING: ", @_;
}

sub Fail($) {
	Log("ERROR: " . (shift @_) . "\n");
	exit 1;
}

sub IsSet($) {
	my ($x) = @_;
	return exists $vars{$x};
}

sub IsGiven($) {
	my ($x) = @_;
	return exists $overriden{$x};
}

sub Get($) {
	my ($x) = @_;
	return $vars{$x};
}

sub Set($;$) {
	my ($x,$y) = @_;
	$y=1 unless defined $y;
	$vars{$x}=$y unless $overriden{$x};
}

sub UnSet($) {
	my ($x) = @_;
	delete $vars{$x} unless $overriden{$x};
}

sub Append($$) {
	my ($x,$y) = @_;
	Set($x, (IsSet($x) ? (Get($x) . " $y") : $y));
}

sub Override($;$) {
	my ($x,$y) = @_;
	$y=1 unless defined $y;
	$vars{$x}=$y;
	$overriden{$x} = 1;
}

sub Test($$$) {
	my ($var,$msg,$sub) = @_;
	Log "$msg ... ";
	if (!IsSet($var)) {
		Set $var, &$sub();
	}
	Log Get($var) . "\n";
}

sub TryFindFile($) {
	my ($f) = @_;
	if (-f $f) {
		return $f;
	} elsif ($f !~ /^\// && -f (Get("SRCDIR")."/$f")) {
		return Get("SRCDIR")."/$f";
	} else {
		return undef;
	}
}

sub FindFile($) {
	my ($f) = @_;
	my $F;
	defined ($F = TryFindFile($f)) or Fail "Cannot find file $f";
	return $F;
}

sub Init($$) {
	my ($srcdir,$defconfig) = @_;
	sub usage($) {
		my ($dc) = @_;
		print STDERR "Usage: [<srcdir>/]configure " . (defined $dc ? "[" : "") . "<config-name>" . (defined $dc ? "]" : "") .
			" [<option>[=<value>] | -<option>] ...\n";
		exit 1;
	}
	Set('CONFIG' => $defconfig) if defined $defconfig;
	if (@ARGV) {
		usage($defconfig) if $ARGV[0] eq "--help";
		if (!defined($defconfig) || $ARGV[0] !~ /^-?[A-Z][A-Z0-9_]*(=|$)/) {
			# This does not look like an option, so read it as a file name
			Set('CONFIG' => shift @ARGV);
		}
	}
	Set("SRCDIR", $srcdir);

	foreach my $x (@ARGV) {
		if ($x =~ /^(\w+)=(.*)/) {
			Override($1 => $2);
		} elsif ($x =~ /^-(\w+)$/) {
			Override($1 => 0);
			delete $vars{$1};
		} elsif ($x =~ /^(\w+)$/) {
			Override($1 => 1);
		} else {
			print STDERR "Invalid option $x\n";
			exit 1;
		}
	}

	defined Get("CONFIG") or usage($defconfig);
	if (!TryFindFile(Get("CONFIG"))) {
		TryFindFile(Get("CONFIG")."/config") or Fail "Cannot find configuration " . Get("CONFIG");
		Override("CONFIG" => Get("CONFIG")."/config");
	}
}

sub Include($) {
	my ($f) = @_;
	$f = FindFile($f);
	Notice "Loading configuration $f\n";
	require $f;
}

sub PostConfig(&) {
	unshift @postconfigs, $_[0];
}

sub AtWrite(&) {
	unshift @atwrites, $_[0];
}

sub Finish() {
	for my $post (@postconfigs) {
		&$post();
	}

	print "\n";

	if (Get("SRCDIR") ne ".") {
		Log "Preparing for compilation from directory " . Get("SRCDIR") . " to obj/ ... ";
		-l "src" and unlink "src";
		symlink Get("SRCDIR"), "src" or Fail "Cannot link source directory to src: $!";
		Override("SRCDIR" => "src");
		-l "Makefile" and unlink "Makefile";
		-f "Makefile" and Fail "Makefile already exists";
		symlink "src/Makefile", "Makefile" or Fail "Cannot link Makefile: $!";
	} else {
		Log "Preparing for compilation from current directory to obj/ ... ";
	}
	if (-d "obj") {
		`rm -rf obj`; Fail "Cannot delete old obj directory" if $?;
	}
	-d "obj" or mkdir("obj", 0777) or Fail "Cannot create obj directory: $!";
	-d "obj/ucw" or mkdir("obj/ucw", 0777) or Fail "Cannot create obj/ucw directory: $!";
	Log "done\n";

	Log "Generating config.mk ... ";
	open X, ">obj/config.mk" or Fail $!;
	print X "# Generated automatically by $0, please don't touch manually.\n";
	foreach my $x (sort keys %vars) {
		print X "$x=$vars{$x}\n";
	}
	print X "s=\${SRCDIR}\n";
	print X "o=obj\n";
	close X;
	Log "done\n";

	for my $wr (@atwrites) {
		&$wr();
	}
}

1;  # OK
