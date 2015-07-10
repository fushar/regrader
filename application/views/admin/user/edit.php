<div class="container">
	<div class="row">
		<div class="col-md-12">
			<ul class="breadcrumb">
				<li><i class="glyphicon glyphicon-user"></i> <?php echo $this->lang->line('users'); ?></li>
				<li><i class="glyphicon glyphicon-list-alt"></i> <?php echo isset($user) ? $this->lang->line('user') . ' ' . $user['id'] : $this->lang->line('new_user'); ?></li>
			</ul>
		</div>
	</div>

	<div class="row">
		<div class="col-md-12">
			<h3><?php echo isset($user) ? $this->lang->line('edit_user') : $this->lang->line('add_user'); ?></h3>
			
			<form class="form-horizontal" action="" method="post">
				<div class="form-group<?php echo form_error('form[username]') == '' ? '' : ' has-error'; ?>">
					<label class="col-sm-2 control-label"><?php echo $this->lang->line('username'); ?></label>
					<div class="col-sm-4">
						<input name="form[username]" type="text" class="col-md-4 form-control" maxlength="30" value="<?php echo set_value('form[username]', @$user['username']); ?>"/>
						<span class="help-block"><?php echo form_error('form[username]'); ?></span>
					</div>
				</div>

				<div class="form-group<?php echo form_error('form[name]') == '' ? '' : ' has-error'; ?>">
					<label class="col-sm-2 control-label"><?php echo $this->lang->line('name'); ?></label>
					<div class="col-sm-4">
						<input name="form[name]" type="text" class="col-md-4 form-control" maxlength="50" value="<?php echo set_value('form[name]', @$user['name']); ?>"/>
						<span class="help-block"><?php echo form_error('form[name]'); ?></span>
					</div>
				</div>

				<div class="form-group<?php echo form_error('form[password]') == '' ? '' : ' has-error'; ?>">
					<label class="col-sm-2 control-label"><?php echo $this->lang->line('password'); ?></label>
					<div class="col-sm-4">
						<input name="form[password]" type="text" class="col-md-4 form-control" maxlength="50" value="<?php echo set_value('form[password]', ''); ?>"/>
						<span class="help-block"><?php echo form_error('form[password]'); ?></span>
						<?php if (isset($user)) : ?>
							<p class="help-block"><?php echo $this->lang->line('only_when_change_password'); ?></p>
						<?php endif; ?>
					</div>
				</div>

				<div class="form-group<?php echo form_error('form[institution]') == '' ? '' : ' has-error'; ?>">
					<label class="col-sm-2 control-label"><?php echo $this->lang->line('institution'); ?></label>
					<div class="col-sm-4">
						<input name="form[institution]" type="text" class="col-md-4 form-control" maxlength="50" value="<?php echo set_value('form[institution]', @$user['institution']); ?>"/>
						<span class="help-block"><?php echo form_error('form[institution]'); ?></span>
					</div>
				</div>

				<div class="form-group<?php echo form_error('form[category_id]') == '' ? '' : ' has-error'; ?>">
					<label class="col-sm-2 control-label"><?php echo $this->lang->line('category'); ?></label>
					<div class="col-sm-4">
						<select name="form[category_id]" class="col-md-4 form-control">
						<?php foreach ($categories as $v) : ?>
							<option value="<?php echo $v['id']; ?>" <?php echo set_select('form[category_id]', $v['id'], isset($user) && $v['id'] == @$user['category_id']); ?>><?php echo $v['name']; ?></option>
						<?php endforeach; ?>
						</select>
						<span class="help-block"><?php echo form_error('form[category_id]'); ?></span>
						<?php if (empty($categories)) : ?>
							<p class="help-block"><i><?php echo $this->lang->line('no_category'); ?></i></p>
						<?php endif; ?>
					</div>
				</div>

				<div class="col-sm-offset-2 form-actions">
					<button type="submit" class="btn btn-danger"><?php echo isset($user) ? '<i class="glyphicon glyphicon-download-alt"></i> ' . $this->lang->line('save') : '<i class="glyphicon glyphicon-plus"></i> ' . $this->lang->line('add'); ?></button>
					<a class="btn btn-default" href="<?php echo site_url('admin/user/viewAll/' . $category_id . '/' . $page_offset); ?>"><?php echo $this->lang->line('cancel'); ?></a>
				</div>
			</form>
		</div>
	</div>
</div>