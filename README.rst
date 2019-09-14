======
bag_oa
======
**bag_oa** is a C++/Cython wrapper around the `OpenAccess
<http://www.si2.org/openaccess/>`_ API. You can use it to speed up layout
generation in BAG 2.

Installation
============
First, set the ``OA_INCLUDE_DIR`` and ``OA_LINK_DIR`` environment variables.

Cython Extension (cybagoa)
--------------------------
You only need this if you're working with BAG 2::

    $ ./build.sh

This will produce a ``cybagoa.cpython-*.so`` library somewhere in *build/*.
Move this to the right place to use it with BAG.

C++ Shared Library
------------------
::

    $ mkdir build && cd build/
    $ cmake ..
    $ make
