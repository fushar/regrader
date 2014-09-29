<div class="container">
	<div class="row">
		<div class="col-md-12">
			<ul class="breadcrumb">
				<li><i class="glyphicon glyphicon-file"></i> <?php echo $this->lang->line('files'); ?></li>
				<li><i class="glyphicon lyphicon-list-alt"></i> <?php echo $this->lang->line('file_list'); ?></li>
				<li><i class="glyphicon glyphicon-plus"></i> <a href="<?php echo site_url('admin/file/add'); ?>"><?php echo $this->lang->line('add_new_file'); ?></a></li>
			</ul>
		</div>
	</div>
	
	<div class="row">
		<div class="col-md-12">
			<?php if ($this->session->flashdata('add_successful')): ?>
			<div class="alert alert-success">
				<?php echo $this->lang->line('add_file_successful'); ?>
			</div>
			<?php endif; ?>

			<?php if ($this->session->flashdata('delete_successful')): ?>
			<div class="alert alert-success">
				<?php echo $this->lang->line('delete_file_successful'); ?>
			</div>
			<?php endif; ?>

			<?php if ($this->session->flashdata('error') != ''): ?>
			<div class="alert alert-danger">
				<?php echo $this->session->flashdata('error'); ?>
			</div>
			<?php endif; ?>

			<h3><?php echo $this->lang->line('file_list'); ?></h3>
			<?php if (empty($files)) : ?>
				<p><i><?php echo $this->lang->line('no_file'); ?></i></p>
			<?php endif; ?>

			<table class="table table-bordered table-striped">
				<thead>
					<tr>
						<th><i class="glyphicon glyphicon-eye-open"></i> <?php echo $this->lang->line('name'); ?></th>
						<th class="operations-th"><i class="glyphicon glyphicon-cog"></i></th>
					</tr>
				</thead>

				<tbody>
					<?php foreach ($files as $v) : ?>
					<tr>
						<td><a href="<?php echo base_url() . 'files/' . rawurlencode($v); ?>"><?php echo $v; ?></a></td>
						<td class="operations-td">
							<a href="<?php echo site_url('admin/file/delete/' . $v); ?>" rel="tooltip" title="<?php echo $this->lang->line('delete'); ?>" onclick="return confirm('<?php printf($this->lang->line('confirm_delete_file'), $v); ?>');"><i class="glyphicon glyphicon-trash"></i></td>
						</tr>
					<?php endforeach; ?>
				</tbody>
			</table>
			
		</div>
	</div>
	
</div>