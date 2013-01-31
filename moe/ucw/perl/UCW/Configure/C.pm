# UCW Library configuration system: OS and C compiler
# (c) 2005--2008 Martin Mares <mj@ucw.cz>
# (c) 2006 Robert Spalek <robert@ucw.cz>
# (c) 2008 Michal Vaner <vorner@ucw.cz>

### OS ###

package UCW::Configure::C;
use UCW::Configure;

use strict;
use warnings;

Test("OS", "Checking on which OS we run", sub {
	my $os = `uname`;
	chomp $os;
	Fail "Unable to determine OS type" if $? || $os eq "";
	return $os;
});

if (Get("OS") eq "Linux") {
	Set("CONFIG_LINUX");
} elsif (Get("OS") eq "Darwin") {
	Set("CONFIG_DARWIN");
} else {
	Fail "Don't know how to run on this operating system.";
}

### Compiler ###

# Default compiler
Test("CC", "Checking for C compiler", sub { return "gcc"; });

# GCC version
Test("GCCVER", "Checking for GCC version", sub {
	my $gcc = Get("CC");
	my $ver = `$gcc --version | sed '2,\$d; s/^\\(.* \\)*\\([0-9]*\\.[0-9]*\\).*/\\2/'`;
	chomp $ver;
	Fail "Unable to determine GCC version" if $? || $ver eq "";
	return $ver;
});
my ($gccmaj, $gccmin) = split(/\./, Get("GCCVER"));
my $gccver = 1000*$gccmaj + $gccmin;
$gccver >= 3000 or Fail "GCC older than 3.0 doesn't support C99 well enough.";

### CPU ###

Test("ARCH", "Checking for machine architecture", sub {
	#
	# We have to ask GCC for the target architecture, because it may
	# differ from what uname tells us. This can happen even if we are
	# not cross-compiling, for example on Linux with amd64 kernel, but
	# i386 userspace.
	#
	my $gcc = Get("CC");
	my $mach = `$gcc -dumpmachine 2>/dev/null`;
	if (!$? && $mach ne "") {
		$mach =~ s/-.*//;
	} else {
		$mach = `uname -m`;
		Fail "Unable to determine machine type" if $? || $mach eq "";
	}
	chomp $mach;
	if ($mach =~ /^i[0-9]86$/) {
		return "i386";
	} elsif ($mach =~ /^(x86[_-]|amd)64$/) {
		return "amd64";
	} else {
		return "unknown ($mach)";
	}
});

sub parse_cpuinfo_linux() {
	open X, "/proc/cpuinfo" || undef;
	my %pc = ();
	while (<X>) {
		chomp;
		/^$/ && last;
		/^([^\t]+)\t+:\s*(.*)$/ and $pc{$1}=$2;
	}
	close X;
	return ($pc{'vendor_id'},
		$pc{'cpu family'},
		$pc{'model'});
}

sub parse_cpuinfo_darwin() {
	my @cpu = (`sysctl -n machdep.cpu.vendor`,
		   `sysctl -n machdep.cpu.family`,
		   `sysctl -n machdep.cpu.model`);
	chomp @cpu;
	return @cpu;
}

sub parse_cpuinfo() {
	my @cpu;
	if (IsSet("CONFIG_LINUX")) {
		@cpu = parse_cpuinfo_linux();
	} elsif (IsSet("CONFIG_DARWIN")) {
		@cpu = parse_cpuinfo_darwin();
	}
	$cpu[0] = "" if !defined $cpu[0];
	$cpu[1] = 0 if !defined $cpu[1];
	$cpu[2] = 0 if !defined $cpu[2];
	return @cpu;
}

