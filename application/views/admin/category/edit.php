<div class="container">
	<div class="row">
		<div class="col-md-12">
			<ul class="breadcrumb">
				<li><i class="glyphicon glyphicon-flag"></i> <?php echo $this->lang->line('categories'); ?></li>
				<li><i class="glyphicon glyphicon-list-alt"></i> <?php echo isset($category) ? $this->lang->line('category') . ' ' . $category['id'] : $this->lang->line('new_category'); ?></li>
			</ul>
		</div>
	</div>

	<div class="row">
		<div class="col-md-12">
			<h3><?php echo isset($category) ? $this->lang->line('edit_category') : $this->lang->line('add_category'); ?></h3>
			<form class="form-horizontal" action="" method="post">
				<div class="form-group<?php echo form_error('form[name]') == '' ? '' : ' error'; ?>">
					<label class="col-sm-2 control-label"><?php echo $this->lang->line('name'); ?></label>
					<div class="col-sm-4">
						<input name="form[name]" type="text" class="col-md-4 form-control" maxlength="50" value="<?php echo set_value('form[name]', @$category['name']); ?>"/>
						<span class="help-inline"><?php echo form_error('form[name]'); ?></span>
					</div>
				</div>

				<div class="form-actions col-sm-offset-2">
					<button type="submit" class="btn btn-danger"><?php echo isset($category) ? '<i class="glyphicon glyphicon-download-alt"></i> ' . $this->lang->line('save') : '<i class="glyphicon glyphicon-plus"></i> ' . $this->lang->line('add'); ?></button>
					<a class="btn btn-default" href="<?php echo site_url('admin/category/viewAll/' . $page_offset); ?>"><?php echo $this->lang->line('cancel'); ?></a>
				</div>
			</form>
		</div>
	</div>
</div>