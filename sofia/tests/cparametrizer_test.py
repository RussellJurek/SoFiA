#! /usr/bin/env python
# -*- coding: utf-8 -*-

import traceback
import sys
import unittest
import numpy as np
from sofia import cparametrizer as cp


class PyUnitInterfaceTests(unittest.TestCase):
    """This tests the cython *interface* of PyUnit (wrapping Unit)"""

    def testConstructor(self):
        u1 = cp.PyUnit()
        u2 = cp.PyUnit('cm')
        u3 = cp.PyUnit(u2)
        u4 = cp.PyUnit(u'cm')

    def testConstructorTypeSafety(self):
        with self.assertRaises(TypeError):
            u = cp.PyUnit(1)
        with self.assertRaises(UnicodeEncodeError):
            u = cp.PyUnit(u'\ua000')

    def testSetter(self):
        u = cp.PyUnit()
        u.set('kg')
        u.set('m')
        u.set(u'kg')

    def testSetterTypeSafety(self):
        with self.assertRaises(TypeError):
            u = cp.PyUnit()
            u.set(1)
        with self.assertRaises(UnicodeEncodeError):
            u = cp.PyUnit()
            u.set(u'\ua000')

    def testSetterUnknownUnit(self):
        with self.assertRaises(ValueError):
            u = cp.PyUnit()
            u.set('bla')

    def testPrintMode(self):
        u = cp.PyUnit()
        u.set('cm/s')
        self.assertEqual(u.asString(), '10^-2 m s^-1')


class PyMeasurementInterfaceTests(unittest.TestCase):
    """This tests the cython *interface* of PyMeasurement (wrapping Measurement)"""

    def testConstructor(self):
        m1 = cp.PyMeasurement()
        m2 = cp.PyMeasurement(m1)

    def testConstructorTypeSafety(self):
        with self.assertRaises(TypeError):
            m = cp.PyMeasurement(1)

    def testSetter(self):
        m = cp.PyMeasurement()
        m.set('speed', 12.345, 0.987, 'm/s')
        m.set('speed', 12.345, 0.987, cp.PyUnit('m/s'))
        m.set(u'speed', 12.345, 0.987, u'm/s')

    def testSetterTypeSafety(self):
        with self.assertRaises(TypeError):
            m = cp.PyMeasurement()
            m.set('speed', 'bla', 0.987, 'm/s')

    def testSetterUnknownUnit(self):
        with self.assertRaises(TypeError):
            m = cp.PyMeasurement()
            m.set('speed', 12.345, 0.987, 1000)

    def testAsString(self):
        m = cp.PyMeasurement()
        m.set('speed', 12.345, 0.987, 'm/s')
        self.assertEqual(m.asString(mode=cp.measurement_default), '12.345 ± 0.987 m s⁻¹')
        self.assertEqual(m.asString(mode=cp.measurement_compact), '12.345 m s⁻¹')
        self.assertEqual(m.asString(mode=cp.measurement_full), 'speed = 12.345 ± 0.987 m s⁻¹')
        self.assertEqual(m.asString(mode=cp.measurement_unit), 'm s⁻¹')
        self.assertEqual(m.asString(decimals=1), '12.3 ± 1.0 m s⁻¹')
        #self.assertEqual(m.asString(scientific=True), '')  # why is this not working?

    def testtestAsStringTypeSafety(self):
        with self.assertRaises(TypeError):
            m = cp.PyMeasurement()
            m.set('speed', 12.345, 0.987, 'm/s')
            m.asString(mode=1000)
        with self.assertRaises(TypeError):
            m = cp.PyMeasurement()
            m.set('speed', 12.345, 0.987, 'm/s')
            m.asString(scientific='')
        with self.assertRaises(TypeError):
            m = cp.PyMeasurement()
            m.set('speed', 12.345, 0.987, 'm/s')
            m.asString(scientific=1)
        with self.assertRaises(TypeError):
            m = cp.PyMeasurement()
            m.set('speed', 12.345, 0.987, 'm/s')
            m.asString(decimals='')

    def testConvert(self):
        m = cp.PyMeasurement()
        m.set('distance', 1., 0.1, cp.unit_kpc)
        self.assertEqual(m.convert(cp.unit_mpc), (0.001, 0.0001))

    def testConvertTypeSafety(self):
        with self.assertRaises(TypeError):
            m = cp.PyMeasurement()
            m.set('distance', 1., 0.1, cp.unit_kpc)
            m.convert(1000)

    def testInvert(self):
        m = cp.PyMeasurement()
        m.set('distance', 10., 0.1, 'm')
        m.invert()
        self.assertEqual(m.asString(), '0.1 ± 0.001 m⁻¹')

    def testInvertZeroDivision(self):
        with self.assertRaises(ZeroDivisionError):
            m = cp.PyMeasurement()
            m.set('distance', 0., 0.1, 'm')
            m.invert()

    def testMathOps(self):
        m1 = cp.PyMeasurement()
        m2 = cp.PyMeasurement()
        m1.set('speed', 12.345, 0.987, 'm/s')
        m2.set('distance', 0.55, 0.10, 'cm')
        self.assertEqual((m1*m2).asString(), '6.78975 ± 1.34858 × 10⁻² m² s⁻¹')
        self.assertEqual((-m1).asString(), '-12.345 ± 0.987 m s⁻¹')

    def testMathOpsIncompatibleUnits(self):
        with self.assertRaises(TypeError):
            m1 = cp.PyMeasurement()
            m2 = cp.PyMeasurement()
            m1.set('speed', 12.345, 0.987, 'm/s')
            m2.set('distance', 0.55, 0.10, 'cm')
            m1+m2

    def testCompOps(self):
        m1 = cp.PyMeasurement()
        m2 = cp.PyMeasurement()
        m3 = cp.PyMeasurement()
        m1.set('speed', 12.345, 0.987, 'm/s')
        m2.set('speed', 12.345, 0.987, 'cm/s')
        m3.set('distance', 0.55, 0.10, 'cm')
        self.assertEqual(m1 < m2, False)
        self.assertEqual(m1 <= m2, False)
        self.assertEqual(m1 > m2, True)
        self.assertEqual(m1 >= m2, True)
        self.assertEqual(m1 == m2, False)
        self.assertEqual(m1 != m2, True)

    def testCompOpsIncompatibleUnits(self):
        with self.assertRaises(TypeError):
            m1 = cp.PyMeasurement()
            m3 = cp.PyMeasurement()
            m1.set('speed', 12.345, 0.987, 'm/s')
            m3.set('distance', 0.55, 0.10, 'cm')
            m1 == m3


