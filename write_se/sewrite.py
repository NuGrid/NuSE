'''This module provides is with an easy way to write se files that 
can be post-processed with mppnp

example useage:
import sewrite as sw
cyc = 10 # I want to write some data here
f=sw.startfile('file_name')
f.write_cattr(cyc,cattr_name='mass',cattr_val=2.5)
f.write_hattr(hattr_name='mass',hattr_val=2.5)
f.write_dcol(cyc,dcol_name='rho',dcol_val=mass_array)

ToDo:

1) Write tool
-not same cycle twice
-input checking 
	test for same length of data columns


2) Testing
 
- with mppnp



'''
import numpy
from write_se import se
import tables
#from itertools import izip as izip

class startfile():
    output_file = ""

    def __init__(self, file_name):
        self.output_file = file_name

    def write_hattr(self, hattr_name, hattr_val):
        if len(hattr_name) != len(hattr_val):
            print ("Error: Write_hattr, list of names and values do not match.")
        else:
            for i in range(len(hattr_name)):
                se.writeattr(self.output_file,-1,hattr_name[i],hattr_val[i])

    def write_table(self, table_name, table_val):
        if not (hasattr(table_val,"shape") and hasattr(table_val,"dtype")):
            table_val = numpy.array(table_val)

        try:
            sefile = tables.openFile(self.output_file, mode = "r+")
        except IOError:
            sefile = tables.openFile(self.output_file, mode = "a")
            sefile.setNodeAttr("/", "SE_version", "1.2")

        table_val_type = tables.Float64Col(shape=table_val.shape[1:]) #table_val_type is float by default
        if (str(table_val.dtype) == "int32") or (str(table_val.dtype) == "int64"):
            table_val_type = tables.Int64Col(shape=table_val.shape[1:])

        description = {"data": table_val_type}
        table = sefile.createTable('/', table_name, description,
                                filters=tables.Filters(complevel = 6))

        row = table.row
        for i in range(len(table_val)):
            row["data"] = table_val[i]
            row.append()
        sefile.close()

    def write_cattr(self, cyc, cattr_name, cattr_val):
        cattr_list = []
        if len(cattr_name) != len(cattr_val):
            print ("Error: Write_cattr, list of names and value list do not match.")
        else:
            for name, val in zip(cattr_name, cattr_val):
                if name == "age":
                    val = numpy.array([val])#h5T.py h5File() requires that age
                                            #is a numpy array
                se.writeattr(self.output_file, int(cyc), name, val)

    def write_dcol(self, cyc, dcol_name, dcol_val):
        dcol_list = []
        if len(dcol_name) != len(dcol_val):
            print ("Error: Write_dcol, list of names and value list do not match.")
        else:
            for name, val_list in zip(dcol_name, dcol_val):
                if not (hasattr(val_list,"shape") and hasattr(val_list,"dtype")):
                    val_list = numpy.array(val_list)

                val_type = tables.Float64Col(shape=val_list.shape[1:]) #val_type is float by default
                if (str(val_list.dtype) == "int32") or (str(val_list.dtype) == "int64"):
                    val_type = tables.Int64Col(shape=val_list.shape[1:])

                dcol_list.append((name,val_type,val_list))#list of tuples
            se.write(self.output_file, int(cyc), dcol_list)
