
.. include:: autodoc_abbr_options_c.rst

.. index::
   single: CC, coupled cluster
   pair: CC; theory

.. _`sec:cc`:

CC: Coupled Cluster Methods
===========================

.. codeauthor:: T. Daniel Crawford
.. sectionauthor:: T. Daniel Crawford

*Module:* :ref:`Keywords <apdx:ccenergy>`, :ref:`PSI Variables <apdx:ccenergy_psivar>`, :source:`CCENERGY <src/bin/ccenergy>`

*Module:* :ref:`Keywords <apdx:cceom>`, :ref:`PSI Variables <apdx:cceom_psivar>`, :source:`CCEOM <src/bin/cceom>`

*Module:* :ref:`Keywords <apdx:ccresponse>`, :ref:`PSI Variables <apdx:ccresponse_psivar>`, :source:`CCRESPONSE <src/bin/ccresponse>`

*Module:* :ref:`Keywords <apdx:cctriples>`, :ref:`PSI Variables <apdx:cctriples_psivar>`, :source:`CCTRIPLES <src/bin/cctriples>`

*Module:* :ref:`Keywords <apdx:ccdensity>`, :ref:`PSI Variables <apdx:ccdensity_psivar>`, :source:`CCDENSITY <src/bin/ccdensity>`

*Module:* :ref:`Keywords <apdx:cchbar>`, :source:`CCHBAR <src/bin/cchbar>`

*Module:* :ref:`Keywords <apdx:cclambda>`, :source:`CCLAMBDA <src/bin/cclambda>`

*Module:* :ref:`Keywords <apdx:ccsort>`, :source:`CCSORT <src/bin/ccsort>`

The coupled cluster approach is one of the most accurate and reliable quantum
chemical techniques for including the effects of electron correlation.
Instead of the linear expansion of the wavefunction used by configuation
interaction, coupled cluster uses an exponential expansion,

.. math::
   :label: CCexpansion

   | \Psi \rangle &= e^{\hat{T}} | \Phi_0 \rangle \\
                  &= \left( 1 + {\hat{T}} + \frac{1}{2} {\hat{T}}^2 + \frac{1}{3!}{\hat{T}}^3 + \cdots \right) | \Phi_0 \rangle,

where the cluster operator :math:`{\hat{T}}` is written as a sum of operators that
generate singly-excited, doubly-excited, *etc.*, determinants:

.. math:: {\hat{T}} = {\hat{T}_1} + {\hat{T}_2} + {\hat{T}_3} + \cdots + {\hat{T}_N},

with

.. math:: 

   {\hat T}_1 | \Phi_0 \rangle &= \sum_{i}^{\rm occ} \sum_a^{\rm vir} t_i^a | \Phi_i^a \rangle \\
   {\hat T}_2 | \Phi_0 \rangle &= \sum_{i<j}^{\rm occ} \sum_{a<b}^{\rm vir} t_{ij}^{ab} | \Phi_{ij}^{ab} \rangle, 

*etc.*  The popular coupled cluster singles and doubles (CCSD) model
[Purvis:1982]_ truncates the expansion at :math:`{\hat{T}} = {\hat{T}_1}
+ {\hat{T}_2}`.  This model has the same number of parameters as
configuration interaction singles and doubles (CISD) but improves upon
it by approximately accounting for higher-order terms using products
of lower-order terms (*e.g.*, the term :math:`{\hat{T}_2}^2` approximately
accounts for quadruple excitations).  The inclusion of such products
makes coupled-cluster methods *size extensive*, meaning that the
quality of the computation should not degrade for larger molecules.
The computational cost for CCSD scales as :math:`{\cal{O}}(o^2 v^4)`, where
:math:`o` is the number of occupied orbitals and :math:`v` is the number of virtual
orbitals.

Improving upon CCSD, the CCSD(T) method [Raghavachari:1989]_ includes
a perturbative estimate of the energy contributed by the :math:`{\hat{T}_3}`
operator.  The computational cost of this additional term scales as
:math:`{\cal{O}}(o^3 v^4)`, making it rather expensive for molecules with more than
a dozen heavy atoms or so.  However, when this method is affordable, it
provides very high quality results in most cases.

|PSIfour| is capable of computing energies and analytic gradients for a
number of coupled cluster models.  It can also compute linear response
properties (such as static or frequency-dependent polarizability,
or optical rotation angles) for some models.  Excited states can
also be computed by the CC2 and CC3 models, or by EOM-CCSD.  Table
:ref:`CC Methods <table:ccsummary>` summarizes these capabilities.  This section
describes how to carry out coupled cluster calculations within |PSIfour|.
For higher-order coupled-cluster methods like CCSDT and CCSDTQ, |PSIfour|
can interface to K\ |a_acute|\ llay's MRCC code (see :ref:`MRCC <sec:mrcc>`).

.. _`table:ccsummary`:

