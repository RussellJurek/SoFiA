from catalog cimport *
cimport numpy as np
from cython.operator cimport dereference as deref, preincrement as inc
from libcpp.string cimport string
#from cpython cimport bool # don't use this, doesn't work!
from libcpp cimport bool
from libcpp.map cimport map
cimport cython

cdef extern from "ModuleParametrisation.h":
    cdef cppclass ModuleParametrisation:
        ModuleParametrisation() except +
        
        int run(float *d, short *m, long dx, long dy, long dz, map[string,string] &fitsHeader, SourceCatalog &initCatalog)
        SourceCatalog getCatalog()

cdef class PyModuleParametrisation:
    cdef ModuleParametrisation *thisptr      # hold a C++ instance which we're wrapping
