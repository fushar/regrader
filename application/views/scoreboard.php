<div class="container">
	<div class="row">
		<div class="col-md-12">
			<table class="table table-bordered table-condensed table-scoreboard">
				<thead>
					<tr>
						<th class="id-th">#</th>
						<?php if ($show_institution_logo): ?>
							<th class="scoreboard-institution-logo-th"></th>
						<?php endif; ?>
						<th class="scoreboard-name-th"><?php echo $this->lang->line('name'); ?></th>
						<th class="scoreboard-score-th" colspan="2"><?php echo $this->lang->line('score'); ?></th>
						<?php foreach ($problems as $v) : ?>
							<th class="scoreboard-problem-th"><?php echo $v['alias']; ?></th>
						<?php endforeach; ?>
					</tr>
				</thead>

				<tbody>
					<?php
						$rank = 0;
						$temp = 1;
						$prev_total_accepted = -1;
						$prev_total_penalty = -1;
						$prev_last_submission = -1;
						foreach ($scores as $v) :
							if ($v['total_accepted'] != $prev_total_accepted or $v['total_penalty'] != $prev_total_penalty or $v['last_submission'] != $prev_last_submission)
							{
								$rank += $temp;
								$temp = 1;
							}
							else
								$temp++;

							$prev_total_accepted = $v['total_accepted'];
							$prev_total_penalty = $v['total_penalty'];
							$prev_last_submission = $v['last_submission'];

							?>
							<tr>
							<td class="id-td"><?php echo $rank; ?></td>
							<?php if ($show_institution_logo): ?>
								<td>
									<div class="scoreboard-institution-logo">
										<img src="<?php echo isset($raw) ? '' : base_url(); ?>files/<?php echo $v['institution']; ?>.jpg" />
									</div>
								</td>
							<?php endif; ?>
							<td>
								<div class="scoreboard-name">
									<?php echo $v['name']; ?>
								</div>
								<div class="scoreboard-institution">
									<?php echo $v['institution']; ?>
								</div>
							</td>


							<td class="scoreboard-total-accepted-td"><?php echo $v['total_accepted']; ?></td>
							<td class="scoreboard-total-penalty-td"><?php echo $v['total_penalty']; ?></td>

							<?php
							foreach ($v['score'] as $w) :
								$verdict_class = '';
								if ($w['is_accepted'] == 1)
									$verdict_class = ' scoreboard-accepted-td';
								else if ($w['submission_cnt'] > 0)
									$verdict_class = ' scoreboard-not-accepted-td';
								?>

								<td class="scoreboard-problem-td<?php echo $verdict_class; ?>"><?php echo $w['submission_cnt'] . ' / ' . ($w['is_accepted'] == 1 ? $w['time_penalty'] : '-'); ?></td>
								
							<?php endforeach; ?>
							</tr>
					<?php endforeach; ?>
				</tbody>
			</table>
		</div>
	</div>
</div>