#!/usr/bin/env python3

import os

print("Content-Type: text/html")
print()

print("<html><head><title>CGI Script</title></head><body>")
print("<h1>Hello from Python CGI script!</h1>")
print("<p>REQUEST_METHOD: {}</p>".format(os.environ.get("REQUEST_METHOD", "")))
print("<p>QUERY_STRING: {}</p>".format(os.environ.get("QUERY_STRING", "")))
print("<p>PATH_INFO: {}</p>".format(os.environ.get("PATH_INFO", "")))
print("</body></html>")