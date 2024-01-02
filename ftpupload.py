import pysftp
import os, subprocess, re, sys, glob, configparser, shutil
from datetime import datetime, date, time

source_folder = 'mseed'
destination_folder = 'uploaded'

files = [f for f in os.listdir(source_folder) if os.path.isfile(os.path.join(source_folder, f))]
files = sorted(files)
#print(files)
ini_file_path = 'ftpconfig.ini'
config = configparser.ConfigParser()
config.read(ini_file_path)
username = config.get('sftp', 'username', fallback='')
password = config.get('sftp', 'password', fallback='')
server = config.get('sftp', 'server', fallback='')
path = config.get('sftp', 'path', fallback='')

sftp = pysftp.Connection(host=server, username=username, password=password)
sftp.chdir(path)
for file in files:
    source_path = os.path.join(source_folder, file)
    sftp.put(source_path)
    sftp.chmod(file,666)
    print("Uploaded " + file)
    destination_path = os.path.join(destination_folder, file)
    shutil.move(source_path, destination_path)
    print(source_path + " -> " + destination_path)
sftp.close()