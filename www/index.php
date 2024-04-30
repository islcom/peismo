<?php
	$files = scandir("uploaded");

	// Get the last 200 files alphabetically

	$files = array_diff($files, array('.', '..'));
	rsort($files);
	$files = array_slice($files, 0, 200);

?>
<html>
	<head>
		<style>
		</style>
<!--		<script src="https://maps.googleapis.com/maps/api/js?v=3.exp"></script> doesn't work on local -->

		<script src="seisplotjs_2.0.1_standalone.js"></script>
		<script type='text/javascript'>var filenamesFromPhp = <?php echo json_encode($files) ?>;</script>
		<script type='text/javascript' src='script.js'></script>
	</head>
	<body>
		<div id="helicorder"></div>
	</body>
</html>
