from setuptools import Extension, setup

module_name = "symnmfmodule"

module = Extension(module_name,
                   sources=[
                       'symnmfmodule.c',
                       'symnmf.c',
                       'libs/matrix.c'
                   ])
setup(name=module_name,
      version='1.0',
      description='A clustering algorithm that is based on Symmetric Non-negative Matrix Factorization (symNMF)',
      ext_modules=[module])
