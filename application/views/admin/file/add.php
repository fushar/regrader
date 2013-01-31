<div class="container">
	<div class="row">
		<div class="span12">
			<ul class="breadcrumb">
				<li><i class="icon-file"></i> <?php echo $this->lang->line('files'); ?></li>
				<li><span class="divider">|</span></li>
				<li><i class="icon-list-alt"></i> <?php echo $this->lang->line('new_file'); ?></li>
			</ul>
		</div>
	</div>

	<div class="row">
		<div class="span12">
			<h3><?php echo $this->lang->line('add_file'); ?></h3>
			<?php echo form_open_multipart('', array('class' => 'form-horizontal'));?>
				<div class="control-group<?php echo form_error('userfile') == '' ? '' : ' error'; ?>">
					<label class="control-label"><?php echo $this->lang->line('file'); ?></label>
					<div class="controls">
						<input name="userfile" type="file" class="span4" />
						<span class="help-inline"><?php echo form_error('userfile'); ?></span>
					</div>
				</div>

				<div class="form-actions">
					<button type="submit" class="btn btn-danger"><i class="icon-plus icon-white"></i> <?php echo $this->lang->line('add'); ?></button>
					<a class="btn" href="<?php echo site_url('admin/file/viewAll/'); ?>"><?php echo $this->lang->line('cancel'); ?></a>
				</div>
				<input name="hidden" type="hidden" value="1"/>
			</form>
		</div>
	</div>
</div>