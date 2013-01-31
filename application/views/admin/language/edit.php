<div class="container">
	<div class="row">
		<div class="span12">
			<ul class="breadcrumb">
				<li><i class="icon-globe"></i> <?php echo $this->lang->line('languages'); ?></li>
				<li><span class="divider">|</span></li>
				<li><i class="icon-list-alt"></i> <?php echo isset($language) ? $this->lang->line('language') . ' ' . $language['id'] : $this->lang->line('new_language'); ?></li>
			</ul>
		</div>
	</div>

	<div class="row">
		<div class="span12">
			<h3><?php echo isset($language) ? $this->lang->line('edit_language') : $this->lang->line('add_language'); ?></h3>
			<form class="form-horizontal" action="" method="post">
				<div class="control-group<?php echo form_error('form[name]') == '' ? '' : ' error'; ?>">
					<label class="control-label"><?php echo $this->lang->line('name'); ?></label>
					<div class="controls">
						<input name="form[name]" type="text" class="span4" maxlength="50" value="<?php echo set_value('form[name]', @$language['name']); ?>"/>
						<span class="help-inline"><?php echo form_error('form[name]'); ?></span>
					</div>
				</div>

				<div class="control-group<?php echo form_error('form[extension]') == '' ? '' : ' error'; ?>">
					<label class="control-label"><?php echo $this->lang->line('extension'); ?></label>
					<div class="controls">
						<input name="form[extension]" type="text" class="span4" maxlength="50" value="<?php echo set_value('form[extension]', @$language['extension']); ?>"/>
						<span class="help-inline"><?php echo form_error('form[extension]'); ?></span>
					</div>
				</div>

				<div class="control-group<?php echo form_error('form[source_name]') == '' ? '' : ' error'; ?>">
					<label class="control-label"><?php echo $this->lang->line('source_name'); ?></label>
					<div class="controls">
						<input name="form[source_name]" type="text" class="span4" maxlength="50" value="<?php echo set_value('form[source_name]', @$language['source_name']); ?>"/>
						<span class="help-inline"><?php echo form_error('form[source_name]'); ?></span>
					</div>
				</div>

				<div class="control-group<?php echo form_error('form[exe_name]') == '' ? '' : ' error'; ?>">
					<label class="control-label"><?php echo $this->lang->line('exe_name'); ?></label>
					<div class="controls">
						<input name="form[exe_name]" type="text" class="span4" maxlength="50" value="<?php echo set_value('form[exe_name]', @$language['exe_name']); ?>"/>
						<span class="help-inline"><?php echo form_error('form[exe_name]'); ?></span>
					</div>
				</div>

				<div class="control-group<?php echo form_error('form[compile_cmd]') == '' ? '' : ' error'; ?>">
					<label class="control-label"><?php echo $this->lang->line('compile_cmd'); ?></label>
					<div class="controls">
						<input name="form[compile_cmd]" type="text" class="span4" value="<?php echo set_value('form[compile_cmd]', @$language['compile_cmd']); ?>"/>
						<span class="help-inline"><?php echo form_error('form[compile_cmd]'); ?></span>
					</div>
				</div>

				<div class="control-group<?php echo form_error('form[run_cmd]') == '' ? '' : ' error'; ?>">
					<label class="control-label"><?php echo $this->lang->line('run_cmd'); ?></label>
					<div class="controls">
						<input name="form[run_cmd]" type="text" class="span4" value="<?php echo set_value('form[run_cmd]', @$language['run_cmd']); ?>"/>
						<span class="help-inline"><?php echo form_error('form[run_cmd]'); ?></span>
					</div>
				</div>

				<div class="control-group">
					<label class="control-label"><?php echo $this->lang->line('limit_memory'); ?></label>
					<div class="controls">
						<input name="form[limit_memory]" type="checkbox" value="1" <?php echo set_checkbox('form[limit_memory]', '1', (bool)@$language['limit_memory']); ?>/>
					</div>
				</div>

				<div class="control-group">
					<label class="control-label"><?php echo $this->lang->line('limit_syscall'); ?></label>
					<div class="controls">
						<input name="form[limit_syscall]" type="checkbox" value="1" <?php echo set_checkbox('form[limit_syscall]', '1', (bool)@$language['limit_syscall']); ?>/>
					</div>
				</div>

				<div class="control-group<?php echo form_error('form[forbidden_keywords]') == '' ? '' : ' error'; ?>">
					<label class="control-label"><?php echo $this->lang->line('forbidden_keywords'); ?></label>
					<div class="controls">
						<textarea name="form[forbidden_keywords]"><?php echo set_value('form[forbidden_keywords]', @$language['forbidden_keywords']); ?></textarea>
						<span class="help-inline"><?php echo form_error('form[forbidden_keywords]'); ?></span>
					</div>
				</div>

				<div class="form-actions">
					<button type="submit" class="btn btn-danger"><?php echo isset($language) ? '<i class="icon-download-alt icon-white"></i> ' . $this->lang->line('save') : '<i class="icon-plus icon-white"></i> ' . $this->lang->line('add'); ?></button>
					<a class="btn" href="<?php echo site_url('admin/language/viewAll/' . $page_offset); ?>"><?php echo $this->lang->line('cancel'); ?></a>
				</div>
			</form>
		</div>
	</div>
</div>