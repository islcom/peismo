import os, re, sys, glob, configparser
from datetime import datetime

max_files = 100

ini_file_path = 'ftpconfig.ini'
config = configparser.ConfigParser()
config.read(ini_file_path)
mseedsuffix_value = config.get('miniseed', 'mseedsuffix', fallback='')

while True:
    unsorted_files = glob.glob("data/*.dat")
    files = sorted(unsorted_files)
    if len(files)==0:
        print("No files")
        sys.exit()
    print("Number of dat files: " + str(len(files)))
    file_list_string = ""
    for i in range(min(len(files), max_files)):
        file_list_string += files[i]
        file_list_string+=" "
    #print('file list ('+str(min(len(files), max_files))+'): ' + file_list_string)
    m1 = re.match('.*?(\d+)', files[0])
    u = int(m1.group(1))
    dt = datetime.utcfromtimestamp(u)
    fn1 = "mseed/" + dt.strftime("%Y-%m-%d_%H%M") + mseedsuffix_value + ".mseed"

    pack_command_string = ('../ascii2mseed-1.4/ascii2mseed ' + file_list_string + '-o ' + fn1)
    rm_command_string = ('rm -f ' + file_list_string)
    chmod_command_string = ('chmod 666 ' + fn1)
    #command_string = ('../ascii2mseed-1.4/ascii2mseed data/*.dat -o ' + fn1)

    #print('Pack Command String: ' + pack_command_string)
    #print('rm Command String: ' + rm_command_string)
    os.system(pack_command_string)
    os.system(rm_command_string)
    os.system(chmod_command_string)