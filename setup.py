from distutils.core import setup, Extension
from distutils.version import StrictVersion, LooseVersion

import sys, os, stat
import warnings

from sofia import __version__ as version


# Dependency checking
dependencies = [
    ['numpy', "1.7"],
    ['scipy', None],
    ['pyfits', None]
]

for pkg, minversion in dependencies:
    try:
        m = __import__(pkg)
        if minversion is not None:
            if StrictVersion(m.__version__) < StrictVersion(minversion):
                if LooseVersion(m.__version__) < LooseVersion(minversion):
                    raise ValueError
                warnings.warn("Version", m.__version__, "of package", pkg, "might not be sufficient")
    except ImportError:
        print "Package", pkg, "not present."
        sys.exit(1)
    except ValueError:
        print "Package", pkg, "has version", m.__version__
        print "Version", minversion, "required."
        sys.exit(1)

import numpy

# Define include directories
include_dirs = []
include_dirs.append('src')
include_dirs.append(numpy.get_include())

# Get external include directories from PATH
ext_include_dirs = []
ext_library_dirs = []
try:
    ext_include_dirs += os.environ['PATH'].split(os.pathsep)
except:
    print "Could not parse PATH"
    sys.exit(1)

# Get external library paths from LD_LIBRARY_PATH, if it exists
try:
    ext_library_dirs += os.environ['LD_LIBRARY_PATH'].split(os.pathsep)
except KeyError:
    pass

if sys.platform == 'darwin':
    # Remove "bin" directories to avoid problems with clang
    ext_include_dirs = [ext_include_dir for ext_include_dir in ext_include_dirs if not "bin" in ext_include_dir]

# C/C++ source code files
# Object linking code
linker_src_base = 'src/linker/'
linker_src_files = [
    "linker.cpp",
    "RJJ_ObjGen_CreateObjs.cpp",
    "RJJ_ObjGen_DetectDefn.cpp",
    "RJJ_ObjGen_MemManage.cpp",
    "RJJ_ObjGen_ThreshObjs.cpp"
]
linker_src = [linker_src_base + f for f in linker_src_files]

# Interface to the parametrization2 code
parametrizer_src_base = 'src/parametrizer/'
parametrizer_src_files = [
    "helperFunctions.cpp",
    "cparametrizer.cpp",
    "ModuleParametrisation.cpp",
    "Unit.cpp",
    "Measurement.cpp",
    "Source.cpp",
    "SourceCatalog.cpp",
    "MaskOptimization.cpp",
    "BusyFit.cpp",
    "DataCube.cpp",
    "MetaData.cpp",
    "Parametrization.cpp",
    #"WorldCoordinateSystem.cpp"
]
parametrizer_src = [parametrizer_src_base + f for f in parametrizer_src_files]

setup(
    name = "sofia",
    version = version,
    ext_package = 'sofia',
    ext_modules = [
        Extension(
            "linker",
            linker_src,
            extra_compile_args = ["-O3"],
            include_dirs = include_dirs
        ),
        Extension(
            "cparametrizer",
            parametrizer_src,
            extra_compile_args = ["-O3"],
            include_dirs = include_dirs + ext_include_dirs,
            library_dirs = ext_library_dirs + ext_include_dirs,
            libraries = [
                "gsl",
                "gslcblas"#,
                #"cfitsio",
                #"wcs"
            ]
        )
    ],
    package_dir = {'sofia' : 'sofia'},
    packages = ['sofia']
)



# make sofia_pipeline.py executable
os.chmod('sofia_pipeline.py', os.stat('sofia_pipeline.py').st_mode | stat.S_IXUSR)
# path to the sofia modules
cwd = os.getcwd()
for ll in os.listdir(cwd+'/build'):
  if 'lib.' in ll:
    sofiaModulesPath = cwd+'/build/'+ll

# set system variable
os.environ['SOFIA_PIPELINE_PATH'] = cwd+'/sofia_pipeline.py'
print cwd+'/sofia_pipeline.py'
print os.environ['SOFIA_PIPELINE_PATH']

# compile the SoFiA gui
os.chdir('gui')
os.system('qmake; make')
if sys.platform=='darwin':
	os.chmod('SoFiA.app/Contents/MacOS/SoFiA', os.stat('SoFiA.app/Contents/MacOS/SoFiA').st_mode | stat.S_IXUSR)
	sofiaApplicationPath = cwd+'/gui/SoFiA.app/Contents/MacOS'
else:
	os.chmod('SoFiA', os.stat('SoFiA').st_mode | stat.S_IXUSR)
	sofiaApplicationPath = cwd+'/gui'
os.chdir('../')

## compile the SoFiA gui
#os.chdir('gui')
#os.system('qmake; make')
#os.chmod('SoFiA', os.stat('SoFiA').st_mode | stat.S_IXUSR)
#os.chdir('../')
#sofiaApplicationPath = cwd+'/gui'

print '\n'
print '-------------------------------------------------------------------------'
print '\n'
print 'Installation done.'
print '\n'
print 'Please add the following lines to your shell configuration file:'
print '  for bash (~/.bashrc)'
print '      export SOFIA_MODULE_PATH='+sofiaModulesPath
print '      export SOFIA_PIPELINE_PATH=\"'+cwd+'/sofia_pipeline.py\"'
print '      export PATH=$PATH:'+sofiaApplicationPath+':'+cwd
print '  for (t)csh (~/.cshrc)'
print '      setenv SOFIA_MODULE_PATH '+sofiaModulesPath
print '      setenv SOFIA_PIPELINE_PATH \"'+cwd+'/sofia_pipeline.py\"'
print '      setenv PATH {$PATH}:'+sofiaApplicationPath+':'+cwd

print 



