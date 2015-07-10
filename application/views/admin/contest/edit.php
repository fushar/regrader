<div class="container">
	<div class="row">
		<div class="col-md-12">
			<ul class="breadcrumb">
				<li><i class="glyphicon glyphicon-fire"></i> <?php echo $this->lang->line('contests'); ?></li>
				<li><i class="glyphicon glyphicon-list-alt"></i> <?php echo isset($contest) ? $this->lang->line('contest') . ' ' . $contest['id'] : $this->lang->line('new_contest'); ?></li>
			</ul>
		</div>
	</div>

	<div class="row">
		<div class="col-md-12">
			<h3><?php echo isset($contest) ? $this->lang->line('edit_contest') : $this->lang->line('add_contest'); ?></h3>
			<form class="form-horizontal" action="" method="post">
				<div class="form-group<?php echo form_error('form[name]') == '' ? '' : ' error'; ?>">
					<label class="col-sm-2 control-label"><?php echo $this->lang->line('name'); ?></label>
					<div class ="col-sm-4">
						<input name="form[name]" type="text" class="form-control" maxlength="50" value="<?php echo set_value('form[name]', @$contest['name']); ?>"/>
						<span class="help-block"><?php echo form_error('form[name]'); ?></span>
					</div>
				</div>

				<?php
					$tnames = array('start_time', 'end_time', 'freeze_time', 'unfreeze_time');
					$tlabels = array($this->lang->line('start_time'), $this->lang->line('end_time'), $this->lang->line('freeze_time'), $this->lang->line('unfreeze_time'));
					for ($i = 0; $i < 4; $i++) : ?>

					<div class="form-group<?php echo form_error('form[' . $tnames[$i] . ']') == '' ? '' : ' error'; ?>">
						<label class="control-label col-sm-2"><?php echo $tlabels[$i]; ?></label>
						<div class ="col-sm-4">
							<div class="input-append">
								<input name="<?php echo 'form[' . $tnames[$i] . ']'; ?>" id="anytime_<?php echo $tnames[$i]; ?>" type="text" class="form-control col-md-4" maxlength="19" value="<?php echo set_value('form[' . $tnames[$i] . ']', @$contest[$tnames[$i]]); ?>"/>
							</div>
							<span class="help-block"><?php echo form_error('form[' . $tnames[$i] . ']'); ?></span>
							<?php if ($i >= 2): ?>
								<p class="help-block"><?php echo $this->lang->line('no_freeze_time'); ?></p>
							<?php endif; ?>
						</div>
					</div>
				<?php endfor; ?>

				<div class="form-group">
					<label class="control-label col-sm-2"><?php echo $this->lang->line('is_active'); ?></label>
					<div class ="col-sm-4">
						<div class="checkbox">
						<label>
							<input name="form[enabled]" type="checkbox" value="1" <?php echo set_checkbox('form[enabled]', '1', (bool)@$contest['enabled']); ?>/> &nbsp;
						</label>
						</div>
					</div>
				</div>

				<div class="form-group">
					<label class="control-label col-sm-2"><?php echo $this->lang->line('show_institution_logo'); ?></label>
					<div class ="col-sm-4">
						<div class="checkbox">
						<label>
							<input name="form[show_institution_logo]" type="checkbox" value="1" <?php echo set_checkbox('form[show_institution_logo]', '1', (bool)@$contest['show_institution_logo']); ?>/> &nbsp;
						</label>
						</div>
					</div>
				</div>

				<div class="form-group">
					<label class="control-label col-sm-2"><?php echo $this->lang->line('category'); ?></label>
					<?php if (count($contest_members) == 1) : ?>
						<p>(<?php echo $this->lang->line('no_category'); ?>)</p>
					<?php endif; ?>
					<div class = "col-sm-10">
						<?php foreach ($contest_members as $k => $v): if ($k == 1) continue; ?>
							<div class="checkbox">
								<label>
									<input name="<?php echo 'c' . $k; ?>" type="checkbox" value="1" <?php echo set_checkbox('c' . $k, '1', @$v['present']); ?>/>
									<?php echo $v['name']; ?>
								</label>
							</div>
						<?php endforeach; ?>
					</div>
				</div>

				<div class="form-group">
					<label class="col-sm-2 control-label"><?php echo $this->lang->line('language'); ?></label>
					<?php if (count($contest_languages) == 0) : ?>
						<p>(<?php echo $this->lang->line('no_language'); ?>)</p>
					<?php endif; ?>
					<div class = "col-sm-10">
						<?php foreach ($contest_languages as $k => $v): ?>
						
							<div class="checkbox">
								<label>
									<input name="<?php echo 'l' . $k; ?>" type="checkbox" value="1" <?php echo set_checkbox('l' . $k, '1', @$v['present']); ?>/>
									<?php echo $v['name']; ?>
								</label>
							</div>
						<?php endforeach; ?>
					</div>
				</div>

				<div class="form-actions col-sm-offset-2">
					<button type="submit" class="btn btn-danger"><?php echo isset($contest) ? '<i class="glyphicon glyphicon-download-alt"></i> ' . $this->lang->line('save') : '<i class="glyphicon glyphicon-plus"></i> ' . $this->lang->line('add'); ?></button>
					<a class="btn btn-default" href="<?php echo site_url('admin/contest/viewAll/' . $page_offset); ?>"><?php echo $this->lang->line('cancel'); ?></a>
				</div>
			</form>
		</div>
	</div>
</div>

<script type="text/javascript">
	AnyTime.picker("anytime_start_time", {format: "%Y-%m-%d %H:%i:%s"});
	AnyTime.picker("anytime_end_time", {format: "%Y-%m-%d %H:%i:%s"});
	AnyTime.picker("anytime_freeze_time", {format: "%Y-%m-%d %H:%i:%s"});
	AnyTime.picker("anytime_unfreeze_time", {format: "%Y-%m-%d %H:%i:%s"});
</script>