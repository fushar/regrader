<div class="container">
	<div class="row">
		<div class="span12">
			<ul class="breadcrumb">
				<li><i class="icon-home"></i> <?php echo $this->lang->line('dashboard'); ?></li>
			</ul>
		</div>
	</div>

	<div class="row">
		<div class="span6">
			<table class="table table-bordered table-striped">
				<thead>
					<tr>
						<th class="id-th">#</th>
						<th><i class="icon-fire icon-white"></i> <?php echo $this->lang->line('active_contests'); ?></th>
					</tr>
				</head>
				<tbody>
					<?php $rank = 0; foreach ($active_contests as $v) : ?>
					<tr>
						<td><?php echo ++$rank; ?></td><td><?php if (strtotime($v['end_time']) > time()) echo '<span class="label label-info">' . $this->lang->line('running') . '</span> '; ?><?php echo $v['name']; ?></td>
					</tr>
					<?php endforeach; ?>
				</tbody>
			</table>

			<table class="table table-bordered table-striped">
				<thead>
					<tr>
						<th class="id-th">#</th>
						<th><i class="icon-user icon-white"></i> <?php echo $this->lang->line('active_users'); ?></th>
					</tr>
				</head>
				<tbody>
					<?php $rank = 0; foreach ($active_users as $v) : ?>
					<tr>
						<td><?php echo ++$rank; ?></td><td><?php echo $v['name']; ?></td>
					</tr>
					<?php endforeach; ?>
				</tbody>
			</table>

		</div>
		<div class="span6">
			<table class="table table-bordered table-striped">
				<thead>
					<tr>
						<th class="id-th">#</th>
						<th><i class="icon-refresh icon-white"></i> <?php echo $this->lang->line('active_graders'); ?></th>
					</tr>
				</head>
				<tbody>
					<?php $rank = 0; foreach ($active_graders as $v) : ?>
					<tr>
						<td><?php echo ++$rank; ?></td><td><?php echo $v['hostname']; ?></td>
					</tr>
					<?php endforeach; ?>
				</tbody>
			</table>

		</div>
	</div>
</div>