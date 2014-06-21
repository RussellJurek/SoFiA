# distutils: language = c++
# distutils: sources = ModuleParametrisation.cpp Unit.cpp Measurement.cpp Source.cpp SourceCatalog.cpp helperFunctions.cpp MaskOptimization.cpp BusyFit.cpp DataCube.cpp MetaData.cpp Parametrization.cpp WorldCoordinateSystem.cpp


from cparametrizer cimport *
from cython.operator cimport dereference as deref, preincrement as inc
from cpython cimport bool as python_bool

unit_std = UNIT_STD
unit_exp = UNIT_EXP
unit_none = UNIT_NONE
unit_jy = UNIT_JY
unit_mjy = UNIT_MJY
unit_deg = UNIT_DEG
unit_arcmin = UNIT_ARCMIN
unit_arcsec = UNIT_ARCSEC
unit_mas = UNIT_MAS
unit_g = UNIT_G
unit_pc = UNIT_PC
unit_kpc = UNIT_KPC
unit_mpc = UNIT_MPC
unit_au = UNIT_AU
unit_ly = UNIT_LY
unit_min = UNIT_MIN
unit_h = UNIT_H
unit_a = UNIT_A
unit_erg = UNIT_ERG
unit_dyn = UNIT_DYN
unit_c = UNIT_C
unit_e = UNIT_E

unit_list = [
    unit_std, unit_exp, unit_none, unit_jy, unit_mjy, unit_g,
    unit_deg, unit_arcmin, unit_arcsec, unit_mas,
    unit_pc, unit_kpc, unit_mpc, unit_au, unit_ly,
    unit_min, unit_h, unit_a, unit_erg, unit_dyn, unit_c, unit_e
    ]

unit_names = [
    'unit_std', 'unit_exp', 'unit_none', 'unit_jy', 'unit_mjy', 'unit_g',
    'unit_deg', 'unit_arcmin', 'unit_arcsec', 'unit_mas',
    'unit_pc', 'unit_kpc', 'unit_mpc', 'unit_au', 'unit_ly',
    'unit_min', 'unit_h', 'unit_a', 'unit_erg', 'unit_dyn', 'unit_c', 'unit_e'
    ]


measurement_default = MEASUREMENT_DEFAULT
measurement_compact = MEASUREMENT_COMPACT
measurement_full = MEASUREMENT_FULL
measurement_unit = MEASUREMENT_UNIT

measurement_list = [
    measurement_default, measurement_compact, measurement_full, measurement_unit
    ]

measurement_names = [
    'measurement_default', 'measurement_compact', 'measurement_full', 'measurement_unit'
    ]

cdef class PyUnit:
    """Wrapper around (C++) Unit class"""

    def __cinit__(self, u=None):
        """Constructor PyUnit(u=None)

        u can be None (empty Unit is returned), a string or another PyUnit"""

        if u is None:
            self.thisptr = new Unit()
        elif isinstance(u, str):
            self.thisptr = new Unit()
            self.thisptr.set(<string> u)
        elif isinstance(u, unicode):
            u = str(u)  # might raise UnicodeEncodeError
            self.thisptr = new Unit()
            self.thisptr.set(<string> u)
        elif isinstance(u, PyUnit):
            self.thisptr = new Unit(deref((<PyUnit> u).thisptr))
        else:
            raise TypeError('PyUnit: Incompatible input type')

    def __dealloc__(self):
        del self.thisptr

    def set(self, s):
        if not (isinstance(s, str) or isinstance(s, unicode)):
            raise TypeError('PyUnit: Incompatible input type (unit name must be str())')
        if isinstance(s, unicode):
            s = str(s)   # might raise UnicodeEncodeError
        res = self.thisptr.set(<string> s)
        if res == 1:
            raise ValueError('Units error')

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

    def asString(self, const unsigned int mode=unit_std):
        # what is mode for? doesn't seem to work
        return self.thisptr.printString(<const unsigned int> mode)

    def copy(self):
        return PyUnit(self)

    def __mul__(self, PyUnit other not None):
        assert isinstance(other, PyUnit), 'operation only allowed for PyUnit type'

        result = PyUnit(self)
        result.thisptr.mult_equal(deref(PyUnit(other).thisptr))
        return result

    def __richcmp__(PyUnit self, PyUnit other not None, int op):
        assert isinstance(other, PyUnit), 'operation only allowed for PyUnit type'

        if op == 2:
            return self.thisptr.isequal(deref(PyUnit(other).thisptr))
        if op == 3:
            return not self.thisptr.isequal(deref(PyUnit(other).thisptr))
        else:
            raise NotImplementedError('PyUnit: requested operator not implemented')


