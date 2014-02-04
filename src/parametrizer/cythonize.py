# Cythonize .pyx if Cython is available
try:
    from Cython.Build import cythonize
    
    pyx_src = [
        'catalog.pyx',
        'parametrizer.pyx',
        'parametrizer2.pyx'
    ]
    
    cythonize(pyx_src)
    
except ImportError:
    pass