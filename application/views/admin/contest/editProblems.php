<div class="container">
	<div class="row">
		<div class="span12">
			<ul class="breadcrumb">
				<li><i class="icon-fire"></i> <?php echo $this->lang->line('contests'); ?></li>
				<li><span class="divider">|</span></li>
				<li><i class="icon-list-alt"></i> <?php echo $this->lang->line('contest'); ?> <?php echo $contest['id']; ?></li>
			</ul>
		</div>
	</div>
	
	<div class="row">
		<div class="span12">
			<h3><?php echo $this->lang->line('edit_contest_problems'); ?></h3>

			<table class="table table-bordered table-striped">
				<thead>
					<tr>
						<th><i class="icon-tag icon-white"></i> <?php echo $this->lang->line('alias'); ?></th>
						<th><i class="icon-book icon-white"></i> <?php echo $this->lang->line('problem'); ?></th>
						<th class="operations-th"><i class="icon-cog icon-white"></i></th>
					</tr>
				</thead>
				<tbody>
					<?php foreach ($contest_problems as $k => $v) : if ($v['present']) : ?>
						<tr>
							<td><?php echo $v['alias']; ?></td>	
							<td><?php echo $v['name']; ?></td>
							<td class="operations-td">
								<a href="<?php echo site_url('admin/contest/deleteProblem/' . $contest['id'] . '/'. $k . '/' . $page_offset); ?>" rel="tooltip" title="<?php echo $this->lang->line('delete'); ?>"><i class="icon-trash"></i></a>
							</td>
						</tr>
					<?php endif; endforeach; ?>
				</tbody>
			</table>

			<hr />

			<h3><?php echo $this->lang->line('add_problem'); ?></h3>
			<form class="form-horizontal" action="" method="post">
				<div class="control-group<?php echo form_error('new_problem_id') == '' ? '' : ' error'; ?>">
					<label class="control-label"><?php echo $this->lang->line('problem'); ?></label>
					<div class="controls">
						<select name="new_problem_id" class="span4">
							<option value="0" selected="selected">-</option>
						<?php foreach ($contest_problems as $k => $v) : if ( ! $v['present']) : ?>
							<option value="<?php echo $k; ?>" <?php echo set_select('new_problem_id', $k, FALSE); ?>><?php echo $v['name']; ?></option>
						<?php endif; endforeach; ?>
						</select>
						<span class="help-inline"><?php echo form_error('new_problem_id'); ?></span>
					</div>
				</div>

				<div class="control-group<?php echo form_error('new_problem_alias') == '' ? '' : ' error'; ?>">
					<label class="control-label"><?php echo $this->lang->line('alias'); ?></label>
					<div class="controls">
						<input name="new_problem_alias" type="text" class="span4" maxlength="50" value="<?php echo set_value('new_problem_alias', ''); ?>"/>
						<span class="help-inline"><?php echo form_error('new_problem_alias'); ?></span>
					</div>
				</div>



				<div class="form-actions">
					<button type="submit" class="btn btn-danger"><i class="icon-plus icon-white"></i> <?php echo $this->lang->line('add'); ?></button>
					<a class="btn" href="<?php echo site_url('admin/contest/viewAll/' . $page_offset); ?>"><?php echo $this->lang->line('cancel'); ?></a>
				</div>
			</form>
		</div>
	</div>
</div>