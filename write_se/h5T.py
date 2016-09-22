#!/usr/bin/python
import os
import numpy as np
import gc
import threading
import time
import glob


#########################################################################################################################
#
#        h5T.py    V1.1
#        Author: Will Hillary
#        email:    will.hillary@gmail.com
#        Date:    Dec 18/09
#
#        Modified to V1.1 by Daniel Conti
#	 Email: hoshi@uvic.ca
#	 Date:  December 2010
#
#        This is a simple python terminal interface for hdf5 files generated using NuGrid Stellar 
#        evolution code.  It provides as simple as possible interface by  allowing for intuitive, 
#        but sparse commands.  Refer to the users manual and docstring below for more information
#
########################################################################################################################


try:
	from ascii_table import *
except ImportError:
	print 'No module ascii_table'
	print 'Please checkout ascii_table.py svn://forum.astro.keele.ac.uk/utils/pylib and add to python path'
try:
    import h5py as mrT
except ImportError :
    os.system('HDF5_DISABLE_VERSION_CHECK=1')
    import h5py as mrT

class Files(threading.Thread):
    '''
	Chnges made by Daniel Conti
	A) Txtfile preprocessor: When the Files class is run in a directory for the first
	   time, a preprocessor file is written, in the same directory.  When loading in 
	   a directory of files, this class opens up every file to find the ages and the 
	   cycles. This file lets the class work faster, by writing the ages and cycles 
	   contained within each file to the preprocessor file.  This allows the class 
	   to not open every h5 file in the directory.  There is one downside to this 
	   however, This Files class has a list of h5File threads, if the a prepocessor
	   file is read, these threads do not get started.  So to solve this, whenever a 
	   method needs some data from one of these h5File threads, it then starts the 
	   thread and adds a True boolean to a list of booleans (that correspond to the
	   list of files) of if that particular thread has been called.  Currently, to 
	   the authors knowledge, this has been taken care of.  If in the future someone
	   adds a method that works with any of these threads in self.h5s. For example,
	   if the command:
	   table=h5.Table
	   where h5 is one of these h5File threads, the future author should have this 
	   aswell:
	   if not self.h5sStarted[self.h5s.index(h5)]
		   h5.start()
		   h5.join()
		   table=h5.Table
		   self.h5sStarted[self.h5s.index(h5)]=True
	   else:
		   table=h5.Table
	   
	   This will run (start) the required thread and adds a True boolean, so we only
	   ever need to start a thread once.
	
	B) Additional functionality to the get method: The get method can now be called
	   with the name of the isotope, instead of the isotope and 'iso_massf'
	   For example if we wanted the abundance of Heleium 3 from cycle 20, we would 
	   call:
	   get(20,'He-3')
	   This would yield identical results to 
	   get(20,'iso_massf','He-3')
	   
	   Also this author found and solved numerous bugs
	   
	C) Added a pattern argument to the init method, A user can use a Unix, 
		like regular expression to find files in a directory.
		
	D) Added a findCycle method, that allows the class to find the closest 
		Cycle to what the user inputs, for example if the user imputs a 
		cycle that DNE , it will find the next closest one
	   
	Assumptions:
	The user must have the module ascii_table and in the python path. It can
	be checked out from the svn svn://forum.astro.keele.ac.uk/utils/pylib
	To use the preprocessor, the user need write access to the folder he is working in
	Cycle numbers are 10 digits long
	Cycle numbers are allways whole numbers
	The preprocessor file will become unstable (ie not return accurite results), if 
		any of the H5 files, internally, change their cycles or ages.
	If any H5 files are added and removed, the method will realize that there are 
		additional or missing files and will write a new preprocessor file.
	If any of the files are renamed, The program realize that files were renamed and
		will write a new preprocessor file.
	If a new selection of files are slected, The program realize this and
		will write a new preprocessor file.
	If a prprocessor file is removed, The program realize this and will write a new 
		preprocessor file.
	In each file name the cycle number is the list of numbers .(surf/out.h5)
'''
    preprocName='h5Preproc.txt' #filename of the preprocessor file
    preprocExists=False		#Boolean of if the preprocessor file exists
    h5files = []
    h5s = []			#list of h5File threads, for each of the file names
    cycles = []
    ages =     []
    hattrs = []
    cattrs = []
    Tables = []
    dcols =     []
    filepaths = []
    isotopes = []
    isomeric_states = []
    A = []
    Z = []
    groundState=1      #The identifyer for ground state
    excitedState=2     #The
    isomerDelimiter='m'#The chartacter string thet we use to seperate the isomer
    		       #identifer from the rest of the isotope name
    h5sStarted=[]      #A list of booleans of if the thread in h5files has had its 
    			#start and join methods called 
    #    This is a list of isos to match to.
    isos = ['Neutron','H','He','Li','Be','B','C','N','O','F','Ne','Na','Mg','Al',
    'Si','P','S','Cl','Ar','K','Ca','Sc','Ti','V','Cr','Mn','Fe','Co','Ni',
    'Cu','Zn','Ga','Ge','As','Se','Br','Kr','Rb','Sr','Y','Zr','Nb','Mo','Tc',
    'Ru','Rh','Pd','Ag','Cd','In','Sn','Sb','Te', 'I','Xe','Cs','Ba','La','Ce',
    'Pr','Nd','Pm','Sm','Eu','Gd','Tb','Dy','Ho','Er','Tm','Yb','Lu','Hf','Ta',
    'W','Re','Os','Ir','Pt','Au','Hg','Tl','Pb','Bi','Po','At']
    
    def findCycle(self,cycNum):
    	    '''
    	    Method that looks through the self.cycles and returns the nearest 
    	    cycle:
    	    
    	    Input:
    	    cycNum: int of the cycle desired cycle 
    	    
    	    '''
    	    
    	    
    	    cycNum=int(cycNum)
    	    i=0
    	    
    	    while i < len(self.cycles):
    	    	    if cycNum < int(self.cycles[i]):
    	    	    	    break
    	    	    i+=1
    	    
    	    if i ==0:
    	    	    return self.cycles[i]
    	    elif i == len(self.cycles):
    	    	    return self.cycles[i-1]
	    lower=int(self.cycles[i-1])
	    higher=int(self.cycles[i])
	    
	    if higher- cycNum >= cycNum-lower:
	    	    return self.cycles[i-1]
	    else:
	    	    return self.cycles[i]
    	    	    	
    	    	    
    	    
    	    
       #    Upon initialization of an h5fileholder, the h5 files are defined (in their own wrapper-see below) and 
    #    and gathers some important data from the files.  
    def __init__(self ,path='.',fName=None,pattern='*',rewrite=False):
    	'''
    	Init method
    	Input:
    	path: The path you would like to work in
    	fName: A File name, or a list of filenames that you would like to work with
    	pattern: A string that contains a particular pattern that the user is 
    		working with. For example '1001.out' would make this method only read 
    		files that contain that string. Note one can imput any regular 
    		expression (ie shell-style wildcards) into this, as one would do
    		in A Unix shell 
    	rewrite: If True, forces a rewrite of the preprocessor file
    	'''
        threading.Thread.__init__(self)
        self.h5files = []
        self.h5s = []
        self.cycles = []
        self.ages =     []
        self.hattrs = []
        self.cattrs = []
        self.Tables = []
        self.dcols =     []
        self.filepaths = []
        self.isotopes = []
        self.elements = []    
        self.isomeric_states = []
        self.A = []
        self.Z = []
        self.h5sStarted=[]


        if fName==None and pattern=='*':
            self.filename = path
            self.files = os.listdir(self.filename)
            
            for fil in self.files:
                if os.path.isfile(self.filename + os.sep + fil) and fil.count('.h5'):
                        self.h5files.append(self.filename + os.sep + fil)
        elif pattern=='*':
            self.filename = path
            if self.filename[-1] == os.sep:
                self.filename = self.filename[:-1]
            self.files = os.listdir(self.filename)
            self.h5files = []
            temps = fName
            if type(temps).__name__ != 'list':
                temps = [temps]
            for arg in temps:
                self.h5files.append(self.filename + os.sep + arg)
        elif fName==None:
            self.filename = path
            if not pattern.endswith('*'):
            	    pattern=pattern+'*'
            if not pattern.startswith('*'):
            	    pattern='*'+pattern	    
            self.files=glob.glob(self.filename + os.sep +pattern)
            for i in xrange(len(self.files)):
            	    self.files[i]=self.files[i].split(os.sep)[-1]
            
            for fil in self.files:
                if os.path.isfile(self.filename + os.sep + fil) and fil.count('.h5'):
                        self.h5files.append(self.filename + os.sep + fil)
        
        #preprocessor stuff
        if self.preprocName.endswith('/'):
		preprocName = str(self.filename)+self.preprocName
		
	else:
		preprocName = str(self.filename)+'/'+self.preprocName
		
        self.preprocExists=os.path.exists(preprocName)
        if rewrite:
        	self.preprocExists=False
        if self.h5files == []:
            print 'There are no h5Files in: ', self.filename, 'please try a better folder.'
        else:
            print 'Searching files, please wait.......'
            for i in xrange(len(self.h5files)):
            	self.h5sStarted.append(False)
            self.h5s.append(h5File(self.h5files[0],True, True))
            
            self.h5s[0].start()
            self.h5s[0].join()
            self.h5sStarted[0]=True
            self.start()
            
           
            self.join()
            #print self.h5s
            
            #self.connect(self.h5s[-1], qc.SIGNAL('finished()'), self.continue_h5s)
    
        
    def run(self):    
        
        self.cycles.extend(self.h5s[0].cycle)
        self.ages.extend(self.h5s[0].age)
        self.hattrs = self.h5s[0].hattr
        self.cattrs =   self.h5s[0].cattr
        self.Tables    =   self.h5s[0].Table
        self.dcols     =   self.h5s[0].dcol
        self.cycle_header = self.h5s[0].cycle_header    #This string handles the name of the cycle
        try:
            self.A = self.h5s[0].A[0]
        except IndexError:
            print "Sorry, there is no A vector. This can cause problems for reading abundances. Continue..."
        try:
            self.Z = self.h5s[0].Z[0]
        except IndexError:
            print "Sorry, there is no Z vector. This can cause problems for reading abundances. Continue... "
        try:
            self.isomeric_states = self.h5s[0].isomeric_state[0]
        except IndexError:
            print "Sorry, there is no isomeric state vector. Continue..."
        new = self.h5s[0].new    #    This boolean handles the changes to the cycle nomenclature format
        
        if self.filename.endswith(os.sep):
			
			b = str(self.filename)+str(self.preprocName)
	else:
			
			b = str(self.filename)+os.sep+str(self.preprocName)
        if self.preprocExists:
        	preprocTable=ascii_table(self.preprocName,self.filename)
        	if int(preprocTable.hattrs[0])<len(self.h5files):
        		self.preprocExists=False
        		print 'A File was added, rewriteing preprocessor file'
        	
        	if self.preprocExists:
			for i in xrange(len(self.h5files)):
				
				if os.path.basename(self.h5files[i])+'-cyc' not in preprocTable.dcols and self.preprocExists:
					print 'A File was renamed, rewriteing preprocessor file'
					print preprocTable.dcols[i], os.path.basename(self.h5files[i])+'-cyc'
					self.preprocExists=False
        
        if not self.preprocExists and os.path.exists(b):
        	
        	os.system('rm '+b)
        