cdef class PyMeasurement:
    """contains a var-name with value, uncertainty and a unit"""

    def __cinit__(self, m=None):
        if m is None:
            self.thisptr = new Measurement[double]()
        elif isinstance(m, PyMeasurement):
            self.thisptr = new Measurement[double](deref((<PyMeasurement> m).thisptr))
        else:
            raise TypeError('PyMeasurement: Incompatible input type')

    def __dealloc__(self):
        del self.thisptr

    def set(self, newName, double newValue, double newUncertainty, newUnit):
        """If newUnit is an int/enum, the values are also multplied by a conversion factor

        possible ints are:
        unit_std, unit_exp, unit_none, unit_jy, unit_mjy, unit_g,
        unit_deg, unit_arcmin, unit_arcsec, unit_mas,
        unit_pc, unit_kpc, unit_mpc, unit_au, unit_ly,
        unit_min, unit_h, unit_a, unit_erg, unit_dyn, unit_c, unit_e"""
        # NOTE, it would be great to use proper enums here, there is a backport
        # of the new python3.4 enum, called 'enum34'
        # should we add this to the requirements?
        if not (isinstance(newName, str) or isinstance(newName, unicode)):
            raise TypeError('PyMeasurement: Incompatible input type (name must be str())')
        if isinstance(newName, unicode):
            newName = str(newName)   # might raise UnicodeEncodeError

        if isinstance(newUnit, int):
            if not newUnit in unit_list:
                raise TypeError(
                    '\n'.join(
                        ['newUnit must be one of'] +
                        unit_names +
                        ['(if an integer/enum is used)']
                        )
                    )
            res = self.thisptr.setInt(
                newName, newValue, newUncertainty, <unsigned int> newUnit
                )
            if res == 1:
                raise ValueError('Units error')
        else:
            newUnit = PyUnit(newUnit)
            self.thisptr.setUnit(
                newName, newValue,newUncertainty, deref((<PyUnit> newUnit).thisptr)
                )


    def setName(self, newName):
        if not (isinstance(newName, str) or isinstance(newName, unicode)):
            raise TypeError('PyMeasurement: Incompatible input type (name must be str())')
        if isinstance(newName, unicode):
            newName = str(newName)   # might raise UnicodeEncodeError
        self.thisptr.setName(<string> newName)

    def setValue(self, double newValue):
        self.thisptr.setValue(newValue)

    def setUncertainty(self, double newUncertainty):
        self.thisptr.setUncertainty(newUncertainty)

    def setUnit(self, newUnit):
        if not isinstance(newUnit, PyUnit):
            newUnit = PyUnit(newUnit)
        self.thisptr.setUnitUnit(deref((<PyUnit> newUnit).thisptr))

    def getName(self):
        return self.thisptr.getName()

    def getValue(self):
        return self.thisptr.getValue()

    def getUncertainty(self):
        return self.thisptr.getUncertainty()

    def getUnit(self):
        pu = PyUnit()
        pu.thisptr = new Unit(self.thisptr.getUnit())
        return pu

    def clear(self):
        self.thisptr.clear()

    def asString(
            self,
            unsigned int mode=measurement_default,
            int decimals=-1,
            python_bool scientific=False
            ):
        if not mode in measurement_list:
            raise TypeError(
                '\n'.join(
                    ['mode must be one of'] +
                    measurement_names
                    )
                )

        return self.thisptr.printString(<unsigned int> mode, decimals, <bool> scientific)

    def convert(self, int unitenum):
        if not unitenum in unit_list:
            raise TypeError(
                '\n'.join(
                    ['newUnit must be one of'] +
                    unit_names +
                    ['(if an integer/enum is used)']
                    )
                )
        cdef double newValue = 0.
        cdef double newUncertainty = 0.
        res = self.thisptr.convert(newValue, newUncertainty, unitenum)
        if res == 1:
            raise ValueError('Units Incompatible')
        return newValue, newUncertainty

    def invert(self):
        res = self.thisptr.invert()
        if res == 1:
            raise ZeroDivisionError('float division by zero')

    def __richcmp__(PyMeasurement self, PyMeasurement other not None, int op):
        assert isinstance(other, PyMeasurement), 'operation only allowed for PyMeasurement type'

        # for some reason, the C++ implementation doesn't check for units being equal
        _unit1 = self.getUnit()
        _unit2 = other.getUnit()
        if _unit1 != _unit2:
            raise TypeError(
                'PyMeasurement: Incompatible units <' +
                _unit1.asString() + '> <' + _unit2.asString() + '>'
                )

        if op == 0:
            return self.thisptr.issmaller(deref(PyMeasurement(other).thisptr))
        elif op == 1:
            return self.thisptr.issmallereq(deref(PyMeasurement(other).thisptr))
        elif op == 2:
            return self.thisptr.isequal(deref(PyMeasurement(other).thisptr))
        elif op == 3:
            return not self.thisptr.isequal(deref(PyMeasurement(other).thisptr))
        elif op == 4:
            return self.thisptr.islarger(deref(PyMeasurement(other).thisptr))
        elif op == 5:
            return self.thisptr.islargereq(deref(PyMeasurement(other).thisptr))
        else:
            raise NotImplementedError('PyMeasurement: requested operator not implemented')


    def copy(self):
        return PyMeasurement(self)

    def __mul__(self, PyMeasurement other not None):
        assert isinstance(other, PyMeasurement), 'operation only allowed for PyMeasurement type'

        result = PyMeasurement(self)
        result.thisptr.mult_equal(deref(PyMeasurement(other).thisptr))
        return result

    def __rdiv__(self, PyMeasurement other not None):
        assert isinstance(other, PyMeasurement), 'operation only allowed for PyMeasurement type'

        result = PyMeasurement(self)
        result.thisptr.div_equal(deref(PyMeasurement(other).thisptr))
        return result

    def __add__(self, PyMeasurement other not None):
        assert isinstance(other, PyMeasurement), 'operation only allowed for PyMeasurement type'

        # this is also handled the C++ implementation but we want an exception raised
        _unit1 = self.getUnit()
        _unit2 = other.getUnit()
        if _unit1 != _unit2:
            raise TypeError(
                'PyMeasurement: Incompatible units [' +
                _unit1.asString() + '] [' + _unit2.asString() + ']'
                )

        result = PyMeasurement(self)
        result.thisptr.plus_equal(deref(PyMeasurement(other).thisptr))
        return result

    def __sub__(self, PyMeasurement other not None):
        assert isinstance(other, PyMeasurement), 'operation only allowed for PyMeasurement type'

        # this is also handled the C++ implementation but we want an exception raised
        _unit1 = self.getUnit()
        _unit2 = other.getUnit()
        if _unit1 != _unit2:
            raise TypeError(
                'PyMeasurement: Incompatible units [' +
                _unit1.asString() + '] [' + _unit2.asString() + ']'
                )

        result = PyMeasurement(self)
        result.thisptr.minus_equal(deref(PyMeasurement(other).thisptr))
        return result

    def __neg__(self):
        result = PyMeasurement(self)
        result.setValue(-self.getValue())
        return result


