#! /usr/bin/env python
# -*- coding: utf-8 -*-

import traceback
import sys
import unittest
import numpy as np
from SoFiA import cparametrizer as cp


class PyUnitInterfaceTests(unittest.TestCase):
    """This tests the cython *interface* of PyUnit (wrapping Unit)"""

    def testConstructor(self):
        print 'Testing PyUnit: constructor'
        u1 = cp.PyUnit()
        u2 = cp.PyUnit('cm')
        u3 = cp.PyUnit(u2)
        u4 = cp.PyUnit(u'cm')

    def testConstructorTypeSafety(self):
        print 'Testing PyUnit: constructor type safety'
        with self.assertRaises(TypeError):
            u = cp.PyUnit(1)
        with self.assertRaises(UnicodeEncodeError):
            u = cp.PyUnit(u'\ua000')

    def testCopy(self):
        """make sure, copies are deep"""
        print 'Testing PyUnit: make sure, copies are deep'
        u1 = cp.PyUnit('m')
        u2 = cp.PyUnit(u1)
        u2.unit = 'K'
        self.assertEqual(u1.unit, 'm')

    def testSetter(self):
        print 'Testing PyUnit: setter'
        u = cp.PyUnit()
        u.setU('kg')
        u.setU('m')
        u.setU(u'kg')

    def testSetterTypeSafety(self):
        print 'Testing PyUnit: setter type safety'
        with self.assertRaises(TypeError):
            u = cp.PyUnit()
            u.setU(1)
        with self.assertRaises(UnicodeEncodeError):
            u = cp.PyUnit()
            u.setU(u'\ua000')

    def testProperties(self):
        print 'Testing PyUnit: "unit" property getter/setter'
        u = cp.PyUnit('cm')
        self.assertEqual(u.unit, '10^-2 m')
        u.unit = 'm'
        self.assertEqual(u.unit, 'm')

    def testSetterUnknownUnit(self):
        print 'Testing PyUnit: correct exceptions for unknown units'
        with self.assertRaises(ValueError):
            u = cp.PyUnit('bla')
        with self.assertRaises(ValueError):
            u = cp.PyUnit()
            u.unit = 'bla'
        with self.assertRaises(ValueError):
            u = cp.PyUnit()
            u.setU('bla')

    def testPrintMode(self):
        print 'Testing PyUnit: print mode'
        u = cp.PyUnit('cm/s')
        self.assertEqual(u.asString(mode=cp.unit_std), '10^-2 m s^-1')
        self.assertEqual(u.asString(mode=cp.unit_exp), '10⁻² m s⁻¹')


