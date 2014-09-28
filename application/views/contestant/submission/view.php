<div class="container">
	<div class="row">
		<div class="col-md-12">
			<ul class="breadcrumb">
				<li><i class="glyph iconglyphicon-share"></i> <?php echo $this->lang->line('submissions'); ?></li>
				<li><i class="glyphicon glyphicon-list-alt"></i> <?php echo $this->lang->line('submission') . ' ' . $submission['id']; ?></li>
				
				<li class="pull-right">
					<i class="glyphicon glyphicon-time"></i> <span id="contest_clock"></span> <?php echo $this->lang->line('remaining'); ?>
				</li>
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
					<tr><td><i class="glyphicon glyphicon-time"></i> <?php echo $this->lang->line('time'); ?></td><td><?php echo $submission['submit_time']; ?></td></tr>
					<tr><td><i class="glyphicon glyphicon-briefcase"></i> <?php echo $this->lang->line('verdict'); ?></td><td><?php echo $this->lang->line('verdict_' . max(0, $submission['verdict'])); ?></td></tr>
				</tbody>
			</table>

			<h3><?php echo $this->lang->line('source_code'); ?></h3>
			<pre class="prettyprint linenums">
<?php echo htmlspecialchars($submission['source_code']); ?></pre>

			<h3><?php echo $this->lang->line('compile_result'); ?></h3>
			<?php if (isset($submission['compile_result'])): ?>
				<pre>
<?php echo htmlspecialchars($submission['compile_result']); ?></pre>
			<?php else: ?>
				<p><i><?php echo $this->lang->line('not_compiled'); ?></i></p>
			<?php endif; ?>
			
			<form>
				<div class="form-actions">
					<a class="btn btn-default" href="<?php echo site_url('contestant/submission/viewAll/' . $page_offset); ?>"><?php echo $this->lang->line('back'); ?></a>
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