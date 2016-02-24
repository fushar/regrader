<div class="container">
	<div class="row">
		<div class="col-md-12 site_footer">
			<hr />
			&copy; 2012-2016 Regrader. Powered by CodeIgniter, Bootstrap, and Moe Contest Environment. Licensed under MIT License.
		</div>
	</div>
</div>

<script type="text/javascript">
	$(document).ready(function(){
		set_server_clock(new Date(<?php echo time() * 1000; ?>));
	});
</script>

<?php if (isset($active_contest_end_time)) : ?>

<script type="text/javascript">
	$(document).ready(function(){
		set_contest_clock(new Date(<?php echo time() * 1000; ?>),
			              new Date(<?php echo strtotime($active_contest_end_time) * 1000; ?>));
	});
</script>

<?php endif; ?>

</body>

</html>