class PyMeasurementInterfaceTests(unittest.TestCase):
    """This tests the cython *interface* of PyMeasurement (wrapping Measurement)"""

    def testConstructor(self):
        print 'Testing PyMeasurement: constructor'
        m1 = cp.PyMeasurement()
        m2 = cp.PyMeasurement(m1)
        m3 = cp.PyMeasurement('speed', 12.345, 0.987, 'm/s')

    def testConstructorTypeSafety(self):
        print 'Testing PyMeasurement: type safety'
        with self.assertRaises(TypeError):
            m = cp.PyMeasurement(1)

    def testCopy(self):
        print 'Testing PyMeasurement: make sure, copies are deep'
        m1 = cp.PyMeasurement('dist', 1., 0., 'm')
        m2 = cp.PyMeasurement(m1)
        m2.setM('temp', 2., 1., 'K')
        self.assertEqual(m1.name, 'dist')
        self.assertEqual(m1.value, 1.)
        self.assertEqual(m1.uncertainty, 0.)
        self.assertEqual(m1.unit.asString(), 'm')

    def testSetter(self):
        print 'Testing PyMeasurement: setter'
        m = cp.PyMeasurement()
        m.setM('speed', 12.345, 0.987, 'm/s')
        m.setM('speed', 12.345, 0.987, cp.PyUnit('m/s'))
        m.setM(u'speed', 12.345, 0.987, u'm/s')

    def testSetterTypeSafety(self):
        print 'Testing PyMeasurement: setter type safety'
        with self.assertRaises(TypeError):
            m = cp.PyMeasurement()
            m.setM('speed', 'bla', 0.987, 'm/s')

    def testSetterUnknownUnit(self):
        print 'Testing PyMeasurement: setter, correct exceptions when unit unknown'
        with self.assertRaises(TypeError):
            m = cp.PyMeasurement('speed', 12.345, 0.987, 1000)
        with self.assertRaises(TypeError):
            m = cp.PyMeasurement()
            m.setM('speed', 12.345, 0.987, 1000)

    def testPropertiesName(self):
        print 'Testing PyMeasurement: "name" property getter/setter'
        m = cp.PyMeasurement('speed', 12.345, 0.987, 'm/s')
        self.assertEqual(m.name, 'speed')
        m.name = 'dist'
        self.assertEqual(m.name, 'dist')

    def testPropertiesValue(self):
        print 'Testing PyMeasurement: "value" property getter/setter'
        m = cp.PyMeasurement('speed', 12.345, 0.987, 'm/s')
        self.assertEqual(m.value, 12.345)
        m.value = 1.0
        self.assertEqual(m.value, 1.0)

    def testPropertiesUncertainty(self):
        print 'Testing PyMeasurement: "uncertainty" property getter/setter'
        m = cp.PyMeasurement('speed', 12.345, 0.987, 'm/s')
        self.assertEqual(m.uncertainty, 0.987)
        m.uncertainty = 1.0
        self.assertEqual(m.uncertainty, 1.0)

    def testPropertiesUnit(self):
        print 'Testing PyMeasurement: "unit" property getter/setter'
        m = cp.PyMeasurement('speed', 12.345, 0.987, 'm/s')
        self.assertIsInstance(m.unit, cp.PyUnit)
        self.assertEqual(m.unit.asString(), 'm s^-1')
        m.unit = cp.unit_kpc
        self.assertIsInstance(m.unit, cp.PyUnit)
        self.assertEqual(m.unit.asString(), 'm')
        self.assertEqual(m.asString(), '3.80927e+20 ± 3.04556e+19 m')

    def testAsString(self):
        print 'Testing PyMeasurement: asstring method'
        m = cp.PyMeasurement('speed', 12.345, 0.987, 'm/s')
        self.assertEqual(m.asString(mode=cp.measurement_default), '12.345 ± 0.987 m s⁻¹')
        self.assertEqual(m.asString(mode=cp.measurement_compact), '12.345 m s⁻¹')
        self.assertEqual(m.asString(mode=cp.measurement_full), 'speed = 12.345 ± 0.987 m s⁻¹')
        self.assertEqual(m.asString(mode=cp.measurement_unit), 'm s⁻¹')
        self.assertEqual(m.asString(decimals=1), '12.3 ± 1.0 m s⁻¹')
        #self.assertEqual(m.asString(scientific=True), '')  # why is this not working?

    def testtestAsStringTypeSafety(self):
        print 'Testing PyMeasurement: asstring method type safety'
        m = cp.PyMeasurement('speed', 12.345, 0.987, 'm/s')
        with self.assertRaises(TypeError):
            m.asString(mode=1000)
        with self.assertRaises(TypeError):
            m.asString(scientific='')
        with self.assertRaises(TypeError):
            m.asString(scientific=1)
        with self.assertRaises(TypeError):
            m.asString(decimals='')

    def testConvert(self):
        print 'Testing PyMeasurement: convert method'
        m = cp.PyMeasurement('distance', 1., 0.1, cp.unit_kpc)
        self.assertEqual(m.convert(cp.unit_mpc), (0.001, 0.0001))

    def testConvertTypeSafety(self):
        print 'Testing PyMeasurement: convert method type safety'
        with self.assertRaises(TypeError):
            m = cp.PyMeasurement('distance', 1., 0.1, cp.unit_kpc)
            m.convert(1000)

    def testInvert(self):
        print 'Testing PyMeasurement: invert method'
        m = cp.PyMeasurement('distance', 10., 0.1, 'm')
        m.invert()
        self.assertEqual(m.asString(), '0.1 ± 0.001 m⁻¹')

    def testInvertZeroDivision(self):
        print 'Testing PyMeasurement: invert method, ZeroDivision exception'
        with self.assertRaises(ZeroDivisionError):
            m = cp.PyMeasurement('distance', 0., 0.1, 'm')
            m.invert()

    def testMathOps(self):
        print 'Testing PyMeasurement: mathematical operations'
        m1 = cp.PyMeasurement('speed', 12.345, 0.987, 'm/s')
        m2 = cp.PyMeasurement('distance', 0.55, 0.10, 'cm')
        self.assertEqual((m1*m2).asString(), '6.78975 ± 1.34858 × 10⁻² m² s⁻¹')
        self.assertEqual((-m1).asString(), '-12.345 ± 0.987 m s⁻¹')

    def testMathOpsIncompatibleUnits(self):
        print 'Testing PyMeasurement: math operations raise exception when units incompatible'
        with self.assertRaises(TypeError):
            m1 = cp.PyMeasurement('speed', 12.345, 0.987, 'm/s')
            m2 = cp.PyMeasurement('distance', 0.55, 0.10, 'cm')
            m1+m2

    def testCompOps(self):
        print 'Testing PyMeasurement: comparison operations'
        m1 = cp.PyMeasurement('speed', 12.345, 0.987, 'm/s')
        m2 = cp.PyMeasurement('speed', 12.345, 0.987, 'cm/s')
        m3 = cp.PyMeasurement('distance', 0.55, 0.10, 'cm')
        self.assertEqual(m1 < m2, False)
        self.assertEqual(m1 <= m2, False)
        self.assertEqual(m1 > m2, True)
        self.assertEqual(m1 >= m2, True)
        self.assertEqual(m1 == m2, False)
        self.assertEqual(m1 != m2, True)
        m1 = cp.PyMeasurement('distance', 1000., 1000., cp.unit_kpc)
        m2 = cp.PyMeasurement('distance', 1., 1., cp.unit_mpc)
        self.assertEqual(m1 == m2, True)
        m1 = cp.PyMeasurement('distance1', 1000., 1000., cp.unit_kpc)
        m2 = cp.PyMeasurement('distance2', 1., 1., cp.unit_mpc)
        self.assertEqual(m1 == m2, True)  # shouldn't this be False?
        m1 = cp.PyMeasurement('distance1', 100., 1000., cp.unit_kpc)
        m2 = cp.PyMeasurement('distance2', 1., 1., cp.unit_mpc)
        self.assertEqual(m1 == m2, False)
        m1 = cp.PyMeasurement('distance1', 1000., 100., cp.unit_kpc)
        m2 = cp.PyMeasurement('distance2', 1., 1., cp.unit_mpc)
        self.assertEqual(m1 == m2, True)  # shouldn't this be False?

    def testCompOpsIncompatibleUnits(self):
        print 'Testing PyMeasurement: comparison operations raise exception when units incompatible'
        with self.assertRaises(TypeError):
            m1 = cp.PyMeasurement('speed', 12.345, 0.987, 'm/s')
            m3 = cp.PyMeasurement('distance', 0.55, 0.10, 'cm')
            m1 == m3


