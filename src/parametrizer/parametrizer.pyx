# distutils: language = c++
# distutils: sources = ModuleParametrisation.cpp Unit.cpp Measurement.cpp Source.cpp SourceCatalog.cpp helperFunctions.cpp MaskOptimization.cpp BusyFit.cpp DataCube.cpp MetaData.cpp Parametrization.cpp WorldCoordinateSystem.cpp


from parametrizer cimport *
from catalog cimport *
from cython.operator cimport dereference as deref, preincrement as inc


cdef class PyModuleParametrisation:
    #cdef ModuleParametrisation *thisptr      # hold a C++ instance which we're wrapping
    def __cinit__(self,sc=None): #,Measurement[double] m=None
        self.thisptr = new ModuleParametrisation()
    def __dealloc__(self):
        del self.thisptr
    def run(self,np.ndarray datacube, np.ndarray maskcube, PySourceCatalog initCatalog, header):
        cdef long dz=datacube.shape[0]
        cdef long dy=datacube.shape[1]
        cdef long dx=datacube.shape[2]
        cdef map[string,string] headermap
        for keys in header.keys():
            headermap[<string>keys]=<string>(header[keys])
        initCatalogPtr = new SourceCatalog(deref((<PySourceCatalog>initCatalog).thisptr))
        self.thisptr.run(<float*>datacube.data, <short*>maskcube.data, dx, dy, dz, headermap, deref(initCatalogPtr))
    def getCatalog(self):
        c=PySourceCatalog()
        c.thisptr=new SourceCatalog(self.thisptr.getCatalog())
        return c