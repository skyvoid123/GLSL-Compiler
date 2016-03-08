import os
from subprocess import *

TEST_DIRECTORY = 'samples'

for _, _, files in os.walk(TEST_DIRECTORY):
  for file in files:
    if not (file.endswith('.glsl') or file.endswith('.frag')):
      continue
    refName = os.path.join(TEST_DIRECTORY, '%s' % file.split('.')[0])
    testName = os.path.join(TEST_DIRECTORY, file)

    result = Popen('./glc < ' + testName + ' >' +os.path.join('', refName) + '.bc', shell = True, stderr = STDOUT, stdout = PIPE)
    result = Popen('llvm-dis ' + os.path.join('', refName) + '.bc', shell = True, stderr = STDOUT, stdout = PIPE)
    result = Popen('./gli ' +  os.path.join('', refName) + '.bc', shell = True, stderr = STDOUT, stdout = PIPE)

    result = Popen('diff -w - ' + refName+'.out', shell = True, stdin = result.stdout, stdout = PIPE)
    print 'Executing test "%s"' % testName
    print ''.join(result.stdout.readlines())