class PySourceInterfaceTests(unittest.TestCase):
    """This tests the cython *interface* of PySource (wrapping Source)"""

    def testConstructor(self):
        s1 = cp.PySource()
        s2 = cp.PySource(s1)

    def testConstructorTypeSafety(self):
        with self.assertRaises(TypeError):
            s = cp.PySource(2)

    def testSetParameter(self):
        s = cp.PySource()
        m = cp.PyMeasurement()
        m.set('speed', 12.345, 0.987, 'm/s')
        s.setParameter(m)
        s.setParameter('speed2', 12.345, 0.987, 'm/s')
        s.setParameter('speed3', 12.345, 0.987, cp.PyUnit('m/s'))

    def testSetParameterTypeSafety(self):
        with self.assertRaises(TypeError):
            s = cp.PySource()
            s.setParameter(1)

    def testGetParameter(self):
        s = cp.PySource()
        s.setParameter('speed1', 12.345, 0.987, 'm/s')
        s.getParameter('speed1')

    def testGetParameterNotFound(self):
        with self.assertRaises(KeyError):
            s = cp.PySource()
            s.setParameter('speed1', 12.345, 0.987, 'm/s')
            s.getParameter('speed2')

    def testGetParameterTypeSafety(self):
        with self.assertRaises(AssertionError):
            s = cp.PySource()
            s.getParameter(1)

    def testSetSourceID(self):
        s = cp.PySource()
        s.setSourceID(111)

    def testSetSourceIDTypeSafety(self):
        with self.assertRaises(OverflowError):
            s = cp.PySource()
            s.setSourceID(-1)
        with self.assertRaises(TypeError):
            s = cp.PySource()
            s.setSourceID('1')

    def testGetSourceID(self):
        s = cp.PySource()
        s.setSourceID(111)
        self.assertEqual(s.getSourceID(), 111)

    def testSetSourceName(self):
        s = cp.PySource()
        s.setSourceName('mysource')

    def testSetSourceNameTypeSafety(self):
        with self.assertRaises(AssertionError):
            s = cp.PySource()
            s.setSourceName(1)

    def testGetSourceName(self):
        s = cp.PySource()
        s.setSourceName('mysource')
        self.assertEqual(s.getSourceName(), 'mysource')

    def testIsDefined(self):
        s = cp.PySource()
        m = cp.PyMeasurement()
        m.set('speed', 12.345, 0.987, 'm/s')
        s.setParameter(m)
        self.assertEqual(s.parameterDefined('speed'), True)
        self.assertEqual(s.parameterDefined('bla'), False)

    def testIsDefinedTypeSafety(self):
        with self.assertRaises(AssertionError):
            s = cp.PySource()
            s.parameterDefined(1)

    def testGetParameters(self):
        s = cp.PySource()
        s.setParameter('speed1', 12.345, 0.987, 'm/s')
        s.setParameter('speed2', 12.345, 0.987, 'm/s')
        s.getParameters()
        self.assertIsInstance(s.getParameters(), dict)
        self.assertEqual(len(s.getParameters()), 2)
        self.assertListEqual(s.getParameters().keys(), ['speed1', 'speed2'])
        self.assertIsInstance(s.getParameters().values()[0], cp.PyMeasurement)

    def testSetParameters(self):
        s1 = cp.PySource()
        s2 = cp.PySource()
        s1.setParameter('speed1', 12.345, 0.987, 'm/s')
        s1.setParameter('speed2', 12.345, 0.987, 'm/s')
        s2.setParameters(s1.getParameters())
        self.assertIsInstance(s2.getParameters(), dict)
        self.assertEqual(len(s2.getParameters()), 2)
        self.assertListEqual(s2.getParameters().keys(), ['speed1', 'speed2'])
        self.assertIsInstance(s2.getParameters().values()[0], cp.PyMeasurement)

    def testSetParametersTypeSafety(self):
        with self.assertRaises(AssertionError):
            s = cp.PySource()
            s.setParameters(1)
        with self.assertRaises(AssertionError):
            s = cp.PySource()
            s.setParameters({})


