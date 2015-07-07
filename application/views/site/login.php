<div class="container">
	<div class="row">
		<div class="col-md-12">
			<ul class="breadcrumb">
				<li>
					<i class="glyphicon glyphicon-off"></i> <?php echo $this->lang->line('please_login'); ?>
				</li>
			</ul>
		</div>
	</div>

	<?php if ($this->session->flashdata('error')) : ?>
		<div class="row">
			<div class="col-md-5">
				<div class="alert alert-danger">
					<?php echo $this->session->flashdata('error'); ?>
			    </div>
			</div>
		</div>
	<?php endif; ?>

	<div class="row">
		<div class="col-md-12">
			<form class="form-horizontal" action="" method="post">
				<div class="form-group<?php echo form_error('form[username]') ? ' has-error' : ''; ?>">
					<label class="col-sm-1 control-label"><?php echo $this->lang->line('username'); ?>:</label>
					<div class="col-sm-4">
						<input name="form[username]" type="text" maxlength="30" class="form-control" value="<?php echo set_value('form[username]'); ?>" autofocus="on"/>
						<span class="help-block"><?php echo form_error('form[username]'); ?></span>
					</div>
				</div>
				<div class="form-group<?php echo form_error('form[password]') ? ' has-error' : ''; ?>">
					<label class="col-sm-1 control-label"><?php echo $this->lang->line('password'); ?>:</label>
					<div class="col-sm-4">
						<input name="form[password]" type="password" maxlength="30" class="form-control"/>
						<span class="help-block"><?php echo form_error('form[password]'); ?></span>
					</div>
				</div>
				<div class="form-actions col-sm-offset-1">
					<button type="submit" class="btn btn-danger col-sm-3"><i class="glyphicon glyphicon-user"></i> <?php echo $this->lang->line('login'); ?></button>
				</div>
			</form>
		</div>
	</div>
</div>