# distutils: language = c++
# distutils: sources = Unit.cpp Measurement.cpp Source.cpp SourceCatalog.cpp helperFunctions.cpp

#from cython.operator cimport dereference as deref, preincrement as inc
#from libcpp.string cimport string
#from libcpp cimport bool
#from libcpp.map cimport map
#cimport cython
from catalog cimport *
from cython.operator cimport dereference as deref, preincrement as inc

# s="""#define UNIT_NONE    0
# #define UNIT_JY      1
# #define UNIT_MJY     2"""
# tmp=[a.split()[1] for a in s.split("\n") if len(a)>0]
# print str("\n").join([t.lower()+"="+t for t in tmp])
unit_std=UNIT_STD
unit_exp=UNIT_EXP
unit_none=UNIT_NONE
unit_jy=UNIT_JY
unit_mjy=UNIT_MJY
unit_deg=UNIT_DEG
unit_arcmin=UNIT_ARCMIN
unit_arcsec=UNIT_ARCSEC
unit_mas=UNIT_MAS
unit_g=UNIT_G
unit_pc=UNIT_PC
unit_kpc=UNIT_KPC
unit_mpc=UNIT_MPC
unit_au=UNIT_AU
unit_ly=UNIT_LY
unit_min=UNIT_MIN
unit_h=UNIT_H
unit_a=UNIT_A
unit_erg=UNIT_ERG
unit_dyn=UNIT_DYN
unit_c=UNIT_C
unit_e=UNIT_E


measurement_default=MEASUREMENT_DEFAULT
measurement_compact=MEASUREMENT_COMPACT
measurement_full=MEASUREMENT_FULL
measurement_unit=MEASUREMENT_UNIT

cdef class PyUnit:
    #cdef Unit *thisptr      # hold a C++ instance which we're wrapping
    def __cinit__(self,u=None):
        if u is None:
            self.thisptr = new Unit()
        elif type(u)==str:
            self.thisptr = new Unit()
            self.thisptr.set(<string>u)
        else:
            self.thisptr = new Unit(deref((<PyUnit>u).thisptr))
    def __dealloc__(self):
        del self.thisptr
    def set(self,s):
        self.thisptr.set(<string>s)
    def getPrefix(self):
        return self.thisptr.getPrefix()
    def invert(self):
        self.thisptr.invert()
    def clear(self):
        self.thisptr.clear()
    def isEmpty(self):
        return self.thisptr.isEmpty()
    def isDefined(self):
        return self.thisptr.isDefined()
    def asString(self,mode=unit_std):
        return self.thisptr.printString(<const unsigned int>mode)
    def copy(self):
        return PyUnit(self)
    def __mul__(self,other not None):
        result=PyUnit(self)
        result.thisptr.mult_equal(deref(PyUnit(other).thisptr))
        return result
    def __richcmp__(PyUnit self,other not None,int op):
        if op==2:
            return self.thisptr.isequal(deref(PyUnit(other).thisptr))
        if op==3:
            return not self.thisptr.isequal(deref(PyUnit(other).thisptr))
        else:
            print "not implemented"






#cdef Measurement[double] Measurement_double

cdef class PyMeasurement:
    #cdef Measurement[double] *thisptr      # hold a C++ instance which we're wrapping
    def __cinit__(self,m=None): #,Measurement[double] m=None
        if m is None:
            self.thisptr = new Measurement[double]()
        else:
            self.thisptr = new Measurement[double](deref((<PyMeasurement>m).thisptr))
    def __dealloc__(self):
        del self.thisptr
    def set(self,str newName,double newValue,double newUncertainty,newUnit):
        if type(newUnit)==int:
            self.thisptr.setInt(newName,newValue,newUncertainty,<unsigned int>newUnit)
        elif type(newUnit)==str:
            self.thisptr.setStr(newName,newValue,newUncertainty,<string> newUnit)
        elif type(newUnit)==PyUnit:
            self.thisptr.setUnit(newName,newValue,newUncertainty,deref((<PyUnit>newUnit).thisptr))
        else:
            print "not implemented"
    def setName(self,string newName):
        self.thisptr.setName(newName)
    def setValue(self,double newValue):
        self.thisptr.setValue(newValue)
    def setUncertainty(self,double newValue):
        self.thisptr.setUncertainty(newValue)
    def setUnit(self,newUnit):
        if type(newUnit)==str:
            self.thisptr.setUnitStr(<string> newUnit)
        elif type(newUnit)==PyUnit:
            self.thisptr.setUnitUnit(deref((<PyUnit>newUnit).thisptr))
    def getName(self):
        return self.thisptr.getName()
    def getValue(self):
        return self.thisptr.getValue()
    def getUncertainty(self):
        return self.thisptr.getUncertainty()
    def getUnit(self):
        pu=PyUnit()
        pu.thisptr=new Unit(self.thisptr.getUnit())
        return pu
    def clear(self):
        self.thisptr.clear()
    def asString(self,unsigned int mode=measurement_default,int decimals=-1,bool scientific=False):
        return self.thisptr.printString(mode,decimals,scientific)
    def convert(self,int mode):
        cdef double newValue=0.
        cdef double newUncertainty=0.
        self.thisptr.convert(newValue,newUncertainty,mode)
        return newValue,newUncertainty
    def invert(self):
        self.thisptr.invert()
    def __richcmp__(PyMeasurement self,other not None,int op):
        if op==0:
            return self.thisptr.issmaller(deref(PyMeasurement(other).thisptr))
        elif op==1:
            return self.thisptr.issmallereq(deref(PyMeasurement(other).thisptr))
        elif op==2:
            return self.thisptr.isequal(deref(PyMeasurement(other).thisptr))
        elif op==3:
            return not self.thisptr.isequal(deref(PyMeasurement(other).thisptr))
        elif op==4:
            return self.thisptr.islarger(deref(PyMeasurement(other).thisptr))
        elif op==5:
            return self.thisptr.islargereq(deref(PyMeasurement(other).thisptr))
        else:
            print "not implemented"
    def copy(self):
        return PyMeasurement(self)
    def __mul__(self,other not None):
        result=PyMeasurement(self)
        result.thisptr.mult_equal(deref(PyMeasurement(other).thisptr))
        return result
    def __rdiv__(self,other not None):
        result=PyMeasurement(self)
        result.thisptr.div_equal(deref(PyMeasurement(other).thisptr))
        return result
    def __add__(self,other not None):
        result=PyMeasurement(self)
        result.thisptr.plus_equal(deref(PyMeasurement(other).thisptr))
        return result
    def __sub__(self,other not None):
        result=PyMeasurement(self)
        result.thisptr.minus_equal(deref(PyMeasurement(other).thisptr))
        return result

    
    


