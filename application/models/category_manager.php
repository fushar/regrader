<?php if ( ! defined('BASEPATH')) exit('No direct script access allowed');

/**
 * Model class that wraps table category
 *
 * This is the active record class wrapper for table 'category' from the database.
 * 
 * @package models
 * @author  Ashar Fuadi <fushar@gmail.com>
 */
class Category_manager extends AR_Model 
{
	/**
	 * Constructs a new model object
	 *
	 * This function constructs a new model.
	 */
	public function __construct()
	{
		parent::__construct('category');
	}

	/**
	 * Counts the number of users in a particular category
	 *
	 * This function return the number of users whose category ID is $category_id.
	 *
	 * @param int $category_id The category ID.
	 *
	 * @return int The number of users.
	 */
	public function count_users($category_id)
	{
		$this->db->select('COUNT(id) AS cnt');
		$this->db->from('user');
		$this->db->where('category_id', $category_id);
		$q = $this->db->get();
		$res = $q->row_array();
		return $res['cnt'];
	}

	/**
	 * Retrieves zero or more rows from table 'category' satisfying some criteria
	 *
	 * This function returns zero or more rows from the wrapped table that satisfy the given criteria. All attributes
	 * are returned.
	 *
	 * This function overrides AR_Model::get_rows(). It can also accept 'contest_id' as a criteria key; i.e., the
	 * contest IDs that this category can compete in.
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
		if (isset($criteria['contest_id']))
		{
			$this->db->join('contest_member', 'contest_member.category_id=id');
			$this->db->where('contest_member.contest_id', $criteria['contest_id']);
		}
		
		return parent::get_rows($criteria, $conditions);
	}
}

/* End of file category_manager.php */
/* Location: ./application/models/contestant/category_manager.php */