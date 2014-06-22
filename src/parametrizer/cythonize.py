# Cythonize .pyx if Cython is available
try:
    from Cython.Build import cythonize

    pyx_src = [
        'cparametrizer.pyx'
        ]

    cythonize(pyx_src)

except ImportError:
    pass
