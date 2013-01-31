#	Perl module for parsing Sherlock configuration files (using the config utility)
#
#	(c) 2002--2005 Martin Mares <mj@ucw.cz>
#
#	This software may be freely distributed and used according to the terms
#	of the GNU Lesser General Public License.

package UCW::Config;

use strict;
use warnings;
use Getopt::Long;

our %Sections = ();

our $DefaultConfigFile = "";
our $Usage = "-C, --config filename   Override the default configuration file
-S, --set sec.item=val  Manual setting of a configuration item";


sub Parse(@) {
	my @options = @_;
	my $defargs = "";
	my $override_config = 0;
	push @options, "config|C=s" => sub { my ($o,$a)=@_; $defargs .= " -C'$a'"; $override_config=1; };
	push @options, "set|S=s" => sub { my ($o,$a)=@_; $defargs .= " -S'$a'"; };
	Getopt::Long::Configure("bundling");
	Getopt::Long::GetOptions(@options) or return 0;
	if (!$override_config && $DefaultConfigFile) {
		$defargs = "-C'$DefaultConfigFile' $defargs";
	}
	foreach my $section (keys %Sections) {
		my $opts = $Sections{$section};
		my $optlist = join(";", keys %$opts);
		my %filtered_opts = map { my $t=$_; $t=~s/[#\$]+$//; $t => $$opts{$_} } keys %$opts;
		my @l = `bin/config $defargs "$section\{$optlist\}"`;
		$? && exit 1;
		foreach my $o (@l) {
			$o =~ /^CF_.*_([^=]+)='(.*)'\n$/ or die "Cannot parse bin/config output: $_";
			my $var = $filtered_opts{$1};
			my $val = $2;
			if (ref $var eq "SCALAR") {
				$$var = $val;
			} elsif (ref $var eq "ARRAY") {
				push @$var, $val;
			} elsif (ref $var) {
				die ("UCW::Config::Parse: don't know how to set $o");
			}
		}
	}
	1;
}

1;  # OK
