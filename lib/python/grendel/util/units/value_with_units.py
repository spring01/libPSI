from abc import ABCMeta, abstractproperty, abstractmethod
from numbers import Real
from grendel import type_checking_enabled
from grendel.util.aliasing import function_alias
from grendel.util.decorators import typechecked
from grendel.util.units.errors import UnknownUnitError, UnitizedObjectError
from grendel.util.units.unit import isunit

__all__ = [
    'hasunits', 'has_units',
    'strip_units', 'stripunits',
    'ValueWithUnits', 'Unitized'
]

#####################
# Utility functions #
#####################

def has_units(obj):
    return hasattr(obj, 'units') and obj.units is not None
hasunits = function_alias('hasunits', has_units)

# TODO Document this
# TODO optional kwarg to strip units after converting to default units (which is non-trivial for composite units)
@typechecked(
    convert_to=(None, isunit),
    assume_units=(None, isunit))
def strip_units(obj, convert_to=None, assume_units=None):
    """ Strips the units off of a unitized object
    (or, if the object is not `Unitized`, just return it).  If `convert_to` is given,
    convert to these units if `obj` is `Unitized`.  If `assume_units` is given, `obj` is
    assumed to have the units given by this argument if (and only if) it is not an
    instance of `Unitized`.
    """
    if assume_units and not convert_to:
        raise TypeError("call of strip_units with 'assume_units'"
                        " argument must also have a 'convert_to' argument"
                        " (otherwise, the assumption of units would make no sense.)")
    if hasunits(obj):
        if hasattr(obj, 'value'):
            if convert_to is not None:
                if hasattr(obj, 'in_units'):
                    return obj.in_units(convert_to).value
                else:
                    try:
                        return (obj * obj.units.to(convert_to)).value
                    except Exception as e:
                        raise UnitizedObjectError(
                            "don't known how to convert {} to {}, since "
                            " the former has no 'in_units' method and can't be "
                            " multiplied by a float to return an object that has"
                            " a 'value' attribute. (error was raised: {})".format(obj, convert_to, e))
            else:
                return obj.value
        else:
            raise UnitizedObjectError("Don't know how to get value of {}".format(obj))
    else:
        # It doesn't have units, just return it as is.
        if assume_units and convert_to:
            return obj * assume_units.to(convert_to)
        return obj
stripunits = function_alias('stripunits', strip_units)

class Unitized(object):
    """ Abstract base class for things with units.
    It requires its subclasses to implement the properties 'value' and 'units'.  Unitized subclasses
    are also expected have `__mul__`, `__div__`, `__truediv__`, `__pow__`, `__add__`, `__sub__`, `__neg__`,
    `__pos__`, and `__abs__` handle units correctly, when implemented in the given subclass.
    """
    __metaclass__ = ABCMeta
    #TODO __add_units__, __mul_units__, etc helper methods

    @abstractproperty
    def value(self):
        return NotImplemented

    @abstractproperty
    def units(self):
        return NotImplemented

    @abstractmethod
    def in_units(self, new_units):
        return NotImplemented

    @classmethod
    def __subclasshook__(cls, subclass):
        if hasattr(subclass, 'units') and hasattr(subclass, 'in_units') and hasattr(subclass, 'value'):
            return True
        else:
            return False


class ValueWithUnits(float, Unitized):
    """ A class for encapsulating a physical constant and it's units.

    Attributes
    ----------
    units : `CompositeUnit` or a class with `Unit` as its metaclass
        The units in which the value of `self` is to be interpreted

    """

    ##############
    # Attributes #
    ##############

    units = None


    ##################
    # Initialization #
    ##################

    def __new__(cls, value, units):
        if not isinstance(value, complex):
            ret_val = float.__new__(ValueWithUnits, value)
        else:
            # Ignore units, we've got bigger problems
            return complex.__new__(complex, value)
        if type_checking_enabled and not isunit(units):
            raise UnknownUnitError(units)
        ret_val.units = units
        return ret_val

    def __init__(self, value, units):
        super(ValueWithUnits, self).__init__(value)


    ##############
    # Properties #
    ##############

    @property
    def value(self):
        return float.__new__(float, self)


    ###################
    # Special Methods #
    ###################

    #-----------------------#
    # Arithmatic Operations #
    #-----------------------#

    def __mul__(self, other):
        if isinstance(other, ValueWithUnits):
            return ValueWithUnits(self.value.__mul__(other.value), self.units * other.units)
        elif isunit(other):
            return ValueWithUnits(self.value, self.units * other)
        else:
            return ValueWithUnits(self.value.__mul__(other), self.units)

    def __rmul__(self, other):
        if isinstance(other, ValueWithUnits): # pragma: no cover
            # Only necessary if a subclass doesn't override __mul__ or something weird like that
            return ValueWithUnits(self.value.__mul__(other.value), self.units * other.units)
        else:
            return ValueWithUnits(self.value.__mul__(other), self.units)

    def __div__(self, other):
        if isinstance(other, ValueWithUnits):
            return ValueWithUnits(self.value.__div__(other.value), self.units / other.units)
        elif isunit(other):
            return ValueWithUnits(self.value, self.units / other)
        else:
            return ValueWithUnits(self.value.__div__(other), self.units)
    __truediv__ = __div__

    def __rdiv__(self, other):
        if isinstance(other, ValueWithUnits): # pragma: no cover
            # Only necessary if a subclass doesn't override __div__ or something weird like that
            return ValueWithUnits(other.value / self.value, other.units / self.units)
        else:
            return ValueWithUnits(other / self.value, self.units**-1)
    __rtruediv__ = __rdiv__

    def __add__(self, other):
        if isinstance(other, ValueWithUnits):
            conv = other.in_units(self.units)
            return ValueWithUnits(self.value.__add__(conv.value), self.units)
        else:
            return NotImplemented

    def __sub__(self, other):
        if isinstance(other, ValueWithUnits):
            conv = other.in_units(self.units)
            return ValueWithUnits(self.value.__sub__(conv.value), self.units)
        else:
            return NotImplemented

    #------------------------#
    # Output Representations #
    #------------------------#

    def __str__(self):
        return super(ValueWithUnits, self).__str__() + " " + str(self.units)
    __repr__ = __str__


    ###########
    # Methods #
    ###########

    def in_units(self, units):
        if not isunit(units):
            raise UnknownUnitError(units)
        return ValueWithUnits(self.value * self.units.to(units), units)
    convert_to = in_units