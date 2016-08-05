#!/usr/bin/env python

import se
import nugridse as mp
import sys
import numpy as np
from shutil import copyfile
import os

#################################
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

def replace_missing_cycle(offending_file,cycle):
	'''This function creates cycle by copying the data from the previous cycle
	which is a fix for MESA SE files when a restart has occurred'''

	# First we want to make a copy of the offending_file and initialise the
	# file as an sefile using the se module:
	
	backupfile=sys.argv[1]+'/'+offending_file[:-6]+'.missing_cycle'
	if os.path.exists(backupfile):
		print ' '
		print '*************************************'
		print ' '
		print 'STOP: '+backupfile+' already exists.'
		print ' '
		print 'By calling this command again the original data could become corrupted.'
		print 'If there was a problem calling the script last time,'
		print 'this file should be moved to its original name before trying again.'
		sys.exit()
	else:
		copyfile(sys.argv[1]+'/'+offending_file,backupfile)
		f=se.seFile(sys.argv[1]+'/'+offending_file)
		groups=f.keys()
		# it has to be the last group (i.e. cycle) that needs to be copied
		# (the last one is 'isomeric_state;, so we need penultimate one:
		group_to_copy=str(groups[-2])
		new_group='cycle'+str(cycle).zfill(10)
		f.copy(group_to_copy,new_group)
		f.close()


# call the function to fix the file:
for i in range(len(missing_cycs)):
	replace_missing_cycle(offending_files[i],missing_cycs[i])
	print 'file '+offending_files[i]+'has been copied to '+offending_files[i][:-6]+'.missing_cycle'
	print 'cycle '+str(missing_cycs[i])+' was added to file '+offending_files[i]


