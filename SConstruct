#
# Fabric Engine 2.1.0-alpha
# EDKObjects EDK Sample
#
# Copyright 2010-2015 Fabric Software Inc. All rights reserved.
#

import os, sys, glob, platform

extensionName = "tensorflow"
try:
  fabricPath = os.environ['FABRIC_DIR']
except:
  print("You must set FABRIC_DIR in your environment.")
  print("Refer to README.txt for more information.")
  sys.exit(1)

try:
  tfPath = os.environ['TENSORFLOW_CAPI']
except:
  print("You must set TENSORFLOW_CAPI in your environment.")
  sys.exit(1)


SConscript('SConscript')
Import('fabricBuildEnv')

fabricBuildEnv.Append(CPPPATH = [
  tfPath,
  'GenCPP/h',
  'CustomCPP'
  ])

if platform.system() == 'Windows':
  fabricBuildEnv.Append(LIBPATH = [
    tfPath,
    ])
  fabricBuildEnv.Append(LIBS = [
    "tensorflow.lib",
    ])
# else:
#   fabricBuildEnv.Append(LIBPATH = [
#     tfPath + "/bin",
#     ])
#   fabricBuildEnv.Append(LIBS = [
#     "libai.so",
#     ])

  fabricBuildEnv.Append( RPATH = fabricBuildEnv.Literal('\\$$ORIGIN'))
  #fabricBuildEnv.Append(RPATH='$ORIGIN')

cppFiles = glob.glob('./GenCPP/cpp/*.cpp')
dll = fabricBuildEnv.Extension(
  extensionName,
  cppFiles
  )

# Copy relevant files into stage directory
print("Installing stuff")
stage_dir = 'exts/' + extensionName
kl_files = glob.glob('./GenKL/*.kl')
fpm_file = glob.glob('./GenKL/'+extensionName+'.fpm.json')

fabricBuildEnv.Install(stage_dir, dll)
fabricBuildEnv.Install(stage_dir, kl_files)
fabricBuildEnv.Install(stage_dir, fpm_file)
if platform.system() == 'Windows':
  fabricBuildEnv.Install(stage_dir, '{0}/{1}.dll'.format(tfPath, extensionName))
else:
  fabricBuildEnv.Install(stage_dir, tfPath + '/bin/tensorflow.so')

# set our file paths to be relative to the proj directory
hFiles = glob.glob('./GenCPP/h/*.h')
for i in range(len(cppFiles)):
  cppFiles[i] = cppFiles[i].replace('\\', '/')
  cppFiles[i] = os.path.relpath(cppFiles[i], './GenCPP')

for i in range(len(hFiles)):
  hFiles[i] = hFiles[i].replace('\\', '/')
  hFiles[i] = os.path.relpath(hFiles[i], './GenCPP')

if platform.system() == 'Windows':
  # Build a VS project to go along with this dll
  # We can use this for debugging the project later.
  fabricBuildEnv.MSVSProject(target = './GenCPP/{0}.{1}'.format(extensionName, fabricBuildEnv['MSVSPROJECTSUFFIX']),
                  srcs = cppFiles,
                  incs = hFiles,
                  buildtarget = dll[0],
                  variant = 'Debug|x64')

