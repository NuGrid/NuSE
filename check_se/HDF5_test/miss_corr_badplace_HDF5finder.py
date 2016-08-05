#!/usr/bin/env python
'''
For extensive se output testing

Finds the corrupted or missing cycles in the HDF5 data of MESA SE and
MPPNP. 

v0.1, 17NOV2014: NuGrid collaboration
(Christian Ritter, Sam Jones, Falk Herwig, Reto Trappitsch)

'''

#Choose from the following modes

#Mode 1: Test for missing cycles (+ test if first cycle is corrupted)
#Mode 2: Testing for corrupted input in cycle intervals 'stepsize'
	 #Test stops at cycle which is corrupted

#Note1: Use both modes on all se files (surf,out,restart)
#Note2: Choose Mode 2 when while reading hdf5 files a  segmentation fault occurs

import sys


########input parameter

directory='/astro/critter/critter1/Results/DATA/DataTest/TestPreCburning/M15Z02FULL3'
delta_cyc=1000 ## HOW LONG ARE YOUR SE FILES?
Test1=True
Test2=True
#if set to true, uses Sams replacement method
#with Umberto's addition to check eventual
#cycles' missplacements
replace_files=True
#filename_write=sys.argv[2] #'hdf5_report_surf.txt'
filename_write='hdf5_report.txt' ###  MY MODIFICATION

#if set to true removes prepr file
remove_prepr=True#False

########

import se
import nugridse as mp
import numpy as np
from shutil import copyfile
import os
import re
import h5py

def replace_missing_cycle(offending_file,cycle):
        '''From Sam. This function creates cycle by copying the data from the previous cycle
        which is a fix for MESA SE files when a restart has occurred'''

        # First we want to make a copy of the offending_file and initialise the
        # file as an sefile using the se module:

	print 'offending file: ',offending_file,', cycle: ',cycle
        backupfile=directory+'/'+offending_file[:-6]+'.missing_cycle'
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
                copyfile(directory+'/'+offending_file,backupfile)
                f=se.seFile(directory+'/'+offending_file) #CR: uses local se.py; with h5py below didn't work
                #f=h5py.File(directory+'/'+offending_file,'r')
                groups=f.keys()
                print 'LEN GROUPS: ',len(groups)

                group_to_copy=str(groups[-2]) ## To be edited according to which group is our target
                new_group='cycle'+str(cycle).zfill(10)
                f.copy(group_to_copy,new_group)  #error here
                f.close()

# first find initialise the data and find the missing cycles:
if remove_prepr==True:
	if os.path.exists(directory+'h5Preproc.txt'):
		print 'h5Preproc.txt will be removed'
		os.remove(directory+'h5Preproc.txt')
		#sys.exit()
	

sefiles=mp.se(directory,'.h5')

#identify type of se file
#slist = os.listdir(directory)
#expr = re.compile('.h5')
#slist=(filter(expr.search,slist))
import re
slist=sefiles.sefiles
expr = re.compile('.h5')
slist=(filter(expr.search,slist))
last_file=sorted(slist)[-1]

if '.se.' in slist[0]:
	filetype='MESA'
if '.out.' in slist[0]:
	filetype='H5_out'
if '.surf.' in slist[0]:
	filetype='H5_surf'
if '.restart.' in slist[0]:
	filetype='H5_restart'


report_txt=('####################################################'+'\n')
report_txt+=('Checking directory '+str(directory)+'\n')
report_txt+=('filetype found: '+filetype+'\n')

print 'filetype found: ',filetype


cycles=sefiles.se.cycles
cycles1=[]
for k in range(len(cycles)):
	cycles1.append(int(cycles[k]))
cycles=cycles1
print '#########################'
missplaced_cycles=[]
missing_cycs=[]
corrupted=[]
idx=0
if (Test1==True):
	for i in range(1,len(cycles)):
		#tests if cycle exists
		if filetype =='MESA':  
			#mesa should have every cycle
			if int(cycles[i]) != int(cycles[i-1])+1:
				missing = int(cycles[i-1])+1
				missing_cycs.append(missing)

		if filetype=='H5_out' or filetype=='H5_surf' or filetype=='H5_restart':
			#to skip over cycle, in case its just inbetween two correct cycles,e.g. restart: 6400,6415,6500
			if idx>0:
			       if i<(idx+1):
				  #print 'skip',i,'cycles',cycles[i]
				  continue
			       if idx==i:
				  idx=0	
			#MPPNP should have certain step size
			if i==1:
				stepsize=int(cycles[1]) - int(cycles[0])
				print 'Start cycle test with steps of ',stepsize
				report_txt+=('Start cycle test with steps of '+str(stepsize)+'\n')		
				if stepsize ==0:
					print 'beginning cycles corrupted'
					#print 'Do again a test after problem solved'
					sys.exit()
			if int(cycles[i]) != int(cycles[i-1])+stepsize:
				missing = int(cycles[i-1])+stepsize
				#maybe just a new cycle inbetween due to restart..
				if cycles[-1]<missing:
					break
				try:
					#print 'missing cycle' ,missing
					idx=cycles.index(missing)
					
					#print 'found cycle at pos ',idx
				except:
					#print 'cycle not found'
					missing_cycs.append(missing)
	if len(missing_cycs)>0 and (filetype=='MESA'):
		# now find the files in which the missing cycles should be by using the fact
		# that they are the last cycle before a restart, and hence should be the last
		# cycle in its corresponding file
		files=slist  #sefiles.sefiles
		files.sort()
		offending_files=[]
		for cyc in missing_cycs:
			for f in files:
				if '0'+str(cyc+1)+'.se.h5' in f:
					offending_file = files[files.index(f)-1]
					offending_files.append(offending_file)
		if replace_files==True:
			# call the function to fix the file:
			for i in range(len(missing_cycs)):
				replace_missing_cycle(offending_files[i],missing_cycs[i])


	#fast check if content of first cycle is corrupted
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
         report_txt+='no missing in sefiles.se.cycles \n'
