<?php if ( ! defined('BASEPATH')) exit('No direct script access allowed');

/**
 * Model class that wraps table clarification
 *
 * This is the active record class wrapper for table 'clarification' from the database.
 * 
 * @package models
 * @author  Ashar Fuadi <fushar@gmail.com>
 */
class Clarification_manager extends AR_Model 
{
	/**
	 * Constructs a new model object
	 *
	 * This function constructs a new model.
	 */
	public function __construct()
	{
		parent::__construct('clarification');
	}

	/**
	 * Broadcast a particular administrator clarification in a particular contest
	 *
	 * This function marks the clarification whose ID is $clarification_id as unread for all users competing in contest
	 * whose ID is $contest_id. The clarification is regarded as an administrator broadcast.
	 * 
	 * @param int @clarificaition_id	The clarification ID.
	 * @param int @contest_id			The contest ID.
	 */
	public function broadcast($clarification_id, $contest_id)
	{
		$this->db->select('user.id AS user_id');
		$this->db->from('user');
		$this->db->join('contest_member', 'contest_member.category_id=user.category_id');
		$this->db->where('contest_member.contest_id', $contest_id);

		$q = $this->db->get();
		foreach ($q->result_array() as $v)
		{
			$this->db->set('clarification_id', $clarification_id);
			$this->db->set('user_id', $v['user_id']);
			$this->db->insert('unread_clarification');
		}
	}

	/**
	 * Notifies the clarifier that the clarification has been answered or updated
	 *
	 * This function marks the clarification whose ID is $clarification_id as unread to the sender of the clarification,
	 * usually when the clarification has been asnwered or updated.
	 * 
	 * @param int @clarificaition_id	The clarification ID.
	 */
	public function notify($clarification_id)
	{
		$clarification = $this->get_row($clarification_id);

		$this->db->set('clarification_id', $clarification_id);
		$this->db->set('user_id', $clarification['user_id']);
		$this->db->insert('unread_clarification');
	}

	/**
	 * Asks a particular clarification to the administrator
	 *
	 * This function marks the clarification whose ID is $clarification_id as unread to the administrator.
	 * 
	 * @param int @clarification_id	The clarification ID.
	 */
	public function ask($clarification_id)
	{
		$clarification = $this->get_row($clarification_id);

		$this->db->set('clarification_id', $clarification_id);
		$this->db->set('user_id', 1);
		$this->db->insert('unread_clarification');
	}

	/**
	 * Count the number of unread clarifications of a particular user in a particular contest
	 *
	 * This function returns the number of unread clarifications of the user whose ID is $user_id in the contest whose
	 * ID is $contest_id.
	 * 
	 * @param int @clarificaition_id	The clarification ID.
	 * @param int @contest_id			The contest ID.
	 * 
	 * @return int The number of unread clarifications.
	 */
	public function count_contestant_unread($user_id, $contest_id)
	{
		$this->db->select('COUNT(*) AS cnt');
		$this->db->from('unread_clarification');
		$this->db->join('clarification', 'clarification.id=clarification_id');
		$this->db->where('unread_clarification.user_id', $user_id);
		$this->db->where('clarification.contest_id', $contest_id);
		$q = $this->db->get();
		$res = $q->row_array();		
		return $res['cnt'];
	}

	/**
	 * Count the number of unread clarifications of the administrator
	 *
	 * This function returns the number of unread clarifications of the administrator.
	 * 
	 * @return int The number of unread clarifications.
	 */
	public function count_admin_unread()
	{
		$this->db->select('COUNT(*) AS cnt');
		$this->db->from('unread_clarification');
		$this->db->join('clarification', 'clarification.id=clarification_id');
		$this->db->where('unread_clarification.user_id', 1);
		$q = $this->db->get();
		$res = $q->row_array();
		return $res['cnt'];
	}

	/**
	 * Retrieves the unread clarifications of a particular user in a particular contest
	 *
	 * This function returns the unread clarifications of the user whose ID is $user_id in the contest whose ID is
	 * $contest_id.
	 * 
	 * @param int @clarificaition_id	The clarification ID.
	 * @param int @contest_id			The contest ID.
	 * 
	 * @return array The unread clarifications.
	 */
	public function get_contestant_unread($user_id, $contest_id)
	{
		$this->db->select('unread_clarification.clarification_id AS id');
		$this->db->from('unread_clarification');
		$this->db->join('clarification', 'clarification.id=clarification_id');
		$this->db->where('unread_clarification.user_id', $user_id);
		$this->db->where('clarification.contest_id', $contest_id);
		$q = $this->db->get();
		return $q->result_array();
	}