# create list of isotopes stored in this h5 file
	
	try:
		for x in xrange(len(self.Tables[0])):
			if self.isomeric_states[x] ==1:
				self.isotopes.append(self.isos[int(self.Tables[1][x])]+'-' +str(int(self.Tables[0][x])))
			else:
				self.isotopes.append(self.isos[int(self.Tables[1][x])]+'-' +str(int(self.Tables[0][x]))+\
				self.isomerDelimiter+str(self.isomeric_states[x]-1))
	except IndexError:
		print 'This file does not contain any tables.  Isotopic data must be contained elsewhere.'
        t1 = time.time()
        
# create list of elements stored in this h5 file
        asaved=''
        for atmp in self.Z:
            if atmp != asaved:
                self.elements.append(self.isos[int(atmp)])
                asaved=atmp        

        thread_list = []
        print(self.h5s[0].filename)
        for x in xrange(len(self.h5files)-1):
            thread_list.append(self.h5s.append(h5File(self.h5files[x+1],False, new)))
            print(self.h5s[x+1].filename)
            if not self.preprocExists:
		    self.h5s[x+1].start()
		    self.h5s[x+1].join()
		    self.h5sStarted[x+1]=True
            #self.h5s[x+1].run()
            #print('Active '+ str(threading.active_count()))
        
        #    Wait until the threads are done.
        #for thread in self.h5s:
        #    print(thread)
        #    thread.join()        
        
        if not self.preprocExists:
		for x in xrange(len(self.h5files)-1):
		    self.cycles.extend(self.h5s[x+1].cycle)
		    self.ages.extend(self.h5s[x+1].age)
		header=[str(len(self.h5files)),'This is a preprocessor file for the directory: '+str(self.filename),\
		'At the time of the creation of this file there were '+str(len(self.h5files))+\
		' h5 files.']
		
		
		try:
		    self.cycles = sorted(self.cycles, cmp=self.numeric_compare)
		except TypeError:
		    print "There was a problem sorting the cycles.  You may have problems later.  Please consider reloading(h5T) and trying again"
		
		try:
		    self.ages = sorted(self.ages, cmp=self.numeric_compare)        
		except TypeError:
		    None
		
		
		print 'Writeing preprocessor files'
		data=[]
		dcols=[]
		length=0
		for i in xrange(len(self.h5s)):
                        print self.h5s[i].filename.rpartition('/')[2]
			dcols.append(os.path.basename(self.h5s[i].filename)+'-cyc')
			dcols.append(os.path.basename(self.h5s[i].filename)+'-age')
			data.append(self.h5s[i].cycle)
			data.append(self.h5s[i].age)
			if len(self.h5s[i].cycle)>length:
				length=len(self.h5s[i].cycle)
			if len(self.h5s[i].age)>length:
				length=len(self.h5s[i].age)
		
		for i in xrange(len(data)):
			for j in xrange(length-len(data[i])):
				data[i].append(3.14159265)
		
		write(self.preprocName,header,dcols,data,sldir=self.filename)
		
        else:
        	print 'Reading preprocessor files'
        	preprocTable=ascii_table(self.preprocName,self.filename)
        	for i in xrange(len(self.h5s)-1):
        		dat=preprocTable.get(os.path.basename(self.h5s[i+1].filename)+'-cyc')
        		dat1=[]
        		for j in xrange(len(dat)):
        			if dat[j]!=3.14159265:
        				dat1.append(dat[j])
        			
        		dat=dat1
        		for j in xrange(len(dat)):
        			dat[j]=str(int(dat[j]))
        			for k in xrange(10-len(dat[j])):
        				dat[j]='0'+dat[j]
        		
        		for j in xrange(len(dat)):
        			self.cycles.append(dat[j])
        		self.h5s[i+1].cycle=dat
        		dat=preprocTable.get(os.path.basename(self.h5s[i+1].filename)+'-age')
        		dat1=[]
        		for j in xrange(len(dat)):
        			if dat[j]!=3.14159265:
        				dat1.append(dat[j])
        		dat=dat1
        		self.h5s[i+1].age=dat
        		for j in xrange(len(dat)):
        			self.ages.append(dat[j])
        	try:
		    self.cycles = sorted(self.cycles, cmp=self.numeric_compare)
		except TypeError:
		    print "There was a problem sorting the cycles.  You may have problems later.  Please consider reloading(h5T) and trying again"
		
		try:
		    self.ages = sorted(self.ages, cmp=self.numeric_compare)        
		except TypeError:
		    None
        print 'File search complete.'
        t2 = time.time()
	print "Total duration is " + str(t2-t1) + " seconds."
        return

    def numeric_compare(self,x, y):
        if int(x)>int(y):
            return 1
        elif int(x)==int(y):
            return 0
        else: # x<y
            return -1
    '''        
    def startThreads(self,threads,IsConcurrent):
         """
         Method that starts the h5 file threads
         Input: Threads- a list of threads that need to be started
         """
         
         if (str(threads.__class__())==("<type 'list'>")):
         	threads=[threads]
    '''     
                
    # This function determines which cycle, which file, which storage mechanism (cattr or data) and returns it 
    def get(self, cycle_list,dataitem=None,isotope=None,sparse=1):
    	'''
        There are three ways to call this function
            option 1
            get(dataitem)
                fetches the dataitem for all cycles, interperates the argument 
                cycle list, as Data Item
           option 2
           get(cycle_list, dataitem)
                fetches the dataitem from the list of cycles. If Dataitem is an 
                isotope, it then returns self.get(cycle_list,'iso_massf',dataitem)
           option 3    [note that we don't need this anymore, as option 2 gives 
                        access to abundances, just kept for backwards compatibility.]
            get(cycle_list, 'iso_massf', isotope)
               isotope Must be in the form 'H-2'
                fetches the isotope data for a list of cycles
                
        Additional Input:
        	sparse - implements a sparsity factor on the fetched data ie
        		 only the i th cycle in cycle_list data is returned
        		 where i = sparse
        '''
        #    Check out the inputs        
        t1=time.time()
        isotope_of_interest = []
        
        if dataitem==None and isotope==None:
            option_ind = 1
            dataitem = cycle_list
            
            if self.hattrs.count(dataitem) == 0: #if data item is not a header attribute
            	
                cycle_list = self.cycles
            else:
                first_file = mrT.File(self.h5s[0].filename,'r')
                dat = first_file.attrs.get(dataitem, None)
		first_file.close()
                return dat
            if dataitem.split('-')[0] in self.isos:
                try:
                    return self.get(cycle_list,'iso_massf',dataitem,sparse=sparse)
		except: # in some old se files there maybe still yps as the name for the abundance arrays
                    return self.get(cycle_list,'yps',dataitem,sparse=sparse) 
        elif isotope==None:
            option_ind = 2
            cycle_list = cycle_list
            dataitem = dataitem
            if dataitem.split('-')[0] in self.isos:
                try:
                    return self.get(cycle_list,'iso_massf',dataitem,sparse=sparse)
		except: # in some old se files there maybe still yps as the name for the abundance arrays
                    return self.get(cycle_list,'yps',dataitem,sparse=sparse) 
        else:
