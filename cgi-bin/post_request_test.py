import os
import sys
import cgi
import cgitb
import html

cgitb.enable()

def main():
	print("Content-Type: text/html")
	print()

	if os.environ['REQUEST_METHOD'] == 'POST':
		try:
			content_length = int(os.environ.get('CONTENT_LENGTH', 0))
			post_data = sys.stdin.read(content_length)
			form = cgi.FieldStorage(fp=sys.stdin, environ=os.environ, keep_blank_values=True)

			# Generate a simple HTML page to display the POST data
			print("<html>")
			print("<head><title>POST Request Received</title></head>")
			print("<body>")
			print("<h1>POST Request Received</h1>")
			print("<p>Here is the data you posted:</p>")
			print("<pre>")
			print(html.escape(post_data))
			print("</pre>")
			print("</body>")
			print("</html>")
		except Exception as e:
			print("<h1>Error processing POST request</h1>")
			print(f"<p>{html.escape(str(e))}</p>")
	else:
		print("<html>")
		print("<head><title>Error</title></head>")
		print("<body>")
		print("<h1>Error</h1>")
		print("<p>This script only handles POST requests.</p>")
		print("</body>")
		print("</html>")

if __name__ == '__main__':
	main()