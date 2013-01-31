#	Perl module for manipulating Sherlock objects
#
#	(c) 2007 Martin Mares <mj@ucw.cz>
#
#	This software may be freely distributed and used according to the terms
#	of the GNU Lesser General Public License.

=head1 NAME

Sherlock::Object -- Manipulation with Sherlock objects

=head1 DESCRIPTION

This module offers a simple interface to Sherlock objects. See F<doc/objects>
for a description of how the object system works.

=head1 METHODS

=over

=item B<new()>

Creates a new empty object.

=item B<< new(I<name> => I<value>, ...) >>

Creates a new object with some attributes initialized.

=item B<< set(I<name> => I<value>, ...) >>

Sets given attributes to specified values. If any of the attributes already
exists, the old value is replaced by the new one.

The value can be:

=over

=item *

a number

=item *

a string

=item *

a reference to an array of values, which creates a multi-valued attribute.

=item *

a reference to another object, which created a nested object. In this case
(and only in this case), the name of the attribute must start with C<(>.

=back

=item B<< unset(I<name>, ...) >>

Removes given attributes.

=item B<< add(I<name> => I<value>, ...) >>

Sets given attributes to specified values. If any of the attributes already
exists, the new value is added to the original one, creating a multi-valued
attribute.

The repertoir of values is the same as in the C<set> method except for references
to arrays, which are not allowed.

=item B<< get(I<name>) >>

Gets the value of the given attribute or C<undef> if it does not exist.
If the attribute is multi-valued, the first value is returned.

=item B<< getarray(I<name>) >>

Gets all values of the given attribute as an array or an empty array if the
attribute does not exist.

=item B<< get_attrs() >>

Get an array of names of all attributes present in the object.

=item B<< read(I<handle>) >>

Reads a textual representation of an object from the given handle and adds it
to the object it is invoked on. The effect is the same as calling the C<add>
method on all read attributes.

Returns 1 if the object has been read successfully, 0 if the input
stream ended before an object started or C<undef> if the input was
malformed.

=item B<< read(I<handle>, raw => 1) >>

Reads an object as above, but add a special attribute called C<< RAW >>, which will
contain the raw form of the object.

=item B<< write(I<handle>) >>

Writes a textual representation of the object to the given handle.

=item B<< write_indented(I<handle>, [I<base_indent>]) >>

Writes an indented textual representation of the object to the given handle.
The I<base_indent> will be prepended to all printed lines, each nested level
will get one tab character more.

This is intended for debugging dumps and the output cannot be read back by any
of the Sherlock libraries.

=back

=head1 AUTHOR

Martin Mares <mj@ucw.cz>

=cut

package Sherlock::Object;

use strict;
use warnings;

sub new($@) {
	my $self = { };
	bless $self;
	shift @_;
	if (@_) {
		$self->set(@_);
	}
	return $self;
}

sub set($@) {
	my $self = shift @_;
	my $attr;
	while (defined($attr = shift @_)) {
		$self->{$attr} = shift @_;
	}
}

sub unset($@) {
	my $self = shift @_;
	foreach my $attr (@_) {
		delete $self->{$attr};
	}
}

sub add($@) {
	my $self = shift @_;
	my $attr;
	while (defined($attr = shift @_)) {
		my $val = shift @_;
		if (!exists $self->{$attr}) {
			$self->{$attr} = $val;
		} elsif (ref $self->{$attr} eq "ARRAY") {
			push @{$self->{$attr}}, $val;
		} else {
			$self->{$attr} = [ $self->{$attr}, $val ];
		}
	}
}

sub get($$) {
	my ($self, $attr) = @_;
	if (!exists $self->{$attr}) {
		return undef;
	} elsif (ref $self->{$attr} eq "ARRAY") {
		return $self->{$attr}->[0];
	} else {
		return $self->{$attr};
	}
}

sub getarray($$) {
	my ($self, $attr) = @_;
	if (!exists $self->{$attr}) {
		return ();
	} elsif (ref $self->{$attr} eq "ARRAY") {
		return @{$self->{$attr}};
	} else {
		return ( $self->{$attr} );
	}
}

sub get_attrs($) {
	my ($self) = @_;
	return keys %$self;
}

sub read($$@) {
	my $self = shift @_;
	my $fh = shift @_;
	my %opts = @_;
	my @stack = ();
	my $read_something = 0;
	my $obj = $self;
	my $raw;
	my $read = $opts{read} ? $opts{read} : sub { my $fh = shift; return $_ = <$fh>; };
	if ($opts{raw}) {
		$raw = $obj->{"RAW"} = [];
	}
	while ($read->($fh)) {
		chomp;
		/^$/ && last;
		my ($a, $v) = /^(.)(.*)$/ or return undef;
		push @$raw, $_ if $raw;
		if ($a eq "(") {
			$a = "$a$v";
			my $new = new Sherlock::Object;
			$obj->add($a, $new);
			push @stack, $obj;
			$obj = $new;
		} elsif ($a eq ")") {
			@stack or return undef;
			$obj = pop @stack;
		} else {
			$obj->add($a, $v);
		}
		$read_something = 1;
	}
	@stack and return undef;
	return $read_something;
}

sub write($$) {
	my ($self, $fh) = @_;
	foreach my $a (keys %$self) {
		my $vals = $self->{$a};
		ref $vals eq "ARRAY" or $vals = [$vals];
		foreach my $v (@{$vals}) {
			if (ref $v eq "") {
				print $fh $a, $v, "\n";
			} elsif (ref $v eq "Sherlock::Object") {
				print $fh $a, "\n";
				$v->write($fh);
				print $fh ")\n";
			} else {
				die;
			}
		}
	}
}

sub write_indented($$$) {
	my ($self, $fh, $indent) = @_;
	defined $indent or $indent = "";
	foreach my $a (sort keys %$self) {
		my $vals = $self->{$a};
		ref $vals eq "ARRAY" or $vals = [$vals];
		foreach my $v (@{$vals}) {
			if (ref $v eq "") {
				print $fh $indent, $a, $v, "\n";
			} elsif (ref $v eq "Sherlock::Object") {
				print $fh $indent, $a, "\n";
				$v->write_indented($fh, $indent . "\t");
				print $fh $indent, ")\n";
			} else {
				die;
			}
		}
	}
}

1;  # OK
