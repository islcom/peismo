from ftplib import FTP
import os, subprocess, sys


if not os.path.exists("data"):
    os.makedirs("data")
if not os.path.exists("mseed"):
    os.makedirs("mseed")
if not os.path.exists("uploaded"):
    os.makedirs("uploaded")

running = False
ss = str(subprocess.check_output(["ps", "-e"])).split("\n")
for s in ss:
	if "peismo" in s:
		running = True

if not running:
	os.system("sudo ./peismo -f >> peismo.log&")
	print("Now running peismo")
	sys.exit()

