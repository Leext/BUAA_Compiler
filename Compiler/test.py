import subprocess
import argparse
import re

parser = argparse.ArgumentParser()
parser.add_argument('--output', type=str)
parser.add_argument('--asm', type=str)
parser.add_argument('--input', type=str, nargs='+')

args = parser.parse_args()

p = subprocess.Popen('java -jar ../Mars4.5.jar %s' %
                     (args.asm), stdout=subprocess.PIPE, stderr=subprocess.STDOUT, stdin=subprocess.PIPE)
rtn = p.communicate(('\n'.join(args.input).encode()))

output = re.split('[\r\n]', rtn[0].decode())[-3]

if output == args.output:
    exit(0)
else:
    exit(1)