# there is an implicite rule here that if you want 2D arrays you have
# to give 3 args, or, in other words you have to give a cycle or cycle
# array; there is no good reason for that, except the programmers
# laziness
            option_ind = 3
            cycle_list = cycle_list
            dataitem =  dataitem
            isotope_of_interest = isotope    
# we need to find out the shellnb to know if any yps array may just be
# a one row array, as - for example- in the surf.h5 files
            shellnb=self.get(cycle_list,'shellnb')
            
        if sparse <1:
        	sparse=1
        
        #    Just in case the user inputs integers 
        try:
            for x in xrange(len(cycle_list)):
                cycle_list[x] = str(cycle_list[x])
        except TypeError:
            cycle_list = [str(cycle_list)]

	
	if option_ind != 1:
		
		try: #if it is a single cycle make sure its formatted correctly
		    if cycle_list.isdigit():
			cycle_list = [cycle_list]
			for cycle in cycle_list:
			    if len(cycle) != len(self.cycles[0]):
				#print "a"
				diff = len(self.cycles[0])-len(cycle)
				OO = ''
				while diff >=1:
				    OO+='0'
	    
				cycle = OO+cycle
	
		except AttributeError: ##if it is a list of cycles make sure its formatted correctly
					
		    
		    if cycle_list[0].isdigit():
					
			for x in xrange(len(cycle_list)):
			    if len(str(cycle_list[x])) != len(str(self.cycles[0])):
				#print "b"
				diff = len(str(self.cycles[0]))-len(str(cycle_list[x]))
				
				OO = ''
				while diff >=1:
				    OO+='0'
				    diff-=1
			    
				try:
				    cycle_list[x] = OO+cycle_list[x]
				except TypeError:
				    cycle_list[0] = OO+cycle_list[0]
	
        if option_ind != 1:
		for i in xrange( len (cycle_list)):
			if cycle_list[i] not in self.cycles:
				
				cycle_list[i]=self.findCycle(cycle_list[i])
        
        #print 'Method mineing data from these cycles'
        #print cycle_list[1:10]
        
        #    if it is a cattr call, it will not have a cycle value
        dat = []        
    #    for h5 in self.h5s:
    #        h5.h5 = mrT.File(h5.filename,'r')
    #'/rpod2/fherwig/tmp/tmp/'
        i=0
        cycle_list.sort()
        for cyc in cycle_list:
            
            if (i%sparse)!=0:
            	    i+=1
            	    continue
            
            i+=1
            for h5 in self.h5s:
            	    
                if h5.cycle.count(int(cyc)) or h5.cycle.count(str(cyc)):
                    
                    if not self.h5sStarted[self.h5s.index(h5)]:
                    	h5.start()
                    	h5.join()
			temp = h5.fetch_data_one(dataitem,cyc)
			self.h5sStarted[self.h5s.index(h5)]=True
		    else:
                        temp = h5.fetch_data_one(dataitem,cyc)
                        
                    # The original H5T stuff below to reduce dimensions
                    # removes useful data.  The red_dim function performs
                    # (or at least should perform...) the reduction without
                    # removing useful data
                    temp = self.red_dim(temp)
                    
