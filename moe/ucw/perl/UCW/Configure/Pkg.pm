# UCW Library configuration system: pkg-config and friends
# (c) 2008 Martin Mares <mj@ucw.cz>

package UCW::Configure::Pkg;
use UCW::Configure;

use strict;
use warnings;

require Exporter;
our @ISA = qw(Exporter);
our @EXPORT = qw(&TryCmd &PkgConfig &TrivConfig);

sub TryCmd($) {
	my ($cmd) = @_;
	my $res = `$cmd`;
	defined $res or return;
	chomp $res;
	return $res unless $?;
	return;
}

sub maybe_manually($) {
	my ($n) = @_;
	if (IsGiven($n)) {
		if (Get("$n")) { Log "YES (set manually)\n"; }
		else { Log "NO (set manually)\n"; }
		return 1;
	}
	return 0;
}

sub PkgConfigTool() {
	Log "Checking for pkg-config ... ";
	if (!maybe_manually("CONFIG_HAVE_PKGCONFIG")) {
		my $ver = TryCmd("pkg-config --version 2>/dev/null");
		if (!defined $ver) {
			Log("NONE\n");
			Set("CONFIG_HAVE_PKGCONFIG", 0);
		} else {
			Log("YES: version $ver\n");
			Set("CONFIG_HAVE_PKGCONFIG", 1);
			Set("CONFIG_VER_PKGCONFIG", $ver);
		}
	}
	return Get("CONFIG_HAVE_PKGCONFIG");
}

sub PkgConfig($@) {
	my $pkg = shift @_;
	my %opts = @_;
	my $upper = $pkg; $upper =~ tr/a-z/A-Z/; $upper =~ s/[^0-9A-Z]+/_/g;
	PkgConfigTool() unless IsSet("CONFIG_HAVE_PKGCONFIG");
	Log "Checking for package $pkg ... ";
	maybe_manually("CONFIG_HAVE_$upper") and return Get("CONFIG_HAVE_$upper");
	if (!Get("CONFIG_HAVE_PKGCONFIG")) {
		Log("NONE: pkg-config missing\n");
		return 0;
	}
	my $ver = TryCmd("pkg-config --modversion $pkg 2>/dev/null");
	if (!defined $ver) {
		Log("NONE\n");
		return 0;
	}
	if (defined($opts{minversion})) {
		my $min = $opts{minversion};
		if (!defined TryCmd("pkg-config --atleast-version=$min $pkg")) {
			Log("NO: version $ver is too old (need >= $min)\n");
			return 0;
		}
	}
	Log("YES: version $ver\n");
	Set("CONFIG_HAVE_$upper" => 1);
	Set("CONFIG_VER_$upper" => $ver);
	my $cf = TryCmd("pkg-config --cflags $pkg");
	Set("${upper}_CFLAGS" => $cf) if defined $cf;
	my $lf = TryCmd("pkg-config --libs $pkg");
	Set("${upper}_LIBS" => $lf) if defined $lf;
	return 1;
}

sub ver_norm($) {
	my ($v) = @_;
	return join(".", map { sprintf("%05s", $_) } split(/\./, $v));
}

sub TrivConfig($@) {
	my $pkg = shift @_;
	my %opts = @_;
	my $upper = $pkg; $upper =~ tr/a-z/A-Z/; $upper =~ s/[^0-9A-Z]+/_/g;
	Log "Checking for package $pkg ... ";
	maybe_manually("CONFIG_HAVE_$upper") and return Get("CONFIG_HAVE_$upper");
	my $pc = $opts{script};
	my $ver = TryCmd("$pc --version 2>/dev/null");
	if (!defined $ver) {
		Log("NONE\n");
		return 0;
	}
	if (defined($opts{minversion})) {
		my $min = $opts{minversion};
		if (ver_norm($ver) lt ver_norm($min)) {
			Log("NO: version $ver is too old (need >= $min)\n");
			return 0;
		}
	}
	Log("YES: version $ver\n");
	Set("CONFIG_HAVE_$upper" => 1);
	Set("CONFIG_VER_$upper" => $ver);

	my $want = $opts{want};
	defined $want or $want = ["cflags", "libs"];
	for my $w (@$want) {
		my $uw = $w; $uw =~ tr/a-z-/A-Z_/;
		my $cf = TryCmd("$pc --$w");
		Set("${upper}_${uw}" => $cf) if defined $cf;
	}
	return 1;
}
# We succeeded
1;
