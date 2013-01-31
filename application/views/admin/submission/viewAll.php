<div class="container">
	<div class="row">
		<div class="span12">
			<ul class="breadcrumb">
				<li><i class="icon-share"></i> <?php echo $this->lang->line('submissions'); ?></li>
				<li><span class="divider">|</span></li>
				<li><i class="icon-list-alt"></i> <?php echo $this->lang->line('submission_list'); ?></li>
			</ul>
		</div>
	</div>
	
	<div class="row">
		<div class="span12">
			<form class="form-inline" method="post">
				<select name="contest_id" class="span3">
					<option value="0">(<?php echo $this->lang->line('all_contests'); ?>)</option>
				<?php foreach ($contests as $v) : ?>
					<option value="<?php echo $v['id']; ?>"<?php if ($v['id'] == $contest_id) echo ' selected="selected"'; ?>><?php echo $v['name']; ?></option>
				<?php endforeach; ?>
				</select>
				<select name="problem_id" class="span3">
					<option value="0">(<?php echo $this->lang->line('all_problems'); ?>)</option>
				<?php foreach ($problems as $v) : ?>
					<option value="<?php echo $v['id']; ?>"<?php if ($v['id'] == $problem_id) echo ' selected="selected"'; ?>><?php echo $v['name']; ?></option>
				<?php endforeach; ?>
				</select>
				<select name="user_id" class="span3">
					<option value="0">(<?php echo $this->lang->line('all_users'); ?>)</option>
				<?php foreach ($users as $v) : if ($v['id'] == 1) continue; ?>
					<option value="<?php echo $v['id']; ?>"<?php if ($v['id'] == $user_id) echo ' selected="selected"'; ?>><?php echo $v['name']; ?></option>
				<?php endforeach; ?>
				</select>
				<button type="submit" class="btn" rel="tooltip" title="<?php echo $this->lang->line('filter'); ?>"><i class="icon-search"></i></button>
			</form>

			<?php if (count($submissions) == 0) : ?>
				<p><i><?php echo $this->lang->line('no_submission'); ?></i></p>
			<?php else: ?>
				<p><?php printf($this->lang->line('showing_submissions'),  (($page_offset-1)*$items_per_page + 1), (($page_offset-1)*$items_per_page + count($submissions)), $total_submissions); ?></p>
			<?php endif; ?>
			
			<table class="table table-bordered table-striped">
				<thead>
					<tr>
						<th class="id-th"><i class="icon-tag icon-white"></i></th>
						<th><i class="icon-fire icon-white"></i> <?php echo $this->lang->line('contest'); ?></th>
						<th><i class="icon-user icon-white"></i> <?php echo $this->lang->line('user'); ?></th>
						<th><i class="icon-book icon-white"></i> <?php echo $this->lang->line('problem'); ?></th>
						<th class="submission-language-th"><i class="icon-globe icon-white"></i> <?php echo $this->lang->line('language'); ?></th>
						<th><i class="icon-time icon-white"></i> <?php echo $this->lang->line('time'); ?></th>
						<th class="submission-verdict-th"><i class="icon-briefcase icon-white"></i> <?php echo $this->lang->line('verdict'); ?></th>
						<th class="operations-th"><i class="icon-cog icon-white"></i></th>
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
								echo '<span class="badge badge-inverse"><i class="icon-refresh icon-white"></i> ' . $this->lang->line('verdict_abbr_0') . '</span>';
							else if ($verdict == 2)
								echo '<span class="badge badge-success"><i class="icon-ok icon-white"></i> ' . $this->lang->line('verdict_abbr_2') . '</span>';
							else if ($verdict == 3)
								echo '<span class="badge badge-important"><i class="icon-remove icon-white"></i> ' . $this->lang->line('verdict_abbr_3') . '</span>';
							else if ($verdict == 99)
								echo '<span class="badge">' . $this->lang->line('verdict_abbr_99') . '</span>';
							else
								echo '<span class="badge badge-warning"><i class="icon-warning-sign icon-white"></i> ' . $this->lang->line('verdict_abbr_' . $verdict) . '</span>';
						?>
						</td>
						<td class="operations-td">
							<a href="<?php echo site_url('admin/submission/view/' . $v['id'] . '/' . $contest_id . '/' . $problem_id . '/' . $user_id . '/' . $page_offset); ?>" rel="tooltip" title="<?php echo $this->lang->line('view'); ?>"><i class="icon-search"></i></a>
							<a href="<?php echo site_url('admin/submission/regrade/' . $v['id'] . '/' . $contest_id . '/' . $problem_id . '/' . $user_id . '/' . $page_offset); ?>" rel="tooltip" title="<?php echo $this->lang->line('regrade'); ?>"><i class="icon-repeat"></i></a>
							<?php if ($v['verdict'] != 99) : ?>
								<a href="<?php echo site_url('admin/submission/ignore/' . $v['id'] . '/' . $contest_id . '/' . $problem_id . '/' . $user_id . '/' . $page_offset); ?>" rel="tooltip" title="<?php echo $this->lang->line('ignore'); ?>"><i class="icon-trash"></i></a>
							<?php endif; ?>
						</td>
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