<div class="container">
	<div class="row">
		<div class="span12">
			<ul class="breadcrumb">
				<li><i class="icon-share"></i> <?php echo $this->lang->line('submissions'); ?></li>
				<li><span class="divider">|</span></li>
				<li><i class="icon-list-alt"></i> <?php echo $this->lang->line('submission_list'); ?></li>
				
				<li class="pull-right">
					<i class="icon-time"></i> <span id="contest_clock"></span> <?php echo $this->lang->line('remaining'); ?>
				</li>
			</ul>
		</div>
	</div>
	
	<div class="row">
		<div class="span12">
			<?php if ($this->session->flashdata('submit_successful')): ?>
			<div class="alert alert-success">
				<?php echo $this->lang->line('submit_successful'); ?>
			</div>
			<?php endif; ?>

			<?php if (count($submissions) == 0) : ?>
				<p><i><?php echo $this->lang->line('no_submission'); ?></i></p>
			<?php else: ?>
				<p><?php printf($this->lang->line('showing_submissions'),  (($page_offset-1)*$items_per_page + 1), (($page_offset-1)*$items_per_page + count($submissions)), $total_submissions); ?></p>
			<?php endif; ?>
			
			<table class="table table-bordered table-striped">
				<thead>
					<tr>
						<th class="id-th"><i class="icon-tag icon-white"></i></th>
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
						<td><?php echo $v['problem_alias'] . ' | ' . $v['problem_name']; ?></td>
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
						<td class="operations-td"><a href="<?php echo site_url('contestant/submission/view/' . $v['id'] . '/' . $page_offset); ?>" rel="tooltip" title="<?php echo $this->lang->line('view'); ?>"><i class="icon-search"></i></a></td>
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