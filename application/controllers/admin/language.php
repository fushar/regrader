<?php if ( ! defined('BASEPATH')) exit('No direct script access allowed');

/**
 * Controller for admin language page
 *
 * This is the controller responsible for showing the admin language page.
 *
 * @package controllers
 * @author Ashar Fuadi <fushar@gmail.com>
 */
class Language extends Admin_Controller 
{
	/**
	 * Constructs a new controller object
	 *
	 * This function constructs a new controller.
	 */
	public function __construct()
	{
		parent::__construct();
		$this->load->model('language_manager');
		$this->load->library('pagination');
		$this->load->language('language');
	}

	/**
	 * Redirects to language viewAll page
	 *
	 * This is the default function for admin Language controller. This function redirects to language viewAll page.
	 */
	public function index()
	{
		redirect('admin/language/viewAll');
	}

	/**
	 * Shows all languages
	 *
	 * This function shows config('items_per_page') languages, starting from $page_offset-th page.
	 * 
	 * @param int $page_offset The page number to show.
	 */
	public function viewAll($page_offset = 1)
	{
		$this->ui['header']['page'] = 'language';
		$this->ui['header']['title'] = $this->lang->line('languages');

		$items_per_page = $this->setting->get('items_per_page');

		$conditions['limit'] = $items_per_page;
		$conditions['offset'] = ($page_offset-1) * $items_per_page;
		$conditions['order_by'] = array('id' => 'ASC');
		
		$this->ui['content']['languages'] = $this->language_manager->get_rows(array(), $conditions);
		$this->ui['content']['total_languages'] = $this->language_manager->count_rows();
		$this->ui['content']['items_per_page'] = $items_per_page;
		$this->ui['content']['page_offset'] = $page_offset;

		$config['base_url'] = site_url('admin/language/viewAll');
		$config['uri_segment'] = 4;
		$config['total_rows'] = $this->ui['content']['total_languages'];

		$this->pagination->initialize($config);
		$this->ui['content']['pager'] = $this->pagination->create_links();

		$this->load->view('admin/header', $this->ui['header']);
		$this->load->view('admin/language/viewAll', $this->ui['content']);
		$this->load->view('footer', $this->ui['footer']);
	}

	/**
	 * Shows the add or edit language page
	 *
	 * This function shows an add language page if $language_id is 0, otherwise it shows an edit language page.
	 * 
	 * @param int $language_id The language ID.
	 * @param int $page_offset The previous viewAll page number.
	 */
	public function edit($language_id, $page_offset)
	{
		$this->form_validation->set_rules('form[name]', $this->lang->line('name'), 'trim|required');
		$this->form_validation->set_rules('form[extension]', $this->lang->line('extension'), 'trim|required');
		$this->form_validation->set_rules('form[source_name]', $this->lang->line('source_name'), 'trim|required');
		$this->form_validation->set_rules('form[exe_name]', $this->lang->line('exe_name'), 'trim|required');
		$this->form_validation->set_rules('form[compile_cmd]', $this->lang->line('compile_cmd'), 'trim|required');
		$this->form_validation->set_rules('form[run_cmd]', $this->lang->line('run_cmd'), 'trim|required');
		
		if ($this->form_validation->run())
		{
			$form = $this->input->post('form');

			if ($language_id == 0)
			{
				$this->language_manager->insert_row($form);
				$this->session->set_flashdata('add_successful', true);
			}
			else
			{
				$this->language_manager->update_row($language_id, $form);
				$this->session->set_flashdata('edit_successful', true);
			}
			
			redirect('admin/language/viewAll');
		}
		else
		{
			if ($language_id == 0)
				$this->ui['header']['title'] = $this->lang->line('add_new_language');
			else
			{
				$this->ui['header']['title'] = $this->lang->line('edit_language') . ' ' . $language_id;
				$this->ui['content']['language'] = $this->language_manager->get_row($language_id);
			}
			$this->ui['header']['page'] = 'language';
			$this->ui['content']['page_offset'] = $page_offset;
			
			$this->load->view('admin/header', $this->ui['header']);
			$this->load->view('admin/language/edit', $this->ui['content']);
			$this->load->view('footer', $this->ui['footer']);
		}
	}

	/**
	 * Deletes a language
	 *
	 * This function deletes the language whose ID is $language_id and then redirects to viewAll page.
	 * 
	 * @param int $language_id The language ID.
	 * @param int $page_offset The previous viewAll page number.
	 */
	public function delete($language_id, $page_offset)
	{
		$this->language_manager->delete_row($language_id);
		$this->session->set_flashdata('delete_successful', true);
		redirect('admin/language/viewAll/' . $page_offset);
	}
}

/* End of file language.php */
/* Location: ./application/controllers/admin/language.php */