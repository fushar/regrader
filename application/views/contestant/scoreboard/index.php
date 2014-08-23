<div class="container">
	<div class="row">
		<div class="col-md-12">
			<ul class="breadcrumb">
				<li><i class="glyphicon glyphicon-list"></i> <?php echo $this->lang->line('scoreboard'); ?></li>
				<?php if ($frozen) : ?>
					<li><?php echo $this->lang->line('frozen_since') . ' ' . $freeze_time; ?></li>
				<?php endif; ?>
				<li class="pull-right">
					<i class="glyphicon glyphicon-time"></i> <span id="contest_clock"></span> <?php echo $this->lang->line('remaining'); ?>
				</li>
			</ul>
		</div>
	</div>
</div>