# UCW Library configuration system: documentation requirements
# (c) 2008 Michal Vaner <vorner@ucw.cz>

package UCW::Configure::Doc;
use UCW::Configure;

use strict;
use warnings;

if (!IsGiven("CONFIG_DOC") || IsSet("CONFIG_DOC")) {
	Test("HAVE_ASCII_DOC", "Checking for AsciiDoc", sub {
		my $version = `asciidoc --version 2>&1`;
		return "none" if !defined $version || $version eq "";
		my( $vnum ) = $version =~ / (\d+\.\S*)$/;
		return $vnum;
	});

	my( $major ) = Get("HAVE_ASCII_DOC") =~ /^(\d+)/;
	if (defined $major && $major >= 7) {
		Set("CONFIG_DOC");
	} else {
		if (IsGiven("CONFIG_DOC")) {
			Fail("Need asciidoc >= 7");
		} else {
			Warn("Need asciidoc >= 7 to build documentation");
			UnSet("CONFIG_DOC");
		}
	}
}

# We succeeded
1;
