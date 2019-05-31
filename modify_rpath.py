import inspect, os, sys
import subprocess
from shutil import copyfile

def add_from(path, fd, lst, chg):
    result = subprocess.run(['otool', '-L', path], stdout=subprocess.PIPE)
    strs = result.stdout.decode('utf-8')
    for s in strs.splitlines():
        if (s[0] == '/'):
            continue
        ss = s.split()
        if len(ss) < 2:
            continue
        if ss[0].startswith("/usr/local/"):
            nm = ss[0][ss[0].rfind('/')+1:]
            if (nm == 'Python'):
                continue
            if nm in lst:
                continue
            lst.append(nm)
            print('copying ' + nm + ' (' + ss[0] + ')')
            copyfile(ss[0], fd + nm)
            if chg:
                subprocess.run(['install_name_tool', '-change', ss[0], '@rpath/' + nm, fd + 'avis'])
            add_from(ss[0], fd, lst, False)

basefd = (os.path.dirname(os.path.abspath(inspect.getfile(inspect.currentframe()))))
fd = basefd + "/package/mac/AViS.app/Contents/MacOS/"

add_from(fd + 'avis', fd, [], True)

#look for dylibs in own directory
print('configuring rpath')
subprocess.run(['install_name_tool', '-add_rpath', '\"@executable_path\"', fd + 'avis'])

print('files written to ' + fd)