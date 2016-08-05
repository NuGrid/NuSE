#!/usr/bin/env python

#####File for extensive se output testing#####
# This script finds the corrupted or 
#missing cycles in the HDF5 data of MESA SE calculations



#Choose from the following modes

#Mode 1: Test for missing cycles (+ test if first cycle is corrupted)
#Mode 2: Testing for corrupted input in cycle intervals 'stepsize'
	 #Test stops at cycle which is corrupted

#Tips: Use both modes on all se files (out,surf,etc.)
#Tips: choose Mode 2 when while reading hdf5 files a  segmentation fault occurs


stepsize==200



import se
import nugridse as mp
import sys
import numpy as np
from shutil import copyfile
import os
import re

# first find initialise the data and find the missing cycles:
if os.path.exists(sys.argv[1]+'h5Preproc.txt'):
        print 'h5Preproc.txt will be removed'
	os.remove(sys.argv[1]+'h5Preproc.txt')
        #sys.exit()

sefiles=mp.se(sys.argv[1],'.h5')

#identify type of se file
slist = os.listdir(sys.argv[1])
expr = re.compile('.h5')
slist=(filter(expr.search,slist))
if '.se.' in slist[0]:
	filetype='MESA'
if '.out.' in slist[0]:
	filetype='H5_out'
if '.surf.' in slist[0]:
	filetype='H5_surf'

print 'filetype found: ',filetype


cycles=sefiles.se.cycles
missing_cycs=[]
corrupted=[]
if (Test1==True):
	for i in range(1,len(cycles)):
		#tests if cycle exists
		if filetype =='MESA':
			#mesa should have every cycle
			if int(cycles[i]) != int(cycles[i-1])+1:
				missing = int(cycles[i-1])+1
				missing_cycs.append(missing)
		if filetype=='H5_out' or filetype=='H5_surf':
			#MPPNP should have certain step size
			if i==1:
				stepsize=int(cycles[i]) - int(cycles[i-1])
				if stepsize ==0:
					print 'beginning cycles corrupted'
					print 'Do again a test after problem solved'
					sys.exit()
			if int(cycles[i]) != int(cycles[i-1])+stepsize:
				missing = int(cycles[i-1])+stepsize
				missing_cycs.append(missing)
		#tests if the MPPNP cycle parameter are correct
		#to exclude 1e-99 values

		#problem: new implementation does nto work with python 
		#if filetype =='MESA':
			
	#fast check if content of first cycle is wrong
	if filetype=='H5_out':
		#test first cycle only because of time limit 
		sum_massfrac=sum(sefiles.get(0,'iso_massf')[0])
		if (sum_massfrac<0.9999) or (sum_massfrac>1.):
			corrupted.append('cycle '+str(cycles[i])+' corrupted')
	if filetype=='H5_surf':
		sum_massfrac=sum(sefiles.get(0,'iso_massf'))
		if (sum_massfrac<0.9999) or (sum_massfrac>1.):
			corrupted.append('cycle '+str(cycles[i])+' corrupted')

#print results
if len(missing_cycs) == 0:
        print 'no missing cycles'
else:
	print 'missing cycles:'
	print missing_cycs
if len(corrupted) == 0:
	print 'no corrupted cycles'
else:
	print 'corrupted cycles:'
	print corrupted


if (test2==True):

	#In the case you extract data (e.g. multiple cycles, array) and get and segmentation fault, check below in the stepsize
	if (filetype=='H5_surf' or filetype=='H5_out') or (filetype=='H5_restart'):
		print 'Testing cycles in steps ',stepsize,'for currupted cycles'
		cycles=sefiles.se.cycles
		#Below will stop when reading error
		for c in range(0,int(sefiles.se.cycles[-1])+stepsize,stepsize):
			print c
			h1=sefiles.get(c,'H-1')	

