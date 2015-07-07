<?php if ( ! defined('BASEPATH')) exit('No direct script access allowed');

/**
 * Model for public file management.
 *
 * This model manages the public files that the administrator can upload and download.
 *
 * @package models
 * @author  Ashar Fuadi <fushar@gmail.com>
 */
class Public_file_manager extends CI_Model 
{
	/**
	 * Retrieves the public files
	 *
	 * This function returns all public files in the server.
	 * 
	 * @return array The public files, sorted by name.
	 */
	public function get_public_files()
	{
		$res = array();

		if ($handle = opendir('files'))
		{
			while (FALSE !== ($entry = readdir($handle)))
			{
				if ($entry == '.' || $entry == '..')
					continue;
		    	$res[] = $entry;
		    }
		}

		sort($res);
		return $res;
	}
}

/* End of file public_file_manager.php */
/* Location: ./application/model/public_file_manager.php */