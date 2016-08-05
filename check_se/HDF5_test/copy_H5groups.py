import h5py
f = h5py.File('M3.00Z0.020.0079001.se.h5')
f.copy('cycle0000079999','cycle0000080000')
