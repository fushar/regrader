<?php  if ( ! defined('BASEPATH')) exit('No direct script access allowed');
/*
| -------------------------------------------------------------------------
| Hooks
| -------------------------------------------------------------------------
| This file lets you define "hooks" to extend CI without hacking the core
| files.  Please see the user guide for info:
|
|	http://codeigniter.com/user_guide/general/hooks.html
|
*/

$hook['pre_system'] = function () {
	$dotenv = new Dotenv\Dotenv(FCPATH);
	$dotenv->load();
	foreach ($_ENV as $k => $v) {
		if (preg_match('/TABLE_NAME$/', $k)) {
			$_ENV[$k] = $_ENV['DB_PREFIX'].$_ENV[$k];
		}
	}
};

/* End of file hooks.php */
/* Location: ./application/config/hooks.php */