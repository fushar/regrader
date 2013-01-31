<div class="container">
	<div class="row">
		<div class="span12">
			<ul class="breadcrumb">
				<li><i class="icon-screenshot"></i> <?php echo $this->lang->line('options'); ?></li>
			</ul>
		</div>
	</div>

	<div class="row">
		<div class="span12">
			<?php if ($this->session->flashdata('edit_successful')): ?>
			<div class="alert alert-success">
				<?php echo $this->lang->line('edit_options_successful'); ?>
			</div>
			<?php endif; ?>

			<h3><?php echo $this->lang->line('edit_options'); ?></h3>
			
			<form class="form-horizontal" action="" method="post">
				<?php
					$fields = array('time_zone', 'web_name', 'top_name', 'bottom_name', 'left_logo', 'right_logo1', 'right_logo2', 'items_per_page');
					foreach ($fields as $v) : ?>

					<div class="control-group<?php echo form_error('form[' . $v . ']') == '' ? '' : ' error'; ?>">
						<label class="control-label"><?php echo $this->lang->line($v); ?></label>
						<div class="controls">
							<input name="form[<?php echo $v; ?>]" type="text" class="span4" maxlength="50" value="<?php echo set_value('form[' . $v . ']', $this->setting->get($v)); ?>"/>
							<span class="help-inline"><?php echo form_error('form[' . $v . ']'); ?></span>
						</div>
					</div>
				<?php endforeach; ?>


				<div class="form-actions">
					<button type="submit" class="btn btn-danger"><?php echo '<i class="icon-download-alt icon-white"></i> ' . $this->lang->line('save'); ?></button>
				</div>
			</form>
		</div>
	</div>
</div>