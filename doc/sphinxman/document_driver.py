#!/usr/bin/python

import sys
import os
import glob
import re


DriverPath = ''
InsertPath = '/../../../'
if (len(sys.argv) == 2):
    DriverPath = sys.argv[1] + '/'
    sys.path.insert(0, os.path.abspath(os.getcwd()))
    import apply_relpath
    IncludePath = apply_relpath.get_topsrcdir_asrelativepathto_objdirsfnxsource()[1]


def pts(category, pyfile):
    print 'Auto-documenting %s file %s' % (category, pyfile)


# Main driver modules in psi4/lib/python
fdriver = open('source/autodoc_driver.rst', 'w')
fdriver.write('\n.. include:: /autodoc_abbr_options_c.rst\n\n')
fdriver.write('.. _`sec:driver`:\n\n')
fdriver.write('=============\n')
fdriver.write('Python Driver\n')
fdriver.write('=============\n\n')

for pyfile in glob.glob(DriverPath + '../../lib/python/*.py'):
    filename = os.path.split(pyfile)[1]
    basename = os.path.splitext(filename)[0]
    div = '=' * len(basename)

    if basename not in ['inpsight', 'pep8', 'sampmod', 'psifiles', 'diatomic_fits', 'pyparsing']:

        pts('driver', basename)
        fdriver.write(basename + '\n')
        fdriver.write(div + '\n\n')
    
        fdriver.write('.. automodule:: %s\n' % (basename))
        fdriver.write('   :members:\n')
        fdriver.write('   :undoc-members:\n')
    
        if basename == 'driver':
            fdriver.write('   :exclude-members: energy, optimize, opt, response, frequency, frequencies, freq, property, prop\n')
        elif basename == 'wrappers':
            fdriver.write('   :exclude-members: nbody, cp, counterpoise_correct, counterpoise_correction,\n')
            fdriver.write('       db, database, cbs, complete_basis_set, highest_1, scf_xtpl_helgaker_3,\n')
            fdriver.write('       scf_xtpl_helgaker_2, corl_xtpl_helgaker_2, n_body\n')
        elif basename == 'physconst':
            fdriver.write('\n.. literalinclude:: %slib/python/%s\n' % (IncludePath, filename))
        elif basename == 'diatomic':
            fdriver.write('   :exclude-members: anharmonicity\n')
        elif basename == 'molutil':
            fdriver.write('   :exclude-members: run_dftd3\n')
        elif basename == 'aliases':
            fdriver.write('   :exclude-members: sherrill_gold_standard, allen_focal_point\n')

    fdriver.write('\n')


# Python-only plugin modules in psi4/lib/python
for basename in os.walk(DriverPath + '../../lib/python').next()[1]:
    div = '=' * len(basename)

    if basename not in []:

        pts('driver', basename)
        fdriver.write(basename + '\n')
        fdriver.write(div + '\n\n')
    
        fdriver.write('.. automodule:: %s\n' % (basename))
        fdriver.write('   :members:\n')
        fdriver.write('   :undoc-members:\n')
    
        for pyfile in glob.glob(DriverPath + '../../lib/python/' + basename + '/*py'):
            filename = os.path.split(pyfile)[1]
            basename2 = os.path.splitext(filename)[0]
            div = '=' * len(basename2)

            fdriver.write('.. automodule:: %s.%s\n' % (basename, basename2))
            fdriver.write('   :members:\n')
            fdriver.write('   :undoc-members:\n')
    
        fdriver.write('\n')
    fdriver.write('\n')
fdriver.close()

