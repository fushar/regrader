<div class="container">
	<div class="row">
		<div class="span12">
			<ul class="breadcrumb">
				<li><i class="icon-globe"></i> <?php echo $this->lang->line('languages'); ?></li>
				<li><span class="divider">|</span></li>
				<li><i class="icon-list-alt"></i> <?php echo $this->lang->line('language_list'); ?></li>
				<li><span class="divider">|</span></li>
				<li><i class="icon-plus"></i> <a href="<?php echo site_url('admin/language/edit/0/' . $page_offset); ?>"><?php echo $this->lang->line('add_new_language'); ?></a></li>
			</ul>
		</div>
	</div>
	
	<div class="row">
		<div class="span12">
			<?php if ($this->session->flashdata('add_successful')): ?>
			<div class="alert alert-success">
				<?php echo $this->lang->line('add_language_successful'); ?>
			</div>
			<?php endif; ?>

			<?php if ($this->session->flashdata('edit_successful')): ?>
			<div class="alert alert-success">
				<?php echo $this->lang->line('edit_language_successful'); ?>
			</div>
			<?php endif; ?>

			<?php if ($this->session->flashdata('delete_successful')): ?>
			<div class="alert alert-success">
				<?php echo $this->lang->line('delete_language_successful'); ?>
			</div>
			<?php endif; ?>

			<?php if ($this->session->flashdata('error') != ''): ?>
			<div class="alert alert-error">
				<?php echo $this->session->flashdata('error'); ?>
			</div>
			<?php endif; ?>

			<h3><?php echo $this->lang->line('language_list'); ?></h3>

			<?php if (empty($languages)) : ?>
				<p><i><?php echo $this->lang->line('no_language'); ?></i></p>
			<?php else: ?>
				<p><?php printf($this->lang->line('showing_languages'),  (($page_offset-1)*$items_per_page + 1), (($page_offset-1)*$items_per_page + count($languages)), $total_languages); ?></p>
			<?php endif; ?>

			<table class="table table-bordered table-striped">
				<thead>
					<tr>
						<th class="id-th"><i class="icon-tag icon-white"></i></th>
						<th><i class="icon-eye-open icon-white"></i> <?php echo $this->lang->line('name'); ?></th>
						<th class="operations-th"><i class="icon-cog icon-white"></i></th>
					</tr>
				</thead>

				<tbody>
					<?php foreach ($languages as $v) : ?>
					<tr>
						<td class="id-td"><?php echo $v['id']; ?></td>
						<td><?php echo $v['name']; ?></td>
						<td class="operations-td"><a href="<?php echo site_url('admin/language/edit/' . $v['id'] . '/' . $page_offset); ?>" rel="tooltip" title="<?php echo $this->lang->line('edit'); ?>"><i class="icon-pencil"></i></a>
							<a href="<?php echo site_url('admin/language/delete/' . $v['id'] . '/' . $page_offset); ?>" rel="tooltip" title="<?php echo $this->lang->line('delete'); ?>" onclick="return confirm('<?php printf($this->lang->line('confirm_delete_language'), $v['id'], $v['name']); ?>');"><i class="icon-trash"></i></td>
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