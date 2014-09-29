<div class="container">
	<div class="modal" id="submit">
		<?php if ($this->session->flashdata('contest_has_ended') || strtotime($active_contest_end_time) < time()) : ?>
			<div class = "modal-dialog">
				<div class = "modal-content">
					<div class="modal-header">
						<button type="button" class="close" data-dismiss="modal">&times;</button>
						<h3><?php echo $problem['alias'] . ' | ' . $problem['name']; ?></h3>
					</div>
					<div class="modal-body">
						<div class="alert alert-danger">
							<?php echo $this->lang->line('contest_has_ended'); ?>
						</div>
					</div>
					<div class="modal-footer">
						<a href="#" class="btn btn-default" data-dismiss="modal"><?php echo $this->lang->line('cancel'); ?></a>
					</div>
				</div>
			</div>
		<?php else: ?>
			<?php echo form_open_multipart('');?>
				<div class = "modal-dialog">
					<div class = "modal-content">
						<div class="modal-header">
							<button type="button" class="close" data-dismiss="modal">&times;</button>
							<h3><?php echo $problem['alias'] . ' | ' . $problem['name']; ?></h3>
						</div>
						<div class="modal-body">
							<?php if ($this->session->flashdata('missing_source_code')) : ?>
								<div class="alert alert-error">
									<?php echo $this->lang->line('missing_source_code'); ?>
								</div>
							<?php endif; ?>

							<label><?php echo $this->lang->line('language'); ?>:</label>
							<select class = "form-control" name="form[language_id]">
							<?php foreach ($languages as $v) : ?>
								<option value="<?php echo $v['id']; ?>"><?php echo $v['name']; ?></option>
							<?php endforeach; ?>
							</select>

							<?php if (empty($languages)) : ?>
								<p class="help-block"><i><?php echo $this->lang->line('no_language'); ?></i></p>
							<?php endif; ?>

							<label><?php echo $this->lang->line('source_code'); ?>:</label>
							<input name="source_code" class="input-file" type="file" />
						</div>
						<div class="modal-footer">
							<button type="submit" class="btn btn-danger"><i class="glyphicon glyphicon-share"></i> <?php echo $this->lang->line('submit'); ?></button>
							<a href="#" class="btn btn-default" data-dismiss="modal"><?php echo $this->lang->line('cancel'); ?></a>
						</div>
					</div>
				</div>
			</form>
		<?php endif; ?>
	</div>

	<div class="row">
		<div class="col-md-12">
			<ul class="breadcrumb">
				<li><i class="glyphicon glyphicon-book"></i> <?php echo $this->lang->line('problems'); ?></li>
				<li><i class="glyphicon glyphicon-list-alt"></i> <?php echo $problem['name']; ?></li>
				<li><i class="glyphicon glyphicon-file"></i> <a data-toggle="modal" href="#submit"><?php echo $this->lang->line('submit_answer'); ?></a></li>
				<li class="pull-right">
					<i class="glyphicon glyphicon-time"></i> <span id="contest_clock"></span> <?php echo $this->lang->line('remaining'); ?>
				</li>
			</ul>
		</div>
	</div>
	<div class="row">
		<div class="col-md-2">
			<ul class="nav nav-pills nav-stacked problem-list">
				<li><a href="#"><i class="glyphicon glyphicon-random"></i> <?php echo $this->lang->line('problems'); ?>:</a></li>
				
				<?php foreach ($problems as $v) : ?>
					<li<?php if ($v['id'] == $problem['id']) echo ' class="active"'; ?>><a href="<?php echo site_url('contestant/problem/view/' . $v['id']); ?>"><i class="glyphicon glyphicon-list-alt<?php if ($v['id'] == $problem['id']) echo ''; ?>"></i> <?php echo $v['alias']; ?></a></li>
				<?php endforeach; ?>
			</ul>
		</div>	
		<div class="col-md-10">		
			<h2 class="problem-title"><?php echo $problem['name']; ?></h2>
			<table class="table table-bordered problem-limits">
				<tr>
					<td><?php echo $this->lang->line('time_limit'); ?></td><td><?php echo $problem['time_limit']; ?> <?php echo $this->lang->line('second'); ?></td>
				</tr>
				<tr>
					<td><?php echo $this->lang->line('memory_limit'); ?></td><td><?php echo $problem['memory_limit']; ?> MB</td>
				</tr>
			</table>
			<?php echo $problem['statement']; ?>
		</div>
	</div>

	<?php if ($this->session->flashdata('submit_error')) : ?>
		<script type="text/javascript">
			$('#submit').modal('show');
		</script>
	<?php endif; ?>
</div>