import cgi
import os
import html

def main():
	form = cgi.FieldStorage()
	name = form.getvalue("name", "Guest")

	print("Content-Type: text/html\n")
	print("<html>")
	print("<head><title>CGI Example</title></head>")
	print("<body>")
	print(f"<h1>Hi, {html.escape(name)}!</h1>")
	print("<p1>Welcome to the evaluation :)</p1>")
	print("</body>")
	print("</html>")

if __name__ == "__main__":
	main()