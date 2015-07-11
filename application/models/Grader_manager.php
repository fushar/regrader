<?php if ( ! defined('BASEPATH')) exit('No direct script access allowed');

/**
 * Model class that wraps table grader
 *
 * This is the active record class wrapper for table 'grader' from the database.
 * 
 * @package models
 * @author  Ashar Fuadi <fushar@gmail.com>
 */
class Grader_manager extends AR_Model 
{
	/**
	 * Constructs a new model object
	 *
	 * This function constructs a new model.
	 */
	public function __construct()
	{
		parent::__construct('grader');
	}

	/**
	 * Retrieves the currently active graders
	 *
	 * This function returns the currently running graders; i.e., the graders that checked it at most 30 seconds ago.
	 *
	 * @return array The currently running graders.
	 */
	public function get_active_graders()
	{
		$q = $this->db->query('SELECT hostname FROM grader WHERE TIMESTAMPDIFF(SECOND, last_activity, NOW()) <= 30');
		return $q->result_array();
	}
}

/* End of file grader_manager.php */
/* Location: ./application/models/contestant/grader_manager.php */