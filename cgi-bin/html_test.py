import os
import cgi
import cgitb

cgitb.enable()

def serve_error(file_path):
	base_dir = os.path.dirname(os.path.abspath(__file__))
	error_page_path = os.path.join(base_dir, file_path)

	if os.path.exists(error_page_path):
		with open(error_page_path, 'r') as file:
			content = file.read()

		print("Content-Type: text/html\n")
		print(content)
	else:
		# Fallback to plain text error message if 404 page is missing
		print("Content-Type: text/plain")
		print()
		print("Error occurred")

def main():
	form = cgi.FieldStorage()
	file_path = form.getvalue('file')

	if file_path:
		try:
			base_dir = os.path.dirname(os.path.abspath(__file__))
			safe_path = os.path.join(base_dir, file_path)

			if not os.path.commonprefix([safe_path, base_dir]) == base_dir:
				raise FileNotFoundError

			with open(safe_path, 'r') as file:
				content = file.read()
			
			print("Content-Type: text/html\n")
			print(content)
		except FileNotFoundError:
			serve_error('html/404_cgi.html')
		except Exception as e:
			print("Content-Type: text/plain\n")
			print(f"Error: {str(e)}")
	else:
		print("Content-Type: text/plain\n")
		print("Error: No file specified")

if __name__ == "__main__":
	main()