Test("CPU_ARCH", "Checking for CPU architecture", sub {
	my $mach = Get("ARCH");
	my $arch = "";
	if ($mach eq "i386") {
		Set("CPU_I386");
		UnSet("CPU_64BIT_POINTERS");
		Set("CPU_LITTLE_ENDIAN");
		UnSet("CPU_BIG_ENDIAN");
		Set("CPU_ALLOW_UNALIGNED");
		Set("CPU_STRUCT_ALIGN" => 4);
		if (IsSet("CONFIG_EXACT_CPU")) {
			my ($vendor, $family, $model) = parse_cpuinfo();
			# Try to understand CPU vendor, family and model [inspired by MPlayer's configure script]
			if ($vendor eq "AuthenticAMD") {
				if ($family >= 6) {
					if ($model >= 31 && $gccver >= 3004) { $arch = "athlon64"; }
					elsif ($model >= 6 && $gccver >= 3003) { $arch = "athlon-xp"; }
					else { $arch = "athlon"; }
				}
			} elsif ($vendor eq "GenuineIntel") {
				if ($family >= 15 && $gccver >= 3003) {
					if ($model >= 4) { $arch = "nocona"; }
					elsif ($model >= 3) { $arch = "prescott"; }
					else { $arch = "pentium4"; }
				} elsif ($family == 6 && $gccver >= 3003) {
					if ($model == 23) { $arch = "nocona"; }
					elsif ($model == 15) { $arch = "prescott"; }
					elsif (($model == 9 || $model == 13) && $gccver >= 3004) { $arch = "pentium-m"; }
					elsif ($model >= 7) { $arch = "pentium3"; }
					elsif ($model >= 3) { $arch = "pentium2"; }
				}
			}

			# No match on vendor, try the family
			if ($arch eq "") {
				if ($family >= 6) {
					$arch = "i686";
				} elsif ($family >= 3) {
					$arch = "i${family}86";
				}
			}
			Log (($arch ne "") ? "(using /proc/cpuinfo) " : "(don't understand /proc/cpuinfo) ");
			return $arch;
		} else {
			return "default";
		}
	} elsif ($mach eq "amd64") {
		Set("CPU_AMD64");
		Set("CPU_64BIT_POINTERS");
		Set("CPU_LITTLE_ENDIAN");
		UnSet("CPU_BIG_ENDIAN");
		Set("CPU_ALLOW_UNALIGNED");
		Set("CPU_STRUCT_ALIGN" => 8);
		if (IsSet("CONFIG_EXACT_CPU")) {
			# In x86-64 world, the detection is somewhat easier so far...
			my ($vendor, $family, $model) = parse_cpuinfo();
			if ($vendor eq "AuthenticAMD") {
				$arch = "athlon64";
			} elsif ($vendor eq "GenuineIntel") {
				$arch = "nocona";
			}
			Log (($arch ne "") ? "(using /proc/cpuinfo) " : "(don't understand /proc/cpuinfo) ");
			return $arch;
		} else {
			return "default";
		}
	} else {
		return "unknown";
	}
});

if (Get("CPU_ARCH") eq "unknown") {
	Warn "CPU architecture not recognized, using defaults, keep fingers crossed.\n";
}

### Compiler and its Options ###

# C flags: tell the compiler we're speaking C99, and disable common symbols
Set("CLANG" => "-std=gnu99 -fno-common");

# C optimizations
Set("COPT" => '-O2');
if (Get("CPU_ARCH") ne "unknown" && Get("CPU_ARCH") ne "default") {
	Append("COPT", '-march=$(CPU_ARCH)');
}

# C optimizations for highly exposed code
Set("COPT2" => '-O3');

# Warnings
Set("CWARNS" => '-Wall -W -Wno-parentheses -Wstrict-prototypes -Wmissing-prototypes -Winline');
Set("CWARNS_OFF" => '');

# Linker flags
Set("LOPT" => "");

# Extra libraries
Set("LIBS" => "");

# Extra flags for compiling and linking shared libraries
Set("CSHARED" => '-fPIC');
if (IsSet("CONFIG_LOCAL")) {
	Set("SONAME_PREFIX" => "lib/");
} else {
	Set("SONAME_PREFIX" => "");
}
if (IsSet("CONFIG_DARWIN")) {
	Set("LSHARED" => '-dynamiclib -install_name $(SONAME_PREFIX)$(@F)$(SONAME_SUFFIX) -undefined dynamic_lookup');
} else {
	Set("LSHARED" => '-shared -Wl,-soname,$(SONAME_PREFIX)$(@F)$(SONAME_SUFFIX)');
}