class PySourceCatalogInterfaceTests(unittest.TestCase):
    """This tests the cython *interface* of PySourceCatalog (wrapping SourceCatalog)"""

    def testConstructor(self):
        sc1 = cp.PySourceCatalog()
        sc2 = cp.PySourceCatalog(sc1)

    def testConstructorTypeSafety(self):
        with self.assertRaises(TypeError):
            sc = cp.PySourceCatalog(2)

    def testDuchampReader(self):
        # TODO: need example duchamp file for this
        return

    def testInsert(self):
        s = cp.PySource()
        s.setParameter('speed', 12.345, 0.987, 'm/s')
        sc = cp.PySourceCatalog()
        sc.insert(s)

    def testInsertTypeSafety(self):
        with self.assertRaises(TypeError):
            sc = cp.PySourceCatalog()
            sc.insert(1)

    def testSetSources(self):
        s1 = cp.PySource()
        s2 = cp.PySource()
        s1.setParameter('speed1', 12.345, 0.987, 'm/s')
        s2.setParameter('speed2', 12.345, 0.987, 'm/s')
        sDict = {1: s1, 2: s2}
        sc = cp.PySourceCatalog()
        sc.setSources(sDict)

    def testSetSourcesTypeSafety(self):
        with self.assertRaises(AssertionError):
            sc = cp.PySourceCatalog()
            sc.setSources(1)
        with self.assertRaises(AssertionError):
            sc = cp.PySourceCatalog()
            sc.setSources({})

    def testGetParameters(self):
        s1 = cp.PySource()
        s2 = cp.PySource()
        s1.setSourceID(0)  # this is important, otherwise sources get overwritten
        s2.setSourceID(1)
        s1.setParameter('speed1', 12.345, 0.987, 'm/s')
        s2.setParameter('speed2', 12.345, 0.987, 'm/s')
        sDict = {1: s1, 2: s2}
        sc = cp.PySourceCatalog()
        sc.setSources(sDict)
        sDictReturned = sc.getSources()
        self.assertIsInstance(sDictReturned, dict)
        self.assertEqual(len(sDict), len(sDictReturned))
        self.assertIsInstance(sDictReturned.values()[0], cp.PySource)



def main():
    unittest.main()

if __name__ == '__main__':
    main()

