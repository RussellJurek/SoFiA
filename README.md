SoFiA
=====

Introduction
------------

SoFiA, the Source Finding Application, is a new HI source finding pipeline 
intended to find and parametrise galaxies in HI data cubes. While the 
software is still under development, the first beta version of SoFiA has 
now been released and can be obtained from this web page. 


Requirements
------------

The following packages and libraries will be required to install and run 
SoFiA:

    Operating system:
        Linux or Unix (e.g. Ubuntu, Mac OS, etc.)
        Terminal with bash or tcsh (other shells should work as well)
    Packages (Python):
        Python (≥ 2.4)
        matplotlib (≥ 1.1)
        numpy (≥ 1.7)
        scipy (≥ 0.7)
        pyfits (≥ 3.0)
    Packages (C++):
        GCC (≥ 4.6)
        GNU Scientific Library (≥ 1.15)
        Qt 4 (≥ 4.7; some tweaking will be necessary if Qt 5 is to be used)

All of the above packages must be installed before SoFiA can be compiled and 
run. It is recommended that you install them through your operating system’s 
package manager. Please ensure that the development packages of the GNU 
Scientific Library and the Qt library are installed as well.


Installation
------------

To install SoFiA on your computer, open a terminal window and change into the 
folder where the downloaded file was saved. Then follow the steps below.

1. Unpack all files

   Unpack the sofia_0.1-beta.tar.gz file into a directory of your choice:

    gunzip sofia_0.1-beta.tar.gz
    tar -xvf sofia_0.1-beta.tar

   This will unpack all files into a directory called sofia_0.1-beta.

2. Enter the installation directory

    cd sofia_0.1-beta

3. Compile and install the SoFiA pipeline and user interface

    python setup.py build --force

4. Set up environment variables

    Follow the instructions given at the end of the installation process to 
    define the required environment variables and paths in your .bashrc or 
    .cshrc file.

5. Launch SoFiA

    Open a new terminal window and type ‘SoFiA &’.


Problems
--------

If you encounter problems when running the setup script, it is likely 
that you are either missing one of the required packages and libraries or 
that some of the packages are outdated. Please check that the required 
versions of all packages are installed and properly set up such that they 
can be found by Python and the GCC.


Documentation
-------------

SoFiA comes with its own built-in help browser that can be launched from 
the help menu in the user interface. Alternatively, you can use your web 
browser to open the index.html file located in the SoFiA_0.1/doc/ 
sub-folder. Please note that the user manual is still under construction, 
and only basic help is available at this stage.


Version history
---------------

SoFiA 0.1 beta
  Released 14/11/2013
  Initial beta release of the package with basic functionality and built-in 
  help system; may still contain bugs and errors.


Copyright and licence
---------------------

SoFiA was created by the following people: Lars Flöer, Nadine Giese, Russell 
Jurek, Martin Meyer, Attila Popping, Paolo Serra, Tobias Westmeier, and 
Benjamin Winkel.

© 2013 The SoFiA Authors

This programme is free software: you can redistribute it and/or modify it 
under the terms of the GNU General Public License as published by the Free 
Software Foundation, either version 3 of the License, or (at your option) any 
later version.

This programme is distributed in the hope that it will be useful, but without 
any warranty; without even the implied warranty of merchantability or fitness 
for a particular purpose. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with 
this programme. If not, see http://www.gnu.org/licenses/.
