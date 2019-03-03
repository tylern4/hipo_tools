#!/usr/bin/env python
from __future__ import print_function
from __future__ import division

import sys

import numpy as np
import time

from hipopy4 import hipo4_reader

file_name = sys.argv[1]

event = hipo4_reader(file_name.encode("utf-8"))

total = 0
start_time = time.time()

for evnt in event:
    total += 1

print(str(time.time() - start_time), "sec")
print(str(total / (time.time() - start_time)), "hz")
