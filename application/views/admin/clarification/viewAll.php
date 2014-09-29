<div class="container">
	<div class="modal" id="submit">
		<form method="post">
			<div class = "modal-dialog">
				<div class = "modal-content">
					<div class="modal-header">
						<button type="button" class="close" data-dismiss="modal">&times;</button>
						<h3><?php echo $this->lang->line('new_clarification'); ?></h3>
					</div>
					<div class="modal-body">
						<div class="form-group<?php echo form_error('form[contest_id]') == '' ? '' : ' has-error'; ?>">
							<label ><?php echo $this->lang->line('contest'); ?>:</label>
							<select name="form[contest_id]" class="form-control">
								<?php foreach ($contests as $v) :?>
									<option value="<?php echo $v['id']; ?>" <?php echo set_select('form[contest_id]', $v['id'], FALSE); ?>><?php echo $v['name']; ?></option>
								<?php endforeach; ?>
							</select>
							<span class="help-block"><?php echo form_error('form[contest_id]'); ?></span>
							<?php if (empty($contests)) : ?>
								<p class="help-block"><i><?php echo $this->lang->line('no_contest'); ?></i></p>
							<?php endif; ?>
						</div>
						<div class="form-group<?php echo form_error('form[title]') == '' ? '' : ' has-error'; ?>">
							<label class="control-label"><?php echo $this->lang->line('title'); ?>:</label>
							<div class="controls">
								<input name="form[title]" type="text" class="form-control" maxlength="255" value="<?php echo set_value('form[title]'); ?>"/>
								<span class="help-block"><?php echo form_error('form[title]'); ?></span>
							</div>
						</div>
						<div class="form-group<?php echo form_error('form[content]') == '' ? '' : ' has-error'; ?>">
							<label class="control-label"><?php echo $this->lang->line('content'); ?>:</label>
							<div class="controls">
								<textarea name="form[content]" type="text" class="form-control"><?php echo set_value('form[content]'); ?></textarea>
								<span class="help-block"><?php echo form_error('form[content]'); ?></span>
							</div>
						</div>
					</div>
					<div class="modal-footer">
						<button type="submit" class="btn btn-danger"><i class="glyphicon glyphicon-plus"></i> <?php echo $this->lang->line('broadcast'); ?></button>
						<a href="#" class="btn btn-default" data-dismiss="modal"><?php echo $this->lang->line('cancel'); ?></a>
					</div>
				</div>
			</div>
		</form>
	</div>

	<div class="row">
		<div class="col-md-12">
			<ul class="breadcrumb">
				<li><i class="glyphicon glyphicon-envelope"></i> <?php echo $this->lang->line('clarifications'); ?></li>
				<li><i class="glyphicon glyphicon-list-alt"></i> <?php echo $this->lang->line('clarification_list'); ?></li>
				<li><i class="glyphicon glyphicon-plus"></i> <a data-toggle="modal" href="#submit"><?php echo $this->lang->line('broadcast_new_clarification'); ?></a></li>
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

			<?php if ($this->session->flashdata('answer_successful')): ?>
			<div class="alert alert-success">
				<?php echo $this->lang->line('answer_clarification_successful'); ?>
			</div>
			<?php endif; ?>

			<form class="form-inline" method="post">
				<select name="contest_id" class="col-md-3">
					<option value="0">(<?php echo $this->lang->line('all_contests'); ?>)</option>
				<?php foreach ($contests as $v) : ?>
					<option value="<?php echo $v['id']; ?>"<?php if ($v['id'] == $contest_id) echo ' selected="selected"'; ?>><?php echo $v['name']; ?></option>
				<?php endforeach; ?>
				</select>
				<select name="user_id" class="col-md-3">
					<option value="0">(<?php echo $this->lang->line('all_users'); ?>)</option>
				<?php foreach ($users as $v) : ?>
					<option value="<?php echo $v['id']; ?>"<?php if ($v['id'] == $user_id) echo ' selected="selected"'; ?>><?php echo $v['name']; ?></option>
				<?php endforeach; ?>
				</select>
				<button type="submit" class="btn btn-default" rel="tooltip" title="<?php echo $this->lang->line('filter'); ?>"><i class="glyphicon glyphicon-search"></i></button>
			</form>
			

			<?php if (count($clarifications) == 0) : ?>
				<p><i><?php echo $this->lang->line('no_clarification'); ?></i></p>
			<?php else: ?>
				<p><?php printf($this->lang->line('showing_clarifications'),  (($page_offset-1)*$items_per_page + 1), (($page_offset-1)*$items_per_page + count($clarifications)), $total_clarifications); ?></p>
			<?php endif; ?>
			
			<table class="table table-bordered table-striped">
				<thead>
					<tr>
						<th class="id-th"><i class="glyphicon glyphicon-tag"></i></th>
						<th><i class="glyphicon glyphicon-fire"></i> <?php echo $this->lang->line('contest'); ?></th>
						<th><i class="glyphicon glyphicon-user"></i> <?php echo $this->lang->line('by'); ?></th>
						<th><i class="glyphicon glyphicon-eye-open"></i> <?php echo $this->lang->line('title'); ?></th>
						<th class="clarification-time-th"><i class="glyphicon glyphicon-time"></i> <?php echo $this->lang->line('time'); ?></th>
						<th class="clarification-status-th"><i class="glyphicon glyphicon-star"></i> <?php echo $this->lang->line('status'); ?></th>
						<th class="operations-th"><i class="glyphicon glyphicon-cog"></i></th>
					</tr>
				</thead>

				<tbody>
					<?php foreach ($clarifications as $v) : ?>
					<tr>
						<td class="id-td"><?php echo $v['id']; ?></td>
						<td><?php echo $v['contest_name']; ?></td>
						<td><?php echo $v['user_id'] == 1 ? '<span class="label label-danger">Administrator</span>' : $v['user_name']; ?></td>
						<td><?php if (isset($v['unread'])) echo '<span class="label label-info">' . $this->lang->line('new') . '</span> '; ?><?php echo $v['title']; ?></td>
						<td><?php echo $v['clar_time']; ?></td>
						<td class="clarification-answered-td"><?php echo $v['user_id'] != 1 ? $v['answered'] ? '<span class="label label-success">' . $this->lang->line('answered') . '</span>' : '<span class="label label-danger">' . $this->lang->line('not_answered') . '</span>' : '-'; ?></td>
						<td class="operations-td"><a href="<?php echo site_url('admin/clarification/view/' . $v['id'] . '/' . $contest_id . '/' . $user_id . '/' . $page_offset); ?>" rel="tooltip" title="<?php echo $this->lang->line('view'); ?>"><i class="glyphicon glyphicon-search"></i></a></td>
						</tr>
					<?php endforeach; ?>
				</tbody>
			</table>
			
			<ul class="pagination">
				<?php echo $pager; ?>
			</ul>
		</div>
	</div>

	<?php if (isset($submit_error)) : ?>
		<script type="text/javascript">
			$('#submit').modal('show');
		</script>
	<?php endif; ?>
</div>

