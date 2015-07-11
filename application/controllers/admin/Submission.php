<?php if ( ! defined('BASEPATH')) exit('No direct script access allowed');

/**
 * Controller for admin submission page
 *
 * This is the controller responsible for showing the admin submission page.
 *
 * @package controllers
 * @author Ashar Fuadi <fushar@gmail.com>
 */
class Submission extends Admin_Controller 
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
		$this->load->model('contest_manager');
		$this->load->model('problem_manager');
		$this->load->model('user_manager');
		$this->load->library('pagination');
		$this->load->language('submission');
	}
	
	/**
	 * Redirects to submission viewAll page
	 *
	 * This is the default function for admin Submission controller. This function redirects to submission viewAll page.
	 */
	public function index()
	{
		redirect('admin/submission/viewAll/0/0/0/1');
	}

	/**
	 * Shows the submissions of a particular contest, problem, and user
	 *
	 * This function shows the submissions starting from $page_offset-th page.
	 * 
	 * @param int $contest_id The contest ID, or 0 to select all contests.
	 * @param int $problem_id The problem ID, or 0 to select all problems.
	 * @param int $user_id The user ID, or 0 to select all users.
	 * @param int $page_offset The page number to show.
	 */
	public function viewAll($contest_id = 0, $problem_id = 0, $user_id = 0, $page_offset = 1)
	{
		if ($this->input->post(NULL))
		{
			$contest_id = $this->input->post('contest_id');
			$problem_id = $this->input->post('problem_id');
			$user_id = $this->input->post('user_id');
			redirect('admin/submission/viewAll/' . $contest_id . '/' . $problem_id . '/' . $user_id . '/' . $page_offset);
		}

		$this->ui['header']['page'] = 'submission';
		$this->ui['header']['title'] = $this->lang->line('submissions');

		$items_per_page = $this->setting->get('items_per_page');

		$criteria = array();
		if ($contest_id > 0)
			$criteria['submission.contest_id'] = $contest_id;
		if ($problem_id > 0)
			$criteria['submission.problem_id'] = $problem_id;
		if ($user_id > 0)
			$criteria['submission.user_id'] = $user_id;

		$conditions['limit'] = $items_per_page;
		$conditions['offset'] = ($page_offset-1) * $items_per_page;
		$conditions['order_by'] = array('submission.id' => 'DESC');

		$submissions = $this->submission_manager->get_rows($criteria, $conditions);
		$contests = $this->contest_manager->get_rows();
		$problems = $this->problem_manager->get_rows();
		$users = $this->user_manager->get_rows();

		$this->ui['content']['submissions'] = $submissions;
		$this->ui['content']['contests'] = $contests;
		$this->ui['content']['problems'] = $problems;
		$this->ui['content']['users'] = $users;
		$this->ui['content']['total_submissions'] = $this->submission_manager->count_rows($criteria);
		$this->ui['content']['items_per_page'] = $items_per_page;
		$this->ui['content']['page_offset'] = $page_offset;
		
		$config['base_url'] = site_url('admin/submission/viewAll/' . $contest_id . '/' . $problem_id . '/' . $user_id);
		$config['uri_segment'] = 7;
		$config['total_rows'] = $this->ui['content']['total_submissions'];
	
		$this->pagination->initialize($config);
		$this->ui['content']['pager'] = $this->pagination->create_links();
		$this->ui['content']['contest_id'] = $contest_id;
		$this->ui['content']['problem_id'] = $problem_id;
		$this->ui['content']['user_id'] = $user_id;

		$this->load->view('admin/header', $this->ui['header']);
		$this->load->view('admin/submission/viewAll', $this->ui['content']);
		$this->load->view('footer', $this->ui['footer']);
	}

	/**
	 * Shows a single submission details
	 *
	 * This function shows the details of submission whose ID is $submission_id.
	 * 
	 * @param int $submission_id The submission ID.
	 * @param int $contest_id The previous viewAll contest ID.
	 * @param int $problem_id The previous viewAll problem ID.
	 * @param int $user_id The previous viewAll user ID.
	 * @param int $page_offset The previous viewAll page offset.
	 */
	public function view($submission_id, $contest_id, $problem_id, $user_id, $page_offset)
	{
		$submission = $this->submission_manager->get_row($submission_id);

		if ( ! $submission)
			show_404();

		$this->ui['header']['title'] = 'Pengumpulan ' . $submission_id;
		$this->ui['header']['page'] = 'submission';
		$this->ui['header']['custom_css'] = array('vendor/google-code-prettify/bin/prettify.min.css');
		$this->ui['header']['custom_js'] = array('vendor/google-code-prettify/bin/prettify.min.js');
		$this->ui['content']['submission'] = $submission;
		
		$judgings = $this->submission_manager->get_judgings($submission_id);
			
		$this->ui['content']['judgings'] = $judgings;
		$this->ui['content']['page_offset'] = $page_offset;
		$this->ui['content']['contest_id'] = $contest_id;
		$this->ui['content']['problem_id'] = $problem_id;
		$this->ui['content']['user_id'] = $user_id;
		
		$this->load->view('admin/header', $this->ui['header']);
		$this->load->view('admin/submission/view', $this->ui['content']);
		$this->load->view('footer', $this->ui['footer']);
	}

	/**
	 * Regrades a submission
	 *
	 * This function regrades the submission whose ID is $submission_id and then redirect to viewAll page.
	 * 
	 * @param int $submission_id The submission ID.
	 * @param int $contest_id The previous viewAll contest ID.
	 * @param int $problem_id The previous viewAll problem ID.
	 * @param int $user_id The previous viewAll user ID.
	 * @param int $page_offset The previous viewAll page offset.
	 */
	public function regrade($submission_id, $contest_id, $problem_id, $user_id, $page_offset)
	{
		$this->submission_manager->regrade($submission_id);
		redirect('admin/submission/viewAll/' . $contest_id . '/' . $problem_id . '/' . $user_id . '/' . $page_offset);
	}

	/**
	 * Ignores a submission
	 *
	 * This function ignores the submission whose ID is $submission_id and then redirect to viewAll page.
	 * 
	 * @param int $submission_id The submission ID.
	 * @param int $contest_id The previous viewAll contest ID.
	 * @param int $problem_id The previous viewAll problem ID.
	 * @param int $user_id The previous viewAll user ID.
	 * @param int $page_offset The previous viewAll page offset.
	 */
	public function ignore($submission_id, $contest_id, $problem_id, $user_id, $page_offset)
	{
		$this->submission_manager->ignore($submission_id);
		redirect('admin/submission/viewAll/' . $contest_id . '/' . $problem_id . '/' . $user_id . '/' . $page_offset);
	}
}

/* End of file submission.php */
/* Location: ./application/controllers/admin/submission.php */
