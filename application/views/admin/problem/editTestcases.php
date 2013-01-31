<div class="container">
	<div class="row">
		<div class="span12">
			<ul class="breadcrumb">
				<li><i class="icon-book"></i> <?php echo $this->lang->line('problems'); ?></li>
				<li><span class="divider">|</span></li>
				<li><i class="icon-list-alt"></i> <?php echo $this->lang->line('problem'); ?> <?php echo $problem['id']; ?></li>
			</ul>
		</div>
	</div>
	
	<div class="row">
		<div class="span12">
			<?php if ($this->session->flashdata('add_successful') != ''): ?>
			<div class="alert alert-success">
				<?php echo $this->lang->line('add_testcase_successful'); ?>
			</div>
			<?php endif; ?>

			<?php if ($this->session->flashdata('delete_successful') != ''): ?>
			<div class="alert alert-success">
				<?php echo $this->lang->line('delete_testcase_successful'); ?>
			</div>
			<?php endif; ?>
			<?php if ($this->session->flashdata('error') != ''): ?>
			<div class="alert alert-error">
				<?php echo $this->session->flashdata('error'); ?>
			</div>
			<?php endif; ?>

			<h3><?php echo $this->lang->line('edit_problem_testcases'); ?></h3>
			<table class="table table-bordered table-striped">
				<thead>
					<tr>
						<th class="id-th"><i class="icon-tag icon-white"></i></th>
						<th><i class="icon-download icon-white"></i> <?php echo $this->lang->line('input'); ?></th>
						<th class="problem-filesize-th"><i class="icon-download icon-white"></i> <?php echo $this->lang->line('size'); ?></th>
						<th><i class="icon-upload icon-white"></i> <?php echo $this->lang->line('output'); ?></th>
						<th class="problem-filesize-th"><i class="icon-upload icon-white"></i> <?php echo $this->lang->line('size'); ?></th>
						<th class="operations-th"><i class="icon-cog icon-white"></i></th>
					</tr>
				</thead>
				<tbody>
					<?php foreach ($testcases as $v) : ?>
						<tr>
							<td class="id-td"><?php echo $v['id']; ?></td>
							<td><?php echo $v['input']; ?></td>	
							<td class="problem-filesize-td"><?php echo $v['input_size']; ?> B</td>	
							<td><?php echo $v['output']; ?></td>
							<td class="problem-filesize-td"><?php echo $v['output_size']; ?> B</td>
							<td class="operations-td">
								<a href="<?php echo site_url('admin/problem/downloadTestcaseInput/' . $v['id']); ?>" rel="tooltip" title="<?php echo $this->lang->line('download_input'); ?>"><i class="icon-download"></i></a>
								<a href="<?php echo site_url('admin/problem/downloadTestcaseOutput/' . $v['id']); ?>" rel="tooltip" title="<?php echo $this->lang->line('download_output'); ?>"><i class="icon-upload"></i></a>
								<a href="<?php echo site_url('admin/problem/deleteTestcase/' . $problem['id'] . '/'. $v['id'] . '/' . $page_offset); ?>" rel="tooltip" title="<?php echo $this->lang->line('delete'); ?>"><i class="icon-trash"></i></a>
							</td>
						</tr>
					<?php endforeach; ?>
				</tbody>
			</table>
			<hr />
			
			<h3><?php echo $this->lang->line('add_testcase'); ?></h3>
			<?php echo form_open_multipart('', array('class' => 'form-horizontal')); ?>
				<div class="control-group<?php echo form_error('new_input') == '' ? '' : ' error'; ?>">
					<label class="control-label"><?php echo $this->lang->line('input'); ?></label>
					<div class="controls">
						<input name="new_input" type="file" class="span4" />
						<span class="help-inline"><?php echo form_error('new_input'); ?></span>
					</div>
				</div>

				<div class="control-group<?php echo form_error('new_output') == '' ? '' : ' error'; ?>">
					<label class="control-label"><?php echo $this->lang->line('output'); ?></label>
					<div class="controls">
						<input name="new_output" type="file" class="span4" />
						<span class="help-inline"><?php echo form_error('new_output'); ?></span>
					</div>
				</div>

				<input name="hidden" type="hidden" value="1" />

				<div class="form-actions">
					<button type="submit" class="btn btn-danger"><i class="icon-plus icon-white"></i> <?php echo $this->lang->line('add'); ?></button>
					<a class="btn" href="<?php echo site_url('admin/problem/viewAll/' . $page_offset); ?>"><?php echo $this->lang->line('cancel'); ?></a>
				</div>
			</form>
		</div>
	</div>
</div>