<div class="container">
	<div class="row">
		<div class="col-md-12">
			<ul class="breadcrumb">
				<li><i class="glyphicon glyphicon-screenshot"></i> <?php echo $this->lang->line('options'); ?></li>
			</ul>
		</div>
	</div>

	<div class="row">
		<div class="col-md-12">
			<?php if ($this->session->flashdata('edit_successful')): ?>
			<div class="alert alert-success">
				<?php echo $this->lang->line('edit_options_successful'); ?>
			</div>
			<?php endif; ?>

			<h3><?php echo $this->lang->line('edit_options'); ?></h3>
			
			<form class="form-horizontal" action="" method="post">
				<?php
					$fields = array('web_name', 'top_name', 'bottom_name', 'left_logo', 'right_logo1', 'right_logo2', 'items_per_page');
					foreach ($fields as $v) : ?>

					<div class="form-group<?php echo form_error('form[' . $v . ']') == '' ? '' : ' error'; ?>">
						<label class="col-sm-2 control-label"><?php echo $this->lang->line($v); ?></label>
						<div class="col-sm-4">
							<input name="form[<?php echo $v; ?>]" type="text" class="col-md-4 form-control" maxlength="50" value="<?php echo set_value('form[' . $v . ']', $this->setting->get($v)); ?>"/>
							<span class="help-block"><?php echo form_error('form[' . $v . ']'); ?></span>
						</div>
					</div>
				<?php endforeach; ?>


				<div class="form-actions col-sm-offset-2">
					<button type="submit" class="btn btn-danger"><?php echo '<i class="glyphicon glyphicon-download-alt"></i> ' . $this->lang->line('save'); ?></button>
				</div>
			</form>
		</div>
	</div>
</div>