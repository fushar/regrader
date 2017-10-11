<div class="container">
	<div class="row">
		<div class="col-md-12">
			<ul class="breadcrumb">
				<li><i class="glyphicon glyphicon-book"></i> <?php echo $this->lang->line('problems'); ?></li>
				<li><i class="glyphicon glyphicon-list-alt"></i> <?php echo isset($problem) ? $this->lang->line('problem') . ' ' . $problem['id'] : $this->lang->line('new_problem'); ?></li>
			</ul>
		</div>
	</div>

	<div class="row">

		<div class="col-md-12">
			<h3><?php echo isset($problem) ? $this->lang->line('edit_problem') : $this->lang->line('add_problem'); ?></h3>
			<form class="form-horizontal" action="" method="post">
				<div class="form-group<?php echo form_error('form[name]') == '' ? '' : ' has-error'; ?>">
					<label class="col-sm-2 control-label"><?php echo $this->lang->line('name'); ?></label>
					<div class="col-sm-4">
						<input name="form[name]" type="text" class="form-control" maxlength="50" value="<?php echo set_value('form[name]', @$problem['name']); ?>"/>
						<span class="help-block"><?php echo form_error('form[name]'); ?></span>
					</div>
				</div>

				<div class="form-group<?php echo form_error('form[author]') == '' ? '' : ' has-error'; ?>">
					<label class="col-sm-2 control-label"><?php echo $this->lang->line('author'); ?></label>
					<div class="col-sm-4">
						<input name="form[author]" type="text" class="form-control" maxlength="50" value="<?php echo set_value('form[author]', @$problem['author']); ?>"/>
						<span class="help-block"><?php echo form_error('form[author]'); ?></span>
					</div>
				</div>

				<div class="form-group<?php echo form_error('form[time_limit]') == '' ? '' : ' has-error'; ?>">
					<label class="col-sm-2 control-label"><?php echo $this->lang->line('time_limit'); ?></label>
					<div class= "col-sm-4">
						<div class="input-group">
							<input name="form[time_limit]" type="text" class="form-control" maxlength="8" value="<?php echo set_value('form[time_limit]', @$problem['time_limit']); ?>"/><span class="input-group-addon"><?php echo $this->lang->line('second'); ?></span>
						</div>
						<span class="help-block"><?php echo form_error('form[time_limit]'); ?></span>
					</div>
				</div>

				<div class="form-group<?php echo form_error('form[memory_limit]') == '' ? '' : ' has-error'; ?>">
					<label class="col-sm-2 control-label"><?php echo $this->lang->line('memory_limit'); ?></label>
					<div class= "col-sm-4">
						<div class="input-group">
							<input name="form[memory_limit]" type="text" class="form-control" maxlength="8" value="<?php echo set_value('form[memory_limit]', @$problem['memory_limit']); ?>"/><span class="input-group-addon">MB</span>
						</div>
						<span class="help-block"><?php echo form_error('form[memory_limit]'); ?></span>
					</div>
				</div>

				<div class="form-group<?php echo form_error('form[statement]') == '' ? '' : ' has-error'; ?>">
					<label class="col-sm-2 control-label"><?php echo $this->lang->line('statement'); ?></label>
					<div class= "col-sm-8">
						<textarea id="editor" name="form[statement]"><?php echo set_value('form[statement]', isset($problem) ? @$problem['statement'] : $this->lang->line('default_statement')); ?></textarea>
						 <script>
			                CKEDITOR.replace('editor', {
							    extraPlugins: 'mathjax',
							    mathJaxLib: '<?php echo base_url("assets/vendor/MathJax/MathJax.js?config=TeX-AMS_HTML"); ?>'
							});
			            </script>
						<span class="help-block"><?php echo form_error('form[statement]'); ?></span>
					<div>
				</div>

				<div class="form-actions">
					<button type="submit" class="btn btn-danger"><?php echo isset($problem) ? '<i class="glyphicon glyphicon-download-alt"></i> ' . $this->lang->line('save') : '<i class="glyphicon glyphicon-plus"></i> ' . $this->lang->line('add'); ?></button>
					<a class="btn btn-default" href="<?php echo site_url('admin/problem/viewAll/' . $page_offset); ?>"><?php echo $this->lang->line('cancel'); ?></a>
				</div>
			</form>
		</div>
	</div>
</div>