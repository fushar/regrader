<?php if ( ! defined('BASEPATH')) exit('No direct script access allowed');

/**
 * Model for user global setting management
 *
 * This model manages the global setting variables.
 *
 * @package models
 * @author  Ashar Fuadi <fushar@gmail.com>
 */
class Setting extends CI_Model 
{
	/**
	 * The cached setting variables
	 *
	 * This is an array of key => value of the setting variables.
	 * 
	 * @var array
	 */
	public $data;

	/**
	 * Constructs a new model object
	 *
	 * This function constructs a new model, and then populates the setting variables from the database.
	 */
	public function __construct()
	{
		parent::__construct();

		if ($this->db->table_exists('setting'))
		{
			$this->db->from('setting');
			$q = $this->db->get();
			foreach ($q->result_array() as $v)
				$this->data[$v['key']] = $v['value'];
		}
	}

	/**
	 * Retrieves the value of a setting variable
	 *
	 * This function returns the value of setting variable $key.
	 *
	 * @param $key The key.
	 * 
	 * @return mixed The value of setting variable $key, of FALSE if there is no such key.
	 */
	public function get($key)
	{
		if ( ! isset($this->data[$key]))
			return FALSE;
		return $this->data[$key];
	}

	/**
	 * Sets the value of a setting variable
	 *
	 * This function sets the value of setting variable $key to $value.
	 *
	 * @param $key The key.
	 * @param $value The value.
	 */
	public function set($key, $value)
	{
		$this->data[$key] = $value;
		$this->db->set('value', $value);
		$this->db->where('key', $key);
		$this->db->update('setting');
	}
}

/* End of file setting.php */
/* Location: ./application/models/setting.php */