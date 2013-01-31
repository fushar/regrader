<?php if ( ! defined('BASEPATH')) exit('No direct script access allowed');

/**
 * Base controller class for Grador project
 *
 * This is the common base class for all controllers in Grador project.
 *
 * @package core
 * @author Ashar Fuadi <fushar@gmail.com>
 */
class MY_Controller extends CI_Controller 
{
	/**
	 * The view data for each page
	 *
	 * This view data consists of the following pairs:
	 * - 'header'  => The view data for the page header an array of <name> => <value> pairs. Some required pairs:
	 *                - 'title' => The page title to be shown in the browser.
	 *                - 'name'  => The page name.
	 * - 'content' => The view data for the page content an array of <name> => <value> pairs.
	 * - 'footer'  => The view data for the page footer an array of <name> => <value> pairs.
	 * 
	 * @var array
	 */
	protected $ui;

	/**
	 * Constructs a new controller object
	 *
	 * This function constructs a new controller.
	 */
	public function __construct()
	{
		parent::__construct();
		date_default_timezone_set(@date_default_timezone_get());
		$this->load->language('common');

		$this->ui['header']['web_name'] = $this->setting->get('web_name');
		$this->ui['header']['top_name'] = $this->setting->get('top_name');
		$this->ui['header']['bottom_name'] = $this->setting->get('bottom_name');
		$this->ui['header']['left_logo'] = $this->setting->get('left_logo');
		$this->ui['header']['right_logo1'] = $this->setting->get('right_logo1');
		$this->ui['header']['right_logo2'] = $this->setting->get('right_logo2');
		$this->ui['content'] = array();
		$this->ui['footer'] = array();

	}
}

/* End of file MY_Controller.php */
/* Location: ./application/core/MY_Controller.php */