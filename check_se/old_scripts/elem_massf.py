#!/usr/bin/env python

"""find cycles that do not contain elem_massf

This program iterates over the files specified as command-line
arguments, and then over all the cycles within each file, and
identifies cycles that don't contain a column named 'elem_massf'.

You can point this at all available surf.h5 files (for example) using
something like

% find /rpod2/fherwig/SEE/data/set1.[12] -name "*.surf.h5" | \
  xargs ./elem_massf.py
"""

import h5py
import sys

for i, filename in enumerate(sys.argv[1:]):
    print filename + "..."
    sys.stdout.flush()

    f = h5py.File(filename, mode='r')

    for cycle in f.keys():
        try:
            dataset = f[cycle]['SE_DATASET']
        except ValueError:
            continue
        except:
            print "Unexpected error:", sys.exc_info()[0]

        try:
            d = dataset['elem_massf']
        except ValueError:
            print filename + '/' + cycle

    f.close()
