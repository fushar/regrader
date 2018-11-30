<?php if ( ! defined('BASEPATH')) exit('No direct script access allowed');

/**
 * Model class that wraps table submission
 *
 * This is the active record class wrapper for table 'submission' from the database.
 * 
 * @package models
 * @author  Ashar Fuadi <fushar@gmail.com>
 */
class Submission_manager extends AR_Model 
{
	/**
	 * Constructs a new model object
	 *
	 * This function constructs a new model.
	 */
	public function __construct()
	{
		parent::__construct('submission');
	}

	/**
	 * Retrieves an existing row from table 'submission'
	 *
	 * This function returns a single row whose ID is $id from the wrapped table. If there is no such row, FALSE will be
	 * returned instead.
	 *
	 * This function overrides AR_Model::get_row().
	 * 
	 * @param  int $clarification_id The row ID to be retrieved.
	 * 
	 * @return mixed            	 The row whose ID is $clarification_id as an array of <attribute-name> => 
	 * 							 	 <attribute-value> pairs, or FALSE if there is no such row.
	 */
	public function get_row($submission_id)
	{
		$this->db->select('submission.id AS id, user_id, submission.contest_id, submission.problem_id, language_id, submit_time, start_judge_time, end_judge_time, verdict,
			               user.name AS user_name, contest.name AS contest_name, problem.name AS problem_name, language.name AS language_name, language.source_name AS language_source_name,
			               alias AS problem_alias');
		$this->db->from('submission');
		$this->db->join('contest', 'contest.id=contest_id');
		$this->db->join('problem', 'problem.id=problem_id');
		$this->db->join('user', 'user.id=user_id');
		$this->db->join('language', 'language.id=language_id');
		$this->db->join('contest_problem', 'contest_problem.contest_id=submission.contest_id AND contest_problem.problem_id=submission.problem_id');
		$this->db->where('submission.id', $submission_id);
		$this->db->limit(1);
		$q = $this->db->get();
		if ($q->num_rows() == 0)
			return FALSE;
		$res = $q->row_array();
		$res['source_code'] = file_get_contents($this->setting->get('submission_path') . '/' . $submission_id . '/source/' . $res['language_source_name']);
		if ($res['verdict'] > 0)
			$res['compile_result'] = file_get_contents($this->setting->get('submission_path') . '/' . $submission_id . '/source/compile');
		return $res;
	}

	/**
	 * Retrieves zero or more rows from table 'submission' satisfying some criteria
	 *
	 * This function returns zero or more rows from the wrapped table that satisfy the given criteria. All attributes
	 * are returned.
	 *
	 * This function overrides AR_Model::get_rows().
	 * 
	 * @param  array $criteria   An array of <attribute-name> => <attribute-value> pairs. Only rows that satisfy all
	 *                           criteria will be retrieved.
	 * @param  array $conditions An array of zero or more of the following pairs:
	 *                           - 'limit'    => The maximum number of rows to be retrieved.
	 *                           - 'offset'   => The starting number of the rows to be retrieved.
	 *                           - 'order_by' => The resulting rows order as an array of zero or more
	 *                                           <attribute-name> => ('ASC' | 'DESC') pairs.
	 * 
	 * @return array             Zero or more rows that satisfies the criteria as an array of <row-ID> => <row>.
	 */
	public function get_rows($criteria = array(), $conditions = array())
	{
		$this->db->select('submission.id AS id, user_id, submission.contest_id, submission.problem_id, language_id, submit_time, start_judge_time, end_judge_time, verdict,
			               user.name AS user_name, contest.name AS contest_name, problem.name AS problem_name, language.name AS language_name,
			               alias AS problem_alias');
		$this->db->join('contest', 'contest.id=contest_id');
		$this->db->join('problem', 'problem.id=problem_id');
		$this->db->join('user', 'user.id=user_id');
		$this->db->join('language', 'language.id=language_id');
		$this->db->join('contest_problem', 'contest_problem.contest_id=submission.contest_id AND contest_problem.problem_id=submission.problem_id');

		return parent::get_rows($criteria, $conditions);
	}

	/**
	 * Inserts a new row into the table
	 *
	 * This function inserts a new row described by $attributes into the wrapped table. If a certain attribute is not
	 * present in $attributes, its default value will be inserted instead.
	 *
	 * This function overrides AR_Model::insert_row(). It will also create the directory for the judging.
	 * 
	 * @param  array $args The row to be inserted as an array of <attribute name> => <attribute value> pairs.
	 * 
	 * @return int The newly inserted row ID.
	 */
	public function insert_row($args)
	{
		$this->load->model('language_manager');

		$this->db->set('user_id', $args['user_id']);
		$this->db->set('contest_id', $args['contest_id']);
		$this->db->set('problem_id', $args['problem_id']);
		$this->db->set('language_id', $args['language_id']);
		$this->db->set('submit_time', date('Y-m-d H:i:s'));
		$this->db->set('start_judge_time', '1000-01-01 00:00:00');
		$this->db->set('end_judge_time', '1000-01-01 00:00:00');

		// temporarily sets the verdict to 99 so that the grader will not judge this submission yet
		$this->db->set('verdict', 99);
		
		$insert_id = $this->db->insert('submission');
		$submission_id = $this->db->insert_id();

		$submission_path = $this->setting->get('submission_path') . '/' . $submission_id;

		// create the directories
		if ( ! is_dir($submission_path))
		{
			mkdir($submission_path);
			chmod($submission_path, 0777);

			mkdir($submission_path . '/source');
			chmod($submission_path . '/source', 0777);

			mkdir($submission_path . '/judging');
			chmod($submission_path . '/judging', 0777);
		}

		$language = $this->language_manager->get_row($args['language_id']);

		$config['upload_path'] = $submission_path . '/source';
		$config['allowed_types'] = '*';
		$config['file_name'] = $language['source_name'];
		$config['max_size']	= '100';
		$config['overwrite'] = true;
		
		// uploads the source code
		$this->load->library('upload', $config);
		$this->upload->do_upload('source_code');

		$this->db->set('verdict', 0);
		$this->db->where('id', $submission_id);
		$this->db->update('submission');

		return $insert_id;
	}

	/**
	 * Retrieves the judge results for a particular submission
	 *
	 * This function returns the judge result against all testcases for the submission whose ID is $submission_id.
	 * 
	 * @param  int $submission_id The submission ID.
	 * 
	 * @return array The judge results.
	 */
	public function get_judgings($submission_id)
	{
		$this->db->select('testcase_id, input, output, time, memory, verdict');
		$this->db->from('judging');
		$this->db->join('testcase', 'testcase.id=testcase_id');
		$this->db->where('submission_id', $submission_id);
		$q = $this->db->get();
		return $q->result_array();
	}

	/**
	 * Marks a particular submission to be regraded
	 *
	 * This function marks the submission whose ID is $submission_id to be regraded, by negating the current verdict.
	 * 
	 * @param  int $submission_id The submission ID.
	 */
	public function regrade($submission_id)
	{
		$this->db->where('submission_id', $submission_id);
		$this->db->delete('judging');
		
		$this->db->select('verdict');
		$this->db->from('submission');
		$this->db->where('id', $submission_id);
		$q = $this->db->get();

		$res = $q->row_array();
		$new_verdict = $res['verdict'];
		if ($res['verdict'] == 99)
			$new_verdict = 0;
		$this->db->set('verdict', - $new_verdict);
		$this->db->where('id', $submission_id);
		$this->db->update('submission');
	}

	/**
	 * Marks a particular submission to be ignored
	 *
	 * This function marks the submission whose ID is $submission_id to be ignored, by setting the verdict to -99.
	 * 
	 * @param  int $submission_id The submission ID.
	 */
	public function ignore($submission_id)
	{
		$this->db->set('verdict', -99);
		$this->db->where('id', $submission_id);
		$this->db->update('submission');
	}
}

/* End of file submission_manager.php */
/* Location: ./application/models/submission_manager.php */