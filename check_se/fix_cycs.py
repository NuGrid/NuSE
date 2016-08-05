#!/usr/bin/env python

import se
import nugridse as mp
import sys
import numpy as np
from shutil import copyfile
import os

#################################
# This testing file i got from Sam. But it contains only the checking-part.
# This script finds the missing cycles in the HDF5 data of MESA SE calculations
# and creates them using a copy of the preceeding cycle.
#
# The original data file (having the missing cycle) is renamed with extension
# '.missing_cycle' instead of '.se.h5', so it may be excluded from the .se class
# initialisation.
#
# 	ex: ./fix_cycs.py ./M25Z1.0E-03
#
#	This command would fix replace the missing cycles in the folder M25Z1.0E-03
#	containing the SE data, changing the extension of the original data files
#	from '.se.h5' to '.missing_cycle' to avoid corruption of the data
#
#################################

# first find initialise the data and find the missing cycles:
if os.path.exists(sys.argv[1]+'h5Preproc.txt'):
	print 'h5Preproc.txt should be removed or renamed'
	sys.exit()

pt=mp.se(sys.argv[1],'.se.h5')
cycles=pt.se.cycles
missing_cycs=[]
for i in range(1,len(cycles)):
	if int(cycles[i]) != int(cycles[i-1])+1:
		missing = int(cycles[i-1])+1
		missing_cycs.append(missing)

if len(missing_cycs) == 0:
	print 'no missing cycle'
	sys.exit()

print 'missing cycles:'
print missing_cycs


# now find the files in which the missing cycles should be by using the fact
# that they are the last cycle before a restart, and hence should be the last
# cycle in its corresponding file

files=pt.sefiles
files.sort()
offending_files=[]
for cyc in missing_cycs:
	for f in files:
		if '0'+str(cyc+1)+'.se.h5' in f:
			offending_file = files[files.index(f)-1]
			offending_files.append(offending_file)

print 'offending files:'
print offending_files

