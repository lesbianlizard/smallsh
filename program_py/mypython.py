#!/usr/bin/env python3
import string
import random
import os
import sys
import functools

filenames = ["file1", "file2", "file3"]
strings = ["", "", ""]
numbers = []

for j in range(3):
  for i in range(10):
    strings[j] += random.choice(string.ascii_lowercase)

  strings[j] += os.linesep
  sys.stdout.write(strings[j])

  file = open(filenames[j], "w")
  file.write(strings[j])
  file.close()

for i in range(2):
  numbers += [random.choice(range(1, 42 + 1))]
  print(numbers[i])

print(functools.reduce(lambda a, b: a*b, numbers)) 
