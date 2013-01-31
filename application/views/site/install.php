<!DOCTYPE html>
<html lang="id">

<head>
	<meta http-equiv="Content-Type" content="text/html; charset=UTF-8" />
	
	<link type="text/css" href="<?php echo base_url(); ?>assets/css/bootstrap.css" rel="stylesheet"/>
	<link type="text/css" href="<?php echo base_url(); ?>assets/css/style.css" rel="stylesheet"/>
	<link rel="shortcut icon" href="<?php echo base_url(); ?>files/favicon.ico">
	<script type="text/javascript" src="<?php echo base_url(); ?>assets/js/jquery.min.js"></script>
	<script type="text/javascript" src="<?php echo base_url(); ?>assets/js/bootstrap.min.js"></script>
	<title>Install | Regrader</title>
</head>

<body>

<div class="navbar navbar-fixed-top">
	<div class="navbar-inner">
		<div class="container">
			<div class="row">
				<div class="span12">
					<ul class="nav">
						<li class="active">
							<a href="#"><i class="icon-hdd icon-white"></i> Installation</a>
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
			<div class="site_name">
				<h1>Regrader</h1>
				<h3>Programming Contest System</h3>
			</div>

		</div>
	</div>
	<div class="row">
		<div class="span12">
			<hr />
		</div>
	</div>
</div>

<div class="container">
	<div class="row">
		<div class="span12">
			<ul class="breadcrumb">
				<li>
					<i class="icon-hdd"></i> Installation
				</li>
			</ul>
		</div>
	</div>
	
	<div class="row">
		<div class="span12">
			<h3><?php echo $heading; ?></h3>	
			<?php echo $content; ?>
			<hr />
			<form method="post" action="">
				<input type="hidden" name="hidden" value="1"/>
				<?php if (isset ($first)) : ?>
					<button type="submit" class="btn btn-inverse" href="">Install</button>
				<?php else : ?>
					<a class="btn" href="<?php echo site_url(''); ?>">Back</a>
				<?php endif; ?>
			</form>
		</div>
	</div>
	
</div>

<?php if (isset($first)) : ?>

<script type="text/javascript">
	$(document).ready(function(){
		$('#installing').hide();
		$('button').click(function(){
			$('#installing').show();
		})
	});
</script>

<?php endif; ?>