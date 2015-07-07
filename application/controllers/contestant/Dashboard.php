<?php if ( ! defined('BASEPATH')) exit('No direct script access allowed');

/**
 * Controller contestant dashboard page
 *
 * This is the controller responsible for showing the contestant dashboard page.
 *
 * @package controllers
 * @author Ashar Fuadi <fushar@gmail.com>
 */
class Dashboard extends Contestant_Controller 
{
	/**
	 * Constructs a new controller object
	 *
	 * This function constructs a new controller.
	 */
	public function __construct()
	{
		parent::__construct();
		$this->load->language('dashboard');
	}

	/**
	 * Shows the contestant dashboard page
	 * 
	 * This is the default action for contestant Dashboard controller. This function shows the contestant dashboard
	 * page.
	 */
	public function index()
	{
		if ($this->input->post('form'))
		{
			$form = $this->input->post('form');
			$contests = $this->contest_manager->get_active_contests($this->identity->get_user_category_id());

			if (! isset($contests[$form['contest_id']]))
				redirect('contestant/dashboard');

			$this->identity->set_current_contest_id($form['contest_id']);
			redirect('contestant/problem');
		}
		else
		{
			$this->identity->set_current_contest_id(0);

			$this->ui['header']['title'] = $this->lang->line('dashboard');
			$this->ui['header']['page'] = 'dashboard';
			$this->ui['content']['contests'] = $this->contest_manager->get_active_contests($this->identity->get_user_category_id());
			
			$this->load->view('contestant/header', $this->ui['header']);
			$this->load->view('contestant/dashboard/index', $this->ui['content']);
			$this->load->view('footer', $this->ui['footer']);
		}
	}
}

/* End of file dashboard.php */
/* Location: ./application/controllers/contestant/dashboard.php */