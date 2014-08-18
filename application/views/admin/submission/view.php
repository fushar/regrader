<div class="container">
	<div class="row">
		<div class="col-md-12">
			<ul class="breadcrumb">
				<li><i class="glyphicon glyphicon-share"></i> <?php echo $this->lang->line('submissions'); ?></li>
				<li><i class="glyphicon glyphicon-list-alt"></i> <?php echo $this->lang->line('submission'); ?> <?php echo $submission['id']; ?></li>
			</ul>
		</div>
	</div>
	
	<div class="row">
		<div class="col-md-12">
			<h3><?php echo $this->lang->line('general_info'); ?></h3>
			<table class="table table-bordered table-striped">
				<thead>
					<tr><th class="submission-info-th"><?php echo $this->lang->line('info'); ?></th><th><?php echo $this->lang->line('value'); ?></th></tr>
				</thead>
				<tbody>
					<tr><td><i class="glyphicon glyphicon-fire"></i> <?php echo $this->lang->line('contest'); ?></td><td><?php echo $submission['contest_name']; ?></td></tr>
					<tr><td><i class="glyphicon glyphicon-user"></i> <?php echo $this->lang->line('user'); ?></td><td><?php echo $submission['user_name']; ?></td></tr>
					<tr><td><i class="glyphicon glyphicon-book"></i> <?php echo $this->lang->line('problem'); ?></td><td><?php echo $submission['problem_alias'] . ' | ' . $submission['problem_name']; ?></td></tr>
					<tr><td><i class="glyphicon glyphicon-globe"></i> <?php echo $this->lang->line('language'); ?></td><td><?php echo $submission['language_name']; ?></td></tr>
					<tr><td><i class="glyphicon glyphicon-time"></i> <?php echo $this->lang->line('submission_time'); ?></td><td><?php echo $submission['submit_time']; ?></td></tr>
					<tr><td><i class="glyphicon glyphicon-time"></i> <?php echo $this->lang->line('start_judge_time'); ?></td><td><?php echo $submission['start_judge_time']; ?></td></tr>
					<tr><td><i class="glyphicon glyphicon-time"></i> <?php echo $this->lang->line('end_judge_time'); ?></td><td><?php echo $submission['end_judge_time']; ?></td></tr>
					<tr><td><i class="glyphicon glyphicon-briefcase"></i> <?php echo $this->lang->line('verdict'); ?></td><td><?php echo $this->lang->line('verdict_' . max(0, $submission['verdict'])); ?></td></tr>
				</tbody>
			</table>

			<h3><?php echo $this->lang->line('source_code'); ?></h3>
			<pre class="prettyprint">
<?php echo htmlspecialchars($submission['source_code']); ?></pre>

			<h3><?php echo $this->lang->line('compile_result'); ?></h3>
			<?php if (isset($submission['compile_result'])): ?>
				<pre>
<?php echo htmlspecialchars($submission['compile_result']); ?></pre>
			<?php else: ?>
				<p><i><?php echo $this->lang->line('not_compiled'); ?></i></p>
			<?php endif; ?>
			<h3><?php echo $this->lang->line('testcase_verdicts'); ?></h3>
			<table class="table table-bordered table-striped">
				<thead>
					<tr>
						<th class="id-th"><i class="glyphicon glyphicon-tag glyphicon glyphicon-white"></i></th>
						<th><i class="glyphicon glyphicon-file glyphicon glyphicon-white"></i> <?php echo $this->lang->line('input'); ?></th>
						<th><i class="glyphicon glyphicon-file glyphicon glyphicon-white"></i> <?php echo $this->lang->line('output'); ?></th>
						<th><i class="glyphicon glyphicon-time glyphicon glyphicon-white"></i> <?php echo $this->lang->line('time'); ?></th>
						<th><i class="glyphicon glyphicon-download-alt glyphicon glyphicon-white"></i> <?php echo $this->lang->line('memory'); ?></th>
						<th><i class="glyphicon glyphicon-briefcase glyphicon glyphicon-white"></i> <?php echo $this->lang->line('verdict'); ?></th>
					</tr>
				</thead>
				<tbody>
					<?php foreach ($judgings as $v) : ?>
					<tr>
						<td><?php echo $v['testcase_id']; ?></td>
						<td><?php echo $v['input']; ?></td>
						<td><?php echo $v['output']; ?></td>
						<td><?php echo $v['time']; ?> ms</td>
						<td><?php echo $v['memory']; ?> KB</td>
						<td><?php echo $this->lang->line('verdict_' . max(0, $v['verdict'])); ?></td>
					</tr>
					<?php endforeach; ?>
				</tbody>
			</table>
			<form>
				<div class="form-actions">
					<a class="btn btn-default" href="<?php echo site_url('admin/submission/viewAll/' . $contest_id . '/' . $problem_id . '/' . $user_id . '/' . $page_offset); ?>"><?php echo $this->lang->line('back'); ?></a>
				</div>
			</form>
		</div>
	</div>
	
</div>

<script type="text/javascript">
	$(document).ready(function(){
		prettyPrint();
	});
</script>