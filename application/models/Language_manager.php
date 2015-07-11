<?php if ( ! defined('BASEPATH')) exit('No direct script access allowed');

/**
 * Model class that wraps table language
 *
 * This is the active record class wrapper for table 'language' from the database.
 * 
 * @package models
 * @author  Ashar Fuadi <fushar@gmail.com>
 */
class Language_manager extends AR_Model 
{
	/**
	 * Constructs a new model object
	 *
	 * This function constructs a new model.
	 */
	public function __construct()
	{
		parent::__construct('language');
	}

	/**
	 * Retrieves zero or more rows from table 'language' satisfying some criteria
	 *
	 * This function returns zero or more rows from the wrapped table that satisfy the given criteria. All attributes
	 * are returned.
	 *
	 * This function overrides AR_Model::get_rows(). It can also accept 'contest_id' as a criteria key; i.e., the
	 * contest IDs that this language is allowed in.
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
			$this->db->join('contest_language', 'contest_language.language_id=id');
			$this->db->where('contest_language.contest_id', $criteria['contest_id']);
		}
		
		return parent::get_rows($criteria, $conditions);
	}
}

/* End of file language_manager.php */
/* Location: ./application/models/contestant/language_manager.php */