class PySourceInterfaceTests(unittest.TestCase):
    """This tests the cython *interface* of PySource (wrapping Source)"""

    def testConstructor(self):
        print 'Testing PySource: constructor'
        s1 = cp.PySource()
        s2 = cp.PySource(s1)

    def testConstructorTypeSafety(self):
        print 'Testing PySource: constructor type safety'
        with self.assertRaises(TypeError):
            s = cp.PySource(2)

    def testCopy(self):
        print 'Testing PySource: make sure, copies are deep'
        s1 = cp.PySource()
        m1 = cp.PyMeasurement('dist', 1., 0.5, 'm')
        s1.setParameter(m1)
        s1.ID = 10
        s1.name = 'source1'
        s2 = s1.copy()
        s2.clear()
        s2.setParameter('temp', 2., 1., 'K')
        s2.ID = 20
        s2.name = 'source2'
        self.assertEqual(m1.name, 'dist')
        self.assertEqual(m1.value, 1.)
        self.assertEqual(m1.uncertainty, 0.5)
        self.assertEqual(m1.unit.asString(), 'm')
        m1 = s1.getParameter('dist')
        self.assertEqual(m1.name, 'dist')
        self.assertEqual(m1.value, 1.)
        self.assertEqual(m1.uncertainty, 0.5)
        self.assertEqual(m1.unit.asString(), 'm')
        self.assertEqual(s1.ID, 10)
        self.assertEqual(s2.ID, 20)
        self.assertEqual(s1.name, 'source1')
        self.assertEqual(s2.name, 'source2')

    def testSetParameter(self):
        print 'Testing PySource: setting single parameter'
        s = cp.PySource()
        m = cp.PyMeasurement('speed', 12.345, 0.987, 'm/s')
        s.setParameter(m)
        s.setParameter('speed2', 12.345, 0.987, 'm/s')
        s.setParameter('speed3', 12.345, 0.987, cp.PyUnit('m/s'))

    def testSetParameterTypeSafety(self):
        print 'Testing PySource: setting single parameter type safety'
        with self.assertRaises(TypeError):
            s = cp.PySource()
            s.setParameter(1)

    def testGetParameter(self):
        print 'Testing PySource: getting single parameter'
        s = cp.PySource()
        s.setParameter('speed1', 12.345, 0.987, 'm/s')
        self.assertEqual(s.getParameter('speed1').asString(), '12.345 ± 0.987 m s⁻¹')

    def testGetParameterNotFound(self):
        print 'Testing PySource: getting single parameter, exception on not found'
        with self.assertRaises(KeyError):
            s = cp.PySource()
            s.setParameter('speed1', 12.345, 0.987, 'm/s')
            s.getParameter('speed2')

    def testGetParameterTypeSafety(self):
        print 'Testing PySource: getting single parameter type safety'
        with self.assertRaises(AssertionError):
            s = cp.PySource()
            s.getParameter(1)

    def testSetSourceID(self):
        print 'Testing PySource: setting source ID'
        s = cp.PySource()
        s.setSourceID(111)

    def testSetSourceIDTypeSafety(self):
        print 'Testing PySource: setting source ID type safety'
        with self.assertRaises(OverflowError):
            s = cp.PySource()
            s.setSourceID(-1)
        with self.assertRaises(TypeError):
            s = cp.PySource()
            s.setSourceID('1')

    def testGetSourceID(self):
        print 'Testing PySource: getting source ID'
        s = cp.PySource()
        s.setSourceID(111)
        self.assertEqual(s.getSourceID(), 111)

    def testSourceIDProperties(self):
        print 'Testing PySource: "ID" property getter/setter'
        s = cp.PySource()
        s.ID = 111
        self.assertEqual(s.ID, 111)

    def testSetSourceName(self):
        print 'Testing PySource: setting source name'
        s = cp.PySource()
        s.setSourceName('mysource')

    def testSetSourceNameTypeSafety(self):
        print 'Testing PySource: setting source name type safety'
        with self.assertRaises(AssertionError):
            s = cp.PySource()
            s.setSourceName(1)

    def testGetSourceName(self):
        print 'Testing PySource: getting source name'
        s = cp.PySource()
        s.setSourceName('mysource')
        self.assertEqual(s.getSourceName(), 'mysource')

    def testSourceNameProperties(self):
        print 'Testing PySource: "name" property getter/setter'
        s = cp.PySource()
        s.name = 'source1'
        self.assertEqual(s.name, 'source1')

    def testIsDefined(self):
        print 'Testing PySource: isDefined method'
        s = cp.PySource()
        m = cp.PyMeasurement('speed', 12.345, 0.987, 'm/s')
        s.setParameter(m)
        self.assertEqual(s.parameterDefined('speed'), True)
        self.assertEqual(s.parameterDefined('bla'), False)

    def testIsDefinedTypeSafety(self):
        print 'Testing PySource: isDefined method type safety'
        with self.assertRaises(AssertionError):
            s = cp.PySource()
            s.parameterDefined(1)

    def testGetParameters(self):
        print 'Testing PySource: getting parameters dictionary'
        s = cp.PySource()
        s.setParameter('speed1', 12.345, 0.987, 'm/s')
        s.setParameter('speed2', 12.345, 0.987, 'm/s')
        s.getParameters()
        self.assertIsInstance(s.getParameters(), dict)
        self.assertEqual(len(s.getParameters()), 2)
        self.assertListEqual(s.getParameters().keys(), ['speed1', 'speed2'])
        self.assertIsInstance(s.getParameters().values()[0], cp.PyMeasurement)

    def testSetParameters(self):
        print 'Testing PySource: setting parameters dictionary'
        s1 = cp.PySource()
        s2 = cp.PySource()
        s1.setParameter('speed1', 12.345, 0.987, 'm/s')
        s1.setParameter('speed2', 12.345, 0.987, 'm/s')
        s2.setParameters(s1.getParameters())
        self.assertIsInstance(s2.getParameters(), dict)
        self.assertEqual(len(s2.getParameters()), 2)
        self.assertListEqual(s2.getParameters().keys(), ['speed1', 'speed2'])
        self.assertIsInstance(s2.getParameters().values()[0], cp.PyMeasurement)
        s2.setParameters({})
        self.assertIsInstance(s2.getParameters(), dict)
        self.assertEqual(len(s2.getParameters()), 0)

    def testSetParametersTypeSafety(self):
        print 'Testing PySource: setting parameters dictionary type safety'
        s = cp.PySource()
        with self.assertRaises(TypeError):
            s.setParameters(1)
        with self.assertRaises(AssertionError):
            s.setParameters({
                0: cp.PyMeasurement('speed', 12.345, 0.987, 'm/s'),
                1: 1
                })

    def testUpdateParameters(self):
        print 'Testing PySource: updating parameters dictionary'
        s = cp.PySource()
        s.setParameter('speed1', 12.345, 0.987, 'm/s')
        s.updateParameters({
                'speed2': cp.PyMeasurement('speed2', 12.345, 0.987, 'm/s')
                })
        self.assertIsInstance(s.getParameters(), dict)
        self.assertEqual(len(s.getParameters()), 2)
        self.assertListEqual(s.getParameters().keys(), ['speed1', 'speed2'])
        self.assertIsInstance(s.getParameters().values()[0], cp.PyMeasurement)
        s.updateParameters({})
        self.assertIsInstance(s.getParameters(), dict)
        self.assertEqual(len(s.getParameters()), 2)
        self.assertListEqual(s.getParameters().keys(), ['speed1', 'speed2'])
        self.assertIsInstance(s.getParameters().values()[0], cp.PyMeasurement)

    def testUpdateParametersTypeSafety(self):
        print 'Testing PySource: updating parameters dictionary type safety'
        s = cp.PySource()
        with self.assertRaises(TypeError):
            s.updateParameters(1)
        with self.assertRaises(AssertionError):
            s.updateParameters({
                0: cp.PyMeasurement('speed', 12.345, 0.987, 'm/s'),
                1: 1
                })


