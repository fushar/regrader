<div class="container">
	<div class="row">
		<div class="span12">
			<ul class="breadcrumb">
				<li><i class="icon-user"></i> <?php echo $this->lang->line('users'); ?></li>
				<li><span class="divider">|</span></li>
				<li><i class="icon-list-alt"></i> <?php echo $this->lang->line('user_list'); ?></li>
				<li><span class="divider">|</span></li>
				<li><i class="icon-plus"></i> <a href="<?php echo site_url('admin/user/edit/0/' . $category_id  . '/' . $page_offset); ?>"><?php echo $this->lang->line('add_new_user'); ?></a></li>
			</ul>
		</div>
	</div>
	
	<div class="row">
		<div class="span12">
			<form class="form-inline" method="post">
				<select name="category_id" class="span3">
					<option value="0">(<?php echo $this->lang->line('all_categories'); ?>)</option>
				<?php foreach ($categories as $v) : ?>
					<option value="<?php echo $v['id']; ?>"<?php if ($v['id'] == $category_id) echo ' selected="selected"'; ?>><?php echo $v['name']; ?></option>
				<?php endforeach; ?>
				</select>
				<button type="submit" class="btn" rel="tooltip" title="Saring"><i class="icon-search"></i></button>
			</form>

			<?php if ($this->session->flashdata('add_successful')): ?>
			<div class="alert alert-success">
				<?php echo $this->lang->line('add_user_successful'); ?>
			</div>
			<?php endif; ?>

			<?php if ($this->session->flashdata('edit_successful')): ?>
			<div class="alert alert-success">
				<?php echo $this->lang->line('edit_user_successful'); ?>
			</div>
			<?php endif; ?>

			<?php if ($this->session->flashdata('delete_successful')): ?>
			<div class="alert alert-success">
				<?php echo $this->lang->line('delete_user_successful'); ?>
			</div>
			<?php endif; ?>

			<?php if ($this->session->flashdata('error') != ''): ?>
			<div class="alert alert-error">
				<i c<?php echo $this->session->flashdata('error'); ?>
			</div>
			<?php endif; ?>

			<h3><?php echo $this->lang->line('user_list'); ?></h3>

			<?php if (count($users) == 0) : ?>
				<p><i><?php echo $this->lang->line('no_user'); ?></i></p>
			<?php else: ?>
				<p><?php printf($this->lang->line('showing_users'),  (($page_offset-1)*$items_per_page + 1), (($page_offset-1)*$items_per_page + count($users)), $total_users); ?></p>
			<?php endif; ?>

			<table class="table table-bordered table-striped">
				<thead>
					<tr>
						<th class="id-th"><i class="icon-tag icon-white"></i></th>
						<th><i class="icon-eye-open icon-white"></i> <?php echo $this->lang->line('username'); ?></th>
						<th><i class="icon-eye-open icon-white"></i> <?php echo $this->lang->line('name'); ?></th>
						<th><i class="icon-briefcase icon-white"></i> <?php echo $this->lang->line('institution'); ?></th>
						<th><i class="icon-flag icon-white"></i> <?php echo $this->lang->line('category'); ?></th>
						<th class="operations-th"><i class="icon-cog icon-white"></i></th>
					</tr>
				</thead>

				<tbody>
					<?php foreach ($users as $v) : ?>
					<tr>
						<td class="id-td"><?php echo $v['id']; ?></td>
						<td><?php echo $v['username']; ?></td>
						<td><?php echo $v['name']; ?></td>
						<td><?php echo $v['institution']; ?></td>
						<td><?php echo $v['category_name']; ?></td>
						<td class="operations-td"><a href="<?php echo site_url('admin/user/edit/' . $v['id'] . '/' . $category_id . '/' . $page_offset); ?>" rel="tooltip" title="<?php echo $this->lang->line('edit'); ?>"><i class="icon-pencil"></i></a>
							<?php if ($v['id'] != 1) : ?>
								<a href="<?php echo site_url('admin/user/delete/' . $v['id'] . '/' . $category_id . '/' . $page_offset); ?>" rel="tooltip" title="Hapus" onclick="return confirm('<?php printf($this->lang->line('confirm_delete_user'), $v['id'], $v['name']); ?>');"><i class="icon-trash"></i></td>
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