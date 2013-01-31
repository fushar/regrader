<!DOCTYPE html>
<html lang="en">

<head>
	<meta http-equiv="Content-Type" content="text/html; charset=UTF-8" />
	
	<link type="text/css" href="<?php echo base_url(); ?>assets/css/bootstrap.css" rel="stylesheet"/>
	<link type="text/css" href="<?php echo base_url(); ?>assets/css/style.css" rel="stylesheet"/>

	<?php foreach ($custom_css as $v) :?>
		<link type="text/css" href="<?php echo base_url(); ?>assets/<?php echo $v; ?>" rel="stylesheet"/>
	<?php endforeach; ?>

	<link rel="shortcut icon" href="<?php echo base_url(); ?>files/favicon.ico">
	<script type="text/javascript" src="<?php echo base_url(); ?>assets/js/jquery.min.js"></script>
	<script type="text/javascript" src="<?php echo base_url(); ?>assets/js/bootstrap.min.js"></script>
	<script type="text/javascript" src="<?php echo base_url(); ?>assets/js/clocks.js"></script>
	
	<?php foreach ($custom_js as $v) :?>
		<script type="text/javascript" src="<?php echo base_url(); ?>assets/<?php echo $v; ?>"></script>
	<?php endforeach; ?>

	<title><?php echo $title . ' | ' . $web_name; ?></title>
</head>

<body>
<div class="navbar navbar-fixed-top">
	<div class="navbar-inner">
		<div class="container">
			<div class="row">
				<div class="span12">
					<ul class="nav">
						<li class="dropdown">
							<a href="#" class="dropdown-toggle" data-toggle="dropdown"><i class="icon-wrench icon-white"></i> <?php echo $this->lang->line('manage'); ?> <b class="caret"></b></a>
							<ul class="dropdown-menu">
								<li><a href="<?php echo site_url('admin/contest'); ?>"><i class="icon-fire"></i> <?php echo $this->lang->line('contests'); ?></a></li>
								<li><a href="<?php echo site_url('admin/problem'); ?>"><i class="icon-book"></i> <?php echo $this->lang->line('problems'); ?></a></li>
								<li><a href="<?php echo site_url('admin/language'); ?>"><i class="icon-globe"></i> <?php echo $this->lang->line('languages'); ?></a></li>
								<li><a href="<?php echo site_url('admin/category'); ?>"><i class="icon-flag"></i> <?php echo $this->lang->line('categories'); ?></a></li>
								<li><a href="<?php echo site_url('admin/user'); ?>"><i class="icon-user"></i> <?php echo $this->lang->line('users'); ?></a></li>
								<li><a href="<?php echo site_url('admin/file'); ?>"><i class="icon-file"></i> <?php echo $this->lang->line('files'); ?></a></li>
								<li><a href="<?php echo site_url('admin/option'); ?>"><i class="icon-screenshot"></i> <?php echo $this->lang->line('options'); ?></a></li>
							</ul>
						</li>
						<?php if ($page == 'contest') : ?><li class="active"><a href="<?php echo site_url('admin/contest'); ?>"><i class="icon-fire icon-white"></i> <?php echo $this->lang->line('contests'); ?></a></li><?php endif; ?>
						<?php if ($page == 'problem') : ?><li class="active"><a href="<?php echo site_url('admin/problem'); ?>"><i class="icon-book icon-white"></i> <?php echo $this->lang->line('problems'); ?></a></li><?php endif; ?>
						<?php if ($page == 'language') : ?><li class="active"><a href="<?php echo site_url('admin/language'); ?>"><i class="icon-globe icon-white"></i> <?php echo $this->lang->line('languages'); ?></a></li><?php endif; ?>
						<?php if ($page == 'category') : ?><li class="active"><a href="<?php echo site_url('admin/category'); ?>"><i class="icon-flag icon-white"></i> <?php echo $this->lang->line('categories'); ?></a></li><?php endif; ?>
						<?php if ($page == 'user') : ?><li class="active"><a href="<?php echo site_url('admin/user'); ?>"><i class="icon-user icon-white"></i> <?php echo $this->lang->line('users'); ?></a></li><?php endif; ?>
						<?php if ($page == 'file') : ?><li class="active"><a href="<?php echo site_url('admin/file'); ?>"><i class="icon-file icon-white"></i> <?php echo $this->lang->line('files'); ?></a></li><?php endif; ?>
						<?php if ($page == 'option') : ?><li class="active"><a href="<?php echo site_url('admin/option'); ?>"><i class="icon-screenshot icon-white"></i> <?php echo $this->lang->line('options'); ?></a></li><?php endif; ?>
					</ul>
					<ul class="nav pull-right">
						<li<?php if ($page == 'dashboard') echo ' class="active"'; ?>>
							<a href="<?php echo site_url('admin/dashboard'); ?>"><i class="icon-home icon-white"></i> <?php echo $this->lang->line('dashboard'); ?></a>
						</li>
						<li<?php if ($page == 'submission') echo ' class="active"'; ?>>
							<a href="<?php echo site_url('admin/submission'); ?>"><i class="icon-share icon-white"></i> <?php echo $this->lang->line('submissions'); ?></a>
						</li>
						<li<?php if ($page == 'clarification') echo ' class="active"'; ?>>
							<a href="<?php echo site_url('admin/clarification'); ?>"><i class="icon-envelope icon-white"></i> <?php echo $this->lang->line('clarifications'); ?><?php if ($unread_clar_cnt > 0) echo ' <span class="unread_clarifications">(' . $unread_clar_cnt . ' ' . $this->lang->line('new') . ')</span>'; ?></a>
						</li>
						<li<?php if ($page == 'scoreboard') echo ' class="active"'; ?>>
							<a href="<?php echo site_url('admin/scoreboard'); ?>"><i class="icon-list icon-white"></i> <?php echo $this->lang->line('scoreboard'); ?></a>
						</li>
						<li class="divider-vertical"></li>
						<li>
							<a href="#"><i class="icon-time icon-white"></i> <span id="server_clock"></span></a>
						</li>
						<li class="divider-vertical"></li>
						<li class="dropdown">
							<a href="#" class="dropdown-toggle" data-toggle="dropdown"><i class="icon-user icon-white"></i> <?php echo $active_user_name; ?> <b class="caret"></b></a>
							<ul class="dropdown-menu">
								<li><a href="<?php echo site_url('site/logout'); ?>"><?php echo $this->lang->line('logout'); ?></a></li>
							</ul>
						</li>
					</ul>
				</div>
			</div>
		</div>
	</div>
</div>

<div class="container">
	<div class="row">
		<div class="span12">
			<?php if ( ! empty($left_logo)) : ?>
			<div class="site_logo">
				<img src="<?php echo base_url(); ?>files/<?php echo $left_logo; ?>" />
			</div>
			<?php endif; ?>
			<div class="site_name">
				<h1><?php echo $top_name; ?></h1>
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
		<div class="span12">
			<hr />
		</div>
	</div>
</div>