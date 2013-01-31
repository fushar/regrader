<div class="container">
	<div class="row">
		<div class="span12">
			<ul class="breadcrumb">
				<li><i class="icon-flag"></i> <?php echo $this->lang->line('categories'); ?></li>
				<li><span class="divider">|</span></li>
				<li><i class="icon-list-alt"></i> <?php echo isset($category) ? $this->lang->line('category') . ' ' . $category['id'] : $this->lang->line('new_category'); ?></li>
			</ul>
		</div>
	</div>

	<div class="row">
		<div class="span12">
			<h3><?php echo isset($category) ? $this->lang->line('edit_category') : $this->lang->line('add_category'); ?></h3>
			<form class="form-horizontal" action="" method="post">
				<div class="control-group<?php echo form_error('form[name]') == '' ? '' : ' error'; ?>">
					<label class="control-label"><?php echo $this->lang->line('name'); ?></label>
					<div class="controls">
						<input name="form[name]" type="text" class="span4" maxlength="50" value="<?php echo set_value('form[name]', @$category['name']); ?>"/>
						<span class="help-inline"><?php echo form_error('form[name]'); ?></span>
					</div>
				</div>

				<div class="form-actions">
					<button type="submit" class="btn btn-danger"><?php echo isset($category) ? '<i class="icon-download-alt icon-white"></i> ' . $this->lang->line('save') : '<i class="icon-plus icon-white"></i> ' . $this->lang->line('add'); ?></button>
					<a class="btn" href="<?php echo site_url('admin/category/viewAll/' . $page_offset); ?>"><?php echo $this->lang->line('cancel'); ?></a>
				</div>
			</form>
		</div>
	</div>
</div>