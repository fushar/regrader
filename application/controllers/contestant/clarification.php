<?php if ( ! defined('BASEPATH')) exit('No direct script access allowed');

/**
 * Controller for contestant clarification page
 *
 * This is the controller responsible for showing the contestant clarification page.
 *
 * @package controllers
 * @author Ashar Fuadi <fushar@gmail.com>
 */
class Clarification extends Contestant_Controller 
{
	/**
	 * Constructs a new controller object
	 *
	 * This function constructs a new controller.
	 */
	public function __construct()
	{
		parent::__construct();
		$this->load->model('clarification_manager');
		$this->load->library('pagination');
		$this->load->language('clarification');
	}

	/**
	 * Redirects to clarification viewAll page
	 *
	 * This is the default function for contestant Clarification controller. This function redirects to clarification
	 * viewAll page.
	 */
	public function index()
	{
		redirect('contestant/clarification/viewAll');
	}

	/**
	 * Shows the clarifications made by the user
	 *
	 * This function shows the clarifications made by the current user, starting from $page_offset-th page.
	 * 
	 * @param int $page_offset The page number to show.
	 */
	public function viewAll($page_offset = 1)
	{
		$this->form_validation->set_rules('form[title]', 'Judul', 'required|max_length[255]');
		$this->form_validation->set_rules('form[content]', 'Isi', 'required');

		if ($this->form_validation->run())
		{
			if (strtotime($this->ui['header']['active_contest_end_time']) < time())
			{
				$this->session->set_flashdata('submit_error', true);
				$this->session->set_flashdata('contest_has_ended', true);
				redirect('contestant/problem/view/' . $problem_id);
			}
			
			$form = $this->input->post('form');
			$form['contest_id'] = $this->identity->get_current_contest_id();
			$form['user_id'] = $this->identity->get_user_id();
			$form['clar_time'] = date('Y-m-d H:i:s');
			$insert_id = $this->clarification_manager->insert_row($form);
			$this->clarification_manager->ask($insert_id);
			$this->session->set_flashdata('submit_successful', true);
			redirect('contestant/clarification/viewAll');
		}
		else
		{
			$this->ui['header']['page'] = 'clarification';
			$this->ui['header']['title'] = $this->lang->line('clarifications');

			$user_id = $this->identity->get_user_id();
			$contest_id = $this->identity->get_current_contest_id();
			$items_per_page = $this->setting->get('items_per_page');

			$criteria['clarification.contest_id'] = $contest_id;
			$criteria['clarification.user_id'] = $this->identity->get_user_id();

			$conditions['limit'] = $items_per_page;
			$conditions['offset'] = ($page_offset-1) * $items_per_page;
			$conditions['order_by'] = array('clarification.id' => 'DESC');

			$clarifications = $this->clarification_manager->get_rows_with_admin($criteria, $conditions);
			$unread_clars = $this->clarification_manager->get_contestant_unread($user_id, $contest_id);

			foreach ($unread_clars as $v)
				if (isset($clarifications[$v['id']]))
					$clarifications[$v['id']]['unread'] = TRUE;

			$this->ui['content']['clarifications'] = $clarifications;

			$this->ui['content']['total_clarifications'] = $this->clarification_manager->count_rows($criteria);
			$this->ui['content']['items_per_page'] = $items_per_page;
			$this->ui['content']['page_offset'] = $page_offset;
			
			$config['base_url'] = site_url('contestant/clarification/viewAll');
			$config['uri_segment'] = 4;
			$config['total_rows'] = $this->ui['content']['total_clarifications'];
		
			$this->pagination->initialize($config);
			$this->ui['content']['pager'] = $this->pagination->create_links();

			if ($this->input->post('form'))
				$this->ui['content']['submit_error'] = TRUE;
		
			$this->load->view('contestant/header', $this->ui['header']);
			$this->load->view('contestant/clarification/viewAll', $this->ui['content']);
			$this->load->view('footer', $this->ui['footer']);
		}
	}

	/**
	 * Shows a single clarification details
	 *
	 * This function shows the details of clarification whose ID is $clarification_id.
	 * 
	 * @param int $clarification_id The clarification ID.
	 * @param int $page_offset 		The previous page in viewAll.
	 */
	public function view($clarification_id, $page_offset)
	{
		$clarification = $this->clarification_manager->get_row($clarification_id);
		$this->clarification_manager->delete_unread($this->identity->get_user_id(), $clarification_id);

		if ( ! $clarification)
			show_404();
		if ($clarification['contest_id'] != $this->identity->get_current_contest_id())
			show_404();
		if ($clarification['user_id'] != $this->identity->get_user_id() && $clarification['user_id'] != 1)
			show_404();
		
		$this->ui['header']['title'] = $this->lang->line('clarification') . ' ' . $clarification_id;
		$this->ui['header']['page'] = 'clarification';
		$this->ui['content']['page_offset'] = $page_offset;
		$this->ui['content']['clarification'] = $clarification;
		
		$this->load->view('contestant/header', $this->ui['header']);
		$this->load->view('contestant/clarification/view', $this->ui['content']);
		$this->load->view('footer', $this->ui['footer']);
	}
}

/* End of file clarification.php */
/* Location: ./application/controllers/contestant/clarification.php */