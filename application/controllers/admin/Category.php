<?php if ( ! defined('BASEPATH')) exit('No direct script access allowed');

/**
 * Controller for admin category page
 *
 * This is the controller responsible for showing the admin category page.
 *
 * @package controllers
 * @author Ashar Fuadi <fushar@gmail.com>
 */
class Category extends Admin_Controller 
{
	/**
	 * Constructs a new controller object
	 *
	 * This function constructs a new controller.
	 */
	public function __construct()
	{
		parent::__construct();
		$this->load->model('category_manager');
		$this->load->library('pagination');
		$this->load->language('category');
	}

	/**
	 * Redirects to category viewAll page
	 *
	 * This is the default function for admin Category controller. This function redirects to category viewAll
	 * page.
	 */
	public function index()
	{
		redirect('admin/category/viewAll');
	}

	/**
	 * Shows all categories
	 *
	 * This function shows config('items_per_page') categories, starting from $page_offset-th page.
	 * 
	 * @param int $page_offset The page number to show.
	 */
	public function viewAll($page_offset = 1)
	{
		$this->ui['header']['page'] = 'category';
		$this->ui['header']['title'] = $this->lang->line('categories');

		$items_per_page = $this->setting->get('items_per_page');

		$conditions['limit'] = $items_per_page;
		$conditions['offset'] = ($page_offset-1) * $items_per_page;
		$conditions['order_by'] = array('id' => 'ASC');
		
		$categories = $this->category_manager->get_rows(array(), $conditions);
		foreach ($categories as &$v)
			$v['user_cnt'] = $this->category_manager->count_users($v['id']);

		$this->ui['content']['categories'] = $categories;
		$this->ui['content']['total_categories'] = $this->category_manager->count_rows();
		$this->ui['content']['items_per_page'] = $items_per_page;
		$this->ui['content']['page_offset'] = $page_offset;

		$config['base_url'] = site_url('admin/category/viewAll');
		$config['uri_segment'] = 4;
		$config['total_rows'] = $this->ui['content']['total_categories'];

		$this->pagination->initialize($config);
		$this->ui['content']['pager'] = $this->pagination->create_links();

		$this->load->view('admin/header', $this->ui['header']);
		$this->load->view('admin/category/viewAll', $this->ui['content']);
		$this->load->view('footer', $this->ui['footer']);
	}
	
	/**
	 * Shows the add or edit category page
	 *
	 * This function shows an add category page if $category_id is 0, otherwise it shows an edit category page.
	 * 
	 * @param int $category_id The category ID.
	 * @param int $page_offset The previous viewAll page number.
	 */
	public function edit($category_id, $page_offset)
	{
		$this->form_validation->set_rules('form[name]', $this->lang->line('name'), 'trim|required');
		
		if ($this->form_validation->run())
		{
			$form = $this->input->post('form');

			if ($category_id == 0)
			{
				$this->category_manager->insert_row($form);
				$this->session->set_flashdata('add_successful', true);
			}
			else
			{
				$this->category_manager->update_row($category_id, $form);
				$this->session->set_flashdata('edit_successful', true);
			}
			
			redirect('admin/category/viewAll');
		}
		else
		{
			if ($category_id == 0)
				$this->ui['header']['title'] = $this->lang->line('add_new_category');
			else
			{
				$this->ui['header']['title'] = $this->lang->line('edit_category') . ' ' . $category_id;
				$this->ui['content']['category'] = $this->category_manager->get_row($category_id);
			}
			$this->ui['header']['page'] = 'category';
			$this->ui['content']['page_offset'] = $page_offset;
			
			$this->load->view('admin/header', $this->ui['header']);
			$this->load->view('admin/category/edit', $this->ui['content']);
			$this->load->view('footer', $this->ui['footer']);
		}
	}

	/**
	 * Deletes a category
	 *
	 * This function deletes the category whose ID is $category_id and then redirects to viewAll page.
	 * 
	 * @param int $category_id The category ID.
	 * @param int $page_offset The previous viewAll page number.
	 */
	public function delete($category_id, $page_offset)
	{
		$user_cnt = $this->category_manager->count_users($category_id);
		if ($user_cnt > 0)
			$this->session->set_flashdata('error', $this->lang->line('category_has_users'));
		else
		{
			$this->category_manager->delete_row($category_id);
			$this->session->set_flashdata('delete_successful', true);
		}
		redirect('admin/category/viewAll');
	}
}

/* End of file category.php */
/* Location: ./application/controllers/admin/category.php */