
.. include:: autodoc_abbr_options_c.rst

======================================
Installation and Runtime Configuration
======================================

Obtaining |PSIfour|
===================

The latest version of the |PSIfour| program package may be obtained at
`www.psicode.org <http://www.psicode.org>`_.  The
source code is available as a gzipped tar archive (named, for example,
``psi4.X.tar.gz``, and binaries may be available for certain architectures.
For detailed installation and testing instructions, please refer to
:ref:`Compiling and Installing <sec:installFilePage>` (same information as 
the file :source:`INSTALL` distributed with the package).

.. comment To avoid dependency problems, a script is provided at
.. comment :source:`lib/scripts/psi4depend-v2.sh` that will download, configure, and
.. comment install all dependencies that the Psi4 developers currently recommend.
.. comment This script simply needs one to edit the top lines to tell it where to
.. comment install all the files to and for the user to put this into their path. The
.. comment script will print out the lines that you need to add to your
.. comment :envvar:`path` and :envvar:`LD_LIBRARY_PATH`. This script will build the
.. comment following: autoconf-2.68, automake-1.11, gcc-4.1.2, mpich2-1.2.1,
.. comment Python-2.6.6, Boost-1.48.0 (as well as some dependencies). After running
.. comment the script, proceed with building |PSIfour| as described in
.. comment :source:`INSTALL`.


.. index:: prerequisites, compiling, installing
.. _`sec:installFile`:

Compiling and Installing
========================

* :ref:`I.   Compilation Prerequisites                                     <sec:install_I>`
* :ref:`II.  Brief Summary of Configuration, Compilation, and Installation <sec:install_II>`
* :ref:`III. Detailed Installation Instructions                            <sec:install_III>`
* :ref:`IV.  Recommendations for BLAS and LAPACK libraries                 <sec:install_IV>`
* :ref:`V.   Miscellaneous architecture-specific notes                     <sec:install_V>`
* :ref:`VI.  Common Problems with PSI Compilation                          <sec:install_VI>`


.. index:: scratch files, psirc, psi4rc
.. _`sec:psirc`:

Scratch Files and the |psirc| File
==================================

One very important part of user configuration at the end of the
installation process (:ref:`details here <sec:install_III_7>`)
is to tell |PSIfour| where to write its temporary
("scratch") files.  Electronic structure packages like |PSIfour| can
create rather large temporary disk files.  It is very important to 
ensure that |PSIfour| is writing its temporary files to a disk drive
phsyically attached to the computer running the computation.  If it
is not, it will significantly slow down the program and the network.
By default, |PSIfour| will write temporary files to ``/tmp``, but this
directory is often not large enough for typical computations.  Therefore,
you need to (a) make sure there is a sufficiently large directory on a
locally attached disk drive (100GB–1TB or more, depending on the size of
the molecules to be studied) and (b) tell |PSIfour| the path to this
directory. Scratch file location can be specified through the 
:envvar:`PSI_SCRATCH` environment variable or, more flexibly, through 
a resource file, |psirc| (example :source:`samples/example_psi4rc_file`). 

For convenience, the Python interpreter will execute the contents of the
|psirc| file in the current user's home area (if present) before performing any
tasks in the input file. The primary use of the |psirc| file is to control the
handling of scratch files.  |PSIfour| has a number of utilities that manage
input and output (I/O) of quantities to and from the hard disk.  Most
quantities, such as molecular integrals, are intermediates that are not of
interest to the user and can be deleted after the computation finishes, but
pertinent details of computations are also written to a checkpoint file and
might be useful in subsequent computations.  All files are written to the
designated scratch numbered by :ref:`content <apdx:psiFiles>` and labeled
with the process id, then are deleted at the end of the computation,
unless otherwise instructed by the user.

A Python callable handle to the |PSIfour| I/O management routines is available,
and is called ``psi4_io``.  To instruct the I/O manager to send all files to
another location, say ``/scratch/user``, add the following command to the |psirc|
file.::

    psi4_io.set_default_path('/scratch/user')

For batch jobs running through a queue, it might be more convenient to use an
environmental variable (in this case ``$MYSCRATCH``) to set the scratch directory;
the following code will do that::

    import os
    scratch_dir = os.environ.get('MYSCRATCH')
    if scratch_dir:
        psi4_io.set_default_path(scratch_dir + '/')

Individual files can be sent to specific locations.  For example, file 32 is
the checkpoint file that the user might want to retain in the working directory
(*i.e.*, where |PSIfour| was launched from) for restart purposes.  This is
accomplished by the commands below::

    psi4_io.set_specific_path(32, './')
    psi4_io.set_specific_retention(32, True)

which is equivalent to ::

    psi4_io.set_specific_path(PSIF_CHKPT, './')
    psi4_io.set_specific_retention(PSIF_CHKPT, True)

A guide to the contents of individual scratch files may be found at :ref:`apdx:psiFiles`.
To circumvent difficulties with running multiple jobs in the same scratch, the
process ID (PID) of the |PSIfour| instance is incorporated into the full file
name; therefore, it is safe to use the same scratch directory for calculations
running simultaneously.

To override any of these defaults for selected jobs, simply place the
appropriate commands from the snippets above in the input file itself.  During
excecution, the |psirc| defaults will be loaded in first, but then the commands
in the input file will be executed.  Executing |PSIfour| with the :option:`psi4 -m` (for
messy) flag will prevent files being deleted at the end of the run::

    psi4 -m

Alternately, the scratch directory can be set through the environment
variable :envvar:`PSI_SCRATCH` (overrides |psirc| settings). (First line
for C shell; second line for bash.) ::

     setenv PSI_SCRATCH /scratch/user
     export PSI_SCRATCH=/scratch/user

The |psirc| file can also be used to define constants that are accessible
in input files or to place any Python statements that should be executed
with every |PSIfour| instance.

.. index:: parallel operation, threading
.. _`sec:threading`:

Threading
=========

Most new modules in |PSIfour| are designed to run efficiently on SMP architectures
via application of several thread models. The de facto standard for |PSIfour|
involves using threaded BLAS/LAPACK (particularly Intel's excellent MKL package)
for most tensor-like operations, OpenMP for more general operations, and Boost
Threads for some special-case operations. Note: Using OpenMP alone is a really
bad idea. The developers make little to no effort to explicitly parallelize
operations which are already easily threaded by MKL or other threaded BLAS. Less
than 20% of the threaded code in |PSIfour| uses OpenMP, the rest is handled by
parallel DGEMM and other library routines. From this point forward, it is
assumed that you have compiled |PSIfour| with OpenMP and MKL (Note that it is
possible to use g++ or another compiler and yet still link against MKL).

Control of threading in |PSIfour| can be accomplished at a variety of levels,
ranging from global environment variables to direct control of thread count in
the input file, to even directives specific to each model. This hierarchy is
explained below. Note that each deeper level trumps all previous levels.

.. rubric:: (1) OpenMP/MKL Environment Variables

The easiest/least visible way to thread |PSIfour| is to set the standard OpenMP/MKL
environment variables :envvar:`OMP_NUM_THREADS` and :envvar:`MKL_NUM_THREADS`. 
For instance, in tcsh::

    setenv OMP_NUM_THREADS 4
    setenv MKL_NUM_THREADS 4

|PSIfour| then detects these value via the API routines in ``<omp.h>`` and
``<mkl.h>``, and runs all applicable code with 4 threads. These environment
variables are typically defined in a ``.tcshrc`` or ``.bashrc``.

.. rubric:: (2) The -n Command Line Flag

To change the number of threads at runtime, the :option:`psi4 -n` flag may be used. An
example is::

    psi4 -i input.dat -o output.dat -n 4

which will run on four threads.

.. rubric:: (3) Setting Thread Numbers in an Input

For more explicit control, the Process::environment class in |PSIfour| can
override the number of threads set by environment variables. This functionality
is accessed via the :py:func:`~util.set_num_threads` Psithon function, which controls
both MKL and OpenMP thread numbers. The number of threads may be changed
multiple times in a |PSIfour| input file. An example input for this feature is::

    # A bit small-ish, but you get the idea
    molecule h2o {
    0 1
    O
    H 1 1.0
    H 1 1.0 2 90.0
    }
    
    set scf {
    basis cc-pvdz
    scf_type df
    }

    # Run from 1 to 4 threads, for instance, to record timings
    for nthread in range(1,5):
        set_num_threads(nthread)
        energy('scf')

.. rubric:: (4) Method-Specific Control

Even more control is possible in certain circumstances. For instance, the
threaded generation of AO density-fitted integrals involves a memory requirement
proportional to the number of threads. This requirement may exceed the total
memory of a small-memory node if all threads are involved in the generation of
these integrals. For general DF algorithms, the user may specify::

    set MODULE_NAME df_ints_num_threads n

to explicitly control the number of threads used for integral formation. Setting
this variable to 0 (the default) uses the number of threads specified by the
:py:func:`~util.set_num_threads` Psithon method or the default environmental variables.

.. _`sec:commandLineOptions`:

Command Line Options
====================

|PSIfour| can be invoked with no command line arguments, as it takes as input
by default the file "input.dat" and directs output by default to "output.dat".
The set of three commands below are completely equivalent, while the fourth is,
perhaps, the most common usage. ::

   psi4
   psi4 -i input.dat -o output.dat
   psi4 input.dat output.dat
   
   psi4 descriptive_filename.in descriptive_filename.out

Command-line arguments to |PSIfour| can be accessed through :option:`psi4 --help`.

.. program:: psi4

.. option:: -a, --append

   Append results to output file. Default: Truncate first

.. option:: -h, --help

   Display the command-line options and usage information.

.. option:: -i <filename>, --input <filename>

   Input file name. Default: input.dat

.. option:: -o <filename>, --output <filename>

   Output file name. Use ``stdout`` as <filename> to redirect 
   to the screen. Default: when the input filename is "input.dat",
   then the output filename defaults to "output.dat".  Otherwise, the
   output filename defaults to the the input filename (subtracting
   any ".in" or ".dat" suffix) plus ".out"

.. option:: -m, --messy

   Leave temporary files after the run is completed.

.. option:: -n <threads>, --nthread <threads>

   Number of threads to use (overrides :envvar:`OMP_NUM_THREADS`)

.. option:: --new-plugin <name>

   Creates a new directory <name> with files for writing a
   new plugin. An additional argument specifies a template
   to use, for example: ``--new-plugin name +mointegrals``.
   See Sec. :ref:`sec:plugins` for available templates.

.. option:: -p <prefix>, --prefix <prefix>

   Prefix for psi files. Default: psi

.. option:: -v, --verbose

   Print a lot of information, including the Psithon translation of the input file

.. option:: -d, --debug

   Flush the outfile at every fprintf. Default: true iff ``--with-debug``

.. option:: -V, --version

   Print version information.

.. option:: -w, --wipe

   Clean out scratch area.


.. _`sec:environmentVariables`:

Environment Variables
=====================

These environment variables will influence |PSIfours| behavior.

.. envvar:: MKL_NUM_THREADS

   Number of threads to use by operations with Intel threaded BLAS libraries.

.. envvar:: OMP_NESTED

   Do access nested DGEMM in OpenMP sections in DFMP2 for multi-socket
   platforms. This is very low-level access to OpenMP functions for
   experienced programmers. Users should leave this variable unset or set
   to ``False``.

.. envvar:: OMP_NUM_THREADS

   Number of threads to use by modules with OpenMP threading.

.. envvar:: PATH

   Path for executables. To run K\ |a_acute|\ llay's MRCC program 
   (see :ref:`MRCC <sec:mrcc>`), the ``dmrcc`` executable must be in :envvar:`PATH`.
   Likewise to run Grimme's dftd3 program (see :ref:`dftd3 <sec:dftd3>`), the 
   ``dftd3`` executable must be in :envvar:`PATH`.

.. envvar:: PSI_SCRATCH

   Directory where scratch files are written. Overrides settings in |psirc|.

.. envvar:: PYTHONPATH

   Path in which the Python interpreter looks for modules to import. For 
   |PSIfour|, these are generally plugins (see :ref:`sec:plugins`) or databases.

   Modification of :envvar:`PYTHONPATH` can be done in three ways, equivalently.

   * Normal Linux shell commands. First line for C shell; second for bash. ::

        setenv PYTHONPATH /home/user/psiadditions:$PYTHONPATH
        PYTHONPATH=/home/user/psiadditions:$PYTHONPATH; export PYTHONPATH

   * Place the path in the |psirc| file so that it is available for 
     every |PSIfour| instance. ::

        sys.path.insert(0, '/home/user/psiadditions')

   * Place the path in the input file, either absolute or relative. ::

        sys.path.insert(0, '../../psiadditions')
        sys.path.insert(0, '/home/user/psiadditions')

