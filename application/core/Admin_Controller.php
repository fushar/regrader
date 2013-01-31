<?php if ( ! defined('BASEPATH')) exit('No direct script access allowed');

/**
 * Base controller class for administrators
 *
 * This is the common base class for all controllers that can only be accessed by administrators; i.e., users whose
 * category is Administrator.
 *
 * @package core
 * @author Ashar Fuadi <fushar@gmail.com>
 */
class Admin_Controller extends MY_Controller 
{
	/**
	 * Constructs a new administrator controller object
	 *
	 * This function constructs a new administrator controller. It also checks whether the user accessing it is an 
	 * administrator. If not, the user will be redirected to the login page.
	 */
	public function __construct()
	{
		parent::__construct();

		if ( ! $this->identity->is_admin())
			redirect('site/login');

		$this->load->model('clarification_manager');

		$this->identity->checkin();
		
		$unread_clar_cnt = $this->clarification_manager->count_admin_unread();

		$this->ui['header']['active_user_name'] = $this->identity->get_user_name();
		$this->ui['header']['custom_css'] = array();
		$this->ui['header']['custom_js'] = array();
		$this->ui['header']['unread_clar_cnt'] = $unread_clar_cnt;
	}
}

/* End of file Admin_Controller.php */
/* Location: ./application/core/Admin_Controller.php */