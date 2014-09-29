<div class="container">
	<div class="row">
		<div class="col-md-12">
			<ul class="breadcrumb">
				<li><i class="glyphicon glyphicon-share"></i> <?php echo $this->lang->line('submissions'); ?></li>
				<li><i class="glyphicon glyphicon-list-alt"></i> <?php echo $this->lang->line('submission_list'); ?></li>
				<li class="pull-right">
					<i class="glyphicon glyphicon-time"></i> <span id="contest_clock"></span> <?php echo $this->lang->line('remaining'); ?>
				</li>
			</ul>
		</div>
	</div>
	
	<div class="row">
		<div class="col-md-12">
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
						<th class="id-th"><i class="glyphicon glyphicon-tag"></i></th>
						<th><i class="glyphicon glyphicon-book"></i> <?php echo $this->lang->line('problem'); ?></th>
						<th class="submission-language-th"><i class="glyphicon glyphicon-globe"></i> <?php echo $this->lang->line('language'); ?></th>
						<th class="submission-time-th"><i class="glyphicon glyphicon-time"></i> <?php echo $this->lang->line('time'); ?></th>
						<th class="submission-verdict-th"><i class="glyphicon glyphicon-briefcase"></i> <?php echo $this->lang->line('verdict'); ?></th>
						<th class="operations-th"><i class="glyphicon glyphicon-cog"></i></th>
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
						<td class="operations-td"><a href="<?php echo site_url('contestant/submission/view/' . $v['id'] . '/' . $page_offset); ?>" rel="tooltip" title="<?php echo $this->lang->line('view'); ?>"><i class="glyphicon glyphicon-search"></i></a></td>
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