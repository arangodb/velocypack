#!/usr/bin/env python
"""
setup.py file for SWIG example
"""
from distutils.core import setup, Extension
jason_module = Extension('_jason',
 sources=['jason_wrap.cxx', 'jason.cpp'],
 )
setup (name = 'jason',
 version = '0.1',
 author = "Max Neunhoeffer",
 description = """Access to the jason libs""",
 ext_modules = [jason_module],
 py_modules = ["jason"],
 )
I
