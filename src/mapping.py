import os
import argparse
from collections import defaultdict

parser = argparse.ArgumentParser()

parser.add_argument('--From', type=str)
parser.add_argument('--To', type=str)

args = parser.parse_args()

fin = open(args.From, 'r', encoding='big5-hkscs')

_dict = defaultdict(list)

for line in fin:
    character , zhuyins = line.split()
    _dict[character].append(character)
    for zhuyin in zhuyins.split('/'):
        _dict[zhuyin[0]].append(character)
fin.close()

with open(args.To, 'w', encoding='big5-hkscs') as fout:
    for key, value in _dict.items():
        fout.write(key+' ' + ' '.join(value) + '\n')