cdef class PySource:
    #cdef Source *thisptr      # hold a C++ instance which we're wrapping
    def __cinit__(self,s=None):
        if s is None:
            self.thisptr = new Source()
        else:
            self.thisptr = new Source(deref((<PySource>s).thisptr))
    def __dealloc__(self):
        del self.thisptr
    def isDefined(self):
        return self.thisptr.isDefined()
    def parameterDefined(self,s):
        return self.thisptr.parameterDefined(<string>s)
    def setParameter(self,name,value=None,uncertainty=None,unit=None):
        if type(name)==str:
            m=PyMeasurement()
            m.set(name,<double>value,<double>uncertainty,<string>unit)
        elif type(name)==PyMeasurement:
            m=name
        self.thisptr.setParameter(deref(PyMeasurement(m).thisptr))
    def getParameter(self,s):
        pm=PyMeasurement()
        pm.thisptr=new Measurement[double](self.thisptr.getParameterMeasurement(<string>s))
        return pm
    def setSourceID(self,unsigned long sid):
        self.thisptr.setSourceID(sid)
    def getSourceID(self):
        return self.thisptr.getSourceID()
    def setSourceName(self,s):
        self.thisptr.setSourceName(<string>s)
    def getSourceName(self):
        return self.thisptr.getSourceName()
    def getParametersDict(self):
        pdict={}
        cdef map[string,Measurement[double]].iterator mapiter
        cdef map[string,Measurement[double]] pmap
        pmap=self.thisptr.getParameters()
        mapiter=pmap.begin()
        while mapiter != pmap.end():
            pm=PyMeasurement()
            pm.thisptr=new Measurement[double](deref(mapiter).second)
            pdict[deref(mapiter).first]=pm
            inc(mapiter)
        return pdict
    def setParametersDict(self,dictionary):
        self.clear()
        for key in dictionary.keys():
            self.setParameter(dictionary[key])
    def clear(self):
        self.thisptr.clear()
    def copy(self):
        return PySource(self)
   



cdef class PySourceCatalog:
    #cdef SourceCatalog *thisptr      # hold a C++ instance which we're wrapping
    def __cinit__(self,sc=None): #,Measurement[double] m=None
        if sc is None:
            self.thisptr = new SourceCatalog()
        else:
            self.thisptr = new SourceCatalog(deref((<PySourceCatalog>sc).thisptr))
    def __dealloc__(self):
        del self.thisptr
    def readDuchampFile(self,filename):
        self.thisptr.readDuchampFile(<string>filename)
    def insert(self,PySource s):
        self.thisptr.insert(deref(PySource(s).thisptr))
    def getSourcesDict(self):
        pdict={}
        cdef map[unsigned long,Source].iterator mapiter
        cdef map[unsigned long,Source] pmap
        pmap=self.thisptr.getSources()
        mapiter=pmap.begin()
        while mapiter != pmap.end():
            pm=PySource()
            pm.thisptr=new Source(deref(mapiter).second)
            pdict[deref(mapiter).first]=pm
            inc(mapiter)
        return pdict
    def setSourcesDict(self,dictionary):
        self.clear()
        for key in dictionary.keys():
            self.insert(dictionary[key])
    def clear(self):
        self.thisptr.clear()
    def copy(self):
        return PySourceCatalog(self)
