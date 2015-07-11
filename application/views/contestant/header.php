<!DOCTYPE html>
<html lang="en">

<head>
	<meta http-equiv="Content-Type" content="text/html; charset=UTF-8" />
	
	<link type="text/css" href="<?php echo base_url(); ?>assets/vendor/bootstrap/dist/css/bootstrap.min.css" rel="stylesheet"/>
	<link type="text/css" href="<?php echo base_url(); ?>assets/css/style.css" rel="stylesheet"/>

	<?php foreach ($custom_css as $v) :?>
		<link type="text/css" href="<?php echo base_url(); ?>assets/<?php echo $v; ?>" rel="stylesheet"/>
	<?php endforeach; ?>

	<link rel="shortcut icon" href="<?php echo base_url(); ?>files/favicon.ico">
	<script type="text/javascript" src="<?php echo base_url(); ?>assets/vendor/jquery/dist/jquery.min.js"></script>
	<script type="text/javascript" src="<?php echo base_url(); ?>assets/vendor/bootstrap/dist/js/bootstrap.min.js"></script>
	<script type="text/javascript" src="<?php echo base_url(); ?>assets/js/clocks.js"></script>

	<?php foreach ($custom_js as $v) :?>
		<script type="text/javascript" src="<?php echo base_url(); ?>assets/<?php echo $v; ?>"></script>
	<?php endforeach; ?>
	
	<title><?php echo $title . ' | ' . $web_name; ?></title>
</head>

<body>

<div class="navbar navbar-inverse navbar-fixed-top">
	<div class="container">
			<div class="row">
				<div class="col-md-12">
					<ul class="nav navbar-nav">
						<?php if ($page == 'dashboard') : ?>
							<li class="active">
								<a href="#"><i class="glyphicon glyphicon-home"></i> <?php echo $this->lang->line('dashboard'); ?></a>
							</li>
						<?php else: ?>
							<li<?php if ($page == 'problem') echo ' class="active"'; ?>>
								<a href="<?php echo site_url('contestant/problem'); ?>"><i class="glyphicon glyphicon-book"></i> <?php echo $this->lang->line('problems'); ?></a>
							</li>
							<li<?php if ($page == 'submission') echo ' class="active"'; ?>>
								<a href="<?php echo site_url('contestant/submission'); ?>"><i class="glyphicon glyphicon-share"></i> <?php echo $this->lang->line('submissions'); ?></a>
							</li>
							<li<?php if ($page == 'clarification') echo ' class="active"'; ?>>
								<a href="<?php echo site_url('contestant/clarification'); ?>"><i class="glyphicon glyphicon-envelope"></i> <?php echo $this->lang->line('clarifications'); ?><?php if ($unread_clar_cnt > 0) echo ' <span class="unread_clarifications">(' . $unread_clar_cnt . ' ' . $this->lang->line('new') . ')</span>'; ?></a>
							</li>
							<li<?php if ($page == 'scoreboard') echo ' class="active"'; ?>>
								<a href="<?php echo site_url('contestant/scoreboard'); ?>"><i class="glyphicon glyphicon-list"></i> <?php echo $this->lang->line('scoreboard'); ?></a>
							</li>
						<?php endif; ?>
					</ul>
					<ul class="nav navbar-nav pull-right">
						<li>
							<a href="#"><i class="glyphicon glyphicon-time"></i> <span id="server_clock"></span></a>
						</li>
						<li class="dropdown">
							<a href="#" class="dropdown-toggle" data-toggle="dropdown"><i class="glyphicon glyphicon-user"></i> <?php echo $active_user_name; ?> <b class="caret"></b></a>
							<ul class="dropdown-menu">
								<li><a href="<?php echo site_url('contestant/dashboard'); ?>"><?php echo $this->lang->line('dashboard'); ?></a></li>
								<li><a href="<?php echo site_url('site/logout'); ?>"><?php echo $this->lang->line('logout'); ?></a></li>
							</ul>
						</li>
					</ul>
				</div>
			</div>
		</div>
</div>

<div class="container">
	<div class="row">
		<div class="col-md-12">
			<?php if ( ! empty($left_logo)) : ?>
			<div class="site_logo">
				<img src="<?php echo base_url(); ?>files/<?php echo $left_logo; ?>" />
			</div>
			<?php endif; ?>
			<div class="site_name">
				<h1><?php echo $page == 'dashboard' ? $top_name : $active_contest_name; ?></h1>
				<h3><?php echo $bottom_name; ?></h3>
			</div>

			<?php if ( ! empty($right_logo1)): ?>
			<div class="site_logo_right">
				<img src="<?php echo base_url(); ?>files/<?php echo $right_logo1; ?>" />
			</div>
			<?php endif; ?>
			<?php if ( ! empty($right_logo2)): ?>
			<div class="site_logo_right">
				<img src="<?php echo base_url(); ?>files/<?php echo $right_logo2; ?>" />
			</div>
			<?php endif; ?>
		</div>
	</div>
	<div class="row">
		<div class="col-md-12">
			<hr />
		</div>
	</div>
</div>