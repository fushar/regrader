<?php if ( ! defined('BASEPATH')) exit('No direct script access allowed');

/**
 * Controller for admin clarification page
 *
 * This is the controller responsible for showing the admin clarification page.
 *
 * @package controllers
 * @author Ashar Fuadi <fushar@gmail.com>
 */
class Clarification extends Admin_Controller 
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
		$this->load->model('contest_manager');
		$this->load->model('user_manager');
		$this->load->library('pagination');
		$this->load->language('clarification');
	}

	/**
	 * Redirects to clarification viewAll page
	 *
	 * This is the default function for admin Clarification controller. This function redirects to clarification viewAll
	 * page.
	 */
	public function index()
	{
		redirect('admin/clarification/viewAll/0/0/1');
	}

	/**
	 * Shows all clarifications in a particular contest for/by a particular user
	 *
	 * This function shows the clarifications for and by the user whose ID is $user_id, in the contest whose ID is
	 * $contest_id, starting from $page_offset-th page.
	 * 
	 * @param int $contest_id	The contest ID.
	 * @param int $user_id		The user ID.
	 * @param int $page_offset 	The page number to show.
	 */
	public function viewAll($contest_id = 0, $user_id = 0, $page_offset = 1)
	{
		if (NULL !== $this->input->post('contest_id'))
		{
			$contest_id = $this->input->post('contest_id');
			$user_id = $this->input->post('user_id');
			redirect('admin/clarification/viewAll/' . $contest_id . '/' . $user_id . '/' . $page_offset);
		}

		$this->form_validation->set_rules('form[contest_id]', $this->lang->line('contest'), 'required');
		$this->form_validation->set_rules('form[title]', $this->lang->line('title'), 'required|max_length[255]');
		$this->form_validation->set_rules('form[content]', $this->lang->line('content'), 'required');

		if ($this->form_validation->run())
		{
			$form = $this->input->post('form');
			$form['user_id'] = 1;
			$form['clar_time'] = date('Y-m-d H:i:s');
			$insert_id = $this->clarification_manager->insert_row($form);
			$this->clarification_manager->broadcast($insert_id, $form['contest_id']);
			$this->session->set_flashdata('submit_successful', true);
			redirect('admin/clarification/viewAll/' . $contest_id . '/' . $user_id . $page_offset . '/');
		}
		else
		{
			$this->ui['header']['page'] = 'clarification';
			$this->ui['header']['title'] = $this->lang->line('clarifications');

			$items_per_page = $this->setting->get('items_per_page');

			$criteria = array();
			if ($contest_id > 0)
				$criteria['clarification.contest_id'] = $contest_id;
			if ($user_id >  0)
				$criteria['clarification.user_id'] = $user_id;
			
			$conditions['limit'] = $items_per_page;
			$conditions['offset'] = ($page_offset-1) * $items_per_page;
			$conditions['order_by'] = array('clarification.id' => 'DESC');

			$clarifications = $this->clarification_manager->get_rows($criteria, $conditions);
			$users = $this->user_manager->get_rows();
			$unread_clars = $this->clarification_manager->get_admin_unread();

			foreach ($unread_clars as $v)
				if (isset($clarifications[$v['id']]))
					$clarifications[$v['id']]['unread'] = TRUE;

			$contests = $this->contest_manager->get_rows();

			$this->ui['content']['clarifications'] = $clarifications;
			$this->ui['content']['users'] = $users;
			$this->ui['content']['contests'] = $contests;
			$this->ui['content']['total_clarifications'] = $this->clarification_manager->count_rows($criteria);
			$this->ui['content']['items_per_page'] = $items_per_page;
			$this->ui['content']['page_offset'] = $page_offset;
			
			$config['base_url'] = site_url('admin/clarification/viewAll/' . $contest_id . '/' . $user_id);
			$config['uri_segment'] = 6;
			$config['total_rows'] = $this->ui['content']['total_clarifications'];
		
			$this->pagination->initialize($config);
			$this->ui['content']['pager'] = $this->pagination->create_links();
			$this->ui['content']['contest_id'] = $contest_id;
			$this->ui['content']['user_id'] = $user_id;

			if ($this->input->post('form'))
				$this->ui['content']['submit_error'] = TRUE;
			
			$this->load->view('admin/header', $this->ui['header']);
			$this->load->view('admin/clarification/viewAll', $this->ui['content']);
			$this->load->view('footer', $this->ui['footer']);
		}
	}

	/**
	 * Shows a single clarification details
	 *
	 * This function shows the details of clarification whose ID is $clarification_id.
	 * 
	 * @param int $clarification_id The clarification ID.
	 * @param int $contest_id		The previous viewAll contest ID.
	 * @param int $user_id			The previous viewAll user ID.
	 * @param int $page_offset		The previous viewAll page offset.
	 */
	public function view($clarification_id, $contest_id, $user_id, $page_offset)
	{
		$clarification = $this->clarification_manager->get_row($clarification_id);
		$this->clarification_manager->delete_unread(1, $clarification_id);

		if ($clarification['user_name'] != NULL)
			$this->form_validation->set_rules('form[answer]', $this->lang->line('the_answer'), 'required');

		if ($clarification['user_name'] != NULL && $this->form_validation->run())
		{
			$form = $this->input->post('form');
			$this->clarification_manager->update_row($clarification_id, $form);
			$this->clarification_manager->notify($clarification_id);
			$this->session->set_flashdata('answer_successful', true);
			redirect('admin/clarification/viewAll/' . $contest_id . '/' . $user_id . '/' . $page_offset);
		}
		else
		{
			$this->ui['header']['title'] = $this->lang->line('clarification') . ' ' . $clarification_id;
			$this->ui['header']['page'] = 'clarification';
			$this->ui['content']['page_offset'] = $page_offset;
			$this->ui['content']['clarification'] = $clarification;
			$this->ui['content']['contest_id'] = $contest_id;
			$this->ui['content']['user_id'] = $user_id;
			
			$this->load->view('admin/header', $this->ui['header']);
			$this->load->view('admin/clarification/view', $this->ui['content']);
			$this->load->view('footer', $this->ui['footer']);
		}
	}
}

/* End of file clarification.php */
/* Location: ./application/controllers/admin/clarification.php */
