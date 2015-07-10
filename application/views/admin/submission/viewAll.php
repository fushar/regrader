<div class="container">
	<div class="row">
		<div class="col-md-12">
			<ul class="breadcrumb">
				<li><i class="glyphicon glyphicon-share"></i> <?php echo $this->lang->line('submissions'); ?></li>
				<li><i class="glyphicon glyphicon-list-alt"></i> <?php echo $this->lang->line('submission_list'); ?></li>
			</ul>
		</div>
	</div>
	
	<div class="row">
		<div class="col-md-12">
			<form class="form-inline" method="post">
				<select name="contest_id" class="col-md-3">
					<option value="0">(<?php echo $this->lang->line('all_contests'); ?>)</option>
				<?php foreach ($contests as $v) : ?>
					<option value="<?php echo $v['id']; ?>"<?php if ($v['id'] == $contest_id) echo ' selected="selected"'; ?>><?php echo $v['name']; ?></option>
				<?php endforeach; ?>
				</select>
				<select name="problem_id" class="col-md-3">
					<option value="0">(<?php echo $this->lang->line('all_problems'); ?>)</option>
				<?php foreach ($problems as $v) : ?>
					<option value="<?php echo $v['id']; ?>"<?php if ($v['id'] == $problem_id) echo ' selected="selected"'; ?>><?php echo $v['name']; ?></option>
				<?php endforeach; ?>
				</select>
				<select name="user_id" class="col-md-3">
					<option value="0">(<?php echo $this->lang->line('all_users'); ?>)</option>
				<?php foreach ($users as $v) : if ($v['id'] == 1) continue; ?>
					<option value="<?php echo $v['id']; ?>"<?php if ($v['id'] == $user_id) echo ' selected="selected"'; ?>><?php echo $v['name']; ?></option>
				<?php endforeach; ?>
				</select>
				<button type="submit" class="btn btn-default" rel="tooltip" title="<?php echo $this->lang->line('filter'); ?>"><i class="glyphicon glyphicon-search"></i></button>
			</form>
			
			<?php if (count($submissions) == 0) : ?>
				<p><i><?php echo $this->lang->line('no_submission'); ?></i></p>
			<?php else: ?>
				<p><?php printf($this->lang->line('showing_submissions'),  (($page_offset-1)*$items_per_page + 1), (($page_offset-1)*$items_per_page + count($submissions)), $total_submissions); ?></p>
			<?php endif; ?>
			
			<table class="table table-bordered table-striped">
				<thead>
					<tr>
						<th class="id-th"><i class="glyphicon glyphicon-tag"></i></th>
						<th><i class="glyphicon glyphicon-fire "></i> <?php echo $this->lang->line('contest'); ?></th>
						<th><i class="glyphicon glyphicon-user "></i> <?php echo $this->lang->line('user'); ?></th>
						<th><i class="glyphicon glyphicon-book "></i> <?php echo $this->lang->line('problem'); ?></th>
						<th class="submission-language-th"><i class="glyphicon glyphicon-globe"></i> <?php echo $this->lang->line('language'); ?></th>
						<th class="submission-time-th"><i class="glyphicon glyphicon-time "></i> <?php echo $this->lang->line('time'); ?></th>
						<th class="submission-verdict-th"><i class="glyphicon glyphicon-briefcase "></i> <?php echo $this->lang->line('verdict'); ?></th>
						<th class="operations-th"><i class="glyphicon glyphicon-cog "></i></th>
					</tr>
				</thead>

				<tbody>
					<?php foreach ($submissions as $v) : ?>
					<tr>
						<td class="id-td"><?php echo $v['id']; ?></td>
						<td><?php echo $v['contest_name']; ?></td>
						<td><?php echo $v['user_name']; ?></td>
						<td><?php echo $v['problem_alias']; ?></td>
						<td><?php echo $v['language_name']; ?></td>
						<td><?php echo $v['submit_time']; ?></td>
						<td class="submission-verdict-td">
						<?php
							$verdict = max(0, $v['verdict']);
							if ($verdict == 0)
								echo '<span class="label label-default"><i class="glyphicon glyphicon-refresh glyphicon"></i> ' . $this->lang->line('verdict_abbr_0') . '</span>';
							else if ($verdict == 2)
								echo '<span class="label label-success"><i class="glyphicon glyphicon-ok glyphicon"></i> ' . $this->lang->line('verdict_abbr_2') . '</span>';
							else if ($verdict == 3)
								echo '<span class="label label-danger"><i class="glyphicon glyphicon-remove glyphicon"></i> ' . $this->lang->line('verdict_abbr_3') . '</span>';
							else if ($verdict == 99)
								echo '<span class="label label-info">' . $this->lang->line('verdict_abbr_99') . '</span>';
							else
								echo '<span class="label label-warning"><i class="glyphicon glyphicon-warning-sign glyphicon"></i> ' . $this->lang->line('verdict_abbr_' . $verdict) . '</span>';
						?>
						</td>
						<td class="operations-td">
							<a href="<?php echo site_url('admin/submission/view/' . $v['id'] . '/' . $contest_id . '/' . $problem_id . '/' . $user_id . '/' . $page_offset); ?>" rel="tooltip" title="<?php echo $this->lang->line('view'); ?>"><i class="glyphicon glyphicon-search"></i></a>
							<a href="<?php echo site_url('admin/submission/regrade/' . $v['id'] . '/' . $contest_id . '/' . $problem_id . '/' . $user_id . '/' . $page_offset); ?>" rel="tooltip" title="<?php echo $this->lang->line('regrade'); ?>"><i class="glyphicon glyphicon-repeat"></i></a>
							<?php if ($v['verdict'] != 99) : ?>
								<a href="<?php echo site_url('admin/submission/ignore/' . $v['id'] . '/' . $contest_id . '/' . $problem_id . '/' . $user_id . '/' . $page_offset); ?>" rel="tooltip" title="<?php echo $this->lang->line('ignore'); ?>"><i class="glyphicon glyphicon-trash"></i></a>
							<?php endif; ?>
						</td>
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