class PySourceCatalogInterfaceTests(unittest.TestCase):
    """This tests the cython *interface* of PySourceCatalog (wrapping SourceCatalog)"""

    def testConstructor(self):
        print 'Testing PySourceCatalog: constructor'
        sc1 = cp.PySourceCatalog()
        sc2 = cp.PySourceCatalog(sc1)

    def testConstructorTypeSafety(self):
        print 'Testing PySourceCatalog: constructor type safety'
        with self.assertRaises(TypeError):
            sc = cp.PySourceCatalog(2)

    def testDuchampReader(self):
        print 'Testing PySourceCatalog: DuchampReader: TODO, not implemented yet'
        # TODO: need example duchamp file for this
        return

    def testInsert(self):
        print 'Testing PySourceCatalog: inserting a source'
        s = cp.PySource()
        s.setParameter('speed', 12.345, 0.987, 'm/s')
        sc = cp.PySourceCatalog()
        sc.insert(s)

    def testDoubleInsert(self):
        print 'Testing PySourceCatalog: exception on inserting a source with same ID'
        s1 = cp.PySource()
        s1.setParameter('speed', 12.345, 0.987, 'm/s')
        s1.ID = 0
        s2 = s1.copy()
        sc = cp.PySourceCatalog()
        sc.insert(s1)
        with self.assertRaises(ValueError):
            sc.insert(s2)

    def testUpdate(self):
        print 'Testing PySourceCatalog: updating a source'
        s1 = cp.PySource()
        s1.setParameter('speed', 12.345, 0.987, 'm/s')
        s1.ID = 0
        s2 = s1.copy()
        sc = cp.PySourceCatalog()
        sc.insert(s1)
        sc.update(s1.ID, s2)

    def testUpdateIDmismatch(self):
        print 'Testing PySourceCatalog: updating a source, mismatch exception on source id and call'
        s1 = cp.PySource()
        s1.setParameter('speed', 12.345, 0.987, 'm/s')
        s1.ID = 0
        s2 = s1.copy()
        self.assertEqual(s1.ID, s2.ID)
        s2.ID = 10
        sc = cp.PySourceCatalog()
        sc.insert(s1)
        with self.assertRaises(AssertionError):
            sc.update(0, s2)

    def testUpdateNotExistent(self):
        print 'Testing PySourceCatalog: exception on updating a source with missing ID'
        s1 = cp.PySource()
        s1.setParameter('speed', 12.345, 0.987, 'm/s')
        s1.ID = 0
        s2 = s1.copy()
        s2.ID = 1000
        sc = cp.PySourceCatalog()
        sc.insert(s1)
        with self.assertRaises(ValueError):
            sc.update(1000, s2)

    def testInsertTypeSafety(self):
        print 'Testing PySourceCatalog: inserting a source, type safety'
        with self.assertRaises(TypeError):
            sc = cp.PySourceCatalog()
            sc.insert(1)

    def testSetSources(self):
        print 'Testing PySourceCatalog: inserting source dictionary'
        s1 = cp.PySource()
        s2 = cp.PySource()
        s1.ID = 10  # this is important, otherwise sources get overwritten
        s2.ID = 11
        s1.setParameter('speed1', 12.345, 0.987, 'm/s')
        s2.setParameter('speed2', 12.345, 0.987, 'm/s')
        sDict = {s1.ID: s1, s2.ID: s2}
        sc = cp.PySourceCatalog()
        sc.setSources(sDict)

    def testSetSourcesTypeSafety(self):
        print 'Testing PySourceCatalog: inserting source dictionary, type safety'
        with self.assertRaises(TypeError):
            sc = cp.PySourceCatalog()
            sc.setSources(1)

    def testUpdateSources(self):
        print 'Testing PySourceCatalog: updating source dictionary'
        s1 = cp.PySource()
        s1.setParameter('speed1', 12.345, 0.987, 'm/s')
        s1.ID = 0
        s2 = s1.copy()
        s2.ID = 1
        sc = cp.PySourceCatalog()
        sc.setSources({s1.ID: s1})
        sc.updateSources({s2.ID: s2})

    def testUpdateSourcesDuplicates(self):
        print 'Testing PySourceCatalog: updating source dictionary, test for assert on duplicate'
        s1 = cp.PySource()
        s1.setParameter('speed1', 12.345, 0.987, 'm/s')
        s1.ID = 0
        sc = cp.PySourceCatalog()
        sc.setSources({s1.ID: s1})
        sc.updateSources({s1.ID: s1}, warn_on_duplicate=False)  # this should silently overwrite
        with self.assertRaises(AssertionError):
            sc.updateSources({s1.ID: s1})

    def testGetSources(self):
        print 'Testing PySourceCatalog: getting source dictionary'
        s1 = cp.PySource()
        s2 = cp.PySource()
        s1.ID = 10  # this is important, otherwise sources get overwritten
        s2.ID = 11
        s1.setParameter('speed1', 12.345, 0.987, 'm/s')
        s2.setParameter('speed2', 12.345, 0.987, 'm/s')
        sDict = {s1.ID: s1, s2.ID: s2}
        sc = cp.PySourceCatalog()
        sc.setSources(sDict)
        sDictReturned = sc.getSources()
        self.assertIsInstance(sDictReturned, dict)
        self.assertEqual(len(sDict), len(sDictReturned))
        self.assertIsInstance(sDictReturned.values()[0], cp.PySource)

    def testGetSourceIDs(self):
        print 'Testing PySourceCatalog: getting source IDs'
        s1 = cp.PySource()
        s2 = cp.PySource()
        s1.ID = 10  # this is important, otherwise sources get overwritten
        s2.ID = 11
        s1.setParameter('speed1', 12.345, 0.987, 'm/s')
        s2.setParameter('speed2', 12.345, 0.987, 'm/s')
        sDict = {s1.ID: s1, s2.ID: s2}
        sc = cp.PySourceCatalog()
        sc.setSources(sDict)
        allIDs = sc.getSourceIDs()
        self.assertIsInstance(allIDs, list)
        self.assertEqual(allIDs, [10, 11])



