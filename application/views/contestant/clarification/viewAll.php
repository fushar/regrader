<div class="container">
	<div class="modal hide" id="submit">

		<?php if ($this->session->flashdata('contest_has_ended') || strtotime($active_contest_end_time) < time()) : ?>
			<div class="modal-header">
				<button type="button" class="close" data-dismiss="modal">&times;</button>
				<h3><?php echo $this->lang->line('new_clarification'); ?></h3>
			</div>
			<div class="modal-body">
				<div class="alert alert-error">
					<?php echo $this->lang->line('contest_has_ended'); ?>
				</div>
			</div>
			<div class="modal-footer">
				<a href="#" class="btn" data-dismiss="modal"><?php echo $this->lang->line('cancel'); ?></a>
			</div>
		<?php else: ?>
			<form method="post">
				<div class="modal-header">
					<button type="button" class="close" data-dismiss="modal">&times;</button>
					<h3><?php echo $this->lang->line('new_clarification'); ?></h3>
				</div>
				<div class="modal-body">
					<div class="control-group<?php echo form_error('form[title]') == '' ? '' : ' error'; ?>">
						<label class="control-label"><?php echo $this->lang->line('title'); ?>:</label>
						<div class="controls">
							<input name="form[title]" type="text" class="col-md-4" maxlength="255" value="<?php echo set_value('form[title]'); ?>"/>
							<span class="help-inline"><?php echo form_error('form[title]'); ?></span>
						</div>
					</div>
					<div class="control-group<?php echo form_error('form[content]') == '' ? '' : ' error'; ?>">
						<label class="control-label"><?php echo $this->lang->line('content'); ?>:</label>
						<div class="controls">
							<textarea name="form[content]" type="text" class="col-md-4"><?php echo set_value('form[content]'); ?></textarea>
							<span class="help-inline"><?php echo form_error('form[content]'); ?></span>
						</div>
					</div>
				</div>
				<div class="modal-footer">
					<button type="submit" class="btn btn-danger"><i class="icon-plus icon-white"></i> <?php echo $this->lang->line('ask'); ?></button>
					<a href="#" class="btn" data-dismiss="modal"><?php echo $this->lang->line('cancel'); ?></a>
				</div>
			</form>
		<?php endif; ?>
	</div>

	<div class="row">
		<div class="col-md-12">
			<ul class="breadcrumb">
				<li><i class="icon-envelope"></i> <?php echo $this->lang->line('clarifications'); ?></li>
				<li><span class="divider">|</span></li>
				<li><i class="icon-list-alt"></i> <?php echo $this->lang->line('clarification_list'); ?></li>
				<li><span class="divider">|</span></li>
				<li><i class="icon-plus"></i> <a data-toggle="modal" href="#submit"><?php echo $this->lang->line('add_new_clarification'); ?></a></li>
				<li class="pull-right">
					<i class="icon-time"></i> <span id="contest_clock"></span> <?php echo $this->lang->line('remaining'); ?>
				</li>
			</ul>
		</div>
	</div>
	
	<div class="row">
		<div class="col-md-12">
			<?php if ($this->session->flashdata('submit_successful')): ?>
			<div class="alert alert-success">
				<?php echo $this->lang->line('add_clarification_successful'); ?>
			</div>
			<?php endif; ?>

			<?php if (count($clarifications) == 0) : ?>
				<p><i><?php echo $this->lang->line('no_clarification'); ?></i></p>
			<?php else: ?>
				<p><?php printf($this->lang->line('showing_clarifications'),  (($page_offset-1)*$items_per_page + 1), (($page_offset-1)*$items_per_page + count($clarifications)), $total_clarifications); ?></p>
			<?php endif; ?>
			
			<table class="table table-bordered table-striped">
				<thead>
					<tr>
						<th class="id-th"><i class="icon-tag icon-white"></i></th>
						<th><i class="icon-user icon-white"></i> <?php echo $this->lang->line('by'); ?></th>
						<th><i class="icon-eye-open icon-white"></i> <?php echo $this->lang->line('title'); ?></th>
						<th><i class="icon-time icon-white"></i> <?php echo $this->lang->line('time'); ?></th>
						<th><i class="icon-star icon-white"></i> <?php echo $this->lang->line('status'); ?></th>
						<th class="operations-th"><i class="icon-cog icon-white"></i></th>
					</tr>
				</thead>

				<tbody>
					<?php foreach ($clarifications as $v) : ?>
					<tr>
						<td class="id-td"><?php echo $v['id']; ?></td>
						<td><?php echo  $v['user_id'] == 1 ? '<span class="label label-important">Administrator</span>' : $v['user_name']; ?></td>
						<td><?php if (isset($v['unread'])) echo '<span class="label label-info">' . $this->lang->line('new') . '</span> '; ?><?php echo $v['title']; ?></td>
						<td><?php echo $v['clar_time']; ?></td>
						<td class="clarification-answered-td"><?php echo $v['user_id'] != 1 ? $v['answered'] ? '<span class="label label-success">' . $this->lang->line('answered') .'</span>' : '<span class="label label-important">' . $this->lang->line('not_answered') . '</span>' : '-'; ?></td>
						<td class="operations-td"><a href="<?php echo site_url('contestant/clarification/view/' . $v['id'] . '/' . $page_offset); ?>" rel="tooltip" title="<?php echo $this->lang->line('view'); ?>"><i class="icon-search"></i></a></td>
						</tr>
					<?php endforeach; ?>
				</tbody>
			</table>
			
			<div class="pagination">
				<?php echo $pager; ?>
			</div>
		</div>
	</div>

	<?php if (isset($submit_error)) : ?>
		<script type="text/javascript">
			$('#submit').modal('show');
		</script>
	<?php endif; ?>
</div>

