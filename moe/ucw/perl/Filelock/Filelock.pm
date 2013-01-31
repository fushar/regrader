# Perl module for setting process limits
#
# (c) 2007 Pavel Charvat <pchar@ucw.cz>
#
# This software may be freely distributed and used according to the terms
# of the GNU Lesser General Public License.
#
#
#
# Interface:
#   UCW::Filelock::fcntl_lock($fd, $cmd, $type, $whence, $start, $len)
#

package UCW::Filelock;

use 5.006;
use strict;
use warnings;

require DynaLoader;

our @ISA = qw(DynaLoader);
unshift @DynaLoader::dl_library_path, "lib";

our $VERSION = '0.01';

bootstrap UCW::Filelock $VERSION;

# Preloaded methods go here.

1;
__END__
