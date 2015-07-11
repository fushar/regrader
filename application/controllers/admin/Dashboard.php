<?php if ( ! defined('BASEPATH')) exit('No direct script access allowed');

/**
 * Controller for admin dashboard page
 *
 * This is the controller responsible for showing the admin dashboard page.
 *
 * @package controllers
 * @author Ashar Fuadi <fushar@gmail.com>
 */
class Dashboard extends Admin_Controller 
{
	/**
	 * Constructs a new controller object
	 *
	 * This function constructs a new controller.
	 */
	public function __construct()
	{
		parent::__construct();
		$this->load->model('contest_manager');
		$this->load->model('user_manager');
		$this->load->model('grader_manager');
		$this->load->language('dashboard');
	}

	/**
	 * Shows the admin dashboard page
	 * 
	 * This is the default action for admin Dashboard controller. This function shows the admin dashboard page.
	 */
	public function index()
	{
		$this->ui['header']['title'] = $this->lang->line('dashboard');
		$this->ui['header']['page'] = 'dashboard';

		$this->ui['content']['active_contests'] = $this->contest_manager->get_active_contests(0);
		$this->ui['content']['active_users'] = $this->user_manager->get_active_users();
		$this->ui['content']['active_graders'] = $this->grader_manager->get_active_graders();
		
		$this->load->view('admin/header', $this->ui['header']);
		$this->load->view('admin/dashboard/index', $this->ui['content']);
		$this->load->view('footer', $this->ui['footer']);
	}
}

/* End of file dashboard.php */
/* Location: ./application/controllers/admin/dashboard.php */