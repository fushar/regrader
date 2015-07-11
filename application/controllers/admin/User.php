<?php if ( ! defined('BASEPATH')) exit('No direct script access allowed');

/**
 * Controller for admin user page
 *
 * This is the controller responsible for showing the admin user page.
 *
 * @package controllers
 * @author Ashar Fuadi <fushar@gmail.com>
 */
class User extends Admin_Controller 
{
	/**
	 * Constructs a new controller object
	 *
	 * This function constructs a new controller.
	 */
	public function __construct()
	{
		parent::__construct();
		$this->load->model('user_manager');
		$this->load->model('category_manager');
		$this->load->library('pagination');
		$this->load->language('user');
	}

	/**
	 * Redirects to user viewAll page
	 *
	 * This is the default function for admin User controller. This function redirects to user viewAll page.
	 */
	public function index()
	{
		redirect('admin/user/viewAll/0/1');
	}

	/**
	 * Shows all users in a particular category
	 *
	 * This function shows config('items_per_page') users, starting from $page_offset-th page.
	 * 
	 * @param int $category_id The category ID, or 0 to select all categories.
	 * @param int $page_offset The page number to show.
	 */
	public function viewAll($category_id = 0, $page_offset = 1)
	{
		if ($this->input->post(NULL))
		{
			$category_id = $this->input->post('category_id');
			redirect('admin/user/viewAll/' . $category_id . '/' . $page_offset);
		}

		$this->ui['header']['page'] = 'user';
		$this->ui['header']['title'] = $this->lang->line('users');

		$items_per_page = $this->setting->get('items_per_page');

		$criteria = array();
		if ($category_id > 0)
			$criteria['user.category_id'] = $category_id;

		$conditions['limit'] = $items_per_page;
		$conditions['offset'] = ($page_offset-1) * $items_per_page;
		$conditions['order_by'] = array('category_id' => 'ASC', 'id' => 'ASC');

		$users = $this->user_manager->get_rows($criteria, $conditions);
		$categories = $this->category_manager->get_rows();
		
		$this->ui['content']['users'] = $users;
		$this->ui['content']['categories'] = $categories;
		$this->ui['content']['total_users'] = $this->user_manager->count_rows($criteria);
		$this->ui['content']['items_per_page'] = $items_per_page;
		$this->ui['content']['page_offset'] = $page_offset;

		$config['base_url'] = site_url('admin/user/viewAll/' . $category_id);
		$config['uri_segment'] = 5;
		$config['total_rows'] = $this->ui['content']['total_users'];

		$this->pagination->initialize($config);
		$this->ui['content']['pager'] = $this->pagination->create_links();
		$this->ui['content']['category_id'] = $category_id;

		$this->load->view('admin/header', $this->ui['header']);
		$this->load->view('admin/user/viewAll', $this->ui['content']);
		$this->load->view('footer', $this->ui['footer']);
	}

	/**
	 * Shows the add or edit user page
	 *
	 * This function shows an add user page if $user_id is 0, otherwise it shows an edit user page.
	 * 
	 * @param int $user_id The user ID.
	 * @param int $category_id The previous viewAll category ID.
	 * @param int $page_offset The previous viewAll page number.
	 */
	public function edit($user_id, $category_id, $page_offset)
	{
		$this->form_validation->set_rules('form[name]', $this->lang->line('name'), 'trim|required');
		$this->form_validation->set_rules('form[username]', $this->lang->line('username'), 'trim|required|callback_check_username[' . $user_id .']');
		$this->form_validation->set_rules('form[password]', $this->lang->line('password'));
		$this->form_validation->set_rules('form[institution]', $this->lang->line('institution'), 'trim|required');
		$this->form_validation->set_rules('form[category_id]', $this->lang->line('category'), 'required');


		if ($this->form_validation->run())
		{
			$form = $this->input->post('form');
			if ( ! empty($form['password']))
				$form['password'] = md5($form['password']);
			else
				unset($form['password']);

			if ($user_id == 0)
			{
				$this->user_manager->insert_row($form);
				$this->session->set_flashdata('add_successful', true);
			}
			else
			{
				$this->user_manager->update_row($user_id, $form);
				$this->session->set_flashdata('edit_successful', true);
			}
			
			redirect('admin/user/viewAll/' . $category_id . '/' . $page_offset);
		}
		else
		{
			if ($user_id == 0)
				$this->ui['header']['title'] = $this->lang->line('add_new_user');
			else
			{
				$this->ui['header']['title'] = $this->lang->line('edit_user') . ' ' . $user_id;
				$this->ui['content']['user'] = $this->user_manager->get_row($user_id);
			}
			$this->ui['header']['page'] = 'user';

			if ($this->identity->is_admin())
				$this->ui['content']['categories'] = $this->category_manager->get_rows(array('id >=' => 1));
			else
				$this->ui['content']['categories'] = $this->category_manager->get_rows(array('id >' => 1));

			$this->ui['content']['page_offset'] = $page_offset;
			$this->ui['content']['category_id'] = $category_id;

			$this->load->view('admin/header', $this->ui['header']);
			$this->load->view('admin/user/edit', $this->ui['content']);
			$this->load->view('footer', $this->ui['footer']);
		}
	}
 
 	/**
	 * Deletes a user
	 *
	 * This function deletes the user whose ID is $user_id and then redirects to viewAll page.
	 * 
	 * @param int $user_id The user ID.
	 * @param int $category_id The previous viewAll category ID.
	 * @param int $page_offset The previous viewAll page number.
	 */
	public function delete($user_id, $category_id, $page_offset)
	{
		$this->user_manager->delete_row($user_id);
		$this->session->set_flashdata('delete_successful', true);
		redirect('admin/user/viewAll/' . $category_id . '/' . $page_offset);
	}

	/**
	 * Checks whether a username is valid
	 *
	 * This function returns whether $username has been used before for user other than $user_id.
	 * 
	 * @param int $user_id		The user ID.
	 * @param string $username 	The username.
	 *
	 * @return boolean TRUE if $username has not been used before, or FALSE otherwise.
	 */
	public function check_username($username, $user_id)
	{
		if ( ! $this->user_manager->is_valid_new_username($user_id, $username))
		{
			$this->form_validation->set_message('check_username', $this->lang->line('username_used'));
			return FALSE;
		}
		return TRUE;
	}
}

/* End of file user.php */
/* Location: ./application/controllers/admin/user.php */
