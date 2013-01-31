<div class="container">
	<div class="row">
		<div class="span12">
			<ul class="breadcrumb">
				<li><i class="icon-user"></i> <?php echo $this->lang->line('users'); ?></li>
				<li><span class="divider">|</span></li>
				<li><i class="icon-list-alt"></i> <?php echo isset($user) ? $this->lang->line('user') . ' ' . $user['id'] : $this->lang->line('new_user'); ?></li>
			</ul>
		</div>
	</div>

	<div class="row">
		<div class="span12">
			<h3><?php echo isset($user) ? $this->lang->line('edit_user') : $this->lang->line('add_user'); ?></h3>
			
			<form class="form-horizontal" action="" method="post">
				<div class="control-group<?php echo form_error('form[username]') == '' ? '' : ' error'; ?>">
					<label class="control-label"><?php echo $this->lang->line('username'); ?></label>
					<div class="controls">
						<input name="form[username]" type="text" class="span4" maxlength="30" value="<?php echo set_value('form[username]', @$user['username']); ?>"/>
						<span class="help-inline"><?php echo form_error('form[username]'); ?></span>
					</div>
				</div>

				<div class="control-group<?php echo form_error('form[name]') == '' ? '' : ' error'; ?>">
					<label class="control-label"><?php echo $this->lang->line('name'); ?></label>
					<div class="controls">
						<input name="form[name]" type="text" class="span4" maxlength="50" value="<?php echo set_value('form[name]', @$user['name']); ?>"/>
						<span class="help-inline"><?php echo form_error('form[name]'); ?></span>
					</div>
				</div>

				<div class="control-group<?php echo form_error('form[password]') == '' ? '' : ' error'; ?>">
					<label class="control-label"><?php echo $this->lang->line('password'); ?></label>
					<div class="controls">
						<input name="form[password]" type="text" class="span4" maxlength="50" value="<?php echo set_value('form[password]', ''); ?>"/>
						<span class="help-inline"><?php echo form_error('form[password]'); ?></span>
						<?php if (isset($user)) : ?>
							<p class="help-block"><?php echo $this->lang->line('only_when_change_password'); ?></p>
						<?php endif; ?>
					</div>
				</div>

				<div class="control-group<?php echo form_error('form[institution]') == '' ? '' : ' error'; ?>">
					<label class="control-label"><?php echo $this->lang->line('institution'); ?></label>
					<div class="controls">
						<input name="form[institution]" type="text" class="span4" maxlength="50" value="<?php echo set_value('form[institution]', @$user['institution']); ?>"/>
						<span class="help-inline"><?php echo form_error('form[institution]'); ?></span>
					</div>
				</div>

				<div class="control-group<?php echo form_error('form[category_id]') == '' ? '' : ' error'; ?>">
					<label class="control-label"><?php echo $this->lang->line('category'); ?></label>
					<div class="controls">
						<select name="form[category_id]" class="span4">
						<?php foreach ($categories as $v) : ?>
							<option value="<?php echo $v['id']; ?>" <?php echo set_select('form[category_id]', $v['id'], isset($user) && $v['id'] == @$user['category_id']); ?>><?php echo $v['name']; ?></option>
						<?php endforeach; ?>
						</select>
						<span class="help-inline"><?php echo form_error('form[category_id]'); ?></span>
						<?php if (empty($categories)) : ?>
							<p class="help-block"><i><?php echo $this->lang->line('no_category'); ?></i></p>
						<?php endif; ?>
					</div>
				</div>

				<div class="form-actions">
					<button type="submit" class="btn btn-danger"><?php echo isset($user) ? '<i class="icon-download-alt icon-white"></i> ' . $this->lang->line('save') : '<i class="icon-plus icon-white"></i> ' . $this->lang->line('add'); ?></button>
					<a class="btn" href="<?php echo site_url('admin/user/viewAll/' . $category_id . '/' . $page_offset); ?>"><?php echo $this->lang->line('cancel'); ?></a>
				</div>
			</form>
		</div>
	</div>
</div>