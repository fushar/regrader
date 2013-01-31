<div class="container">
	<div class="row">
		<div class="span12">
			<ul class="breadcrumb">
				<li><i class="icon-flag"></i> <?php echo $this->lang->line('categories'); ?></li>
				<li><span class="divider">|</span></li>
				<li><i class="icon-list-alt"></i> <?php echo $this->lang->line('category_list'); ?></li>
				<li><span class="divider">|</span></li>
				<li><i class="icon-plus"></i> <a href="<?php echo site_url('admin/category/edit/0/' . $page_offset); ?>"><?php echo $this->lang->line('add_new_category'); ?></a></li>
			</ul>
		</div>
	</div>
	
	<div class="row">
		<div class="span12">
			<?php if ($this->session->flashdata('add_successful')): ?>
			<div class="alert alert-success">
				<?php echo $this->lang->line('add_category_successful'); ?>
			</div>
			<?php endif; ?>

			<?php if ($this->session->flashdata('edit_successful')): ?>
			<div class="alert alert-success">
				<?php echo $this->lang->line('edit_category_successful'); ?>
			</div>
			<?php endif; ?>

			<?php if ($this->session->flashdata('delete_successful')): ?>
			<div class="alert alert-success">
				<?php echo $this->lang->line('delete_category_successful'); ?>
			</div>
			<?php endif; ?>

			<?php if ($this->session->flashdata('error') != ''): ?>
			<div class="alert alert-error">
				<?php echo $this->session->flashdata('error'); ?>
			</div>
			<?php endif; ?>

			<h3><?php echo $this->lang->line('category_list'); ?></h3>

			<?php if (empty($categories)) : ?>
				<p><i><?php echo $this->lang->line('no_category'); ?></i></p>
			<?php else: ?>
				<p><?php printf($this->lang->line('showing_categories'),  (($page_offset-1)*$items_per_page + 1), (($page_offset-1)*$items_per_page + count($categories)), $total_categories); ?></p>
			<?php endif; ?>

			<table class="table table-bordered table-striped">
				<thead>
					<tr>
						<th class="id-th"><i class="icon-tag icon-white"></i></th>
						<th><i class="icon-eye-open icon-white"></i> <?php echo $this->lang->line('name'); ?></th>
						<th class="category-membercount-th"><i class="icon-user icon-white"></i> <?php echo $this->lang->line('number_of_users'); ?></th>
						<th class="operations-th"><i class="icon-cog icon-white"></i></th>
					</tr>
				</thead>

				<tbody>
					<?php foreach ($categories as $v) : ?>
					<tr>
						<td class="id-td"><?php echo $v['id']; ?></td>
						<td><?php echo $v['name']; ?></td>
						<td class="category-membercount-td"><?php echo $v['user_cnt']; ?></td>
						<td class="operations-td"><a href="<?php echo site_url('admin/category/edit/' . $v['id'] . '/' . $page_offset); ?>" rel="tooltip" title="<?php echo $this->lang->line('edit'); ?>"><i class="icon-pencil"></i></a>
							<?php if ($v['id'] > 1) : ?>
								<a href="<?php echo site_url('admin/category/delete/' . $v['id'] . '/' . $page_offset); ?>" rel="tooltip" title="<?php echo $this->lang->line('delete'); ?>" onclick="return confirm('<?php printf($this->lang->line('confirm_delete_category'), $v['id'], $v['name']); ?>');"><i class="icon-trash"></i></td>
							<?php endif; ?>
						</tr>
					<?php endforeach; ?>
				</tbody>
			</table>
			
			<div class="pagination">
				<?php echo $pager; ?>
			</div>
		</div>
	</div>
	
</div>