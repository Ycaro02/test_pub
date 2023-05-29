<!DOCTYPE html>
<html lang="en">
<head>
	<meta charset="UTF-8">
	<meta http-equiv="X-UA-Compatible" content="IE=edge">
	<meta name="viewport" content="width=device-width, initial-scale=1.0">
	<title>Document</title>
</head>
<body>
	<form action="" method="post" enctype="multipart/form-data" >
		<input type="file" name="file">
		<input type="submit" value="Submit">
	</form>

	<?php
	if ($_SERVER['REQUEST_METHOD'] === 'POST') {
		$file = $_FILES['file'];
		if ($file['error'] === UPLOAD_ERR_OK) {
			$dir = '../uploads/';
			if (!file_exists($dir)) {
				mkdir($dir);
			}
			$destination = $dir . $file['name'];
			move_uploaded_file($file['tmp_name'], $destination);
			
			echo 'File uploaded successfully.';
		} else {
			echo 'Error uploading file.';
		}
	}
	?>
</body>
</html>