else:
	report_txt+= 'missing cycles in sefiles.se.cycles:\n'
	for k in range(len(missing_cycs)):
		report_txt+=('Cycle '+str(missing_cycs[k])+'\n')
		if replace_files==True:
                      report_txt+=('file '+offending_files[i]+'has been copied to '+offending_files[i][:-6]+'.missing_cycle'+'\n')
                      report_txt+=( 'cycle '+str(missing_cycs[i])+' was added to file '+offending_files[i]+'\n')
			
if len(corrupted) == 0:
	report_txt+= 'No corrupted cycles\n'
else:
	report_txt+= 'List of corrupted cycles:\n'
	for k in range(len(corrupted)):
		report_txt+=(','+corrupted[k])
	report_txt+='\n'


if (Test2==True):
	#Test each hdf5 file by opening it individually
	#In the case you extract data (e.g. multiple cycles, array) and get and segmentation fault;
        #Also, checks if any cycle is placed in the wrong file.
	import h5py
	import sys
	report_txtl=len(report_txt)
	for i, filename in enumerate(slist):    #sys.argv[1:]):
            filename2=filename    
	    filename=directory+'/'+filename
	    print 'Test file '+filename
	    sys.stdout.flush()
	    try:
		    f = h5py.File(filename, mode='r')
                    ff=mp.se(directory,filename2)  ## MY MODIFICATION
            except:
                    report_txt+=( 'Cannot open file '+filename+'\n')
		    continue		
	    if True:
		    try:
			testerror=f.keys()
		    except:
			report_txt+=( 'Cannot read elements (f.keys()) in file '+filename+'\n')
			continue
		    for cycle in f.keys():
			#for Mesa files which have [u'A', u'Z', u'cycle0000005100', u'isomeric_state']
			if not 'cycle' in cycle:
				continue
			try:
			    dataset = f[cycle]['SE_DATASET']
			#except ValueError:
			#    continue
			except:
			    report_txt+=( 'SE_DATASET not available in file '+filename+ '/' + cycle+'\n')
			    #print "Unexpected error:", sys.exc_info()[0]

			if (filetype=='H5_surf' or filetype=='H5_out') or (filetype=='H5_restart'):
				try:
				    d = dataset['iso_massf']  #['elem_massf']
				except ValueError:
				    report_txt+=('Iso_massf missing in '+filename + '/' + cycle+'\n')
			if filetype == 'MESA':
				try:
			    	    d = dataset['delta_mass']
				except ValueError:
				    report_txt+=( 'delta_mass missing in '+filename + '/' + cycle+'\n')

            if filetype == 'MESA':  ### check for eventual missplacements.

                    if ((int(ff.se.cycles[len(ff.se.cycles)-1])%delta_cyc)!=0):
                            missplaced_cycles.append(str(int(ff.se.cycles[len(ff.se.cycles)-1])+1))
                            report_txt+=('Possible missplaced cycles: '+missplaced_cycles[len(missplaced_cycles)-1])

	    f.close() 
	if len(report_txt) == report_txtl:
	    report_txt+=('Check for SE_DATASETs successfull'+'\n')  

print '########Report result###############'
print report_txt

'''
if (filetype=='H5_surf' or filetype=='H5_out') or (filetype=='H5_restart'):
	print 'Testing cycles in steps ',stepsize,'for currupted cycles'
	cycles=sefiles.se.cycles
	stepsize=int(cycles[1]) - int(cycles[0])
	#Below will stop when reading error
	for c in range(0,int(sefiles.se.cycles[-1])+stepsize,stepsize):
		print c
		h1=sefiles.get(c,'H-1')	
'''




#def replace_missing_cycle(offending_file,cycle):
#        '''From Sam. This function creates cycle by copying the data from the previous cycle
#        which is a fix for MESA SE files when a restart has occurred'''
#
#        # First we want to make a copy of the offending_file and initialise the
#        # file as an sefile using the se module:
#
#        backupfile=directory+'/'+offending_file[:-6]+'.missing_cycle'
#        if os.path.exists(backupfile):
#                print ' '
#                print '*************************************'
#                print ' '
#                print 'STOP: '+backupfile+' already exists.'
#                print ' '
#                print 'By calling this command again the original data could become corrupted.'
#                print 'If there was a problem calling the script last time,'
#                print 'this file should be moved to its original name before trying again.'
#                sys.exit()
#        else:
#                copyfile(directory+'/'+offending_file,backupfile)
#                f=se.seFile(directory+'/'+offending_file)
#                groups=f.keys()
#                # it has to be the last group (i.e. cycle) that needs to be copied
#                # (the last one is 'isomeric_state;, so we need penultimate one:
#                group_to_copy=str(groups[-2])
#                new_group='cycle'+str(cycle).zfill(10)
#                f.copy(group_to_copy,new_group)
#                f.close()

f1=open(filename_write,'a')
f1.write(report_txt)
f1.close()
