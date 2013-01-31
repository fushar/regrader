<?php if ( ! defined('BASEPATH')) exit('No direct script access allowed');

/**
 * Controller for admin category page
 *
 * This is the controller responsible for showing the admin category page.
 *
 * @package controllers
 * @author Ashar Fuadi <fushar@gmail.com>
 */
class File extends Admin_Controller 
{
	/**
	 * Constructs a new controller object
	 *
	 * This function constructs a new controller.
	 */
	public function __construct()
	{
		parent::__construct();
		$this->load->model('public_file_manager');
		$this->load->language('public_file');
	}

	/**
	 * Redirects to file viewAll page
	 *
	 * This is the default function for admin File controller. This function redirects to file viewAll page.
	 */
	public function index()
	{
		redirect('admin/file/viewAll');
	}

	/**
	 * Shows all public files
	 *
	 * This function shows all public files.
	 */
	public function viewAll()
	{
		$this->ui['header']['page'] = 'file';
		$this->ui['header']['title'] = $this->lang->line('files');

		$this->ui['content']['files'] = $this->public_file_manager->get_public_files();
		
		$this->load->view('admin/header', $this->ui['header']);
		$this->load->view('admin/file/viewAll', $this->ui['content']);
		$this->load->view('footer', $this->ui['footer']);
	}

	/**
	 * Shows add file page
	 *
	 * This function shows the add new file page.
	 */
	public function add()
	{
		$this->form_validation->set_rules('hidden', 'Hidden', 'required');
		$this->form_validation->set_rules('userfile', $this->lang->line('file'), 'callback__check_userfile');

		if ($this->form_validation->run())
		{
			$config['upload_path'] = 'files';
			$config['allowed_types'] = '*';
			$config['remove_spaces'] = FALSE;
			$config['overwrite'] = TRUE;
			
			$this->load->library('upload', $config);
			$this->upload->do_upload('userfile');

			$this->session->set_flashdata('add_successful', TRUE);
			redirect('admin/file/viewAll');
		}
		else
		{
			$this->ui['header']['page'] = 'file';
			$this->ui['header']['title'] = $this->lang->line('add_new_file');

			$this->load->view('admin/header', $this->ui['header']);
			$this->load->view('admin/file/add', $this->ui['content']);
			$this->load->view('footer', $this->ui['footer']);
		}
	}

	/**
	 * Checks whether a new file is supplied
	 *
	 * This function checks whether the user has chosen a file to add.
	 *
	 * @return boolean TRUE if a new file is supplied, or FALSE otherwise.
	 */
	public function _check_userfile()
	{
		if (empty($_FILES['userfile']['name']))
		{
			$this->form_validation->set_message('_check_userfile', $this->lang->line('file_required'));
			return FALSE;
		}
		return TRUE;
	}

	/**
	 * Deletes a file
	 *
	 * This function deletes file $file and redirects to file viewAll page.
	 *
	 * @param string $file The filename to delete.
	 */
	public function delete($file)
	{
		unlink('files/' . rawurldecode($file));
		$this->session->set_flashdata('delete_successful', TRUE);
		redirect('admin/file/viewAll');
	}
}

/* End of file public_file_manager.php */
/* Location: ./application/controllers/admin/public_file_manager.php */