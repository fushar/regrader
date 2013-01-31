<?php if ( ! defined('BASEPATH')) exit('No direct script access allowed');

/**
 * Model class that wraps table contest
 *
 * This is the active record class wrapper for table 'contest' from the database.
 * 
 * @package models
 * @author  Ashar Fuadi <fushar@gmail.com>
 */
class Contest_manager extends AR_Model 
{
	/**
	 * Constructs a new model object
	 *
	 * This function constructs a new model.
	 */
	public function __construct()
	{
		parent::__construct('contest');
	}

	/**
	 * Retrieves the active contests for users in a particular category
	 *
	 * This function returns the currently active contests for users in category whose ID is $category_id; i.e., 
	 * contests whose starting times are before now.
	 * 
	 * @param int $category_id The category ID.
	 * 
	 * @return array A list of the currently running contests as an array of <contest-ID> => array of
	 *               (<contest-ID>, <contest-name>).
	 */
	public function get_active_contests($category_id)
	{
		$this->db->select('id, name, end_time');
		$this->db->from('contest');
		$this->db->join('contest_member', 'contest_member.contest_id=contest.id', 'left outer');
		if ($category_id != 0)
			$this->db->where('contest_member.category_id', $category_id);
		$this->db->where('start_time <=', date('Y-m-d H:i:s'));
		$this->db->where('enabled', true);
		$q = $this->db->get();
		
		$res = array();
		foreach ($q->result_array() as $v)
			$res[$v['id']] = $v;
		return $res;
	}

	/**
	 * Checks whether a problem is enabled and assigned to a contest
	 *
	 * This function checks whether problem whose ID is $problem_id is enabled and assigned to contest whose ID is
	 * $contest_id.
	 * 
	 * @param  int  $contest_id The contest ID.
	 * @param  int  $problem_id The problem ID.
	 * 
	 * @return bool             TRUE if problem whose ID is $problem_id is enabled and assigned to contest whose ID is
	 *                          $contest_id, or FALSE otherwise.
	 */
	public function has_problem($contest_id, $problem_id)
	{
		$this->db->from('contest_problem');
		$this->db->where('contest_id', $contest_id);
		$this->db->where('problem_id', $problem_id);
		$this->db->limit(1);
		$q = $this->db->get();
		return $q->num_rows() == 1;
	}

	/**
	 * Update the members of a particular contest
	 *
	 * This function sets the categories in $cats as the only members of the contest whose ID is $contest_id.
	 * 
	 * @param  int  	$contest_id The contest ID.
	 * @param  array  	$cats 		The categories.
	 */
	public function update_members($contest_id, $cats)
	{
		$this->db->where('contest_id', $contest_id);
		$this->db->delete('contest_member');

		foreach ($cats as $k)
		{
			$this->db->set('contest_id', $contest_id);
			$this->db->set('category_id', $k);
			$this->db->insert('contest_member');
		}
	}

	/**
	 * Update the languages of a particular contest
	 *
	 * This function sets the languages in $cats as the only languages of the contest whose ID is $contest_id.
	 * 
	 * @param  int  	$contest_id The contest ID.
	 * @param  array  	$langs 		The languages.
	 */
	public function update_languages($contest_id, $langs)
	{
		$this->db->where('contest_id', $contest_id);
		$this->db->delete('contest_language');

		foreach ($langs as $k)
		{
			$this->db->set('contest_id', $contest_id);
			$this->db->set('language_id', $k);
			$this->db->insert('contest_language');
		}
	}

	/**
	 * Adds a problem to a particular contest
	 *
	 * This function adds the problem whose ID is $args['problem_id'] with alias $args['alias'] to the contest whose ID
	 * is $args['contest_id'].
	 * 
	 * @param array $args The parameters.
	 */
	public function add_problem($args)
	{
		if ($args['problem_id'] == 0)
			return;
		$this->db->set('contest_id', $args['contest_id']);
		$this->db->set('problem_id', $args['problem_id']);
		$this->db->set('alias', $args['alias']);
		$this->db->insert('contest_problem');
	}

	/**
	 * Deletes a problem from a particular contest
	 *
	 * This function removes the problem whose ID is $problem_id from the contest whose ID is $contest_id.
	 * 
	 * @param int $contest_id	The contest ID.
	 * @param int $problem_id	The problem ID.
	 */
	public function delete_problem($contest_id, $problem_id)
	{
		$this->db->where('contest_id', $contest_id);
		$this->db->where('problem_id', $problem_id);
		$this->db->delete('contest_problem');
	}
}

/* End of file contest_manager.php */
/* Location: ./application/models/contestant/contest_manager.php */