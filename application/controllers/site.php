<?php if ( ! defined('BASEPATH')) exit('No direct script access allowed');

/**
 * Controller for front page
 *
 * This is the main controller of the site. It is mainly responsible for showing the login page.
 *
 * @package controllers
 * @author Ashar Fuadi <fushar@gmail.com>
 */
class Site extends MY_Controller 
{
	public function __construct()
	{
		parent::__construct();
		$this->load->language('site');
	}

	/**
	 * Redirects user depending on the role
	 * 
	 * This is the default action for this controller. If the user has already logged in, the user will be redirected to
	 * the contestant/administrator dashboard. If not, the user will be redirected to the login page instead.
	 */
	public function index()
	{
		if ( ! $this->setting->get('sess_id'))
			$this->install();
		else if ($this->identity->is_contestant())
			redirect('contestant/dashboard');
		else if ($this->identity->is_admin())
			redirect('admin/dashboard');
		else
			redirect('site/login');
	}

	/**
	 * Shows the login page
	 *
	 * This function shows the login page. If the user has already logged in, the user will be redirected to the
	 * contestant/administrator dashboard instead.
	 */
	public function login()
	{
		if ($this->identity->is_admin())
			redirect('admin/dashboard');
		if ($this->identity->is_contestant())
			redirect('contestant/dashboard');
		
		$this->form_validation->set_rules('form[username]', $this->lang->line('username'), 'trim|required|max_length[50]');
		$this->form_validation->set_rules('form[password]', $this->lang->line('password'), 'trim|required|max_length[50]');

		if ($this->form_validation->run())
		{
			$credentials = $this->input->post('form');
			
			if ($this->identity->login($credentials))
			{
				if ($this->identity->is_admin())
					redirect('admin/dashboard');
				else
					redirect('contestant/dashboard');
			}
			else
			{
				$this->session->set_flashdata('error', $this->lang->line('wrong_credentials'));
				redirect('site/login');
			}
		}
		else
		{
			$this->ui['header']['title'] = $this->lang->line('login');
			$this->ui['header']['page'] = 'login';
			$this->load->view('site/header', $this->ui['header']);
			$this->load->view('site/login', $this->ui['content']);
			$this->load->view('footer', $this->ui['footer']);
		}
	}

	/**
	 * Logs out the current user
	 *
	 * This function logs out the current user and redirects to the login page.
	 */
	public function logout()
	{
		$this->identity->logout();
		redirect('site/login');
	}

	/**
	 * Runs the installer
	 *
	 * This function runs the installer.
	 */
	private function install()
	{
		$this->form_validation->set_rules('hidden', 'Hidden', 'required');
		if ($this->form_validation->run())
		{
			if ( ! $this->check_writable())
			{
				$data['heading'] = 'Error: cannot write to the main directory';
				$data['content'] = '<p>Please make sure that the web server has the permission to write to <b>' . getcwd() . '</b>.</p><p>Installation failed.</p>';
				$this->load->view('site/install', $data);
				return;		
			}

			if ( ! $this->check_php_version())
			{
				$data['heading'] = 'Error: PHP version not supported';
				$data['content'] = '<p>Please make sure that you have PHP 5.1 or newer.</p><p>Installation failed.</p>';
				$this->load->view('site/install', $data);
				return;		
			}

			$grader_output = $this->build_grader();
			if ( ! empty($grader_output))
			{
				$data['heading'] = 'Error: cannot build grader engine';
				$data['content'] = '<p>Please make sure that the web server has the permission to write to <b>' . getcwd() . '/moe</b>.<p>Build output:</p><pre>' . html_entity_decode($grader_output) . '</pre>';
				$this->load->view('site/install', $data);
				return;	
			}

			$this->create_db_schema();
			$this->create_db_constraints();
			$this->insert_default_rows();
			$this->create_secret_directory('submission');
			$this->create_secret_directory('testcase');
			$this->create_secret_directory('checker');
			$this->create_directory('files');
			$this->create_sess_id();

			$data['heading'] = 'Congratulations';
			$data['content'] = '<p>You have successfully installed <b>Regrader</b>. You can login with username <b>admin</b> and password <b>admin</b>.</p><p>Installation finished.</p>';
			$this->load->view('site/install', $data);
		}
		else
		{
			$data['heading'] = 'Welcome';
			$data['content'] = '<p>You are about to install <b>Regrader</b>. Please click Install to continue.</p>' .
							   '<div id="installing">' . 
							   '<p><i>Installing....</p></p>' .
			                   '<div class="progress progress-striped active">' .
                                    '<div class="bar" style="width: 100%;"></div>' .
                               '</div>' .
                               '</div>';
			$data['first'] = true;
			$this->load->view('site/install', $data);
		}
	}