	/**
	 * Retrieves the unread clarifications of the administrator
	 *
	 * This function returns the unread clarifications of the administrator.
	 * 
	 * @return array The unread clarifications.
	 */
	public function get_admin_unread()
	{
		$this->db->select('unread_clarification.clarification_id AS id');
		$this->db->from('unread_clarification');
		$this->db->join('clarification', 'clarification.id=clarification_id');
		$this->db->where('unread_clarification.user_id', 1);
		$q = $this->db->get();
		return $q->result_array();
	}

	/**
	 * Marks a particular clarification as read to a particular user
	 *
	 * This function marks the clarification whose ID is $clarification_id as read to the user whose ID is $user_id. 
	 * 
	 * @param int @user_id				The user  ID.
	 * @param int @clarification_id		The clarification ID.
	 */
	public function delete_unread($user_id, $clarification_id)
	{
		$this->db->where('user_id', $user_id);
		$this->db->where('clarification_id', $clarification_id);
		$this->db->delete('unread_clarification');
	}

	/**
	 * Retrieves an existing row from table 'clarification'
	 *
	 * This function returns a single row whose ID is $id from the wrapped table. If there is no such row, FALSE will be
	 * returned instead.
	 *
	 * This function overrides AR_Model::get_row().
	 * 
	 * @param  int $clarification_id The row ID to be retrieved.
	 * 
	 * @return mixed            	 The row whose ID is $clarification_id as an array of <attribute-name> => 
	 * 							 	 <attribute-value> pairs, or FALSE if there is no such row.
	 */
	public function get_row($clarification_id)
	{
		$this->db->select('clarification.id AS id, user_id, clarification.contest_id, clar_time, title, content, answer, user.name AS user_name, contest.name AS contest_name');
		$this->db->from('clarification');
		$this->db->join('contest', 'contest.id=contest_id');
		$this->db->join('user', 'user.id=user_id', 'left outer');
		$this->db->where('clarification.id', $clarification_id);
		$this->db->limit(1);
		$q = $this->db->get();
		if ($q->num_rows() == 0)
			return FALSE;
		return $q->row_array();
	}

	/**
	 * Retrieves zero or more rows from table 'clarification' satisfying some criteria
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
		$this->db->select('clarification.id AS id, user_id, clarification.contest_id, clar_time, title, (answer IS NOT NULL) AS answered, user.name AS user_name, contest.name AS contest_name');
		
		$this->db->join('contest', 'contest.id=contest_id');
		$this->db->join('user', 'user.id=user_id', 'left outer');
		
		if (isset($criteria['clarification.user_id']))
		{
			$this->db->where('user_id', $criteria['clarification.user_id']);
			unset($criteria['clarification.user_id']);
		}

		return parent::get_rows($criteria, $conditions);
	}

	/**
	 * Retrieves zero or more rows from table 'clarification' satisfying some criteria
	 *
	 * This function returns zero or more rows from the wrapped table that satisfy the given criteria. All attributes
	 * are returned.
	 *
	 * If there is 'clarification.user_id' in $criteria, the rows with user_id=1 will be also returned.
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
	public function get_rows_with_admin($criteria = array(), $conditions = array())
	{
		$this->db->select('clarification.id AS id, user_id, clarification.contest_id, clar_time, title, (answer IS NOT NULL) AS answered, user.name AS user_name, contest.name AS contest_name');
		$this->db->join('contest', 'contest.id=contest_id');
		$this->db->join('user', 'user.id=user_id', 'left outer');
		
		if (isset($criteria['clarification.user_id']))
		{
			$this->db->where('(user_id=1 OR user_id=' . $criteria['clarification.user_id'] . ')');
			unset($criteria['clarification.user_id']);
		}

		return parent::get_rows($criteria, $conditions);
	}

	/**
	 * Counts the number of rows satisfying some criteria
	 *
	 * This function returns the number of rows from the wrapped table that satisfy the given criteria.
	 *
	 * If there is 'clarification.user_id' in $criteria, the rows with user_id=1 will be also counted.
	 * 
	 * @param  array $criteria  An array of <attribute-name> => <attribute-value> pairs. Only rows that satisfy all
	 *                          criteria will be counted.
	 * 
	 * @return int              The number of rows satisfying the criteria
	 */
	public function count_rows($criteria = array())
	{
		$this->db->select('COUNT(id) AS cnt');
		$this->db->from('clarification');
		if (isset($criteria['clarification.user_id']))
		{
			$this->db->where('(user_id=1 OR user_id=' . $criteria['clarification.user_id'] . ')');
			unset($criteria['clarification.user_id']);
		}
		foreach ($criteria as $k => $v)
			$this->db->where($k, $v);
		$q = $this->db->get();
		$res = $q->row_array();
		return $res['cnt'];
	}
}

/* End of file clarification_manager.php */
/* Location: ./application/models/contestant/clarification_manager.php */