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
		<h2>GET FORM :</h2>
		<form action="" method="get">
			<input type="text" name="name">
			<input type="submit" value="submit">
		</form>
		
		<?php 
		if (isset($_GET["name"]))
		{
			echo "Hello ".$_GET["name"];
		}
		?>
	</center>

</body>
</html>
