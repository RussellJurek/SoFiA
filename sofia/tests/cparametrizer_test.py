#! /usr/bin/env python
# -*- coding: utf-8 -*-

import traceback
import sys
import unittest
from sofia import cparametrizer as cp

def eprint():
    _exc_info = sys.exc_info()[1]
    _trace = traceback.format_tb(sys.exc_info()[2])
    return '\n'.join([
        '',
        type(_exc_info).__name__,
        _exc_info.message,
        #'\n'.join([str(a) for a in _trace])
        ])

# Note: if you want to test, whether something raises the correct error,
# you will need one test per exception, otherwise only the first line will be tested
# in a try-except clause (also if you test other things in the same
# block, everything below the exception-raiser won't be tested obviously)


class PyUnitInterfaceTests(unittest.TestCase):
    """This tests the cython *interface* of PyUnit (wrapping Unit)"""

    def testConstructor(self):
        try:
            u1 = cp.PyUnit()
            u2 = cp.PyUnit('cm')
            u3 = cp.PyUnit(u2)
        except:
            emsg = eprint()
            self.fail('Unexpected exception thrown:\n' + emsg)

    def testConstructorUnicode(self):
        try:
            u = cp.PyUnit(u'cm')
            u = cp.PyUnit(u'\ua000')
        except UnicodeEncodeError:
            pass
        except:
            emsg = eprint()
            self.fail('Unexpected exception thrown:\n' + emsg)

    def testConstructorTypeSafety(self):
        try:
            u = cp.PyUnit(1)  # this should raise TypeError
        except TypeError:
            pass
        except:
            emsg = eprint()
            self.fail('Unexpected exception thrown:\n' + emsg)
        else:
            self.fail('ExpectedException not thrown')

    def testSetter(self):
        try:
            u = cp.PyUnit()
            u.set('kg')
            u.set('m')
        except:
            emsg = eprint()
            self.fail('Unexpected exception thrown:\n' + emsg)

    def testSetterUnicode(self):
        try:
            u = cp.PyUnit()
            u.set(u'kg')
            u.set(u'\ua000')
        except UnicodeEncodeError:
            pass
        except:
            emsg = eprint()
            self.fail('Unexpected exception thrown:\n' + emsg)

    def testSetterTypeSafety(self):
        try:
            u = cp.PyUnit()
            u.set(1)  # this should raise TypeError
        except TypeError:
            pass
        except:
            emsg = eprint()
            self.fail('Unexpected exception thrown:\n' + emsg)
        else:
            self.fail('ExpectedException not thrown')

    def testSetterUnknownUnit(self):
        try:
            u = cp.PyUnit()
            u.set('bla')  # this should raise ValueError
        except ValueError:
            pass
        except:
            emsg = eprint()
            self.fail('Unexpected exception thrown:\n' + emsg)
        else:
            self.fail('ExpectedException not thrown')

    def testPrintMode(self):
        try:
            u = cp.PyUnit()
            u.set('cm/s')
            self.assertEqual(u.asString(), '10^-2 m s^-1')
        except:
            emsg = eprint()
            self.fail('Unexpected exception thrown:\n' + emsg)


class PyMeasurementInterfaceTests(unittest.TestCase):
    """This tests the cython *interface* of PyMeasurement (wrapping Measurement)"""

    def testConstructor(self):
        try:
            m1 = cp.PyMeasurement()
            m2 = cp.PyMeasurement(1)  # this should raise TypeError
        except TypeError:
            pass
        except:
            emsg = eprint()
            self.fail('Unexpected exception thrown:\n' + emsg)
        else:
            self.fail('ExpectedException not thrown')

    def testSetter(self):
        try:
            m = cp.PyMeasurement()
            m.set('speed', 12.345, 0.987, 'm/s')
            m.set('speed', 12.345, 0.987, cp.PyUnit('m/s'))
            m.set(u'speed', 12.345, 0.987, u'm/s')
            m.set('speed', 'bla', 0.987, 'm/s')  # this should raise TypeError
        except TypeError:
            pass
        except:
            emsg = eprint()
            self.fail('Unexpected exception thrown:\n' + emsg)
        else:
            self.fail('ExpectedException not thrown')

    def testSetterUnknownUnit(self):
        try:
            m = cp.PyMeasurement()
            m.set('speed', 12.345, 0.987, 1000)  # this should raise TypeError
        except TypeError:
            pass
        except:
            emsg = eprint()
            self.fail('Unexpected exception thrown:\n' + emsg)
        else:
            self.fail('ExpectedException not thrown')

    def testPrintMode(self):
        try:
            m = cp.PyMeasurement()
            m.set('speed', 12.345, 0.987, 'm/s')
            self.assertEqual(m.asString(mode=cp.measurement_default), '12.345 ± 0.987 m s⁻¹')
            self.assertEqual(m.asString(mode=cp.measurement_compact), '12.345 m s⁻¹')
            self.assertEqual(m.asString(mode=cp.measurement_full), 'speed = 12.345 ± 0.987 m s⁻¹')
            self.assertEqual(m.asString(mode=cp.measurement_unit), 'm s⁻¹')
            m.asString(mode=1000)  # this should raise TypeError
        except TypeError:
            pass
        except:
            emsg = eprint()
            self.fail('Unexpected exception thrown:\n' + emsg)
        else:
            self.fail('ExpectedException not thrown')

    def testConvert(self):
        try:
            m = cp.PyMeasurement()
            m.set('distance', 1., 0.1, cp.unit_kpc)
            self.assertEqual(m.convert(cp.unit_mpc), (0.001, 0.0001))
            m.convert(1000)  # this should raise TypeError
        except TypeError:
            pass
        except:
            emsg = eprint()
            self.fail('Unexpected exception thrown:\n' + emsg)
        else:
            self.fail('ExpectedException not thrown')

    def testInvert(self):
        try:
            m = cp.PyMeasurement()
            m.set('distance', 10., 0.1, 'm')
            m.invert()
            self.assertEqual(m.asString(), '0.1 ± 0.001 m⁻¹')
            m.set('distance', 0., 0.1, 'm')
            m.invert()  # this should raise ZeroDivisionError
        except ZeroDivisionError:
            pass
        except:
            emsg = eprint()
            self.fail('Unexpected exception thrown:\n' + emsg)
        else:
            self.fail('ExpectedException not thrown')

    def testMathOps(self):
        try:
            m1 = cp.PyMeasurement()
            m2 = cp.PyMeasurement()
            m1.set('speed', 12.345, 0.987, 'm/s')
            m2.set('distance', 0.55, 0.10, 'cm')
            self.assertEqual((m1*m2).asString(), '6.78975 ± 1.34858 × 10⁻² m² s⁻¹')
            self.assertEqual((-m1).asString(), '-12.345 ± 0.987 m s⁻¹')
            m1+m2  # this should raise TypeError
        except TypeError:
            pass
        except:
            emsg = eprint()
            self.fail('Unexpected exception thrown:\n' + emsg)
        else:
            self.fail('ExpectedException not thrown')

    def testCompOps(self):
        try:
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
            m1 == m3  # this should raise TypeError
        except TypeError:
            pass
        except:
            emsg = eprint()
            self.fail('Unexpected exception thrown:\n' + emsg)
        else:
            self.fail('ExpectedException not thrown')



def main():
    unittest.main()

if __name__ == '__main__':
    main()

