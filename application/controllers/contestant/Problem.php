<?php if ( ! defined('BASEPATH')) exit('No direct script access allowed');

/**
 * Controller for contestant problem page
 *
 * This is the controller responsible for showing the contestant problem page.
 *
 * @package controllers
 * @author Ashar Fuadi <fushar@gmail.com>
 */
class Problem extends Contestant_Controller 
{
	/**
	 * Constructs a new controller object
	 *
	 * This function constructs a new controller.
	 */
	public function __construct()
	{
		parent::__construct();
		$this->load->model('problem_manager');
		$this->load->model('language_manager');
		$this->load->model('submission_manager');
		$this->load->model('scoreboard_manager');
		$this->load->language('problem');
	}

	/**
	 * Shows the first problem assigned in the current contest
	 *
	 * This is the default action for contestant Problem controller. This function shows the first problem assigned in
	 * the current contest, or an empty page if the current contest does not have any problems.
	 */
	public function index()
	{
		$contest_id = $this->identity->get_current_contest_id();
		$problems = $this->problem_manager->get_rows(array('contest_id' => $contest_id), array('order_by' => array('alias' => 'ASC')));

		if (empty($problems))
			redirect('contestant/problem/view/0');
		else
		{
			$problem = reset($problems);
			redirect('contestant/problem/view/' . $problem['id']);
		}
	}

	/**
	 * Shows the problem page
	 *
	 * This function shows the problem whose ID is $problem_id. If that problem is not assigned to the current contest,
	 * 404 error page is shown instead.
	 * 
	 * @param int $problem_id The problem ID. If $problem_id == 0, no problem is shown.
	 */
	public function view($problem_id = 0)
	{
		$contest_id = $this->identity->get_current_contest_id();
		if ($problem_id != 0 && ! $this->contest_manager->has_problem($contest_id, $problem_id))
			show_404();

		$this->form_validation->set_rules('form[language_id]', $this->lang->line('language'), 'required');

		if ($this->form_validation->run())
		{
			$form = $this->input->post('form');
			if (empty($_FILES['source_code']['name']))
			{
				$this->session->set_flashdata('submit_error', true);
				$this->session->set_flashdata('missing_source_code', true);
				redirect('contestant/problem/view/' . $problem_id);
			}
			else if (strtotime($this->ui['header']['active_contest_end_time']) < time())
			{
				$this->session->set_flashdata('submit_error', true);
				$this->session->set_flashdata('contest_has_ended', true);
				redirect('contestant/problem/view/' . $problem_id);
			}
			else
			{
				$args['user_id'] = $this->identity->get_user_id();
				$args['contest_id'] = $contest_id;
				$args['problem_id'] = $problem_id;
				$args['language_id'] = $form['language_id'];
				$this->submission_manager->insert_row($args);

				$this->session->set_flashdata('submit_successful', true);
				redirect('contestant/submission/viewAll');
			}
		}
		else
		{
			$this->ui['header']['page'] = 'problem';

			$script = <<<'SCRIPTSTART'
<script type="text/x-mathjax-config">
	MathJax.Hub.Config({
		extensions: ["tex2jax.js"],
		jax: ["input/TeX", "output/HTML-CSS"],
		tex2jax: {
			inlineMath: [ ['$','$']],
			displayMath: [ ['$$','$$'], ["\\[","\\]"] ],
			processEscapes: true
		},
		"HTML-CSS": { availableFonts: ["TeX"] }
	});
</script>
SCRIPTSTART;
			$this->ui['header']['pre_custom_js_script'] = $script;
			$this->ui['header']['custom_js'] = array('vendor/MathJax/MathJax.js');

			$problem = $this->problem_manager->get_row($problem_id);
			$problems = $this->problem_manager->get_rows(array('contest_id' => $contest_id), array('order_by' => array('alias' => 'ASC')));

			foreach ($problems as $v)
				if ($v['id'] == $problem_id)
					$problem['alias'] = $v['alias'];
			
			$this->ui['content']['problem'] = $problem;
			$this->ui['content']['problems'] = $problems;
			$this->ui['content']['languages'] = $this->language_manager->get_rows(array('contest_id' => $contest_id));

			if ($problem_id == 0)
			{
				$this->ui['header']['title'] = $this->lang->line('problem');
				$this->load->view('contestant/header', $this->ui['header']);
				$this->load->view('contestant/problem/view_no_problem', $this->ui['content']);
			}
			else
			{
				$this->ui['header']['title'] = $problem['alias'];
				$this->load->view('contestant/header', $this->ui['header']);
				$this->load->view('contestant/problem/view', $this->ui['content']);
			}
			$this->load->view('footer', $this->ui['footer']);
		}
	}
}

/* End of file problem.php */
/* Location: ./application/controllers/contestant/problem.php */