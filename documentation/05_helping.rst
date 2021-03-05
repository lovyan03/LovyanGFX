***************
How can I help?
***************

We realise we've embarked on a large journey with a very small team and we could use some help with pretty much everything we're doing right now. Help is particularly welcome in the following areas:

* **Coding**: help us develop new widgets or debug the existing ones.

* **Documentation**: help document LoyvanGFX, or proofread existing documentation to help us improve it.

* **Suggestions**: Use LoyvanGFX for your own projects and tell us what's missing, or what could be done differently.


This project lives in its GitHub repository. The best way to talk to us about problems or suggestions regarding code or documentation is to file an issue in the repository. Feel free to include a Pull Request if you know how to fix the problem yourself.


Writing documentation
=====================

You'll notice that the documentation for LoyvanGFX is displayed with the help of `Read the Docs <readthedocs.org>`_. They compile the version they display from the 'documentation' directory in our library. The source for the documentation is written in `reStructuredText` (.rst) format. To get started with that, check out `this primer <https://www.sphinx-doc.org/en/master/usage/restructuredtext/basics.html>`_. 

While you are making changes to the source of the documentation, you'll probably want to know what your changes look like in the format that ReadTheDocs renders to from time to time.

For this you will need to:

* install Python

* install the necessary packages: ``python -m pip install Sphinx sphinx-rtd-theme breathe``

* install `Doxygen <https://www.doxygen.nl/download.html>`_ and make sure the ``doxygen`` command line utility is a directory in your PATH.

  .. note::

    On a Mac you can use the DMG from the Doxygen downloads page. I then used ``ln -s /Applications/Doxygen.app/Contents/Resources/doxygen doxygen`` in my 'bin' directory to make sure the command line utility was on my path.

* install `Graphviz <http://www.graphviz.org/download/>`_ and make sure its ``dot`` utility is in a directory in your PATH.

After doing this, go the `documentation` directory and type ``make clean html && open _build/html/index.html``.