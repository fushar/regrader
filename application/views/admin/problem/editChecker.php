<div class="container">
	<div class="row">
		<div class="col-md-12">
			<ul class="breadcrumb">
				<li><i class="glyphicon glyphicon-book"></i> <?php echo $this->lang->line('problems'); ?></li>
				<li><i class="glyphicon glyphicon-list-alt"></i> <?php echo $this->lang->line('problem'); ?> <?php echo $problem['id']; ?></li>
			</ul>
		</div>
	</div>
	
	<div class="row">
		<div class="col-md-12">
			<?php if ($this->session->flashdata('add_successful') != ''): ?>
			<div class="alert alert-success">
				<?php echo $this->lang->line('add_checker_successful'); ?>
			</div>
			<?php endif; ?>

			<?php if ($this->session->flashdata('delete_successful') != ''): ?>
			<div class="alert alert-success">
				<?php echo $this->lang->line('delete_checker_successful'); ?>
			</div>
			<?php endif; ?>
			<?php if ($this->session->flashdata('error') != ''): ?>
			<div class="alert alert-danger">
				<?php echo $this->session->flashdata('error'); ?>
			</div>
			<?php endif; ?>

			<h3><?php echo $this->lang->line('edit_problem_checker'); ?></h3>
			<table class="table table-bordered table-striped">
				<thead>
					<tr>
						<th class="id-th"><i class="glyphicon glyphicon-tag"></i></th>
						<th><i class="glyphicon glyphicon-download"></i> <?php echo $this->lang->line('checker'); ?></th>
						<th class="problem-filesize-th"><i class="glyphicon glyphicon-download"></i> <?php echo $this->lang->line('size'); ?></th>
						<th class="operations-th"><i class="glyphicon glyphicon-cog"></i></th>
					</tr>
				</thead>
				<tbody>
					<tr>
						<?php if (!empty($checker)) :?>
							<td class="id-td"><?php echo $checker['id']; ?></td>
							<td><?php echo $checker['checker']; ?></td>	
							<td class="problem-filesize-td"><?php echo $checker['checker_size']; ?> B</td>	
							<td class="operations-td">
								<a href="<?php echo site_url('admin/problem/downloadChecker/' . $checker['id']); ?>" rel="tooltip" title="<?php echo $this->lang->line('download_checker'); ?>"><i class="glyphicon glyphicon-download"></i></a>
								<a href="<?php echo site_url('admin/problem/deleteChecker/' . $problem['id'] . '/'. $checker['id'] . '/' . $page_offset); ?>" rel="tooltip" title="<?php echo $this->lang->line('delete'); ?>"><i class="glyphicon glyphicon-trash"></i></a>
							</td>
						<?php endif;?>
					</tr>
				</tbody>
			</table>
			<hr />
			
			<h3><?php echo $this->lang->line('add_checker'); ?></h3>
			<?php echo form_open_multipart('', array('class' => 'form-horizontal')); ?>
				<div class="form-group<?php echo form_error('new_checker') == '' ? '' : ' error'; ?>">
					<label class="col-sm-2 control-label"><?php echo $this->lang->line('checker'); ?></label>
					<div class = "col-sm-4">
						<input name="new_checker" type="file" class="col-md-4 form-control" />
						<span class="help-block"><?php echo form_error('new_checker'); ?></span>
					</div>
				</div>

				<input name="hidden" type="hidden" value="1" />

				<div class="col-sm-offset-2 form-actions">
					<button type="submit" class="btn btn-danger"><i class="glyphicon glyphicon-plus"></i> <?php echo $this->lang->line('add'); ?></button>
					<a class="btn btn-default" href="<?php echo site_url('admin/problem/viewAll/' . $page_offset); ?>"><?php echo $this->lang->line('cancel'); ?></a>
				</div>
			</form>
		</div>
	</div>
</div>