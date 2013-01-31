<?php if ( ! defined('BASEPATH')) exit('No direct script access allowed');

/**
 * Model for scoreboard management
 *
 * This model manages the scores of the scoreboard.
 *
 * @package models
 * @author  Ashar Fuadi <fushar@gmail.com>
 */
class Scoreboard_manager extends CI_Model 
{
	/**
	 * Adds a particular submission to the scoreboard
	 *
	 * This function adds the submission whose ID is $submission_id to either contestant scoreboard or admin scoreboard,
	 * depending on the submission time.
	 *
	 * @param int $submission_id The submission ID.
	 */
	public function add_submission($submission_id)
	{
		$submission = $this->submission_manager->get_row($submission_id);
		$contest = $this->contest_manager->get_row($submission['contest_id']);

		if (strtotime($submission['submit_time']) <= strtotime($contest['freeze_time']))
			$this->add_submission_helper('contestant', $submission_id);
		$this->add_submission_helper('admin', $submission_id);
	}

	/**
	 * Adds a particular submission to the scoreboard
	 *
	 * This function adds the submission whose ID is $submission_id to $target scoreboard.
	 *
	 * @param string $target 	 The target scoreboard.
	 * @param int $submission_id The submission ID.
	 */
	private function add_submission_helper($target, $submission_id)
	{
		$submission = $this->submission_manager->get_row($submission_id);
		$contest = $this->contest_manager->get_row($submission['contest_id']);

		$res['contest_id'] = $submission['contest_id'];
		$res['user_id'] = $submission['user_id'];
		$res['problem_id'] = $submission['problem_id']; 
		$res['submission_cnt'] = 0;
		$res['time_penalty'] = 0;
		$res['is_accepted'] = 0;

		// checks whether there has been an entry in the target scoreboard
		$this->db->from('scoreboard_' . $target);
		$this->db->where('contest_id', $submission['contest_id']);
		$this->db->where('user_id', $submission['user_id']);
		$this->db->where('problem_id', $submission['problem_id']);
		$this->db->limit(1);
		$q = $this->db->get();

		$is_present = FALSE;
		if ($q->num_rows() == 1)
		{
			// there is an entry
			$score = $q->row_array();
			$res['submission_cnt'] = $score['submission_cnt'];
			$res['time_penalty'] = $score['time_penalty'];
			$res['is_accepted'] = $score['is_accepted'];

			// all submissions after the first AC verdict will be ignored
			if ($res['is_accepted'] == 1)
				return;
			$is_present = TRUE;
		}

		$res['submission_cnt']++;
		$res['time_penalty'] = ceil((strtotime($submission['submit_time']) - strtotime($contest['start_time'])) / 60);

		// an AC submission
		if ($submission['verdict'] == 2)
			$res['is_accepted'] = 1;

		if ($is_present)
		{
			// if there has been an entry, update it
			$this->db->where('contest_id', $res['contest_id']);
			$this->db->where('user_id', $res['user_id']);
			$this->db->where('problem_id', $res['problem_id']);
			$this->db->set('submission_cnt', $res['submission_cnt']);
			$this->db->set('time_penalty', $res['time_penalty']);
			$this->db->set('is_accepted', $res['is_accepted']);
			$this->db->update('scoreboard_' . $target);
		}
		else
		{
			// else, insert it
			foreach ($res as $k => $v)
				$this->db->set($k, $v);
			$this->db->insert('scoreboard_' . $target);
		}
	}

	/**
	 * Recalculates the scores in all scoreboards
	 *
	 * This function reconsiders alls submissions and updates all scoreboards. The criteria is given in $args. Usually
	 * used when there is a regrade.
	 *
	 * @param array $args The parameters.
	 */
	public function recalculate_scores($args)
	{
		// removes all scores
		$this->db->where('contest_id', $args['contest_id']);
		if (isset($args['user_id']))
			$this->db->where('user_id', $args['user_id']);
		if (isset($args['problem_id']))
			$this->db->where('problem_id', $args['problem_id']);
		$this->db->delete('scoreboard_contestant');

		$this->db->where('contest_id', $args['contest_id']);
		if (isset($args['user_id']))
			$this->db->where('user_id', $args['user_id']);
		if (isset($args['problem_id']))
			$this->db->where('problem_id', $args['problem_id']);
		$this->db->delete('scoreboard_admin');

		// retrieves all submissions
		$this->db->select('id, verdict');
		$this->db->from('submission');
		$this->db->where('contest_id', $args['contest_id']);
		if (isset($args['user_id']))
			$this->db->where('user_id', $args['user_id']);
		if (isset($args['problem_id']))
			$this->db->where('problem_id', $args['problem_id']);
		$q = $this->db->get();
		$res = $q->result_array();

		// readd the submissions
		foreach ($res as $v)
			// if the submission is not ignored
			if ($v['verdict'] != 99)
				$this->add_submission($v['id']);
	}

