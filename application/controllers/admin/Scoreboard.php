<?php if ( ! defined('BASEPATH')) exit('No direct script access allowed');

/**
 * Controller for admin scoreboard page
 *
 * This is the controller responsible for showing the admin scoreboard page.
 *
 * @package controllers
 * @author Ashar Fuadi <fushar@gmail.com>
 */
class Scoreboard extends Admin_Controller 
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
		$this->load->model('scoreboard_manager');
		$this->load->language('scoreboard');
	}

	/**
	 * Redirects to scoreboard viewAll page
	 *
	 * This is the default function for admin Scoreboard controller. This function redirects to scoreboard view page.
	 */
	public function index()
	{
		$contests = $this->contest_manager->get_rows();
		if (empty($contests))
			redirect('admin/scoreboard/view/0/admin');
		else
		{
			$contest = reset($contests);
			redirect('admin/scoreboard/view/' . $contest['id'] . '/admin');
		}
	}

	/**
	 * Shows the scoreboard
	 *
	 * This function shows the $target scoreboard for the contest whose ID is $contest_id.
	 * 
	 * @param int $contest_id		The previous viewAll contest ID.
	 * @param int $page_offset		The previous viewAll page offset.
	 */
	public function view($contest_id, $target)
	{
		if ($this->input->post('contest_id'))
		{
			$contest_id = $this->input->post('contest_id');
			$target = $this->input->post('target');
			redirect('admin/scoreboard/view/' . $contest_id . '/' . $target);
		}
		else
		{
			$this->ui['header']['page'] = 'scoreboard';
			$this->ui['header']['title'] = $this->lang->line('scoreboard');

			$this->load->view('admin/header', $this->ui['header']);

			if ($contest_id > 0)
			{
				$contest = $this->contest_manager->get_row($contest_id);
				$contests = $this->contest_manager->get_rows();
				$res = $this->scoreboard_manager->get_scores($target, $contest_id);
				$this->ui['content']['contest_id'] = $contest_id;
				$this->ui['content']['contest_name'] = $contest['name'];
				$this->ui['content']['target'] = $target;
				$this->ui['content']['contests'] = $contests;
				$this->ui['content']['scores'] = $res['scores'];
				$this->ui['content']['show_institution_logo'] = $contest['show_institution_logo'];
				$this->ui['content']['problems'] = $res['problems'];
			}
			
			$this->load->view('admin/scoreboard/view', $this->ui['content']);
			
			if ($contest_id > 0)
				$this->load->view('scoreboard', $this->ui['content']);
			
			$this->load->view('footer', $this->ui['footer']);
		}
	}
}

/* End of file scoreboard.php */
/* Location: ./application/controllers/admin/scoreboard.php */