#                    #    Occasionally data comes out formatted in a funny way (arrays nested in arrays....)
#                    #    This strips the nested arrays until the actual data is found
#                    if option_ind != 3:
#                        
#                        while np.ndim(temp) > 1:
#                            temp = temp[0]
#
#                    #elif option_ind == 3 and shellnb == 1:
#                        # this is the case of 2D arrays which only have one row
#                        # as, e.g. in surf.h5 files
#                        while np.ndim(temp) > 1:
#                            temp = temp[0]
#                    
#                    else:
#                        
#                        while np.ndim(temp) > 2:
#                            temp = temp[0]
#                        
#                        while len(temp) < 2:
#                            temp = temp[0]
                    if option_ind == 3 and isotope_of_interest != []:
                        #    Figure out the index
                        index = 0
                        iso_tmp = []
                        
                        #This if/else statement was produceing a bug, assumed that get was called 
                        #with three arguments, it was looking at isotopesdataitem
                        #if option_ind ==3:
                        #    iso_tmp = self.isotopes
                        #else:
                        #    iso_tmp = self.elements
                        
                        if 'iso' in dataitem: #if we are looking at an isotope
                            iso_tmp = self.isotopes
                        else:
                            iso_tmp = self.elements
                            
                        for x, iso in enumerate(iso_tmp): #finds the location of the isotope
                            
                            if iso == isotope_of_interest:
                                index = x
                                #print 'The index of this element or isotope is '+str(index)
                                break
                        islist=True
                        
                        if len(cycle_list)==1:
                        	islist=False
                        	
                        if islist:
                        	if shellnb[cycle_list.index(cyc)] == 1:    # again take care of 1-row 2D arrays
					temp = temp[index]
				else:
					temp = temp[:,index]
                	else:
				if shellnb == 1:    # again take care of 1-row 2D arrays
					temp = temp[index]
				else:
					temp = temp[:,index]

                    #    Now add the information to the list we pass back
                    
                    try:
                    	
                        dat.append(temp)
                        
                    except AttributeError:
                        np.append(dat, temp)  
              
                                                    
        if len(dat) < 2 and option_ind != 3:

            try:
                dat = dat[0]
            except IndexError:
                None
            except TypeError:
                None
        try:
            if len(dat) < 2 and isotope_of_interest != []:
                dat = dat[0]
        except TypeError:
            None    
        except IndexError:
            None
        
        
        t2=time.time()
