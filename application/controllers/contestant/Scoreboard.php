<?php if ( ! defined('BASEPATH')) exit('No direct script access allowed');

/**
 * Controller for contestant scoreboard page
 *
 * This is the controller responsible for showing the contestant scoreboard page.
 *
 * @package controllers
 * @author Ashar Fuadi <fushar@gmail.com>
 */
class Scoreboard extends Contestant_Controller 
{
	/**
	 * Constructs a new controller object
	 *
	 * This function constructs a new controller.
	 */
	public function __construct()
	{
		parent::__construct();
		$this->load->model('scoreboard_manager');
		$this->load->language('scoreboard');
	}

	/**
	 * Shows the contestant scoreboard
	 *
	 * This is the default function for contestant Scoreboard controller. This function shows the scoreboard to be seen
	 * by the contestants; i.e., it takes account of the freeze and the unfreeze times.
	 */
	public function index()
	{
		$contest_id = $this->identity->get_current_contest_id();
		$contest = $this->contest_manager->get_row($contest_id);
		$res = $this->scoreboard_manager->get_scores('contestant', $contest_id);

		$this->ui['header']['page'] = 'scoreboard';
		$this->ui['header']['title'] = $this->lang->line('scoreboard');
		$this->ui['content']['scores'] = $res['scores'];
		$this->ui['content']['freeze_time'] = $contest['freeze_time'];
		$this->ui['content']['show_institution_logo'] = $contest['show_institution_logo'];
		$this->ui['content']['problems'] = $res['problems'];
		$this->ui['content']['frozen'] = (time() > strtotime($contest['freeze_time'])) && (time() <= strtotime($contest['unfreeze_time']));
		
		$this->load->view('contestant/header', $this->ui['header']);
		$this->load->view('contestant/scoreboard/index', $this->ui['content']);
		$this->load->view('scoreboard', $this->ui['content']);
		$this->load->view('footer', $this->ui['footer']);
	}
}

/* End of file scoreboard.php */
/* Location: ./application/controllers/admin/scoreboard.php */