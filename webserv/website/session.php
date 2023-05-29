<?php
	session_start();
	if (!isset($_COOKIE["cookie"]))
	{
		setcookie("cookie", -1, time() + 10);
	}
	setcookie("cookie", intval($_COOKIE["cookie"]) + 1 . "", time() + 10);
	
	echo "<h2>Cookie : " . $_COOKIE["cookie"] . "</h2>";
	if (!isset($_SESSION["newinput"]))
	{
		$_SESSION["newinput"] = 0;
	}
	echo "<h2>Session : " . $_SESSION["newinput"] . "</h2>";
	$_SESSION["newinput"] = intval($_SESSION["newinput"]) + 1 . "";

?>