# Extra switches depending on GCC version:
if ($gccver == 3000) {
	Append("COPT" => "-fstrict-aliasing");
} elsif ($gccver == 3003) {
	Append("CWARNS" => "-Wundef -Wredundant-decls");
	Append("COPT" => "-finline-limit=20000 --param max-inline-insns-auto=1000");
} elsif ($gccver == 3004) {
	Append("CWARNS" => "-Wundef -Wredundant-decls");
	Append("COPT" => "-finline-limit=2000 --param large-function-insns=5000 --param inline-unit-growth=200 --param large-function-growth=400");
} elsif ($gccver >= 4000) {
	Append("CWARNS" => "-Wundef -Wredundant-decls -Wno-pointer-sign -Wdisabled-optimization -Wno-missing-field-initializers");
	Append("CWARNS_OFF" => "-Wno-pointer-sign");
	Append("COPT" => "-finline-limit=5000 --param large-function-insns=5000 --param inline-unit-growth=200 --param large-function-growth=400");
	if ($gccver >= 4002) {
		Append("COPT" => "-fgnu89-inline");
	}
} else {
	Warn "Don't know anything about this GCC version, using default switches.\n";
}

if (IsSet("CONFIG_DEBUG")) {
	# If debugging:
	Set("DEBUG_ASSERTS");
	Set("DEBUG_DIE_BY_ABORT") if Get("CONFIG_DEBUG") > 1;
	Set("CDEBUG" => "-ggdb");
} else {
	# If building a release version:
	Append("COPT" => "-fomit-frame-pointer");
	Append("LOPT" => "-s");
}

if (IsSet("CONFIG_DARWIN")) {
	# gcc-4.0 on Darwin doesn't set this in the gnu99 mode
	Append("CLANG" => "-fnested-functions");
	# Directory hierarchy of the fink project
	Append("LIBS" => "-L/sw/lib");
	Append("COPT" => "-I/sw/include");
	# Fill in some constants not found in the system header files
	Set("SOL_TCP" => 6);		# missing in /usr/include/netinet/tcp.h
	if (IsGiven("CONFIG_DIRECT_IO") && IsSet("CONFIG_DIRECT_IO")) {
		Fail("Direct I/O is not available on darwin");
	} else {
		UnSet("CONFIG_DIRECT_IO");
	}
	if (!IsSet("CONFIG_POSIX_REGEX") && !IsSet("CONFIG_PCRE")) {
		Set("CONFIG_POSIX_REGEX" => 1);
		Warn "BSD regex library on Darwin isn't compatible, using POSIX regex.\n";
	}
}

### Writing C headers with configuration ###

sub ConfigHeader($$) {
	my ($hdr, $rules) = @_;
	Log "Generating $hdr ... ";
	open X, ">obj/$hdr" or Fail $!;
	print X "/* Generated automatically by $0, please don't touch manually. */\n";

	sub match_rules($$) {
		my ($rules, $name) = @_;
		for (my $i=0; $i < scalar @$rules; $i++) {
			my ($r, $v) = ($rules->[$i], $rules->[$i+1]);
			return $v if $name =~ $r;
		}
		return 0;
	}

	foreach my $x (sort keys %UCW::Configure::vars) {
		next unless match_rules($rules, $x);
		my $v = $UCW::Configure::vars{$x};
		# Try to add quotes if necessary
		$v = '"' . $v . '"' unless ($v =~ /^"/ || $v =~ /^\d*$/);
		print X "#define $x $v\n";
	}
	close X;
	Log "done\n";
}

AtWrite {
	ConfigHeader("autoconf.h", [
		# Symbols with "_" anywhere in their name are exported
		"_" => 1
	]);
};

# Return success
1;
