"""a wrapper around PyTables for reading and writing SE files

SE files are just HDF5 files with particular conventions to support
simple reading and writing of data from 1-D stellar evolution and
nucleosynthesis codes.  This module provides a Python wrapper for
PyTables that implements the conventions and restrictions expressed in
the SE library.

Because I wrote this when I was just learning Python, the methods in
this module are as simple as possible (and also nearly maximally
inefficient).  Most methods expect a filename string as an argument,
and they save no state about the SE file between invocations; in other
words, the SE file is opened from scratch in each method and closed at
the end of each operation.

Several people have realized enormous speed gains by using PyTables
(or h5py) to open files manually and operate on them directly.  In an
ideal world, I would rewrite this module with the knowledge I now have
about functionality and conventions of Python, PyTables, h5py, Numpy,
and other packages.  In reality, both because a couple people are
already using this and because I probably should spend my time in
other ways, this module will remain horribly inefficient for now.
"""

import tables
import warnings

warnings.simplefilter('ignore', tables.exceptions.NaturalNameWarning)

def getversion(sefile):

    """Return the SE version string associated with the sefile object

    sefile must be a tables File object.  In other words, this method
    is really intended for internal use only.
    """

    try:
        attr = sefile.get_node_attr("/", "SE_version")
    except AttributeError:
        attr = "1.0"
    
    return attr

def getcycles(filename):

    """Return a list of the names of all cycles in filename

    The actual HDF5 construct corresponding to a "cycle" in SE-speak
    has changed over time.  SE v1.0 files used HDF5 datasets (aka
    "Tables" in PyTables) to represent cycles.  Starting with SE v1.1,
    cycles have been HDF5 groups containing datasets named
    "SE_DATASET"; this allowed us to add additional datasets under the
    same group to represent "array attributes".

    In fact, the desire to support array attributes attached to cycles
    is what prompted us to introduce SE v1.1, and version numbering in
    general.
    """

    sefile = tables.open_file(filename, mode = "r")
    ver = getversion(sefile)

    if ver == "1.0":
        classname = "Table"
    else:
        classname = "Group"

    cyclenames = [x._v_name for x in sefile.listNodes("/", classname)]

    sefile.close()

    return cyclenames

def read(filename, cyclenumber, *vecs):

    """read(filename, cyclenumber[, col1, ...])

    Read some subset of the columns (structure members) from a SE
    file, and return them as a list of numpy arrays.

    This method should probably take a list of column names, rather
    than a variable number of additional arguments.  I guess the
    difference isn't that big.
    """

    sefile = tables.open_file(filename, mode = "r")
    ver = getversion(sefile)

    if ver == "1.0":
        node = "/"
        name = namefromcycle(cyclenumber, ver)
    else:
        node = sefile.get_node("/", namefromcycle(cyclenumber, ver))
        name = "SE_DATASET"
        
    cycle = sefile.get_node(node, name)

    data = [cycle.col(x) for x in vecs]

    sefile.close()

    return data

def write(filename, cyclenumber, vecs):

    """write(filename, cyclenumber, [ (name, type, value), ...])

    Write some records to a SE file.  Arrays to write should be
    specified as (name, type, value) triplets in a list, where type
    must be a tables type specifier like Float64Col().
    """

    try:
        sefile = tables.open_file(filename, mode = "r+")
    except IOError:
        writeattr(filename, -1, "SE_version", "1.2")
        sefile = tables.open_file(filename, mode = "a")

    ver = getversion(sefile)

    if ver == "1.0":
        node = "/"
        name = namefromcycle(cyclenumber, ver)
    else:
        node = sefile.create_group("/", namefromcycle(cyclenumber, ver))
        name = "SE_DATASET"

    description = dict((x[0], x[1]) for x in vecs)
    table = sefile.create_table(node, name, description,
                               filters=tables.Filters(complevel = 6))

    row = table.row
    for i in range(min(len(x[2]) for x in vecs)):
        for x in vecs:
            row[x[0]] = x[2][i]
        row.append()

    sefile.close()

