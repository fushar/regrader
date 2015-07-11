<?php if ( ! defined('BASEPATH')) exit('No direct script access allowed');

/**
 * Controller for contestant submission page
 *
 * This is the controller responsible for showing the contestant submission page.
 *
 * @package controllers
 * @author Ashar Fuadi <fushar@gmail.com>
 */
class Submission extends Contestant_Controller 
{
	/**
	 * Constructs a new controller object
	 *
	 * This function constructs a new controller.
	 */
	public function __construct()
	{
		parent::__construct();
		$this->load->model('submission_manager');
		$this->load->model('scoreboard_manager');
		$this->load->library('pagination');
		$this->load->language('submission');
	}

	/**
	 * Redirects to submission viewAll page
	 *
	 * This is the default function for contestant Submission controller. This function redirects to submission viewAll
	 * page.
	 */
	public function index()
	{
		redirect('contestant/submission/viewAll');
	}

	/**
	 * Shows the submissions made by the user
	 *
	 * This function shows the submissions made by the current user, starting from $page_offset-th page.
	 * 
	 * @param int $page_offset The page number to show.
	 */
	public function viewAll($page_offset = 1)
	{
		$this->ui['header']['page'] = 'submission';
		$this->ui['header']['title'] = $this->lang->line('submissions');

		$contest_id = $this->identity->get_current_contest_id();
		$items_per_page = $this->setting->get('items_per_page');

		$criteria['submission.contest_id'] = $contest_id;
		$criteria['submission.user_id'] = $this->identity->get_user_id();

		$conditions['limit'] = $items_per_page;
		$conditions['offset'] = ($page_offset-1) * $items_per_page;
		$conditions['order_by'] = array('submission.id' => 'DESC');

		$submissions = $this->submission_manager->get_rows($criteria, $conditions);

		$this->ui['content']['submissions'] = $submissions;
		$this->ui['content']['total_submissions'] = $this->submission_manager->count_rows($criteria);
		$this->ui['content']['items_per_page'] = $items_per_page;
		$this->ui['content']['page_offset'] = $page_offset;
		
		$config['base_url'] = site_url('contestant/submission/viewAll');
		$config['uri_segment'] = 4;
		$config['total_rows'] = $this->ui['content']['total_submissions'];
	
		$this->pagination->initialize($config);
		$this->ui['content']['pager'] = $this->pagination->create_links();

		$this->load->view('contestant/header', $this->ui['header']);
		$this->load->view('contestant/submission/viewAll', $this->ui['content']);
		$this->load->view('footer', $this->ui['footer']);
	}

	/**
	 * Shows a single submission details
	 *
	 * This function shows the details of submission whose ID is $submission_id.
	 * 
	 * @param int $submission_id The submission ID.
	 * @param int $page_offset 	 The previous page in viewAll.
	 */
	public function view($submission_id, $page_offset)
	{
		$submission = $this->submission_manager->get_row($submission_id);

		if ( ! $submission)
			show_404();
		if ($submission['contest_id'] != $this->identity->get_current_contest_id())
			show_404();
		if ($submission['user_id'] != $this->identity->get_user_id())
			show_404();

		$this->ui['header']['title'] = $this->lang->line('submission') . ' ' . $submission_id;
		$this->ui['header']['page'] = 'submission';
		$this->ui['header']['custom_css'] = array('vendor/google-code-prettify/bin/prettify.min.css');
		$this->ui['header']['custom_js'] = array('vendor/google-code-prettify/bin/prettify.min.js');
		$this->ui['content']['page_offset'] = $page_offset;
		$this->ui['content']['submission'] = $submission;
		
		$this->load->view('contestant/header', $this->ui['header']);
		$this->load->view('contestant/submission/view', $this->ui['content']);
		$this->load->view('footer', $this->ui['footer']);
	}
}

/* End of file submission.php */
/* Location: ./application/controllers/contestant/submission.php */