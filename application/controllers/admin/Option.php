<?php if ( ! defined('BASEPATH')) exit('No direct script access allowed');

/**
 * Controller for admin option page
 *
 * This is the controller responsible for showing the admin option page.
 *
 * @package controllers
 * @author Ashar Fuadi <fushar@gmail.com>
 */
class Option extends Admin_Controller 
{
	/**
	 * Constructs a new controller object
	 *
	 * This function constructs a new controller.
	 */
	public function __construct()
	{
		parent::__construct();
		$this->load->language('option');
	}

	/**
	 * Redirects to option edit page
	 *
	 * This is the default function for admin Option controller. This function redirects to option edit page.
	 */
	public function index()
	{
		redirect('admin/option/edit');
	}

	/**
	 * Shows the edit option page
	 *
	 * This function shows an edit option page.
	 */
	public function edit()
	{
		$this->form_validation->set_rules('form[web_name]', $this->lang->line('web_name'), 'trim|required');
		$this->form_validation->set_rules('form[top_name]', $this->lang->line('top_name'), 'trim|required');
		$this->form_validation->set_rules('form[bottom_name]', $this->lang->line('bottom_name'), 'trim|required');
		$this->form_validation->set_rules('form[items_per_page]', $this->lang->line('items_per_page'), 'trim|required|integer|greater_than[0]');
		
		if ($this->form_validation->run())
		{
			$form = $this->input->post('form');

			foreach ($form as $k => $v)
				$this->setting->set($k, $v);
			$this->session->set_flashdata('edit_successful', true);

			redirect('admin/option/edit');
		}
		else
		{
			$this->ui['header']['title'] = $this->lang->line('edit_options');
			$this->ui['header']['page'] = 'option';
			
			$this->load->view('admin/header', $this->ui['header']);
			$this->load->view('admin/option/edit', $this->ui['content']);
			$this->load->view('footer', $this->ui['footer']);
		}
	}
}

/* End of file option.php */
/* Location: ./application/controllers/admin/option.php */