class PyModuleParametrisationInterfaceTests(unittest.TestCase):
    """This tests the cython *interface* of PyModuleParametrisation
    (wrapping ModuleParametrisation)"""

    def testConstructor(self):
        print 'Testing PyModuleParametrisation: constructor'
        p = cp.PyModuleParametrisation()

    def testRunTypeSafety(self):
        print 'Testing PyModuleParametrisation: run method type safety'
        p = cp.PyModuleParametrisation()
        cube = np.zeros((100, 100, 100), dtype=np.float32)
        mask = np.zeros((100, 100, 100), dtype=np.int16)
        initcatalog = cp.PySourceCatalog()
        doMaskOptimization = True
        doBusyFitting = True
        p.setFlags(doMaskOptimization, doBusyFitting)
        with self.assertRaises(ValueError):
            p.run(cube.astype(np.float64), mask, initcatalog)
        with self.assertRaises(ValueError):
            p.run(cube, mask.astype(np.float64), initcatalog)
        with self.assertRaises(TypeError):
            p.run(cube, mask, 1)

    def testRunTypeSafety(self):
        print 'Testing PyModuleParametrisation: set flags type safety'
        p = cp.PyModuleParametrisation()
        with self.assertRaises(TypeError):
            p.setFlags('1', False)
        with self.assertRaises(TypeError):
            p.setFlags(False, '1')

    def testRun(self):
        print 'Testing PyModuleParametrisation: run method (artificial data)'

        def addGauss(sName, sID, cube, mask, amp, x0, y0, z0, xw, yw, zw):
            """adds a fake source (3D gauss) and initial mask (few pixels large)

            since cube/mask are ndarrays, it's 'call-by-reference'

            returns a PySource with just the right parameters"""

            def gaussian1D(dist, sigma):
                return np.exp(- dist**2 / 2. / sigma**2)

            # first need xyz indices
            z, y, x = np.indices(cube.shape)

            cube += amp * gaussian1D(x-x0, xw) * gaussian1D(y-y0, yw) * gaussian1D(z-z0, zw)
            mask[
                z0 - 3*zw: z0 + 3*zw,
                y0 - 3*yw: y0 + 3*yw,
                x0 - 3*xw: x0 + 3*xw,
                ] = sID

            s = cp.PySource()
            s.name = sName
            s.ID = sID
            pDict = {
                'X': cp.PyMeasurement('X', x0, 0.0, ''),
                'Y': cp.PyMeasurement('Y', y0, 0.0, ''),
                'Z': cp.PyMeasurement('Z', z0, 0.0, ''),
                'BBOX_X_MIN': cp.PyMeasurement('BBOX_X_MIN', x0 - 3*xw, 0.0, ''),
                'BBOX_X_MAX': cp.PyMeasurement('BBOX_X_MAX', x0 + 3*xw, 0.0, ''),
                'BBOX_Y_MIN': cp.PyMeasurement('BBOX_Y_MIN', y0 - 3*yw, 0.0, ''),
                'BBOX_Y_MAX': cp.PyMeasurement('BBOX_Y_MAX', y0 + 3*yw, 0.0, ''),
                'BBOX_Z_MIN': cp.PyMeasurement('BBOX_Z_MIN', z0 - 3*zw, 0.0, ''),
                'BBOX_Z_MAX': cp.PyMeasurement('BBOX_Z_MAX', z0 + 3*zw, 0.0, ''),
                }
            s.setParameters(pDict)

            return s


        cube = np.random.normal(0., 0.01, (100, 100, 100)).astype(np.float32)
        mask = np.zeros((100, 100, 100), dtype=np.int16)

        # add some sources
        initcatalog = cp.PySourceCatalog()
        initcatalog.insert(addGauss("source1", 1, cube, mask, 10., 20, 25, 30, 2, 3, 5))
        initcatalog.insert(addGauss("source2", 2, cube, mask, 20., 50, 55, 60, 5, 3, 2))

        for k, v in initcatalog.getSources().iteritems():
            print '\n-----------------'
            print 'source', k
            print '-----------------'
            for _k, _v in sorted(v.getParameters().iteritems()):
                print _k, _v.asString()

        print "\nmax amp"
        print np.where(cube == np.max(cube)), np.max(cube)

        p = cp.PyModuleParametrisation()
        doMaskOptimization = True
        doBusyFitting = False
        p.setFlags(doMaskOptimization, doBusyFitting)
        p.run(cube, mask, initcatalog)
        results = p.getCatalog()

        for k, v in results.getSources().iteritems():
            print '\n-----------------'
            print 'source', k
            print '-----------------'
            for _k, _v in sorted(v.getParameters().iteritems()):
                print _k, _v.asString()

        self.assertIsInstance(results.getSources(), dict)
        self.assertEqual(len(results.getSources()), 2)

        s1params = results.getSources()[1].getParameters()
        s2params = results.getSources()[2].getParameters()
        self.assertAlmostEqual(s1params['RMS_CUBE'].value, 0.01, 1)
        self.assertAlmostEqual(s1params['F_PEAK'].value, 10., 1)
        self.assertAlmostEqual(s1params['X'].value, 20., 1)
        self.assertAlmostEqual(s1params['Y'].value, 25., 1)
        self.assertAlmostEqual(s1params['Z'].value, 30., 1)
        self.assertAlmostEqual(s2params['F_PEAK'].value, 20., 1)
        self.assertAlmostEqual(s2params['X'].value, 50., 1)
        self.assertAlmostEqual(s2params['Y'].value, 55., 1)
        self.assertAlmostEqual(s2params['Z'].value, 60., 1)

        from SoFiA import store_ascii
        store_ascii.make_ascii(results, ['*'], outname='/tmp/sofia_catalog_test1.txt')
        store_ascii.make_ascii(results, ['X', 'Y', 'Z'], outname='/tmp/sofia_catalog_test2.txt')
        from SoFiA import store_xml
        store_xml.make_xml(results, outname='/tmp/sofia_catalog_test1.xml')

def main():
    unittest.main()

if __name__ == '__main__':
    main()