def readattr(filename, cyclenumber, name):

    """Read an attribute from an SE cycle.

    Returns the attribute value.  Pass -1 as cyclenumber to read
    global (i.e., file-level) attributes.
    """

    sefile = tables.open_file(filename, mode = "r")

    ver = getversion(sefile)

    if cyclenumber == -1:
        attr = sefile.get_node_attr("/", name)
    else:
        attr = sefile.get_node_attr("/", name, namefromcycle(cyclenumber, ver))

    sefile.close()

    return attr

def writeattr(filename, cyclenumber, name, value):

    """Attach an attribute to an SE cycle.

    Pass -1 as cyclenumber to write global (i.e., file-level)
    attributes.  Note that, unlike the main SE library, this
    implementation requires the cycle to exist before attaching
    attributes to it.  TODO: fix that.
    """

    try:
        sefile = tables.open_file(filename, mode = "r+")
    except IOError:
        sefile = tables.open_file(filename, mode = "a")
        sefile.set_node_attr("/", "SE_version", "1.2")

    ver = getversion(sefile)

    if cyclenumber == -1:
        sefile.set_node_attr("/", name, value)
    else:
        sefile.set_node_attr("/", name, value, namefromcycle(cyclenumber, ver))

    sefile.close()

def readarrayattr(filename, cyclenumber, tablename, colname='data'):

    """Read an array attribute from an SE cycle.

    Pass -1 as cyclenumber to read global (i.e., file-level)
    attributes.

    This command can also be used to read a single column of a table.

    (Added March 2010: I'm not sure why this method was designed to
    have the ability to read a single column from any data table.
    It's true that, underneath, SE "array attributes" are just tables
    with a table name other than "SE_DATASET".  In the SE library,
    "cycle" reads and writes and "array attribute" reads and writes
    are implemented with the same functions, and the same thing could
    be done here.  But exposing the colname attribute in this method
    requires users to know that array attributes store their data in a
    column named "data", which is completely an implementation detail.

    On the other hand, there are many things that I would change about
    this module now, with the benefit of hindsight, if people weren't
    already using it.  So it stays.  But I can default it to "data"!
    Thanks Python.)
    """

    sefile = tables.open_file(filename, mode = "r")

    ver = getversion(sefile)

    if cyclenumber == -1:
        table = sefile.get_node("/", tablename)
    else:
        node = sefile.get_node("/", namefromcycle(cyclenumber, ver))
        table = sefile.get_node(node, tablename)
        
    data = table.col(colname)

    sefile.close()

    return data


def writearrayattr(filename, cyclenumber, name, type, value):

    """Attach an array attribute to an SE cycle.

    Pass -1 as cyclenumber to write global (i.e., file-level)
    attributes.  type must be a tables type specifier like
    Float64Col().
    """

    try:
        sefile = tables.open_file(filename, mode = "r+")
    except IOError:
        sefile = tables.open_file(filename, mode = "a")
        sefile.set_node_attr("/", "SE_version", "1.2")

    ver = getversion(sefile)

    if ver == "1.0":
        raise tables.NodeError("v1.0 files don't support array attributes.")

    if cyclenumber == -1:
        node = "/"
    else:
        node = "/" + namefromcycle(cyclenumber, ver)

    description = {'data': type}
    table = sefile.create_table(node, name, description,
                               filters=tables.Filters(complevel = 6))

    row = table.row
    for x in value:
        row['data'] = x
        row.append()

    sefile.close()

def namefromcycle(cyclenumber, version):
    """Return the dataset name corresponding to the given cycle number.

    SE 1.0 and 1.1 used dashes; SE 1.2 uses zero-padded integers with
    10-digit field width.
    """
    if version == b'1.2':
        return "cycle%010d" % cyclenumber
    else:
        return "cycle-" + str(cyclenumber)
