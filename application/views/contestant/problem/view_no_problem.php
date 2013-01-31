<div class="container">
	<div class="row">
		<div class="span12">
			<ul class="breadcrumb">
				<li><i class="icon-book"></i> <?php echo $this->lang->line('problems'); ?></li>
				<li class="pull-right">
					<i class="icon-time"></i> <span id="contest_clock"></span> <?php echo $this->lang->line('remaining'); ?>
				</li>
			</ul>
		</div>
	</div>
	<div class="row">
		<div class="span2">
			<ul class="nav nav-tabs nav-stacked problem-list">
				<li><a href="#"><i class="icon-random"></i> <?php echo $this->lang->line('problems'); ?>:</a></li>
			</ul>
		</div>	
		<div class="span10">		
			<p><em><?php echo $this->lang->line('no_problem'); ?></em></p>
		</div>
	</div>
</div>