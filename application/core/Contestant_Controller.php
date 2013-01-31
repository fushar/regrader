<?php if ( ! defined('BASEPATH')) exit('No direct script access allowed');

/**
 * Base controller class for contestants
 *
 * This is the common base class for all controllers that can only be accessed by contestants; i.e., users whose
 * category is not Administrator.
 *
 * @package core
 * @author Ashar Fuadi <fushar@gmail.com>
 */
class Contestant_Controller extends MY_Controller 
{
	/**
	 * Constructs a new contestant controller object
	 *
	 * This function constructs a new contestant controller. It also checks whether the user accessing it is a 
	 * contestant. If not, the user will be redirected to the login page.
	 */
	public function __construct()
	{
		parent::__construct();

		if ( ! $this->identity->is_contestant())
			redirect('site/login');

		$this->load->model('user_manager');
		$this->load->model('contest_manager');
		$this->load->model('clarification_manager');

		$this->identity->checkin();

		$user_id = $this->identity->get_user_id();
		$user = $this->user_manager->get_row($user_id);

		$contest_id = $this->identity->get_current_contest_id();
		$contest = $this->contest_manager->get_row($contest_id);
		$unread_clar_cnt = $this->clarification_manager->count_contestant_unread($user_id, $contest_id);
		
		$this->ui['header']['custom_css'] = array();
		$this->ui['header']['custom_js'] = array();
		$this->ui['header']['active_user_name'] = $user['name'];
		$this->ui['header']['active_contest_name'] = $contest['name'];
		$this->ui['header']['active_contest_end_time'] = $contest['end_time'];
		$this->ui['header']['unread_clar_cnt'] = $unread_clar_cnt;
		$this->ui['footer']['active_contest_end_time'] = $contest['end_time'];
	}
}

/* End of file Contestant_Controller.php */
/* Location: ./application/core/Contestant_Controller.php */