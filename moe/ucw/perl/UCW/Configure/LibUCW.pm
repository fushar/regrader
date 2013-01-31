# UCW Library configuration system: parameters of the library
# (c) 2005--2008 Martin Mares <mj@ucw.cz>
# (c) 2006 Robert Spalek <robert@ucw.cz>
# (c) 2008 Michal Vaner <vorner@ucw.cz>

package UCW::Configure::LibUCW;
use UCW::Configure;

use strict;
use warnings;

# Determine page size
Test("CPU_PAGE_SIZE", "Determining page size", sub {
	my $p;
	if (IsSet("CONFIG_DARWIN")) {
		$p = `sysctl -n hw.pagesize`;
		defined $p or Fail "sysctl hw.pagesize failed";
	} elsif (IsSet("CONFIG_LINUX")) {
		$p = `getconf PAGE_SIZE`;
		defined $p or Fail "getconf PAGE_SIZE failed";
	}
	chomp $p;
	return $p;
});

if (IsSet("CONFIG_LARGE_FILES") && IsSet("CONFIG_LINUX")) {
	# Use 64-bit versions of file functions
	Set("CONFIG_LFS");
}

# Decide how will ucw/partmap.c work
Set("CONFIG_UCW_PARTMAP_IS_MMAP") if IsSet("CPU_64BIT_POINTERS");

# Option for ucw/mempool.c
Set("CONFIG_UCW_POOL_IS_MMAP");

# Guess optimal bit width of the radix-sorter
if (Get("CPU_ARCH") eq "default" || Get("CPU_ARCH") =~ /^i[345]86$/) {
	# This should be safe everywhere
	Set("CONFIG_UCW_RADIX_SORTER_BITS" => 10);
} else {
	# Use this on modern CPU's
	Set("CONFIG_UCW_RADIX_SORTER_BITS" => 12);
}

PostConfig {
	AtWrite {
		UCW::Configure::C::ConfigHeader("ucw/autoconf.h", [
			# Excluded symbols (danger of collision)
			'^CONFIG_DEBUG$' => 0,

			# Included symbols
			'^CONFIG_' => 1,
			'^CPU_' => 1,
			'^(SHERLOCK|UCW)_VERSION(_|$)' => 1,

		]);
	} if Get("CONFIG_INSTALL_API");

	# Include direct FB?
	if (!IsSet("CONFIG_UCW_THREADS") || !IsSet("CONFIG_DIRECT_IO")) {
		if (IsGiven("CONFIG_UCW_FB_DIRECT") && IsSet("CONFIG_UCW_FB_DIRECT")) {
			if (!IsSet("CONFIG_UCW_THREADS")) {
				Fail("CONFIG_UCW_FB_DIRECT needs CONFIG_UCW_THREADS");
			} else {
				Fail("CONFIG_UCW_FB_DIRECT needs CONFIG_DIRECT_IO");
			}
		}
		UnSet("CONFIG_UCW_FB_DIRECT");
	}
};

# We succeeded
1;