	private function check_writable()
	{
		return is_writable('.');
	}

	private function check_php_version()
	{
		return function_exists('version_compare') && version_compare(PHP_VERSION, '5.1', '>=');
	}

	private function create_db_schema()
	{
		$this->db->query("SET foreign_key_checks = 0");

		$this->db->query("DROP TABLE IF EXISTS `category`");
		$this->db->query("CREATE TABLE IF NOT EXISTS `category` (
		                    `id` int(4) unsigned NOT NULL AUTO_INCREMENT,
		                    `name` varchar(50) NOT NULL,
		                    PRIMARY KEY (`id`)
		                  ) ENGINE=InnoDB  DEFAULT CHARSET=utf8");

		$this->db->query("DROP TABLE IF EXISTS `clarification`");
		$this->db->query("CREATE TABLE IF NOT EXISTS `clarification` (
			                `id` int(4) unsigned NOT NULL AUTO_INCREMENT,
			                `user_id` int(4) unsigned NOT NULL,
			                `contest_id` int(4) unsigned NOT NULL,
			                `clar_time` datetime NOT NULL,
			                `title` varchar(255) NOT NULL,
			                `content` text NOT NULL,
			                `answer` text,
			                PRIMARY KEY (`id`),
			                KEY `contest_id` (`contest_id`),
			                KEY `user_id` (`user_id`)
			              ) ENGINE=InnoDB  DEFAULT CHARSET=utf8");

		$this->db->query("DROP TABLE IF EXISTS `contest`");
		$this->db->query("CREATE TABLE IF NOT EXISTS `contest` (
			                `id` int(4) unsigned NOT NULL AUTO_INCREMENT,
			                `name` varchar(50) NOT NULL,
			                `start_time` datetime NOT NULL,
			                `end_time` datetime DEFAULT NULL,
			                `freeze_time` datetime DEFAULT NULL,
			                `unfreeze_time` datetime DEFAULT NULL,
			                `enabled` tinyint(1) NOT NULL,
			                `show_institution_logo` tinyint(1) NOT NULL,
			                PRIMARY KEY (`id`)
			              ) ENGINE=InnoDB  DEFAULT CHARSET=utf8");

		$this->db->query("DROP TABLE IF EXISTS `contest_language`");
		$this->db->query("CREATE TABLE IF NOT EXISTS `contest_language` (
			                `contest_id` int(4) unsigned NOT NULL,
			                `language_id` int(4) unsigned NOT NULL,
			                PRIMARY KEY (`contest_id`,`language_id`),
			                KEY `contest_id` (`contest_id`),
			                KEY `language_id` (`language_id`)
			              ) ENGINE=InnoDB DEFAULT CHARSET=utf8");

		$this->db->query("DROP TABLE IF EXISTS `contest_member`");
		$this->db->query("CREATE TABLE IF NOT EXISTS `contest_member` (
			                `contest_id` int(4) unsigned NOT NULL,
			                `category_id` int(4) unsigned NOT NULL,
			                PRIMARY KEY (`contest_id`,`category_id`),
			                KEY `category_id` (`category_id`),
			                KEY `contest_id` (`contest_id`)
			              ) ENGINE=InnoDB DEFAULT CHARSET=utf8");

		$this->db->query("DROP TABLE IF EXISTS `contest_problem`");
		$this->db->query("CREATE TABLE IF NOT EXISTS `contest_problem` (
			                `contest_id` int(4) unsigned NOT NULL,
			                `problem_id` int(4) unsigned NOT NULL,
			                `alias` varchar(50) NOT NULL,
			                PRIMARY KEY (`contest_id`,`problem_id`),
			                KEY `problem_id` (`problem_id`),
			                KEY `contest_id` (`contest_id`)
			              ) ENGINE=InnoDB DEFAULT CHARSET=utf8");

		$this->db->query("DROP TABLE IF EXISTS `grader`");
		$this->db->query("CREATE TABLE IF NOT EXISTS `grader` (
			                `id` int(4) unsigned NOT NULL AUTO_INCREMENT,
			                `hostname` varchar(50) NOT NULL,
			                `last_activity` datetime NOT NULL,
			                PRIMARY KEY (`id`)
			              ) ENGINE=InnoDB  DEFAULT CHARSET=utf8");

		$this->db->query("DROP TABLE IF EXISTS `judging`");
		$this->db->query("CREATE TABLE IF NOT EXISTS `judging` (
			                `id` int(4) unsigned NOT NULL AUTO_INCREMENT,
			                `submission_id` int(4) unsigned NOT NULL,
			                `testcase_id` int(4) unsigned NOT NULL,
			                `time` int(10) unsigned NOT NULL,
			                `memory` int(10) unsigned NOT NULL,
			                `verdict` int(4) NOT NULL,
			                PRIMARY KEY (`id`),
			                KEY `submission_id` (`submission_id`),
			                KEY `testcase_id` (`testcase_id`)
			              ) ENGINE=InnoDB  DEFAULT CHARSET=utf8");

		$this->db->query("DROP TABLE IF EXISTS `language`");
		$this->db->query("CREATE TABLE IF NOT EXISTS `language` (
			                `id` int(4) unsigned NOT NULL AUTO_INCREMENT,
			                `name` varchar(50) NOT NULL,
			                `extension` varchar(50) NOT NULL,
			                `source_name` varchar(50) NOT NULL,
			                `exe_name` varchar(50) NOT NULL,
			                `compile_cmd` text NOT NULL,
			                `run_cmd` text NOT NULL,
			                `limit_memory` tinyint(1) NOT NULL,
			                `limit_syscall` tinyint(1) NOT NULL,
			                `forbidden_keywords` text NOT NULL,
			                PRIMARY KEY (`id`)
			              ) ENGINE=InnoDB  DEFAULT CHARSET=utf8");

		$this->db->query("DROP TABLE IF EXISTS `problem`");
		$this->db->query("CREATE TABLE IF NOT EXISTS `problem` (
			                `id` int(4) unsigned NOT NULL AUTO_INCREMENT,
			                `name` varchar(50) NOT NULL,
			                `author` varchar(50) NOT NULL,
			                `statement` text NOT NULL,
			                `time_limit` int(4) unsigned NOT NULL,
			                `memory_limit` int(4) unsigned NOT NULL,
			                PRIMARY KEY (`id`)
			              ) ENGINE=InnoDB  DEFAULT CHARSET=utf8");

		$this->db->query("DROP TABLE IF EXISTS `scoreboard_admin`");
		$this->db->query("CREATE TABLE IF NOT EXISTS `scoreboard_admin` (
			                `contest_id` int(4) unsigned NOT NULL,
			                `user_id` int(4) unsigned NOT NULL,
			                `problem_id` int(4) unsigned NOT NULL,
			                `submission_cnt` int(4) unsigned NOT NULL,
			                `time_penalty` int(4) unsigned NOT NULL,
			                `is_accepted` tinyint(1) unsigned NOT NULL,
			                PRIMARY KEY (`contest_id`,`user_id`,`problem_id`),
			                KEY `contest_id` (`contest_id`),
			                KEY `user_id` (`user_id`),
			                KEY `problem_id` (`problem_id`)
                          ) ENGINE=InnoDB DEFAULT CHARSET=utf8");

		$this->db->query("DROP TABLE IF EXISTS `scoreboard_contestant`");
		$this->db->query("CREATE TABLE IF NOT EXISTS `scoreboard_contestant` (
			                `contest_id` int(4) unsigned NOT NULL,
			                `user_id` int(4) unsigned NOT NULL,
			                `problem_id` int(4) unsigned NOT NULL,
			                `submission_cnt` int(4) unsigned NOT NULL,
			                `time_penalty` int(4) unsigned NOT NULL,
			                `is_accepted` tinyint(1) unsigned NOT NULL,
			                PRIMARY KEY (`contest_id`,`user_id`,`problem_id`),
			                KEY `contest_id` (`contest_id`),
			                KEY `user_id` (`user_id`),
			                KEY `problem_id` (`problem_id`)
                          ) ENGINE=InnoDB DEFAULT CHARSET=utf8");

		$this->db->query("DROP TABLE IF EXISTS `setting`");
		$this->db->query("CREATE TABLE IF NOT EXISTS `setting` (
			                `key` varchar(50) NOT NULL,
			                `value` varchar(50) NOT NULL,
			                PRIMARY KEY (`key`)
                          ) ENGINE=InnoDB DEFAULT CHARSET=utf8");

		$this->db->query("DROP TABLE IF EXISTS `submission`");
		$this->db->query("CREATE TABLE IF NOT EXISTS `submission` (
			              `id` int(4) unsigned NOT NULL AUTO_INCREMENT,
			              `user_id` int(4) unsigned NOT NULL,
			              `contest_id` int(4) unsigned NOT NULL,
			              `problem_id` int(4) unsigned NOT NULL,
			              `language_id` int(4) unsigned NOT NULL,
			              `submit_time` datetime NOT NULL,
			              `start_judge_time` datetime NOT NULL,
			              `end_judge_time` datetime NOT NULL,
			              `verdict` int(4) NOT NULL DEFAULT '-1',
			              PRIMARY KEY (`id`),
			              KEY `user_id` (`user_id`),
			              KEY `problem_id` (`problem_id`),
			              KEY `contest_id` (`contest_id`),
			              KEY `language_id` (`language_id`),
			              KEY `contest_id_2` (`contest_id`,`problem_id`)
			            ) ENGINE=InnoDB  DEFAULT CHARSET=utf8");

		$this->db->query("DROP TABLE IF EXISTS `testcase`");
		$this->db->query("CREATE TABLE IF NOT EXISTS `testcase` (
			                `id` int(4) unsigned NOT NULL AUTO_INCREMENT,
		                    `problem_id` int(4) unsigned NOT NULL,
		                    `input` varchar(255) NOT NULL,
		                    `input_size` int(4) unsigned NOT NULL,
		                    `output` varchar(255) NOT NULL,
		                    `output_size` int(4) unsigned NOT NULL,
		                    PRIMARY KEY (`id`),
		                    KEY `problem_id` (`problem_id`)
		                  ) ENGINE=InnoDB  DEFAULT CHARSET=utf8");

		$this->db->query("DROP TABLE IF EXISTS `checker`");
		$this->db->query("CREATE TABLE IF NOT EXISTS `checker` (
			                `id` int(4) unsigned NOT NULL AUTO_INCREMENT,
		                    `problem_id` int(4) unsigned NOT NULL,
		                    `checker` varchar(255) NOT NULL,
		                    `checker_size` int(4) unsigned NOT NULL,
		                    PRIMARY KEY (`id`),
		                    KEY `problem_id` (`problem_id`)
		                  ) ENGINE=InnoDB  DEFAULT CHARSET=utf8");

		$this->db->query("DROP TABLE IF EXISTS `unread_clarification`");
		$this->db->query("CREATE TABLE IF NOT EXISTS `unread_clarification` (
			                `user_id` int(4) unsigned DEFAULT NULL,
			                `clarification_id` int(4) unsigned NOT NULL,
			                KEY `unread_clarification_ibfk_2` (`clarification_id`),
			                KEY `unread_clarification_ibfk_1` (`user_id`)
			              ) ENGINE=InnoDB DEFAULT CHARSET=utf8");

		$this->db->query("DROP TABLE IF EXISTS `user`");
		$this->db->query("CREATE TABLE IF NOT EXISTS `user` (
			                `id` int(4) unsigned NOT NULL AUTO_INCREMENT,
			                `name` varchar(50) NOT NULL,
			                `username` varchar(30) NOT NULL,
			                `password` char(32) NOT NULL,
			                `institution` varchar(50) NOT NULL,
			                `category_id` int(4) unsigned NOT NULL,
			                `last_activity` datetime NOT NULL,
			                PRIMARY KEY (`id`),
			                KEY `category_id` (`category_id`)
			              ) ENGINE=InnoDB  DEFAULT CHARSET=utf8");

		$this->db->query("SET foreign_key_checks = 1");
	}

