<div class="container">
	<div class="row">
		<div class="col-md-12">
			<ul class="breadcrumb">
				<li><i class="glyphicon glyphicon-list"></i> <?php echo $this->lang->line('scoreboard'); ?></li>

			<?php if (isset($contest_id)) : ?>
				<li><i class="glyphicon glyphicon-fire"></i> <?php echo $contest_name; ?>
				<li><a href="<?php echo site_url('scoreboard/view/' . $contest_id); ?>"><i class="glyphicon glyphicon-list"></i> <?php echo $this->lang->line('public_scoreboard'); ?></a></li>
				<li><a href="<?php echo site_url('scoreboard/rawview/' . $contest_id); ?>"><i class="glyphicon glyphicon-list"></i> <?php echo $this->lang->line('public_raw_scoreboard'); ?></a></li>
			<?php endif; ?>
			</ul>
		</div>
	</div>
	
	<div class="row">
		<div class="col-md-12">
		<?php if ( ! isset($contest_id)) : ?>
			<p><i><?php echo $this->lang->line('no_contest'); ?></i></p>
		<?php else : ?>
			<form class="form-inline" method="post">
				<select name="contest_id" class="col-md-6">
				<?php foreach ($contests as $v) : ?>
					<option value="<?php echo $v['id']; ?>"<?php if ($v['id'] == $contest_id) echo ' selected="selected"'; ?>><?php echo $v['name']; ?></option>
				<?php endforeach; ?>
				</select>
				<select name="target" class="col-md-3">
					<option value="admin"<?php if ($target=='admin') echo ' selected="selected"'; ?>><?php echo $this->lang->line('as_admin'); ?></option>
					<option value="contestant"<?php if ($target=='contestant') echo ' selected="selected"'; ?>><?php echo $this->lang->line('as_contestant'); ?></option>
				</select>
				<button type="submit" class="btn btn-default" rel="tooltip" title="<?php echo $this->lang->line('view'); ?>"><i class="glyphicon glyphicon-search"></i></button>
			</form>
		<?php endif; ?>
		</div>
	</div>
</div>