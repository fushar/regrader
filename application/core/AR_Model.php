<?php if ( ! defined('BASEPATH')) exit('No direct script access allowed');

/**
 * Base active record model class
 *
 * This is the common base class for active record wrapper classes. Each descendant of this class wraps a single table
 * of the database. The wrapped table must have an auto-incremented primary key named 'id'. Each row of the wrapped
 * table is represented as an array of <attribute-name> => <attribute-value> pairs.
 * 
 * This base class supports basic CRUD (Create, Read, Update, Delete) operations.
 *
 * @package core
 * @author Ashar Fuadi <fushar@gmail.com>
 */
class AR_Model extends CI_Model 
{
	/**
	 * The wrapped table name
	 *
	 * The name must exactly match the corresponding table name in the database.
	 * 
	 * @var string
	 */
	protected $table;

	/**
	 * Constructs a new active record model object
	 *
	 * This function constructs a new active record object that wraps $table from the database.
	 * 
	 * @param string $table The table name.
	 */
	public function __construct($table)
	{
		parent::__construct();
		$this->table = $table;
	}

	/**
	 * Inserts a new row into the table
	 *
	 * This function inserts a new row described by $attributes into the wrapped table. If a certain attribute is not
	 * present in $attributes, its default value will be inserted instead.
	 * 
	 * @param  array $attributes The row to be inserted as an array of <attribute name> => <attribute value> pairs.
	 * 
	 * @return int               The newly inserted row ID.
	 */
	public function insert_row($attributes)
	{
		foreach ($attributes as $k => $v)
			$this->db->set($k, $v);
		$this->db->insert($this->table);
		
		return $this->db->insert_id();
	}

	/**
	 * Retrieves an existing row from the table
	 *
	 * This function returns a single row whose ID is $id from the wrapped table. If there is no such row, FALSE will be
	 * returned instead.
	 * 
	 * @param  int $id           The row ID to be retrieved.
	 * 
	 * @return mixed             The row whose ID is $id as an array of <attribute-name> => <attribute-value> pairs, or 
	 *                           FALSE if there is no such row.
	 */
	public function get_row($id)
	{
		$this->db->from($this->table);
		$this->db->where($this->table . '.id', $id);
		$this->db->limit(1);
		$q = $this->db->get();

		if ($q->num_rows() == 0)
			return FALSE;
		return $q->row_array();
	}

	/**
	 * Retrieves zero or more rows from the table satisfying some criteria
	 *
	 * This function returns zero or more rows from the wrapped table that satisfy the given criteria. All attributes
	 * are returned.
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
		$this->db->from($this->table);
		
		foreach ($criteria as $k => $v)
			$this->db->where($k, $v);

		if (isset($conditions['limit']))
		{
			if (isset($conditions['offset']))
				$this->db->limit($conditions['limit'], $conditions['offset']);
			else
				$this->db->limit($conditions['limit']);
		}

		if (isset($conditions['order_by']))
		{
			foreach ($conditions['order_by'] as $k => $v)
				$this->db->order_by($k, $v);	
		}

		$q = $this->db->get();
		$res = array();
		foreach ($q->result_array() as $v)
			$res[$v['id']] = $v;
		return $res;
	}

	/**
	 * Counts the number of rows satisfying some criteria
	 *
	 * This function returns the number of rows from the wrapped table that satisfy the given criteria.
	 * 
	 * @param  array $criteria  An array of <attribute-name> => <attribute-value> pairs. Only rows that satisfy all
	 *                          criteria will be counted.
	 * 
	 * @return int              The number of rows satisfying the criteria
	 */
	public function count_rows($criteria = array())
	{
		$this->db->select('COUNT(id) AS cnt');
		$this->db->from($this->table);
		
		foreach ($criteria as $k => $v)
			$this->db->where($k, $v);

		$q = $this->db->get();
		$res = $q->row_array();
		return $res['cnt'];
	}

	/**
	 * Retrieves an attribute of an existing row from the table
	 *
	 * This function returns the value of the attribute $attribute of the row whose ID is $id from the wrapped table. If
	 * there is no such row, FALSE will be returned instead.
	 * 
	 * @param  int $id           The row ID whose attribute value to be retrieved.
	 * @param  string $attribute The attribute name.
	 * 
	 * @return mixed             The retrieved attribute value.
	 */
	public function get_attribute($id, $attribute)
	{
		$this->db->select($attribute);
		$this->db->from($this->table);
		$this->db->where('id', $id);
		$q = $this->db->get();

		if ($q->num_rows() == 0)
			return FALSE;
		$res = $q->row_array();
		return $res[$attribute];
	}

	/**
	 * Updates an existing row from the table
	 *
	 * This function updates the attribute values of the row whose ID is $id from the wrapped table.
	 * 
	 * @param int   $id         The row ID to be updated.
	 * @param array $attributes The attributes to be updated as an array of <attribute-name> => <attribute-new-value>
	 *                          pairs.
	 */
	public function update_row($id, $attributes)
	{
		foreach ($attributes as $k => $v)
			$this->db->set($k, $v);
		$this->db->where('id', $id);
		$this->db->update($this->table);
	}

	/**
	 * Deletes an existing row in the table
	 *
	 * This function deletes the row whose ID is $id from the wrapped table.
	 * 
	 * @param int $id The row ID to be deleted.
	 */
	public function delete_row($id)
	{
		$this->db->where('id', $id);
		$this->db->delete($this->table);
	}
}

/* End of file AR_Model.php */
/* Location: ./application/core/AR_Model.php */