.. table:: Current coupled cluster capabilities of |PSIfour|

   +---------------+------------+-----------+-----------+---------------+-----------+
   | Method        | Reference  | Energy    | Gradient  | Exc. Energies | LR Props  |
   +===============+============+===========+===========+===============+===========+
   | CC2           | RHF        | Y         | ---       | Y             | Y         |
   +               +------------+-----------+-----------+---------------+-----------+
   |               | UHF        | Y         | ---       | Y             | ---       |
   +               +------------+-----------+-----------+---------------+-----------+
   |               | ROHF       | Y         | ---       | Y             | ---       |
   +---------------+------------+-----------+-----------+---------------+-----------+
   | CCSD          | RHF        | Y         | Y         | Y             | Y         |
   +               +------------+-----------+-----------+---------------+-----------+
   |               | UHF        | Y         | Y         | Y             | ---       |
   +               +------------+-----------+-----------+---------------+-----------+
   |               | ROHF       | Y         | Y         | Y             | ---       |
   +---------------+------------+-----------+-----------+---------------+-----------+
   | CCSD(T)       | RHF        | Y         | ---       | n/a           | n/a       |
   +               +------------+-----------+-----------+---------------+-----------+
   |               | UHF        | Y         | Y         | n/a           | n/a       |
   +               +------------+-----------+-----------+---------------+-----------+
   |               | ROHF       | Y         | ---       | n/a           | n/a       |
   +---------------+------------+-----------+-----------+---------------+-----------+
   | a-CCSD(T)     | RHF        | Y         | ---       | n/a           | n/a       |
   +---------------+------------+-----------+-----------+---------------+-----------+
   | CC3           | RHF        | Y         | ---       | Y             | ---       |
   +               +------------+-----------+-----------+---------------+-----------+
   |               | UHF        | Y         | ---       | Y             | ---       |
   +               +------------+-----------+-----------+---------------+-----------+
   |               | ROHF       | Y         | ---       | Y             | ---       |
   +---------------+------------+-----------+-----------+---------------+-----------+
   | CCD           | Brueckner  | Y         | N         | N             | N         |
   +---------------+------------+-----------+-----------+---------------+-----------+
   | CCD(T)        | Brueckner  | Y         | N         | n/a           | n/a       |
   +---------------+------------+-----------+-----------+---------------+-----------+

The following wavefunctions are currently recognized by |PSIfour| as arguments
to functions like :py:func:`~driver.energy`: ``'ccsd'``, ``'ccsd(t)'``, ``'a-ccsd(t)'``, ``'cc2'``,
``'cc3'``, ``'bccd'`` (CCD with Brueckner orbitals), ``'bccd(t)'`` (CCD(T) with
Brueckner orbitals), ``'eom-ccsd'``, ``'eom-cc2'`` (CC2 for excited states),
``'eom-cc3'`` (CC3 for excited states).  Response properties can be obtained
by calling the function :py:func:`~driver.property` (instead of, for example, :py:func:`~driver.energy`,
*e.g.*, ``property('ccsd')``.  There are many sample
coupled cluster inputs provided in :source:`samples`.

Basic Keywords
^^^^^^^^^^^^^^

A complete list of keywords related to coupled-cluster computations is
provided in the appendices, with the majority of the relevant
keywords appearing in Appendix :ref:`apdx:ccenergy`.  For a standard ground-state 
CCSD or CCSD(T) computation, the following keywords are common:

.. include:: autodir_options_c/ccenergy__reference.rst
.. include:: autodir_options_c/ccenergy__r_convergence.rst
.. include:: autodir_options_c/ccenergy__maxiter.rst
.. include:: autodir_options_c/ccenergy__brueckner_orbs_r_convergence.rst
.. include:: autodir_options_c/ccenergy__restart.rst
.. include:: autodir_options_c/ccenergy__cachelevel.rst
.. include:: autodir_options_c/ccenergy__cachetype.rst
.. include:: autodir_options_c/ccenergy__num_amps_print.rst
.. include:: autodir_options_c/ccenergy__mp2_amps_print.rst

Larger Calculations
^^^^^^^^^^^^^^^^^^^

Here are a few recommendations for carrying out large-basis-set coupled
cluster calculations with |PSIfour|: 

* In most cases it is reasonable to set the ``memory`` keyword to 90% of 
  the available physical memory, at most.  There is a small amount of overhead 
  associated with the
  coupled cluster modules that is not accounted for by the internal CC memory
  handling routines.  Thus, the user should *not* specify the entire
  physical memory of the system, or swapping is likely.  However, for especially large
  calculations, it is better to set the ``memory`` keyword to a value less than 16 GB.

* Set the |ccenergy__cachelevel| keyword to ``0``.
  This will turn off cacheing, which, for very large calculations, can
  lead to heap fragmentation and memory faults, even when sufficient
  physical memory exists.

* Set the |globals__print| keyword to ``2``.  This 
  will help narrow where memory bottlenecks or other errors exist in the 
  event of a crash.

.. _`sec:eomcc`:

Excited State Coupled Cluster Calculations
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

A complete list of keywords related to 
coupled cluster linear response is provided in Appendix :ref:`apdx:cceom`.
The most important keywords associated with EOM-CC calculations are:

.. include:: autodir_options_c/cceom__roots_per_irrep.rst
.. include:: autodir_options_c/cceom__e_convergence.rst
.. include:: autodir_options_c/cceom__singles_print.rst
.. include:: autodir_options_c/cceom__schmidt_add_residual_tolerance.rst
.. include:: autodir_options_c/cceom__eom_guess.rst

Linear Response (CCLR) Calculations
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Linear response computations are invoked like ``property('ccsd')``
or ``property('cc2')``, along with a list of requested properties.  
A complete list of keywords related to
coupled cluster linear response is provided in Appendix :ref:`apdx:ccresponse`.

The most important keywords associated with CC-LR calculations are as follows.

.. include:: autodir_options_c/ccresponse__property.rst
.. include:: autodir_options_c/ccresponse__omega.rst
.. include:: autodir_options_c/ccresponse__gauge.rst

