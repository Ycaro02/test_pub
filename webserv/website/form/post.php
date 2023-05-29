<!DOCTYPE html>
<html lang="en">
<head>
	<meta charset="UTF-8">
	<meta http-equiv="X-UA-Compatible" content="IE=edge">
	<meta name="viewport" content="width=device-width, initial-scale=1.0">
	<title>Document</title>
</head>
<body>
	<center>
		<h1>PHP</h1>
		<h2>POST FORM :</h2>
		<form action="" method="post">
			<input type="text" name="name">
			<input type="submit" value="submit">
		</form>

		<?php 
			if (isset($_POST["name"]))
			{
				echo "Hello ".$_POST["name"];
			}
		?>
	</center>
</body>
</html>
