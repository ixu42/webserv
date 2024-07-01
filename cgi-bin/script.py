#!/usr/bin/python3

import os
import sys

def main():
	print("Content-Type: text/plain")
	print()

	for key, value in os.environ.items():
		print(f"{key} = {value}")

if __name__ == "__main__":
	main()