# A Perl module for communicating with the MO Submit Server
# (c) 2007 Martin Mares <mj@ucw.cz>

package MO::Submit;

use strict;
use warnings;

use IO::Socket::INET;
use IO::Socket::SSL; # qw(debug3);
use Sherlock::Object;
use POSIX;

sub new($) {
	my $user = $ENV{"USER"} or die "Environment variable USER not set\n";
	my $home = $ENV{"HOME"} or die "Environment variable HOME not set\n";
	my $mo = "$home/.mo";
	my $root = $ENV{"MO_ROOT"} or die "Environment variable MO_ROOT not set\n";
	my $self = {
		"Contest" => "CEOI 2007",
		"Server" => "ceoi-gamma:8888",
		"Key" => "$mo/key.pem",		# Keys and certificates
		"Cert" => "$mo/cert.pem",
		"CACert" => "$mo/ca-cert.pem",
		"Trace" => defined $ENV{"MO_SUBMIT_TRACE"},
		"Checks" => 1,			# Run `check' before submitting
		"AllowOverride" => 1,		# Allow overriding a failed check
		"History" => "$home/.history",	# Keep submission history in this directory
		"RefreshTimer" => 60000,	# How often GUI sends STATUS commands [ms]
		"root" => $root,
		"user" => $user,
		"sk" => undef,
		"error" => undef,
	};
	return bless $self;
}

sub DESTROY($) {
	my $self = shift @_;
	$self->disconnect;
}

sub log($$) {
	my ($self, $msg) = @_;
	print STDERR "SUBMIT: $msg\n" if $self->{"Trace"};
}

sub err($$) {
	my ($self, $msg) = @_;
	print STDERR "ERROR: $msg\n" if $self->{"Trace"};
	$self->{"error"} = $msg;
	$self->disconnect;
}

sub is_connected($) {
	my $self = shift @_;
	return defined $self->{"sk"};
}

sub disconnect($) {
	my $self = shift @_;
	if ($self->is_connected) {
		close $self->{"sk"};
		$self->{"sk"} = undef;
		$self->log("Disconnected");
	}
}

sub connect($) {
	my $self = shift @_;
	$self->disconnect;

	$self->log("Connecting to submit server");
	my $sk = new IO::Socket::INET(
		PeerAddr => $self->{"Server"},
		Proto => "tcp",
	);
	if (!defined $sk) {
		$self->err("Cannot connect to server: $!");
		return undef;
	}
	my $z = <$sk>;
	if (!defined $z) {
		$self->err("Server failed to send a welcome message");
		close $sk;
		return undef;
	}
	chomp $z;
	if ($z !~ /^\+/) {
		$self->err("Server rejected the connection: $z");
		close $sk;
		return undef;
	}
	if ($z =~ /TLS/) {
		$self->log("Starting TLS");
		$sk = IO::Socket::SSL->start_SSL(
			$sk,
			SSL_version => 'TLSv1',
			SSL_use_cert => 1,
			SSL_key_file => $self->{"Key"},
			SSL_cert_file => $self->{"Cert"},
			SSL_ca_file => $self->{"CACert"},
			SSL_verify_mode => 3,
		);
		if (!defined $sk) {
			$self->err("Cannot establish TLS connection: " . IO::Socket::SSL::errstr());
			return undef;
		}
	}
	$self->{"sk"} = $sk;
	$sk->autoflush(0);

	$self->log("Logging in");
	my $req = new Sherlock::Object("U" => $self->{"user"});
	my $reply = $self->request($req);
	my $err = $reply->get("-");
	if (defined $err) {
		$self->err("Cannot log in: $err");
		return undef;
	}

	$self->log("Connected");
	return 1;
}

sub request($$) {
	my ($self, $obj) = @_;
	my $sk = $self->{"sk"};
	local $SIG{'PIPE'} = 'ignore';
	$obj->write($sk);
	print $sk "\n";
	$sk->flush();
	if ($sk->error) {
		$self->err("Connection broken");
		return undef;
	}
	return $self->reply;
}

sub reply($) {
	my ($self, $obj) = @_;
	my $sk = $self->{"sk"};
	my $reply = new Sherlock::Object;
	if ($reply->read($sk)) {
		return $reply;
	} else {
		$self->err("Connection broken");
		return undef;
	}
}

sub send_file($$$) {
	my ($self, $fh, $size) = @_;
	my $sk = $self->{"sk"};
	local $SIG{'PIPE'} = 'ignore';
	while ($size) {
		my $l = ($size < 4096 ? $size : 4096);
		my $buf = "";
		if ($fh->read($buf, $l) != $l) {
			$self->err("File shrunk during upload");
			return undef;
		}
		$sk->write($buf, $l);
		if ($sk->error) {
			$self->err("Connection broken");
			return undef;
		}
		$size -= $l;
	}
	return $self->reply;
}

sub write_history($$$$$) {
	my ($self, $task, $part, $ext, $filename) = @_;
	my $hist = $self->{"History"};
	-d $hist or mkdir $hist or return "Unable to create $hist: $!";
	my $now = POSIX::strftime("%H:%M:%S", localtime(time));
	my $maybe_part = ($part eq $task) ? "" : ":$part";
	my $name = "$hist/$now-$task$maybe_part.$ext";
	$self->log("Backing up to $name");
	`cp "$filename" "$name"`;
	return "Unable to back up $filename as $name" if $?;
	return undef;
}

1;