cdef class PySource:

    def __cinit__(self, s=None):
        if s is None:
            self.thisptr = new Source()
        elif isinstance(s, PySource):
            self.thisptr = new Source(deref((<PySource> s).thisptr))
        else:
            raise TypeError('PySource: Incompatible input type')

    def __dealloc__(self):
        del self.thisptr

    def isDefined(self):
        return self.thisptr.isDefined()

    def parameterDefined(self, s):
        assert isinstance(s, str) or isinstance(s, unicode), 'parameter name must be str or unicode'

        return self.thisptr.parameterDefined(<string> s)

    def setParameter(self, name, value=None, uncertainty=None, unit=None):
        if isinstance(name, PyMeasurement):
            m = PyMeasurement(name)
        elif isinstance(name, str) or isinstance(name, unicode):
            if isinstance(name, unicode):
                name = str(name)   # might raise UnicodeEncodeError

            # TODO: should one raise an error, if value/error/unit is none, or
            # silently make it zero? for now, we choose the latter
            if value is None:
                value = 0.0
            if uncertainty is None:
                uncertainty = 0.0
            if unit is None:
                unit = PyUnit(unit)
            m = PyMeasurement()
            m.set(name, value, uncertainty, unit)
        else:
            raise TypeError('PySource: Incompatible input type (name must be str or PyMeasurement)')

        self.thisptr.setParameter(deref(PyMeasurement(m).thisptr))

    def getParameter(self, s):
        assert isinstance(s, str) or isinstance(s, unicode), 'parameter name must be str or unicode'
        if isinstance(s, unicode):
            s = str(s)   # might raise UnicodeEncodeError

        pm = PyMeasurement()
        pm.thisptr = new Measurement[double](self.thisptr.getParameterMeasurement(<string> s))
        if pm.getName() == 'notfound':
            raise KeyError('PySource: Parameter not found')
        return pm

    def setSourceID(self, unsigned long sid):
        """Note: floats will be silently converted (truncated) to Int"""
        self.thisptr.setSourceID(sid)

    def getSourceID(self):
        return self.thisptr.getSourceID()

    def setSourceName(self, s):
        assert isinstance(s, str) or isinstance(s, unicode), 'source name must be str or unicode'
        if isinstance(s, unicode):
            s = str(s)
        self.thisptr.setSourceName(<string> s)

    def getSourceName(self):
        return self.thisptr.getSourceName()

    def getParameters(self):
        pdict = {}
        cdef map[string, Measurement[double]].iterator mapiter
        cdef map[string, Measurement[double]] pmap
        pmap = self.thisptr.getParameters()
        mapiter = pmap.begin()
        while mapiter != pmap.end():
            pm = PyMeasurement()
            pm.thisptr = new Measurement[double](deref(mapiter).second)
            pdict[deref(mapiter).first] = pm
            inc(mapiter)
        return pdict

    def setParameters(self, dictionary):
        assert isinstance(dictionary, dict), 'dictionary is not a python dict'
        assert len(dictionary) > 0, 'dictionary empty'  # is this desired?
        # one could also just let it be cleared

        self.clear()
        for key in dictionary.keys():
            self.setParameter(dictionary[key])

    def clear(self):
        self.thisptr.clear()

    def copy(self):
        return PySource(self)


