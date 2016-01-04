DbModel
=======

Provide Qt models based on DbStruct pile.

Introduction
------------

The central structure provided by this
pile is the DbModel class. It is a Qt model
that can be installed in appropriate widgets
but especially in tables.

Implementation
--------------

The central model uses a series of helper classes:
- DbModelPrivate is a QAbstractTableModel incarnation
that is in charge with providing the actual data;
in this context DbModel acts only as a wrapper
for sorting and filtering;
- DbModelTbl and DbModelCol wrap corresponding
classes from DbStruct pile;
- DbModelManager holds common resources used by 
all DbModel instances.
