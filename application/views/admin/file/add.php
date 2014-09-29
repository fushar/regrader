<div class="container">
	<div class="row">
		<div class="col-md-12">
			<ul class="breadcrumb">
				<li><i class="glyphicon glyphicon-file"></i> <?php echo $this->lang->line('files'); ?></li>
				<li><i class="glyphicon glyphicon-list-alt"></i> <?php echo $this->lang->line('new_file'); ?></li>
			</ul>
		</div>
	</div>

	<div class="row">
		<div class="col-md-12">
			<h3><?php echo $this->lang->line('add_file'); ?></h3>
			<?php echo form_open_multipart('', array('class' => 'form-horizontal'));?>
				<div class="form-group<?php echo form_error('userfile') == '' ? '' : ' error'; ?>">
					<label class="col-sm-2 control-label"><?php echo $this->lang->line('file'); ?></label>
					<div class="col-sm-4">
						<input name="userfile" type="file" class="col-md-4 form-control" />
						<span class="help-block"><?php echo form_error('userfile'); ?></span>
					</div>
				</div>

				<div class="form-actions col-sm-offset-2">
					<button type="submit" class="btn btn-danger"><i class="glyphicon glyphicon-plus"></i> <?php echo $this->lang->line('add'); ?></button>
					<a class="btn btn-default" href="<?php echo site_url('admin/file/viewAll/'); ?>"><?php echo $this->lang->line('cancel'); ?></a>
				</div>
				<input name="hidden" type="hidden" value="1"/>
			</form>
		</div>
	</div>
</div>