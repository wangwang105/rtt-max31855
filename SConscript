Import('rtconfig')
from building import *

cwd = GetCurrentDir()

src = Glob('*.c')
inc = [cwd]

CXXFLAGS = ''

group = DefineGroup('apid', src, depend = ['PKG_USING_MAX31855'], CPPPATH = inc, CXXFLAGS = CXXFLAGS)

Return('group')