	private function create_db_constraints()
	{
		$this->db->query("ALTER TABLE `clarification`
			                ADD CONSTRAINT `clarification_ibfk_1` FOREIGN KEY (`contest_id`) REFERENCES `contest` (`id`) ON DELETE CASCADE ON UPDATE CASCADE,
			                ADD CONSTRAINT `clarification_ibfk_2` FOREIGN KEY (`user_id`) REFERENCES `user` (`id`) ON DELETE CASCADE ON UPDATE CASCADE");

		$this->db->query("ALTER TABLE `contest_language`
			                ADD CONSTRAINT `contest_language_ibfk_1` FOREIGN KEY (`contest_id`) REFERENCES `contest` (`id`) ON DELETE CASCADE ON UPDATE CASCADE,
			                ADD CONSTRAINT `contest_language_ibfk_2` FOREIGN KEY (`language_id`) REFERENCES `language` (`id`) ON DELETE CASCADE ON UPDATE CASCADE");
	
		$this->db->query("ALTER TABLE `contest_member`
			                ADD CONSTRAINT `contest_member_ibfk_1` FOREIGN KEY (`contest_id`) REFERENCES `contest` (`id`) ON DELETE CASCADE ON UPDATE CASCADE,
			                ADD CONSTRAINT `contest_member_ibfk_2` FOREIGN KEY (`category_id`) REFERENCES `category` (`id`) ON DELETE CASCADE ON UPDATE CASCADE");
	
		$this->db->query("ALTER TABLE `contest_problem`
			                ADD CONSTRAINT `contest_problem_ibfk_1` FOREIGN KEY (`contest_id`) REFERENCES `contest` (`id`) ON DELETE CASCADE ON UPDATE CASCADE,
			                ADD CONSTRAINT `contest_problem_ibfk_2` FOREIGN KEY (`problem_id`) REFERENCES `problem` (`id`) ON DELETE CASCADE ON UPDATE CASCADE");

		$this->db->query("ALTER TABLE `judging`
			                ADD CONSTRAINT `judging_ibfk_1` FOREIGN KEY (`submission_id`) REFERENCES `submission` (`id`) ON DELETE CASCADE ON UPDATE CASCADE,
			                ADD CONSTRAINT `judging_ibfk_2` FOREIGN KEY (`testcase_id`) REFERENCES `testcase` (`id`) ON DELETE CASCADE ON UPDATE CASCADE");
	
		$this->db->query("ALTER TABLE `scoreboard_admin`
			                ADD CONSTRAINT `scoreboard_admin_ibfk_1` FOREIGN KEY (`contest_id`) REFERENCES `contest` (`id`) ON DELETE CASCADE ON UPDATE CASCADE,
			                ADD CONSTRAINT `scoreboard_admin_ibfk_2` FOREIGN KEY (`problem_id`) REFERENCES `problem` (`id`) ON DELETE CASCADE ON UPDATE CASCADE,
			                ADD CONSTRAINT `scoreboard_admin_ibfk_3` FOREIGN KEY (`user_id`) REFERENCES `user` (`id`) ON DELETE CASCADE ON UPDATE CASCADE");
	
		$this->db->query("ALTER TABLE `scoreboard_contestant`
			                ADD CONSTRAINT `scoreboard_contestant_ibfk_1` FOREIGN KEY (`contest_id`) REFERENCES `contest` (`id`) ON DELETE CASCADE ON UPDATE CASCADE,
			                ADD CONSTRAINT `scoreboard_contestant_ibfk_2` FOREIGN KEY (`user_id`) REFERENCES `user` (`id`) ON DELETE CASCADE ON UPDATE CASCADE,
			                ADD CONSTRAINT `scoreboard_contestant_ibfk_3` FOREIGN KEY (`problem_id`) REFERENCES `problem` (`id`) ON DELETE CASCADE ON UPDATE CASCADE");
	
		$this->db->query("ALTER TABLE `submission`
			                ADD CONSTRAINT `submission_ibfk_1` FOREIGN KEY (`user_id`) REFERENCES `user` (`id`) ON DELETE CASCADE ON UPDATE CASCADE,
			                ADD CONSTRAINT `submission_ibfk_2` FOREIGN KEY (`problem_id`) REFERENCES `problem` (`id`) ON DELETE CASCADE ON UPDATE CASCADE,
			                ADD CONSTRAINT `submission_ibfk_3` FOREIGN KEY (`contest_id`) REFERENCES `contest` (`id`) ON DELETE CASCADE ON UPDATE CASCADE,
			                ADD CONSTRAINT `submission_ibfk_4` FOREIGN KEY (`language_id`) REFERENCES `language` (`id`) ON DELETE CASCADE ON UPDATE CASCADE");

		$this->db->query("ALTER TABLE `testcase`
			                ADD CONSTRAINT `testcase_ibfk_1` FOREIGN KEY (`problem_id`) REFERENCES `problem` (`id`) ON DELETE CASCADE ON UPDATE CASCADE");

		$this->db->query("ALTER TABLE `checker`
			                ADD CONSTRAINT `checker_ibfk_1` FOREIGN KEY (`problem_id`) REFERENCES `problem` (`id`) ON DELETE CASCADE ON UPDATE CASCADE");
	
		$this->db->query("ALTER TABLE `unread_clarification`
			                ADD CONSTRAINT `unread_clarification_ibfk_1` FOREIGN KEY (`user_id`) REFERENCES `user` (`id`) ON DELETE CASCADE ON UPDATE CASCADE,
			                ADD CONSTRAINT `unread_clarification_ibfk_2` FOREIGN KEY (`clarification_id`) REFERENCES `clarification` (`id`) ON DELETE CASCADE ON UPDATE CASCADE");
	
		$this->db->query("ALTER TABLE `user`
			                ADD CONSTRAINT `user_ibfk_1` FOREIGN KEY (`category_id`) REFERENCES `category` (`id`) ON DELETE CASCADE ON UPDATE CASCADE");
	}

	private function insert_default_rows()
	{
		$this->db->query("INSERT INTO `category` (`id`, `name`) VALUES (1, 'Administrator')");
		
		$this->db->query("INSERT INTO `user` (`id`, `name`, `username`, `password`, `institution`, `category_id`, `last_activity`) VALUES (1, 'Administrator', 'admin', MD5('admin'), '-', 1, '0000-00-00 00:00:00')");
	
		$this->db->query("INSERT INTO `language` (`id`, `name`, `extension`, `source_name`, `exe_name`, `compile_cmd`, `run_cmd`, `limit_memory`, `limit_syscall`, `forbidden_keywords`) VALUES (1, 'Pascal', 'pas', 'source.pas', 'source', '/usr/bin/fpc -O2 -XS -Sg [PATH]/source.pas', '[PATH]/source', 1, 1, 'uses\nUses\nuSes\nUSes\nusEs\nUsEs\nuSEs\nUSEs\nuseS\nUseS\nuSeS\nUSeS\nusES\nUsES\nuSES\nUSES')");
		$this->db->query("INSERT INTO `language` (`id`, `name`, `extension`, `source_name`, `exe_name`, `compile_cmd`, `run_cmd`, `limit_memory`, `limit_syscall`, `forbidden_keywords`) VALUES (2, 'Java', 'java', 'Main.java', 'Main.class', '/usr/lib/jvm/jdk1.7.0_09/bin/javac [PATH]/Main.java', '/usr/lib/jvm/jdk1.7.0_09/bin/java -Xmx[MEMORY_LIMIT]M -cp [PATH] Main', 0, 0, 'java.applet\njava.awt\njava.beans\njava.lang.annotation\njava.lang.instrument\njava.lang.management\njava.lang.ref\njava.lang.net\njava.nio\njava.rmi\njava.security\njava.sql\njava.util.concurrent\njava.util.jar\njava.util.logging\njava.util.prefs\njava.util.spi\njava.util.zip\njavax\nCompiler\nInheritableThreadLocal\nPackage\nProcess\nProcessBuilder\nRuntime\nRuntimePermission\nSecurityManager\nThread\nThreadGroup\nThreadLocal\nFile\nFileDescriptor\nFileInputStream\nFileOutputStream\nFileReader\nFileWriter')");
		$this->db->query("INSERT INTO `language` (`id`, `name`, `extension`, `source_name`, `exe_name`, `compile_cmd`, `run_cmd`, `limit_memory`, `limit_syscall`, `forbidden_keywords`) VALUES (3, 'C++', 'cpp', 'source.cpp', 'source', '/usr/bin/g++ -o [PATH]/source [PATH]/source.cpp -O2 -s -static -lm', '[PATH]/source', 1, 1, '')");
		$this->db->query("INSERT INTO `language` (`id`, `name`, `extension`, `source_name`, `exe_name`, `compile_cmd`, `run_cmd`, `limit_memory`, `limit_syscall`, `forbidden_keywords`) VALUES (4, 'C', 'c', 'source.c', 'source', '/usr/bin/gcc -o [PATH]/source [PATH]/source.c -std=gnu99 -O2 -s -static -lm', '[PATH]/source', 1, 1, '')");
	
		$this->db->query("INSERT INTO `setting` (`key`, `value`) VALUES ('web_name', 'Regrader')");
		$this->db->query("INSERT INTO `setting` (`key`, `value`) VALUES ('top_name', 'Regrader')");
		$this->db->query("INSERT INTO `setting` (`key`, `value`) VALUES ('bottom_name', 'Programming Contest System')");
		$this->db->query("INSERT INTO `setting` (`key`, `value`) VALUES ('left_logo', '')");
		$this->db->query("INSERT INTO `setting` (`key`, `value`) VALUES ('right_logo1', '')");
		$this->db->query("INSERT INTO `setting` (`key`, `value`) VALUES ('right_logo2', '')");
		$this->db->query("INSERT INTO `setting` (`key`, `value`) VALUES ('items_per_page', '20')");
		$this->db->query("INSERT INTO `setting` (`key`, `value`) VALUES ('submission_path', '')");
		$this->db->query("INSERT INTO `setting` (`key`, `value`) VALUES ('testcase_path', '')");
		$this->db->query("INSERT INTO `setting` (`key`, `value`) VALUES ('checker_path', '')");
		$this->db->query("INSERT INTO `setting` (`key`, `value`) VALUES ('sess_id', '')");

		$this->db->query("INSERT INTO `grader` (`id`, `hostname`, `last_activity`) VALUES (1, '" . php_uname('n') . "', '0000-00-00 00:00:00')");
	}

	private function create_secret_directory($name)
	{
		sleep(2);
		$this->load->helper('string');
		$secret_name = 'secret-' . random_string('md5');
		mkdir($secret_name);
		chmod($secret_name, 0777);
		$this->db->query("UPDATE `setting` SET `value`='{$secret_name}' WHERE `key`='{$name}_path'");
	}

	private function create_sess_id()
	{
		sleep(2);
		$this->load->helper('string');
		$sess_id = random_string('md5');
		$this->db->query("UPDATE `setting` SET `value`='{$sess_id}' WHERE `key`='sess_id'");
	}

	private function create_directory($name)
	{
		mkdir($name);
		chmod($name, 0775);
	}

	private function build_grader()
	{
		$output1 = $output2 = array();
		chdir('moe');
		exec('./configure', $output1, $retval1);
		if ($retval1 == 0)
			exec('make', $output2, $retval2);
		chdir('..');
		return $retval1 == 0 && $retval2 == 0 ? '' : implode("\n", array_merge($output1, $output2));
	}
}

/* End of file site.php */
/* Location: ./application/controllers/site.php */
