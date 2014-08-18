<!DOCTYPE html>
<html lang="id">

<head>
	<meta http-equiv="Content-Type" content="text/html; charset=UTF-8" />
	
	<link type="text/css" href="assets/css/bootstrap.css" rel="stylesheet"/>
	<link type="text/css" href="assets/css/style.css" rel="stylesheet"/>
	<link rel="shortcut icon" href="assets/img/favicon.ico">
	<script type="text/javascript" src="assets/js/jquery.min.js"></script>
	<script type="text/javascript" src="assets/js/bootstrap.min.js"></script>
	<title><?php echo $title . ' | ' . $web_name; ?></title>
</head>

<body>

<div class="navbar navbar-fixed-top">
	<div class="navbar-inner">
		<div class="container">
			<div class="row">
				<div class="col-md-12">
					<ul class="nav">
						<li class="active">
							<a href="#">
							<?php if ($page == 'login') : ?>
								<i class="icon-off icon-white"></i> <?php echo $this->lang->line('login'); ?>
							<?php else : ?>
								<i class="icon-list icon-white"></i> <?php echo $this->lang->line('scoreboard'); ?>
							<?php endif; ?>
							</a>
						</li>
					</ul>
					<ul class="nav pull-right">
						<li class="divider-vertical"></li>
						<li>
							<a href="#"><i class="icon-time icon-white"></i> <span id="server_clock"></span></a>
						</li>
						<li class="divider-vertical"></li>
						<li class="dropdown">
							<a href="#" class="dropdown-toggle" data-toggle="dropdown"><i class="icon-user icon-white"></i> <?php echo $this->lang->line('guest'); ?> <b class="caret"></b></a>
							
						</li>
					</ul>
				</div>
			</div>
		</div>
	</div>
</div>

<div class="container">
	<div class="row">
		<div class="site_logo">
			<img src="files/<?php echo $left_logo; ?>" />
		</div>
		<div class="site_name">
			<h1><?php echo $top_name; ?></h1>
			<h3><?php echo $bottom_name; ?></h3>
		</div>

		<div class="site_logo_right">
			<img src="files/<?php echo $right_logo1; ?>" />
		</div>
		<div class="site_logo_right">
			<img src="files/<?php echo $right_logo2; ?>" />
		</div>
		<div class="col-md-12">
			<hr />
		</div>
	</div>
</div>