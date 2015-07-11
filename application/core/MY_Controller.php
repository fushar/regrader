<?php if ( ! defined('BASEPATH')) exit('No direct script access allowed');

/**
 * Base controller class for Grador project
 *
 * This is the common base class for all controllers in Grador project.
 *
 * @package core
 * @author  Ashar Fuadi <fushar@gmail.com>
 * @author  Pusaka Kaleb Setyabudi <sokokaleb@gmail.com>
 */
class MY_Controller extends CI_Controller 
{
	/**
	 * The view data for each page
	 *
	 * This view data consists of the following pairs:
	 * - 'header'  => The view data for the page header an array of <name> => <value> pairs. Some required pairs:
	 *                - 'title' => The page title to be shown in the browser.
	 *                - 'name'  => The page name.
	 * - 'content' => The view data for the page content an array of <name> => <value> pairs.
	 * - 'footer'  => The view data for the page footer an array of <name> => <value> pairs.
	 * 
	 * @var array
	 */
	protected $ui;

	/**
	 * Constructs a new controller object
	 *
	 * This function constructs a new controller.
	 */
	public function __construct()
	{
		parent::__construct();
		$this->load->language('common');

		$this->ui['header']['web_name'] = $this->setting->get('web_name');
		$this->ui['header']['top_name'] = $this->setting->get('top_name');
		$this->ui['header']['bottom_name'] = $this->setting->get('bottom_name');
		$this->ui['header']['left_logo'] = $this->setting->get('left_logo');
		$this->ui['header']['right_logo1'] = $this->setting->get('right_logo1');
		$this->ui['header']['right_logo2'] = $this->setting->get('right_logo2');
		$this->ui['content'] = array();
		$this->ui['footer'] = array();

	}
}

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

/* End of file MY_Controller.php */
/* Location: ./application/core/MY_Controller.php */