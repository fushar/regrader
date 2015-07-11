<?php if ( ! defined('BASEPATH')) exit('No direct script access allowed');

/**
 * Model class that wraps table user
 *
 * This is the active record class wrapper for table 'user' from the database.
 * 
 * @package models
 * @author  Ashar Fuadi <fushar@gmail.com>
 */
class User_manager extends AR_Model
{
	/**
	 * Constructs a new model object
	 *
	 * This function constructs a new model.
	 */
	public function __construct()
	{
		parent::__construct('user');
	}

	/**
	 * Retrieves zero or more rows from table 'problem' satisfying some criteria
	 *
	 * This function returns zero or more rows from the wrapped table that satisfy the given criteria. All attributes
	 * are returned.
	 *
	 * This function overrides AR_Model::get_rows().
	 * 
	 * @param  array $criteria   An array of <attribute-name> => <attribute-value> pairs. Only rows that satisfy all
	 *                           criteria will be retrieved.
	 * @param  array $conditions An array of zero or more of the following pairs:
	 *                           - 'limit'    => The maximum number of rows to be retrieved.
	 *                           - 'offset'   => The starting number of the rows to be retrieved.
	 *                           - 'order_by' => The resulting rows order as an array of zero or more
	 *                                           <attribute-name> => ('ASC' | 'DESC') pairs.
	 * 
	 * @return array             Zero or more rows that satisfies the criteria as an array of <row-ID> => <row>.
	 */
	public function get_rows($criteria = array(), $conditions = array())
	{
		$this->db->select('user.id, user.name, username, password, institution, category_id, category.name AS category_name, last_activity');
		$this->db->join('category', 'category.id=category_id');

		return parent::get_rows($criteria, $conditions);
	}

	/**
	 * Retrieves the active users
	 *
	 * This function returns the active users; i.e., users that has last accessed the site at most half an hour ago.
	 * 
	 * @return array The active users.
	 */
	public function get_active_users()
	{
		$q = $this->db->query('SELECT name FROM user WHERE TIMESTAMPDIFF(SECOND, last_activity, NOW()) <= 60 * 30 AND id != 1 ORDER BY category_id, name');
		return $q->result_array();
	}

	/**
	 * Checks whether a username is valid
	 *
	 * This function returns whether $username has been used before for user other than $user_id.
	 * 
	 * @param int $user_id		The user ID.
	 * @param string $username 	The username.
	 *
	 * @return boolean TRUE if $username has not been used before, or FALSE otherwise.
	 */
	public function is_valid_new_username($user_id, $username)
	{
		$this->db->select('id');
		$this->db->from('user');
		$this->db->where('username', $username);
		$this->db->where('id !=', $user_id);
		$q = $this->db->get();
		return $q->num_rows() == 0;
	}
}

/* End of file user_manager.php */
/* Location: ./application/models/contestant/user_manager.php */