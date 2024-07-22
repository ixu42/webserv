import cgi
import os
from datetime import datetime
# import time

<<<<<<< HEAD:cgi-bin/script.py
# time.sleep(50000)

=======
>>>>>>> nikkxll/cgi-format-improvements:cgi-bin/default.py
current_time = datetime.now().strftime("%Y-%m-%d %H:%M:%S")

print("Content-Type: text/html")
print()

print(f"""<!DOCTYPE html>
<html lang="en">
<head>
	<meta charset="UTF-8">
	<meta name="viewport" content="width=device-width, initial-scale=1.0">
	<title>webserv</title>
	<style>
		body {{
			font-family: Arial, sans-serif;
			margin: 0;
			padding: 0;
			display: flex;
			justify-content: center;
			align-items: center;
			height: 100vh;
			background-color: #f0f0f0;
		}}
		.container {{
			text-align: center;
		}}
		h1 {{
			color: #333;
		}}
		p {{
			color: #666;
		}}
		span {{
			font-weight: bold;
		}}
		img {{
			width: 300px;
			height: auto;
		}}
	</style>
</head>
<body>
	<div class="container">
		<h1>&#128187 Welcome to the Python-generated HTML page &#128187</h1>
		<p>Project was made by Hive Helsinki students: <span>ixu</span>, <span>vshchuki</span> and <span>dnikifor</span></p>
		<p>This is a CGI-ran Python script</p>
		<h2>Environment Variables:</h2>
		<ul>""")

for key, value in os.environ.items():
    print(f"<li>{key} = {value}</li>")

print(f"""		</ul>
	</div>
</body>
</html>""")
