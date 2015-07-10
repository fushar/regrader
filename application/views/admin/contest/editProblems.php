<div class="container">
	<div class="row">
		<div class="col-md-12">
			<ul class="breadcrumb">
				<li><i class="glyphicon glyphicon-fire"></i> <?php echo $this->lang->line('contests'); ?></li>
				<li><i class="glyphicon glyphicon-list-alt"></i> <?php echo $this->lang->line('contest'); ?> <?php echo $contest['id']; ?></li>
			</ul>
		</div>
	</div>
	
	<div class="row">
		<div class="col-md-12">
			<h3><?php echo $this->lang->line('edit_contest_problems'); ?></h3>

			<table class="table table-bordered table-striped">
				<thead>
					<tr>
						<th><i class="glyphicon glyphicon-tag"></i> <?php echo $this->lang->line('alias'); ?></th>
						<th><i class="glyphicon glyphicon-book"></i> <?php echo $this->lang->line('problem'); ?></th>
						<th class="operations-th"><i class="glyphicon glyphicon-cog"></i></th>
					</tr>
				</thead>
				<tbody>
					<?php foreach ($contest_problems as $k => $v) : if ($v['present']) : ?>
						<tr>
							<td><?php echo $v['alias']; ?></td>	
							<td><?php echo $v['name']; ?></td>
							<td class="operations-td">
								<a href="<?php echo site_url('admin/contest/deleteProblem/' . $contest['id'] . '/'. $k . '/' . $page_offset); ?>" rel="tooltip" title="<?php echo $this->lang->line('delete'); ?>"><i class="glyphicon glyphicon-trash"></i></a>
							</td>
						</tr>
					<?php endif; endforeach; ?>
				</tbody>
			</table>

			<hr />

			<h3><?php echo $this->lang->line('add_problem'); ?></h3>
			<form class="form-horizontal" action="" method="post">
				<div class="form-group<?php echo form_error('new_problem_id') == '' ? '' : ' error'; ?>">
					<label class="col-sm-2 control-label"><?php echo $this->lang->line('problem'); ?></label>
					<div class = "col-sm-4">
						<select class = "form-control" name="new_problem_id" class="col-md-4">
							<option value="0" selected="selected">-</option>
						<?php foreach ($contest_problems as $k => $v) : if ( ! $v['present']) : ?>
							<option value="<?php echo $k; ?>" <?php echo set_select('new_problem_id', $k, FALSE); ?>><?php echo $v['name']; ?></option>
						<?php endif; endforeach; ?>
						</select>
						<span class="help-block"><?php echo form_error('new_problem_id'); ?></span>
					</div>
				</div>

				<div class="form-group<?php echo form_error('new_problem_alias') == '' ? '' : ' error'; ?>">
					<label class="col-sm-2 control-label"><?php echo $this->lang->line('alias'); ?></label>
					<div class="col-sm-4">
						<input name="new_problem_alias" type="text" class="form-control col-md-4" maxlength="50" value="<?php echo set_value('new_problem_alias', ''); ?>"/>
						<span class="help-block"><?php echo form_error('new_problem_alias'); ?></span>
					</div>
				</div>



				<div class="form-actions col-sm-offset-2">
					<button type="submit" class="btn btn-danger"><i class="glyphicon glyphicon-plus"></i> <?php echo $this->lang->line('add'); ?></button>
					<a class="btn btn-default" href="<?php echo site_url('admin/contest/viewAll/' . $page_offset); ?>"><?php echo $this->lang->line('cancel'); ?></a>
				</div>
			</form>
		</div>
	</div>
</div>