# http://localhost:8006/cgi-bin/query.py?name=Dmitrii&login=dnikifor&team=%23div
import os
import cgi
import urllib.parse
import html

form = cgi.FieldStorage()

query_string = os.environ.get('QUERY_STRING', '')
parsed_query = urllib.parse.parse_qs(query_string)

print("Content-Type: text/html\n")
print("<html>")
print("<head><title>CGI Query String Example</title></head>")
print("<body>")
print("<h1>CGI Query String Example</h1>")
print("<p>Parsed Query String:</p>")
print("<ul>")

for key, values in parsed_query.items():
	for value in values:
		print(f"<li><strong>{html.escape(key)}:</strong> {html.escape(value)}</li>")

print("</ul>")
print("</body>")
print("</html>")
