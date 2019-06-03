import inspect, os, sys
import subprocess
from shutil import copyfile

def add_from(path, fd, lst):
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
                subprocess.run(['install_name_tool', '-change', ss[0], '@rpath/Python.framework/Versions/3.7/Python', path])
            
            else:
                subprocess.run(['install_name_tool', '-change', ss[0], '@rpath/' + nm, path])
                if nm in lst:
                    continue
                lst.append(nm)
                print('copying ' + nm + ' (' + ss[0] + ')')
                copyfile(ss[0], fd + nm)
                add_from(fd + nm, fd, lst)

basefd = (os.path.dirname(os.path.abspath(inspect.getfile(inspect.currentframe()))))
fd = basefd + "/package/mac/AViS.app/Contents/MacOS/"

copied_libs = []

add_from(fd + 'avis', fd, copied_libs)

print('copying libRadeonRays.dylib')
copyfile(basefd + '/radeonrays/build/bin/libRadeonRays.dylib', fd + 'libRadeonRays.dylib')
print('copying libOpenImageDenoise.0.dylib')
copyfile(basefd + '/oidn/build/libOpenImageDenoise.0.dylib', fd + 'libOpenImageDenoise.0.dylib')
add_from(fd + 'libOpenImageDenoise.0.dylib', fd, copied_libs)

#look for dylibs in own directory
print('configuring rpath')
subprocess.run(['install_name_tool', '-add_rpath', '@executable_path', fd + 'avis'])

print('files written to ' + fd)

subprocess.run(['touch', basefd + '/package/mac/AViS.app'])
subprocess.run(['touch', basefd + '/package/mac/AViS.app/Contents/Info.plist'])