	/**
	 * Retrieves the scores of a particular scoreboard of a particular contest
	 *
	 * This function returns the scores of $target scoreboard of the contest whose ID is $contest_id.
	 *
	 * @param string $target 	The target scoreboard.
	 * @param int $contest_id 	The contest ID.
	 */
	public function get_scores($target, $contest_id)
	{
		$contest = $this->contest_manager->get_row($contest_id);

		// if the current time is greater than the unfreeze time, change target to admin.
		if ($target == 'contestant' && time() > strtotime($contest['unfreeze_time']))
			$target = 'admin';

		$this->db->select('user.id, user.name, user.username, user.institution');
		$this->db->from('user');
		$this->db->join('contest_member', 'contest_member.category_id=user.category_id');
		$this->db->where('contest_member.contest_id', $contest_id);
		$q = $this->db->get();
		$users = $q->result_array();

		$this->db->select('problem.id, problem.name, alias');
		$this->db->from('problem');
		$this->db->join('contest_problem', 'contest_problem.problem_id=problem.id');
		$this->db->where('contest_problem.contest_id', $contest_id);
		$this->db->order_by('alias');
		$q = $this->db->get();
		$res['problems'] = $q->result_array();
		$res['scores'] = array();

		$this->db->from('scoreboard_' . $target);
		$this->db->where('contest_id', $contest_id);
		$q = $this->db->get();
		$scores = $q->result_array();

		foreach ($users as $v) 
		{
			$res['scores'][$v['id']]['name'] = $v['name'];
			$res['scores'][$v['id']]['username'] = $v['username'];
			$res['scores'][$v['id']]['institution'] = $v['institution'];
			$res['scores'][$v['id']]['total_accepted'] = 0;
			$res['scores'][$v['id']]['total_penalty'] = 0;
			$res['scores'][$v['id']]['last_submission'] = 0;
			$res['scores'][$v['id']]['score'] = array();

			foreach ($res['problems'] as $w)
			{
				$res['scores'][$v['id']]['score'][$w['id']]['submission_cnt'] = 0;
				$res['scores'][$v['id']]['score'][$w['id']]['time_penalty'] = 0;
				$res['scores'][$v['id']]['score'][$w['id']]['is_accepted'] = 0;
			}
		}

		foreach ($scores as $v)
		{
			if (isset($res['scores'][$v['user_id']]['score'][$v['problem_id']]))
			{
				$res['scores'][$v['user_id']]['score'][$v['problem_id']]['submission_cnt'] = $v['submission_cnt'];
				$res['scores'][$v['user_id']]['score'][$v['problem_id']]['time_penalty'] = $v['time_penalty'];
				$res['scores'][$v['user_id']]['score'][$v['problem_id']]['is_accepted'] = $v['is_accepted'];

				$res['scores'][$v['user_id']]['total_accepted'] += $v['is_accepted'];

				if ($v['is_accepted'])
				{
					$res['scores'][$v['user_id']]['total_penalty'] += $v['time_penalty'] + 20 * ($v['submission_cnt'] - 1);
				
					if ($res['scores'][$v['user_id']]['last_submission'] < $v['time_penalty'])
						$res['scores'][$v['user_id']]['last_submission'] = $v['time_penalty'];
				}
			}
		}

		usort($res['scores'], array('Scoreboard_manager', 'score_cmp'));
		return $res;
	}

	/**
	 * The comparison function of the scores
	 *
	 * This function returns the relative ordering of the user whose ID is $a and the user whose ID is $b.
	 *
	 * @param int $a A user ID.
	 * @param int $b Another user ID.
	 *
	 * @return int The ordering.
	 */
	private function score_cmp($a, $b)
	{
		if ($a['total_accepted'] != $b['total_accepted'])
			return $b['total_accepted'] - $a['total_accepted'];

		if ($a['total_penalty'] != $b['total_penalty'])
			return $a['total_penalty'] - $b['total_penalty'];

		if ($a['last_submission'] != $b['last_submission'])
			return $a['last_submission'] - $b['last_submission'];

		if ($a['name'] == $b['name'])
			return 0;

		return $a['name'] < $b['name'] ? -1 : 1;
	}
}

/* End of file scoreboard_manager.php */
/* Location: ./application/models/scoreboard_manager.php */