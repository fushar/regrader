<?php if ( ! defined('BASEPATH')) exit('No direct script access allowed');

/**
 * Controller for public scoreboard page
 *
 * This is the controller responsible for showing the public scoreboard page.
 *
 * @package controllers
 * @author Ashar Fuadi <fushar@gmail.com>
 */
class Scoreboard extends MY_Controller 
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
		$this->load->model('scoreboard_manager');
		$this->load->language('scoreboard');
	}

	/**
	 * Shows the public scoreboard of a particular contest
	 *
	 * This function shows the public scoreboard of the contest whose ID is $contest_id. If that contest is not present
	 * or is not active, an 404 error page is shown instead.
	 * 
	 * @param int $contest_id The contest ID.
	 */
	public function view($contest_id)
	{
		$contest = $this->contest_manager->get_row($contest_id);
		if ( ! $contest)
			show_404();
		if ( ! $contest['enabled'])
			show_404();

		$res = $this->scoreboard_manager->get_scores('contestant', $contest_id);

		$this->ui['header']['page'] = 'scoreboard';
		$this->ui['header']['title'] = $this->lang->line('scoreboard');
		$this->ui['header']['top_name'] = $contest['name'];
		$this->ui['content']['scores'] = $res['scores'];
		$this->ui['content']['freeze_time'] = $contest['freeze_time'];
		$this->ui['content']['show_institution_logo'] = $contest['show_institution_logo'];
		$this->ui['content']['problems'] = $res['problems'];
		$this->ui['content']['frozen'] = (time() > strtotime($contest['freeze_time'])) && (time() <= strtotime($contest['unfreeze_time']));
		$this->ui['footer']['active_contest_end_time'] = $contest['end_time'];

		$this->load->view('site/header', $this->ui['header']);
		$this->load->view('site/scoreboard', $this->ui['content']);
		$this->load->view('scoreboard', $this->ui['content']);
		$this->load->view('footer', $this->ui['footer']);
	}

	/**
	 * Shows the raw public scoreboard of a particular contest
	 *
	 * This function shows the raw public scoreboard of the contest whose ID is $contest_id. All resources in the page 
	 * will use relative paths. If that contest is not present or is not active, an 404 error page is shown instead.
	 *
	 * This is useful for exporting the public scoreboard to another site.
	 * 
	 * @param int $contest_id The contest ID.
	 */
	public function rawview($contest_id)
	{
		$contest = $this->contest_manager->get_row($contest_id);
		if ( ! $contest)
			show_404();
		if ( ! $contest['enabled'])
			show_404();

		$res = $this->scoreboard_manager->get_scores('contestant', $contest_id);

		$this->ui['header']['page'] = 'scoreboard';
		$this->ui['header']['title'] = 'Peringkat';
		$this->ui['header']['top_name'] = $contest['name'];
		$this->ui['content']['raw'] = true;
		$this->ui['content']['scores'] = $res['scores'];
		$this->ui['content']['freeze_time'] = $contest['freeze_time'];
		$this->ui['content']['show_institution_logo'] = $contest['show_institution_logo'];
		$this->ui['content']['problems'] = $res['problems'];
		$this->ui['content']['frozen'] = (time() > strtotime($contest['freeze_time'])) && (time() <= strtotime($contest['unfreeze_time']));
		$this->ui['footer']['active_contest_end_time'] = $contest['end_time'];
		
		$this->load->view('site/raw_header', $this->ui['header']);
		$this->load->view('site/scoreboard', $this->ui['content']);
		$this->load->view('scoreboard', $this->ui['content']);
		$this->load->view('footer', $this->ui['footer']);
	}
}
/* End of file scoreboard.php */
/* Location: ./application/controllers/scoreboard.php */