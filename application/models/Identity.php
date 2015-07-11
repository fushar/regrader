<?php if ( ! defined('BASEPATH')) exit('No direct script access allowed');

/**
 * Model for user identification management
 *
 * This model performs user identification tasks such as logging in users to the website, logging users out, and
 * authenticating password.
 *
 * @package models
 * @author  Ashar Fuadi <fushar@gmail.com>
 */
class Identity extends CI_Model 
{
	/**
	 * Logs a user into the website
	 *
	 * This function tries to authenticate the user. If the credentials are valid, then it logs the user into the
	 * website and returns TRUE. If not, it returns FALSE instead.
	 * 
	 * @param  array $credentials The user credentials as an array of the following pairs:
	 *                            - 'username' => The user's username.
	 *                            - 'password' => The user's password.
	 * 
	 * @return boolean            TRUE if the user is successfully logged in, or FALSE otherwise.
	 */
	public function login(array $credentials)
	{
		$this->load->model('user_manager');

		$res = $this->user_manager->get_rows(
			array('username' => $credentials['username'], 'password' => md5($credentials['password'])),
			array('limit' => 1)
		);

		if (empty($res))
			return FALSE;

		$res = reset($res);

		$this->set_session_data('id', $res['id']);
		$this->set_session_data('name', $res['name']);
		$this->set_session_data('username', $res['username']);
		$this->set_session_data('category_id', $res['category_id']);
		$this->set_session_data('contest_id', 0);

		return TRUE;
	}

	/**
	 * Logs out the current user from the website
	 * 
	 * This function logs out the current user and destroy all session variables associated to the user.
	 */
	public function logout()
	{
		$this->session->sess_destroy();
	}

	/**
	 * Checks in the current user
	 *
	 * This function set the current value of 'last_activity' attribute of the current user to the current time. 
	 * This is a wa for checking whether the current user is active or not.
	 */
	public function checkin()
	{
		$this->db->set('last_activity', date('Y-m-d H:i:s'));
		$this->db->where('id', $this->get_session_data('id'));
		$this->db->update('user');
	}

	/**
	 * Checks if the current user is a guest
	 * 
	 * This function checks if the current user has not logged in yet.
	 *
	 * @return bool TRUE if the current has not logged in, or FALSE otherwise.
	 */
	public function is_guest()
	{
		return NULL !== $this->get_session_data('id') && ! $this->get_session_data('id');
	}

	/**
	 * Checks if the current user is a contestant
	 * 
	 * This function checks if the current user is not an administrator.
	 *
	 * @return boolean TRUE if the current is not an administrator, or FALSE otherwise.
	 */
	public function is_contestant()
	{
		return $this->get_session_data('category_id') > 1;
	}

	/**
	 * Checks if the current user is an administrator
	 * 
	 * This function checks if the current user is an administrator.
	 *
	 * @return boolean TRUE if the current is an administrator, or FALSE otherwise.
	 */
	public function is_admin()
	{
		return $this->get_session_data('category_id') == 1;
	}

	/**
	 * Retrieves the current user ID
	 *
	 * This function returns the ID of the current user.
	 * 
	 * @return int The ID of the current user.
	 */
	public function get_user_id()
	{
		return $this->get_session_data('id');
	}

	/**
	 * Retrieves the current user name
	 *
	 * This function returns the name of the current user.
	 * 
	 * @return string The name of the current user.
	 */
	public function get_user_name()
	{
		return $this->get_session_data('name');
	}

	/**
	 * Retrieves the current user category ID
	 *
	 * This function returns the category ID of the current user.
	 * 
	 * @return int The category ID of the current user.
	 */
	public function get_user_category_id()
	{
		return $this->get_session_data('category_id');
	}

	/**
	 * Retrieves the current contest ID
	 *
	 * This function returns the contest ID in which the current user is currently competing.
	 * 
	 * @return int The current contest ID, or 0 if the current user is not competing in any contest.
	 */
	public function get_current_contest_id()
	{
		return $this->get_session_data('contest_id');
	}

	/**
	 * Sets the current contest ID
	 *
	 * This function marks the contest which in the current user starts competing.
	 */
	public function set_current_contest_id($contest_id)
	{
		$this->set_session_data('contest_id', $contest_id);
	}

	/**
	 * Sets a session data
	 *
	 * This function sets the session data $key to $value. This function uses a random string for security.
	 * 
	 * @param string $key 	The key.
	 * @param mixed $value 	The value.
	 */
	private function set_session_data($key, $value)
	{
		$this->session->set_userdata($key . '_' . $this->setting->get('sess_id'), $value);
	}

	/**
	 * Retrieves a session data
	 *
	 * This function returns the content of the session data $key. This function uses a random string for security.
	 * 
	 * @param string $key 	The key.
	 * 
	 * @return mixed The retrieved value.
	 */
	private function get_session_data($key)
	{
		return $this->session->userdata($key . '_' . $this->setting->get('sess_id'));
	}
}

/* End of file identitiy.php */
/* Location: ./application/models/identitiy.php */
