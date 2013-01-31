<div class="container">
	<div class="row">
		<div class="span12">
			<ul class="breadcrumb">
				<li><i class="icon-list"></i> <?php echo $this->lang->line('scoreboard'); ?></li>
				<?php if ($frozen) : ?>
					<li><span class="divider">|</span></li>
					<li><?php echo $this->lang->line('frozen_since') . ' ' .  $freeze_time; ?></li>
				<?php endif; ?>
					<li class="pull-right">
						<i class="icon-time"></i> <span id="contest_clock"></span> <?php echo $this->lang->line('remaining'); ?>
					</li>
			</ul>
		</div>
	</div>
</div>