cdef class PySourceCatalog:

    def __cinit__(self, sc=None):
        if sc is None:
            self.thisptr = new SourceCatalog()
        elif isinstance(sc, PySourceCatalog):
            self.thisptr = new SourceCatalog(deref((<PySourceCatalog> sc).thisptr))
        else:
            raise TypeError('PySource: Incompatible input type')

    def __dealloc__(self):
        del self.thisptr

    def readDuchampFile(self, filename):
        self.thisptr.readDuchampFile(<string> filename)

    def insert(self,PySource s):
        self.thisptr.insert(deref(PySource(s).thisptr))

    def getSources(self):
        pdict = {}
        cdef map[unsigned long, Source].iterator mapiter
        cdef map[unsigned long, Source] pmap
        pmap = self.thisptr.getSources()
        mapiter = pmap.begin()
        while mapiter != pmap.end():
            pm = PySource()
            pm.thisptr = new Source(deref(mapiter).second)
            pdict[deref(mapiter).first] = pm
            inc(mapiter)
        return pdict

    def setSources(self, dictionary):
        assert isinstance(dictionary, dict), 'dictionary is not a python dict'
        assert len(dictionary) > 0, 'dictionary empty'  # is this desired?
        # one could also just let it be cleared

        self.clear()
        for key in dictionary.keys():
            self.insert(dictionary[key])

    def clear(self):
        self.thisptr.clear()

    def copy(self):
        return PySourceCatalog(self)

cdef class PyModuleParametrisation:

    def __cinit__(self):
        self.thisptr = new ModuleParametrisation()

    def __dealloc__(self):
        del self.thisptr

    def run(
            self,
            np.ndarray[float, ndim=3] datacube,
            np.ndarray[short, ndim=3] maskcube,
            PySourceCatalog initCatalog,
            ):
        """Note: for each source id there must be a corresponding region (subcube) in the
        maskcube, having pixel values that equal source id"""
        assert datacube.dtype is np.dtype('float32'), 'data must have type float (np.float32 aka <f4)'
        assert maskcube.dtype is np.dtype('int16'), 'mask must have type short (np.int16 aka <i2)'
        assert datacube.ndim == 3

        cdef long dz = datacube.shape[0]
        cdef long dy = datacube.shape[1]
        cdef long dx = datacube.shape[2]

        initCatalogPtr = new SourceCatalog(deref((<PySourceCatalog> initCatalog).thisptr))
        self.thisptr.run(
            <float*> datacube.data,
            <short*> maskcube.data,
            dx,
            dy,
            dz,
            deref(initCatalogPtr)
            )

    def getCatalog(self):
        c = PySourceCatalog()
        c.thisptr = new SourceCatalog(self.thisptr.getCatalog())
        return c

    def setFlags(self, python_bool doMaskOptimization, python_bool doBusyFitting):
        self.thisptr.setFlags(<bool> doMaskOptimization, <bool> doBusyFitting)