#        print "Total get time "+ str(t2-t1)
        return dat    

    #    uses the index information to build list of isos from tables A,Z    
    def fetch_isos(self):
        isos = []
        try:
            for x in range(len(self.Tables[1])):
                isos.append([self.isos[int(self.Tables[1][x])], self.Tables[0][x]])
        except IndexError:
            None
        return isos
        

    def red_dim(self, array):
        """
        This function reduces the dimensions of an array until it is no longer
        of length 1.
        """
        while isinstance(array, list) == True or \
        isinstance(array, np.ndarray) == True:
            try:
                if len(array) == 1:
                    array = array[0]
                else:
                    break
            except:
                break
                
        return array
    
#    This wrapper class allows some automated activity when an h5file is initialized.
#    upon inmitialization the h5 file is opened and certain bits of data is read.  
#    This class also interacts with the h5fileholder class to access needed data.

class h5File(threading.Thread):
    h5 = None
    filename = None
    cycle = []
    age = []
    dcol = []
    data = []
    skipped_nodes = 0
    ver = ''
    classname = ''
    hattr = []
    cattr=[]
    Table = []
    isomeric_state = []
    A = []
    Z = []
    new = True
    


    #    Initialize the class
    def __init__(self, filepath,deep_search, new):
        
        threading.Thread.__init__(self)
        #    Instantiate
        self.h5 = None
        self.filename = None
        self.cycle = []
        self.age = []
        self.dcol = []
        self.data = []
        self.skipped_nodes = 0
        self.ver = ''
        self.classname = ''
        self.hattr = []
        self.cattr=[]
        self.Table = []
        self.isomeric_state = []
        self.A = []
        self.Z = []
        self.new = True
        
        #    Build
        self.new = new
        self.filename = filepath
        self.deep_search = deep_search
        
        
        if self.new:
            self.cycle_header = 'cycle'
        else:
            self.cycle_header = 'cycle-'
    
    def run(self):
    
        if self.deep_search:
            self.search_deep()
        else:
            self.search_shallow()
        try:
            self.h5.close()
        except:
            None
        
        return None
    
    #    Fetches a single category of information
    def fetch_data_one(self,dataitem,cycle):

        self.h5 = mrT.File(self.filename,'r')
            
        try:
	    print self.h5[self.cycle_header+str(cycle)]
	    print type(self.h5[self.cycle_header+str(cycle)])
	    print dataitem
            data = self.h5[self.cycle_header+str(cycle)][dataitem]
        except ValueError:
            try:
                data = self.h5[self.cycle_header+str(cycle)].attrs.get(dataitem, None)
            except TypeError:
                data = self.h5[self.cycle_header+str(cycle)][dataitem]    

        try:        
            while data.shape[0] < 2:
                data = data[0]
        except (IndexError, AttributeError):
            None    
            
        
        self.h5.close()
        return data


    #    The typical search algirthm when a h5file class is initialized
    def search_shallow(self):
        self.h5 = mrT.File(self.filename,'r')        
        temp = self.h5.keys()
        
        for te in temp:
            if te[0] == 'c':
                if te[5:].isdigit():
                    self.cycle.append(str(te[5:]))
                    
                    try:
                        self.age.append(self.h5[te].attrs.get("age",None)[0])
                    except TypeError:
                        self.age.append(self.h5[te].attrs.get("age",None))
                else:
                    self.cycle.append(str(te.split('-')[1]))
                    try:
                        self.age.append(self.h5[te].attrs.get("age", None)[0])
                    except TypeError:
                        self.age.append(self.h5[te].attrs.get("age",None))
                        self.cycle.sort()        
                        self.age.sort()

        
        
    def search_deep(self):
        self.h5 = mrT.File(self.filename,'r')        
        temp = self.h5.keys()
        
        #    Handles the change in cycle nomenclature
        self.new = True
        for te in temp:
            if te.count('-'):
                self.new = False
                break
        
        if self.new:
            self.cycle_header = 'cycle'
            for te in temp:
                if te[0] == 'c':
                    if te[5:].isdigit():
                        self.cycle.append(str(te[5:]))
                        try:
                            self.age.append(self.h5[te].attrs.get("age",None)[0])
                        except TypeError:
                            self.age.append(self.h5[te].attrs.get("age",None))
                    else:
                        self.isomeric_state.append(self.h5[te]['data'])
                else:

                    obj = self.h5[te].__iter__()
                    if str(te).count('A'):
                        holder = []
                        for ob in obj:
                            holder.append(ob[0])
                        self.Table.append(holder)
                        self.A.append(holder)
                    elif str(te).count('Z'):
                        holder = []
                        for ob in obj:
                            holder.append(ob[0])
                        self.Table.append(holder)
                        self.Z.append(holder)    
                    else:
                        holder = []
                        for ob in obj:
                            holder.append(ob[0])
                        self.Table.append(holder)
                        self.isomeric_state.append(holder)    
                    
            try:
                temp =  self.h5.__getitem__(self.cycle_header+str(self.cycle[0])).__getitem__('SE_DATASET').dtype.__str__().split(',')
            except ValueError:
                temp =  self.h5.__getitem__(self.cycle_header+str(self.cycle[0])).dtype.__str__().split(',')
        
        else:        
            self.cycle_header = 'cycle-'
            for te in temp:
                try:
                    self.cycle.append(str(te.split('-')[1]))

                    try:
                        self.age.append(self.h5[te].attrs.get("age", None)[0])
                    except TypeError:
                        self.age.append(self.h5[te].attrs.get("age", None))
                except IndexError:
            
                    obj =  self.h5[te].__iter__()
            
                    if str(te).count('A'):
                        holder = []
                        for ob in obj:
                            holder.append(ob[0])
                        self.Table.append(holder)
                        self.A.append(holder)
                    elif str(te).count('Z'):
                        holder = []
                        for ob in obj:
                            holder.append(ob[0])
                        self.Table.append(holder)
                        self.Z.append(holder)    
                    else:
                        holder = []
                        for ob in obj:
                            holder.append(ob[0])
                        self.Table.append(holder)
                        self.isomeric_state.append(holder)                        
                        
            self.cycle.sort()    
        
        # This is kind of stupid, but I have not found a way to access this information directly.
        
            try:
                temp =  self.h5.__getitem__(self.cycle_header+str(self.cycle[0])).__getitem__('SE_DATASET').dtype.__str__().split(',')
            except ValueError:
                temp =  self.h5.__getitem__(self.cycle_header+str(self.cycle[0])).dtype.__str__().split(',')

        
        for tem in temp:
            if tem.count('<') ==0:
                try:
                    self.dcol.append(tem.split('\'')[1])
                except IndexError:
                    None

        attrs = self.h5.attrs
        for at in attrs:
            self.hattr.append(at)
        self.cattr = self.h5[self.cycle_header+str(self.cycle[0])].attrs.keys()
        
        table = []
        grp = self.h5[self.cycle_header+str(self.cycle[0])]
        for gr in grp:
            try:
                table.append(float(gr[0]))
            except ValueError:
                None
        
        self.h5.close()
        return None


        
        
