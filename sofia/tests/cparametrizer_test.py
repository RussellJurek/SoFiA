#! /usr/bin/env python

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


class PyUnitInterfaceTests(unittest.TestCase):
    """This tests the cython *interface* of PyUnit (wrapping Unit)"""

    def testConstructorWorking(self):
        try:
            u1 = cp.PyUnit()
            u2 = cp.PyUnit('cm')
            u3 = cp.PyUnit(u2)
        except:
            emsg = eprint()
            self.fail('Unexpected exception thrown:\n' + emsg)

    def testConstructorUnicodeWorking(self):
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
            u = cp.PyUnit(1)
        except TypeError:
            pass
        except:
            emsg = eprint()
            self.fail('Unexpected exception thrown:\n' + emsg)
        else:
            self.fail('ExpectedException not thrown')

    def testSetterWorking(self):
        try:
            u1 = cp.PyUnit()
            u1.set('kg')
            u1.set('m')
        except:
            emsg = eprint()
            self.fail('Unexpected exception thrown:\n' + emsg)

    def testSetterUnicodeWorking(self):
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
            u1 = cp.PyUnit()
            u1.set(1)
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

