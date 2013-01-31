# UCW Library configuration system: find UCW build system
# (c) 2008 Michal Vaner <vorner@ucw.cz>

# This module asks pkg-config for a path to UCW build system
# and sets propper variables for it (or fails, as it is expected
# the build system is crucial).

package UCW::Configure::Build;
use UCW::Configure;

use strict;
use warnings;

if (!IsGiven("BUILDSYS")) {
	Test("BUILDSYS", "Looking for UCW build system", sub {
		my $path=`pkg-config libucw --variable=build_system`;
		if($? || not defined $path) {
			Fail("Not found (is libUCW installed and PKG_CONFIG_PATH set?)");
		}
		chomp $path;
		return $path;
	});
}

# We succeeded
1;
