<?php if ( ! defined('BASEPATH')) exit('No direct script access allowed');

/**
 * Controller for admin contest page
 *
 * This is the controller responsible for showing the admin contest page.
 *
 * @package controllers
 * @author Ashar Fuadi <fushar@gmail.com>
 */
class Contest extends Admin_Controller 
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
		$this->load->model('category_manager');
		$this->load->model('problem_manager');
		$this->load->model('language_manager');
		$this->load->model('scoreboard_manager');
		$this->load->model('submission_manager');
		$this->load->library('pagination');
		$this->load->language('contest');
	}

	/**
	 * Redirects to contest viewAll page
	 *
	 * This is the default function for admin Language controller. This function redirects to contest viewAll page.
	 */
	public function index()
	{
		redirect('admin/contest/viewAll');
	}

	/**
	 * Shows all contests
	 *
	 * This function shows config('items_per_page') contests, starting from $page_offset-th page.
	 * 
	 * @param int $page_offset The page number to show.
	 */
	public function viewAll($page_offset = 1)
	{
		$this->ui['header']['page'] = 'contest';
		$this->ui['header']['title'] = $this->lang->line('contests');

		$items_per_page = $this->setting->get('items_per_page');

		$conditions['limit'] = $items_per_page;
		$conditions['offset'] = ($page_offset-1) * $items_per_page;
		$conditions['order_by'] = array('id' => 'ASC');
		
		$this->ui['content']['contests'] = $this->contest_manager->get_rows(array(), $conditions);
		$this->ui['content']['total_contests'] = $this->contest_manager->count_rows();
		$this->ui['content']['items_per_page'] = $items_per_page;
		$this->ui['content']['page_offset'] = $page_offset;

		$config['base_url'] = site_url('admin/contest/viewAll');
		$config['uri_segment'] = 4;
		$config['total_rows'] = $this->ui['content']['total_contests'];

		$this->pagination->initialize($config);
		$this->ui['content']['pager'] = $this->pagination->create_links();

		$this->load->view('admin/header', $this->ui['header']);
		$this->load->view('admin/contest/viewAll', $this->ui['content']);
		$this->load->view('footer', $this->ui['footer']);
	}

	/**
	 * Shows the add or edit contest page
	 *
	 * This function shows an add contest page if $contest_id is 0, otherwise it shows an edit contest page.
	 * 
	 * @param int $contest_id The contest ID.
	 * @param int $page_offset The previous viewAll page number.
	 */
	public function edit($contest_id, $page_offset)
	{
		$this->form_validation->set_rules('form[name]', $this->lang->line('name'), 'trim|required');
		$this->form_validation->set_rules('form[start_time]', $this->lang->line('start_time'), 'required');
		$this->form_validation->set_rules('form[end_time]', $this->lang->line('end_time'), 'required');
		$this->form_validation->set_rules('form[freeze_time]', $this->lang->line('freeze_time'), 'required');
		$this->form_validation->set_rules('form[unfreeze_time]', $this->lang->line('unfreeze_time'), 'required');

		$categories = $this->category_manager->get_rows();
		$contest_members = array();
		foreach ($categories as $k => $v)
		{
			$contest_members[$k]['name'] = $v['name'];
			$contest_members[$k]['present'] = FALSE;
		}
		$categories = $this->category_manager->get_rows(array('contest_id' => $contest_id));
		foreach ($categories as $k => $v)
			$contest_members[$k]['present'] = TRUE;

		$languages = $this->language_manager->get_rows();
		$contest_languages = array();
		foreach ($languages as $k => $v)
		{
			$contest_languages[$k]['name'] = $v['name'];
			$contest_languages[$k]['present'] = FALSE;
		}
		$languages = $this->language_manager->get_rows(array('contest_id' => $contest_id));
		foreach ($languages as $k => $v)
			$contest_languages[$k]['present'] = TRUE;

		if ($this->form_validation->run())
		{
			$form = $this->input->post('form');
			$form['enabled'] = isset($form['enabled']) ? 1 : 0;
			$form['show_institution_logo'] = isset($form['show_institution_logo']) ? 1 : 0;
			
			if ($contest_id == 0)
			{
				$contest_id = $this->contest_manager->insert_row($form);
				$this->session->set_flashdata('add_successful', true);
			}
			else
			{
				$this->contest_manager->update_row($contest_id, $form);
				$this->scoreboard_manager->recalculate_scores(array('contest_id' => $contest_id));
				$this->session->set_flashdata('edit_successful', true);
			}
			
			$cats = array();
			foreach ($contest_members as $k => $v)
				if ($this->input->post('c' . $k) == 1)
					$cats[] = $k;
			$this->contest_manager->update_members($contest_id, $cats);

			$langs = array();

			foreach ($contest_languages as $k => $v)
				if ($this->input->post('l' . $k) == 1)
					$langs[] = $k;
			$this->contest_manager->update_languages($contest_id, $langs);

			redirect('admin/contest/viewAll/' . $page_offset);
		}
		else
		{
			$this->ui['content']['contest_members'] = $contest_members;
			$this->ui['content']['contest_languages'] = $contest_languages;

			if ($contest_id == 0)
				$this->ui['header']['title'] = $this->lang->line('add_new_contest');
			else
			{
				$this->ui['header']['title'] = $this->lang->line('edit_contest') . ' ' . $contest_id;
				$this->ui['content']['contest'] = $this->contest_manager->get_row($contest_id);
			}
			$this->ui['header']['page'] = 'contest';
			$this->ui['header']['custom_css'] = array('vendor/ama3-anytime/styles/anytime.compressed.css');
			$this->ui['header']['custom_js'] = array('vendor/ama3-anytime/anytime.js', 'vendor/ama3-anytime/jquery-migrate-1.0.0.js');
			$this->ui['content']['page_offset'] = $page_offset;
			
			$this->load->view('admin/header', $this->ui['header']);
			$this->load->view('admin/contest/edit', $this->ui['content']);
			$this->load->view('footer', $this->ui['footer']);
		}
	}

	/**
	 * Shows the edit problems page
	 *
	 * This function shows the edit problems page.
	 * 
	 * @param int $contest_id The problem ID.
	 * @param int $page_offset The previous viewAll page number.
	 */
	public function editProblems($contest_id, $page_offset)
	{
		$contest_problems = array();
		$problems = $this->problem_manager->get_rows(array('contest_id' => $contest_id), array('order_by' => array('alias' => 'ASC')));
		foreach ($problems as $k => $v)
		{
			$contest_problems[$k]['name'] = $v['name'];
			$contest_problems[$k]['alias'] = $v['alias'];
			$contest_problems[$k]['present'] = TRUE;
		}
		$problems = $this->problem_manager->get_rows();
		
		foreach ($problems as $k => $v)
		{
			if ( ! isset($contest_problems[$k]))
			{
				$contest_problems[$k]['name'] = $v['name'];
				$contest_problems[$k]['present'] = FALSE;
			}
		}

		$this->form_validation->set_rules('new_problem_id', $this->lang->line('problem'), 'required');
		$this->form_validation->set_rules('new_problem_alias', $this->lang->line('alias'), 'required');

		if ($this->form_validation->run())
		{
			$args['contest_id'] = $contest_id;
			$args['problem_id'] = $this->input->post('new_problem_id');
			$args['alias'] = $this->input->post('new_problem_alias');

			$this->contest_manager->add_problem($args);
			redirect('admin/contest/editProblems/' . $contest_id . '/' . $page_offset);
		}
		else
		{
			$this->ui['content']['contest_problems'] = $contest_problems;
			$this->ui['content']['contest'] = $this->contest_manager->get_row($contest_id);

			$this->ui['header']['title'] = $this->lang->line('edit_contest_problems') . ' ' . $contest_id;
			$this->ui['header']['page'] = 'contest';
			$this->ui['content']['page_offset'] = $page_offset;

			$this->load->view('admin/header', $this->ui['header']);
			$this->load->view('admin/contest/editProblems', $this->ui['content']);
			$this->load->view('footer', $this->ui['footer']);
		}
	}

	/**
	 * Deletes a problem
	 *
	 * This function deletes a problem whose ID is $problem_id and then redirects to edit problems page.
	 * 
	 * @param int $contest_id The contest ID.
	 * @param int $problem_id The problem ID.
	 * @param int $page_offset The previous viewAll page number.
	 */
	public function deleteProblem($contest_id, $problem_id, $page_offset)
	{
		$this->contest_manager->delete_problem($contest_id, $problem_id);
		redirect('admin/contest/editProblems/' . $contest_id . '/' . $page_offset);
	}
 
 	/**
	 * Deletes a contest
	 *
	 * This function deletes the contest whose ID is $contest_id and then redirects to viewAll page.
	 * 
	 * @param int $contest_id The contest ID.
	 * @param int $page_offset The previous viewAll page number.
	 */
	public function delete($contest_id, $page_offset)
	{
		$this->contest_manager->delete_row($contest_id);
		$this->session->set_flashdata('delete_successful', true);
		redirect('admin/contest/viewAll/' . $page_offset);
	}
}

/* End of file contest.php */
/* Location: ./application/controllers/admin/contest.php */
