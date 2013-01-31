# UCW Library configuration system: installation paths
# (c) 2005--2009 Martin Mares <mj@ucw.cz>
# (c) 2006 Robert Spalek <robert@ucw.cz>
# (c) 2008 Michal Vaner <vorner@ucw.cz>

package UCW::Configure::Paths;
use UCW::Configure;

use strict;
use warnings;

Log "Determining installation prefix ... ";
if (IsSet("CONFIG_LOCAL")) {
	Log("local build\n");
	Set("INSTALL_PREFIX", "");
	Set("INSTALL_USR_PREFIX", "");
	Set("INSTALL_VAR_PREFIX", "");
} else {
	Set("PREFIX", "/usr/local") unless IsSet("PREFIX");
	my $ipx = Get("PREFIX");
	$ipx =~ s{/$}{};
	Set("INSTALL_PREFIX", "$ipx/");
	my $upx = ($ipx eq "" ? "/usr/" : "$ipx/");
	Set("INSTALL_USR_PREFIX", $upx);
	$upx =~ s{^/usr\b}{/var};
	Set("INSTALL_VAR_PREFIX", $upx);
	Log(Get("PREFIX") . "\n");
}

Set('CONFIG_DIR', 'etc') unless IsSet('CONFIG_DIR');
Set('INSTALL_CONFIG_DIR', Get('INSTALL_PREFIX') . Get('CONFIG_DIR'));
Set('INSTALL_BIN_DIR', Get('INSTALL_USR_PREFIX') . 'bin');
Set('INSTALL_SBIN_DIR', Get('INSTALL_USR_PREFIX') . 'sbin');
Set('INSTALL_LIB_DIR', Get('INSTALL_USR_PREFIX') . 'lib');
Set('INSTALL_INCLUDE_DIR', Get('INSTALL_USR_PREFIX') . 'include');
Set('INSTALL_PKGCONFIG_DIR', Get('INSTALL_USR_PREFIX') . 'lib/pkgconfig');
Set('INSTALL_SHARE_DIR', Get('INSTALL_USR_PREFIX') . 'share');
Set('INSTALL_MAN_DIR', Get('INSTALL_USR_PREFIX') . 'share/man');
Set('INSTALL_LOG_DIR', Get('INSTALL_VAR_PREFIX') . 'log');
Set('INSTALL_STATE_DIR', Get('INSTALL_VAR_PREFIX') . 'lib');
Set('INSTALL_RUN_DIR', Get('INSTALL_VAR_PREFIX') . 'run');
Set('INSTALL_DOC_DIR', Get('INSTALL_USR_PREFIX') . 'share/doc');
Set('INSTALL_PERL_DIR', Get('INSTALL_LIB_DIR') . '/perl5');

# Remember PKG_CONFIG_PATH used for building, so that it will be propagated to
# pkg-config's run locally in the makefiles.
Set("PKG_CONFIG_PATH", $ENV{"PKG_CONFIG_PATH"}) if defined $ENV{"PKG_CONFIG_PATH"};

# We succeeded
1;
