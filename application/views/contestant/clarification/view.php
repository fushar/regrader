<div class="container">
	<div class="row">
		<div class="col-md-12">
			<ul class="breadcrumb">
				<li><i class="glyphicon glyphicon-envelope"></i> <?php echo $this->lang->line('clarifications'); ?></li>
				<li><i class="glyphicon glyphicon-list-alt"></i> <?php echo $this->lang->line('clarification') . ' ' . $clarification['id']; ?></li>
				
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
					<tr><th class="clarification-info-th"><?php echo $this->lang->line('info'); ?></th><th><?php echo $this->lang->line('value'); ?></th></tr>
				</thead>
				<tbody>
					<tr><td><i class="glyphicon glyphicon-fire"></i> <?php echo $this->lang->line('contest'); ?></td><td><?php echo $clarification['contest_name']; ?></td></tr>
					<tr><td><i class="glyphicon glyphicon-user"></i> <?php echo $this->lang->line('by'); ?></td><td><?php echo $clarification['user_id'] == 1 ? '<span class="label label-danger">Administrator</span>' : $clarification['user_name']; ?></td></tr>
					<tr><td><i class="glyphicon glyphicon-time"></i> <?php echo $this->lang->line('time'); ?></td><td><?php echo $clarification['clar_time']; ?></td></tr>
				</tbody>
			</table>

			<h3><?php echo $this->lang->line('title'); ?></h3>
			<?php echo htmlspecialchars($clarification['title']); ?>
			<h3><?php echo $this->lang->line('content'); ?></h3>
			<div class="well">
				<?php echo nl2br(htmlspecialchars($clarification['content'])); ?>
			</div>

			<?php if ($clarification['user_id'] != 1) : ?>
				<h3><?php echo $this->lang->line('answer'); ?></h3>
				<?php if ($clarification['answer'] != NULL) : ?>
					<div class="well">
						<?php echo nl2br(htmlspecialchars($clarification['answer'])); ?>
					</div>
				<?php else : ?>
					<p><i><?php echo $this->lang->line('clarification_not_answered'); ?></i></p>
				<?php endif; ?>
			<?php endif; ?>
			<form>
				<div class="form-actions">
					<a class="btn btn-default" href="<?php echo site_url('contestant/clarification/viewAll/' . $page_offset); ?>"><?php echo $this->lang->line('back'); ?></a>
				</div>
			</form>
		</div>
	</div>
</div>