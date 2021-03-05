*************************
Contributions / Community
*************************

Acknowledgements
================

Created by
----------

	.. include:: ../README.rst
	   :start-after: created-start
	   :end-before:  created-end

Contributors
------------

	.. include:: ../README.rst
	   :start-after: contributors-start
	   :end-before:  contributors-end

Thanks to
---------

	.. include:: ../README.rst
	   :start-after: thanks-start
	   :end-before:  thanks-end


Contributing
============

We realise we've embarked on a large journey with a very small team and we could use some help with pretty much everything we're doing right now. Help is particularly welcome in the following areas:

* **Coding**: help us support new hardware or improve support for the existing hardware.

* **Documentation**: help document LoyvanGFX, or proofread existing documentation to help us improve it.

* **Suggestions**: Use LoyvanGFX for your own projects and tell us what's missing, or what could be done differently.

* **Sponsorship**: with your support, even more time can be spent coding LovyanGFX. Click `here <https://github.com/sponsors/lovyan03>`__ if you can help out financially. Yourhelp is much appreciated.

This project lives in its GitHub repository. The best way to talk to us about problems or suggestions regarding code or documentation is to file an issue in the repository.


Writing documentation
---------------------

You'll notice that the documentation for LoyvanGFX is displayed with the help of `Read the Docs <readthedocs.org>`_. They compile the version they display from the 'documentation' directory in our library. The source for the documentation is written in `reStructuredText` (.rst) format. To get started with that, check out `this primer <https://www.sphinx-doc.org/en/master/usage/restructuredtext/basics.html>`_. 

While you are making changes to the source of the documentation, you'll probably want to know what your changes look like in the format that ReadTheDocs renders to from time to time.

For this you will need to:

* install Python

* install the necessary packages: ``python -m pip install Sphinx sphinx-rtd-theme breathe``

* install `Doxygen <https://www.doxygen.nl/download.html>`_ and make sure the ``doxygen`` command line utility is a directory in your PATH. To (re)build the doxygen information, use ``doxygen doxygen.conf`` in the documentation directory.

  .. note::

    On a Mac you can use the DMG from the Doxygen downloads page. I then used ``ln -s /Applications/Doxygen.app/Contents/Resources/doxygen doxygen`` in my 'bin' directory to make sure the command line utility was on my path.

* install `Graphviz <http://www.graphviz.org/download/>`_ and make sure its ``dot`` utility is in a directory in your PATH.

After doing this, go the `documentation` directory and type ``make clean html && open _build/html/index.html``.


Legal Information
=================

Included Libraries
------------------

	.. include:: ../README.rst
	   :start-after: included-libs-start
	   :end-before:  included-libs-end
   
Licenses
--------

	.. include:: ../README.rst
	   :start-after: licenses-start
	   :end-before:  licenses-end