cimport numpy as np
from cython.operator cimport dereference as deref, preincrement as inc
from libcpp.string cimport string
from libcpp cimport bool
from libcpp.map cimport map
cimport cython


cdef extern from "Unit.h":
    cpdef int UNIT_STD
    cpdef int UNIT_EXP
    cpdef int UNIT_NONE
    cpdef int UNIT_JY
    cpdef int UNIT_MJY
    cpdef int UNIT_DEG
    cpdef int UNIT_ARCMIN
    cpdef int UNIT_ARCSEC
    cpdef int UNIT_MAS
    cpdef int UNIT_G
    cpdef int UNIT_PC
    cpdef int UNIT_KPC
    cpdef int UNIT_MPC
    cpdef int UNIT_AU
    cpdef int UNIT_LY
    cpdef int UNIT_MIN
    cpdef int UNIT_H
    cpdef int UNIT_A
    cpdef int UNIT_ERG
    cpdef int UNIT_DYN
    cpdef int UNIT_C
    cpdef int UNIT_E
    cdef cppclass Unit:
        Unit() except +
        Unit(Unit &) except +

        int getPrefix()

        int set(const string value)
        string printString(const unsigned int mode)

        void invert()

        void clear()
        bool isEmpty()
        bool isDefined()

        Unit & equal "operator=" (Unit &)
        bool isequal "operator==" (Unit &)
        Unit & mult_equal "operator*=" (Unit &)


cdef class PyUnit:
    cdef Unit *thisptr  # holds the C++ instance being wrapped


cdef extern from "Measurement.h":
    cpdef int MEASUREMENT_DEFAULT
    cpdef int MEASUREMENT_COMPACT
    cpdef int MEASUREMENT_FULL
    cpdef int MEASUREMENT_UNIT
    cdef cppclass Measurement[T]:
        Measurement() except +
        Measurement(Measurement[T] &) except +

        void clear()

        int setInt "set" (const string newName, T newValue, T newUncertainty, const int mode)
        int setStr "set" (const string newName, T newValue, T newUncertainty, const string newUnit)
        void setUnit "set" (const string newName, T newValue, T newUncertainty, const Unit newUnit)

        void setName(const string newName)
        void setValue(T newValue)
        void setUncertainty(T newValue)
        void setUnitUnit "setUnit" (const Unit newUnit)
        int setUnitStr "setUnit" (const string newUnitStr)

        string printString "print" (unsigned int mode, int decimals, bool scientific)

        string getName()
        T getValue()
        T getUncertainty()
        Unit getUnit()

        int invert()

        int convert(T &newValue, T &newUncertainty, unsigned int mode)

        Measurement & equal "operator=" (Measurement &)
        bool isequal "operator==" (Measurement &)
        bool islarger "operator>" (Measurement &)
        bool islargereq "operator>=" (Measurement &)
        bool issmaller "operator<" (Measurement &)
        bool issmallereq "operator<=" (Measurement &)
        Measurement & mult_equal "operator*=" (Measurement &)
        Measurement & div_equal "operator/=" (Measurement &)
        Measurement & plus_equal "operator+=" (Measurement &)
        Measurement & minus_equal "operator-=" (Measurement &)




cdef class PyMeasurement:
    cdef Measurement[double] *thisptr  # holds the C++ instance being wrapped


cdef extern from "Source.h":
    cdef cppclass Source:
        Source() except +
        Source(const Source &source) except +

        bool isDefined()
        bool parameterDefined(const string &name)
        unsigned int findParameter(const string &name)

        int setParameter(const string parameter, double value, double uncertainty, string &unit)
        int setParameter(const string parameter, double value, double uncertainty)
        int setParameter(Measurement[double] &measurement)
        Measurement[double] & getParameterMeasurement(const string &parameter)

        int setSourceID(unsigned long sid)
        unsigned long getSourceID()

        int setSourceName(const string name)
        string   getSourceName()

        map[string,Measurement[double]] getParameters()
        void clear()

cdef class PySource:
    cdef Source *thisptr  # holds the C++ instance being wrapped

cdef extern from "SourceCatalog.h":
    cdef cppclass SourceCatalog:
        SourceCatalog() except +
        SourceCatalog(const SourceCatalog &sourceCatalog) except +

        int insert(Source &source)
        int update(unsigned long sourceID, Source &source)
        map[unsigned long,Source] getSources()
        void clear()

        int readDuchampFile(const string &filename)

cdef class PySourceCatalog:
    cdef SourceCatalog *thisptr  # holds the C++ instance being wrapped

cdef extern from "ModuleParametrisation.h":
    cdef cppclass ModuleParametrisation:
        ModuleParametrisation() except +

        int run(
            float *d,
            short *m,
            long dx,
            long dy,
            long dz,
            SourceCatalog &initCatalog
            )

        SourceCatalog getCatalog()
        void setFlags(bool doMO, bool doBF)

cdef class PyModuleParametrisation:
    cdef ModuleParametrisation *thisptr  # holds the C++ instance being wrapped
