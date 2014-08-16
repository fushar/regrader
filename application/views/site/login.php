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
			<div class="col-md-4">
				<div class="alert alert-error">
					<?php echo $this->session->flashdata('error'); ?>
			    </div>
			</div>
		</div>
	<?php endif; ?>

	<div class="row">
		<div class="col-md-12">
			<form action="" method="post">
				<div class="form-group<?php echo form_error('form[username]') ? ' error' : ''; ?>">
					<label><?php echo $this->lang->line('username'); ?>:</label><br>
					<input name="form[username]" type="text" class="col-md-4" maxlength="30" value="<?php echo set_value('form[username]'); ?>"/>
					<span class="help-block"><?php echo form_error('form[username]'); ?></span>
				</div>
				<br>
				<div class="form-group<?php echo form_error('form[password]') ? ' error' : ''; ?>">
					<label><?php echo $this->lang->line('password'); ?>:</label><br>
					<input name="form[password]" type="password" class="col-md-4" maxlength="30"/>
					<span class="help-block"><?php echo form_error('form[password]'); ?></span>
				</div>
				<br><br><br>
				<div class="form-actions">
					<button type="submit" class="btn btn-danger col-md-3"><i class="glyphicon glyphicon-user glyphicon-white"></i> <?php echo $this->lang->line('login'); ?></button>
				</div>
			</form>
		</div>
	</div>
</div>