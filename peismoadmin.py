from ftplib import FTP
import os, subprocess, sys

running = False
ss = str(subprocess.check_output(["ps", "-e"])).split("\n")
for s in ss:
	if "peismo" in s:
		running = True

if not running:
	os.system("sudo ./peismo -f >> peismo.log&")
	print("Now running peismo")
	sys.exit()

