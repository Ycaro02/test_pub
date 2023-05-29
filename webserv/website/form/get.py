import cgi


def show_header():
	print("""
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
	""")

def show_form():
	print("""
			<h1>Python</h1>
			<h2>GET FORM :</h2>
			<form action="" method="get">
				<input type="text" name="name">
				<input type="submit" value="submit">
			</form>
	""")

def show_name():
	form = cgi.FieldStorage()
	name = form.getvalue('name')
	if name:
		print("Hello ", name)

def show_footer():
	print("""
			</center>
		</body>
		</html>
	""")

def main():
	show_header()
	show_form()
	show_name()
	show_footer()
	return 0

if __name__ == "__main__":
	main()