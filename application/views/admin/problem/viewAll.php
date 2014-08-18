<div class="container">
	<div class="row">
		<div class="col-md-12">
			<ul class="breadcrumb">
				<li><i class="glyphicon glyphicon-book"></i> <?php echo $this->lang->line('problems'); ?></li>
				<li><i class="glyphicon glyphicon-list-alt"></i> <?php echo $this->lang->line('problem_list'); ?></li>
				<li><i class="glyphicon glyphicon-plus"></i> <a href="<?php echo site_url('admin/problem/edit/0/' . $page_offset); ?>"><?php echo $this->lang->line('add_new_problem'); ?></a></li>
			</ul>
		</div>
	</div>
	
	<div class="row">
		<div class="col-md-12">
			<?php if ($this->session->flashdata('add_successful')): ?>
			<div class="alert alert-success">
				<?php echo $this->lang->line('add_problem_successful'); ?>
			</div>
			<?php endif; ?>

			<?php if ($this->session->flashdata('edit_successful')): ?>
			<div class="alert alert-success">
				<?php echo $this->lang->line('edit_problem_successful'); ?>
			</div>
			<?php endif; ?>

			<?php if ($this->session->flashdata('delete_successful')): ?>
			<div class="alert alert-success">
				<?php echo $this->lang->line('delete_problem_successful'); ?>
			</div>
			<?php endif; ?>

			<?php if ($this->session->flashdata('error') != ''): ?>
			<div class="alert alert-error">
				<?php echo $this->session->flashdata('error'); ?>
			</div>
			<?php endif; ?>

			<h3><?php echo $this->lang->line('problem_list'); ?></h3>

			<?php if (count($problems) == 0) : ?>
				<p><i><?php echo $this->lang->line('no_problem'); ?></i></p>
			<?php else: ?>
				<p><?php printf($this->lang->line('showing_problems'),  (($page_offset-1)*$items_per_page + 1), (($page_offset-1)*$items_per_page + count($problems)), $total_problems); ?></p>
			<?php endif; ?>
			
			<table class="table table-bordered table-striped">
				<thead>
					<tr>
						<th class="id-th"><i class="glyphicon glyphicon-tag glyphicon-white"></i></th>
						<th><i class="glyphicon glyphicon-eye-open glyphicon-white"></i> <?php echo $this->lang->line('name'); ?></th>
						<th><i class="glyphicon glyphicon-user glyphicon-white"></i> <?php echo $this->lang->line('author'); ?></th>
						<th class="problem-limits-th"><i class="glyphicon glyphicon-file glyphicon-white"></i> <?php echo $this->lang->line('testcases'); ?></th>
						<th class="problem-limits-th"><i class="glyphicon glyphicon-calendar glyphicon-white"></i> <?php echo $this->lang->line('time_limit'); ?></th>
						<th class="problem-limits-th"><i class="glyphicon glyphicon-download-alt glyphicon-white"></i> <?php echo $this->lang->line('memory_limit'); ?></th>
						<th class="operations-th"><i class="glyphicon glyphicon-cog glyphicon-white"></i></th>
					</tr>
				</thead>

				<tbody>
					<?php foreach ($problems as $v) : ?>
					<tr>
						<td class="id-td"><?php echo $v['id']; ?></td>
						<td><?php echo $v['name']; ?></td>
						<td><?php echo $v['author']; ?></td>
						<td><?php echo $v['tc_count']; ?></td>
						<td><?php echo $v['time_limit']; ?> <?php echo $this->lang->line('second'); ?></td>
						<td><?php echo $v['memory_limit']; ?> MB</td>
						<td class="operations-td">
							<a href="<?php echo site_url('admin/problem/edit/' . $v['id'] . '/' . $page_offset); ?>" rel="tooltip" title="<?php echo $this->lang->line('edit'); ?>"><i class="glyphicon glyphicon-pencil"></i></a>
							<a href="<?php echo site_url('admin/problem/editTestcases/' . $v['id'] . '/' . $page_offset); ?>" rel="tooltip" title="<?php echo $this->lang->line('testcase'); ?>"><i class="glyphicon glyphicon-file"></i></a>
							<a href="<?php echo site_url('admin/problem/editChecker/' . $v['id'] . '/' . $page_offset); ?>" rel="tooltip" title="<?php echo $this->lang->line('checker'); ?>"><i class="glyphicon glyphicon-check"></i></a>
							<a href="<?php echo site_url('admin/problem/delete/' . $v['id'] . '/' . $page_offset); ?>" rel="tooltip" title="<?php echo $this->lang->line('delete'); ?>" onclick="return confirm('<?php printf($this->lang->line('confirm_delete_problem'), $v['id'], $v['name']); ?>');"><i class="glyphicon glyphicon-trash"></i></td>
						</tr>
					<?php endforeach; ?>
				</tbody>
			</table>
			
			<ul class="pagination">
				<?php echo $pager; ?>
			</ul>
		</div>
	</div>
	
</div>