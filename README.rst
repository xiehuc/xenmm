================================
xenmm --- xen memory management
================================

:Authors: -  wzzhang-HIT <wzzhang@hit.edu.cn>
          -  xiehuc      <xiehuc@gmail.com>
:License: GPLv3

this simple program is used for adjust memory of xen virtual machines
it is based on xen balloon driver and adjust balance algorithm.

build
======

depends on ``xenstore-util`` and ``libxen-dev`` packages. if xen<=4.1
also depends ``libxenlight-dev`` package if xen>=4.4  

use ``apt-get install`` to install them.

.. code:: bash

   $ mkdir build
   $ cd build
   $ cmake ..
   $ make
   $ sudo make install
   $ sudo make package # optional, make a deb package when DEB=On

other build option
--------------------

use ``cmake .. -D<<OPTION>>=<<VALUE>>`` to add more control. such as ``cmake ..
-DONLY_CLIENT=On``

-  **ONLY_CLIENT** : only compile ``mmclient`` program, you should consider this
   when build on guest virtual machines to simplify build
-  **DEB** : make a deb package to directly install for many virtual machines
-  **RPM** : make a rpm package to directly install for many virtual machines
-  **TAX_RATE** : set tax rate directly, the default value is **AUTO** the
   possible values are:

   - **AUTO** : enable dynamic tax rate ability
   - **0.0~1.0** : set tax rate as a static constant value

-  **SOCK** : enable socket, that means you can visit **127.0.0.1:9091** to get
   vms' memory footprint as JSON format. and ``canvas/start.sh`` is avaliable

usage
=======

to use program correctly, you should do following steps:

1. configure static ip and ssh according to vm-configure.rst

2. compile program, and generate deb package. then use ``script/deply-deb.sh``
   to install the deb into all vm's

3. use ``script/set-mem.sh`` to set maxmem when halt virtual machines. as well
   as current memory.  the program adjust effect wouldn't exceed maxmem, this
   is strict limit.  this script would also set write permission for xenstore.

4. start client program in guest vms, because there is a script to help do this
   work, just simply use ``sudo service mmclientd start`` to start client. 

.. tip::
   if your machine is systemd, you can just copy commands in mmcliend and enter
   in terminal by hand.

5. use ``virsh console domain-id`` to login into one vm, and do experiments.

6. open another terminal, and run adjust program in domain-0:

.. code:: bash

    cd build
    sudo src/mmserver -N <Num> -f <Num> -d <output>
    
then you can see adjust output logs on terminal. 

7. close running program in vm by hand or by script, when test finished. then
   close adjust program by Ctrl+C.

it would create a new directory named <output>. the log files stays in it with
name of domain's id if you didn't use -d parameter, directory name would came
to ``date_times``.  since you have logs, you can draw curves with tools such as
gnuplot, matlab, ... as well as libreoffice draw. 

benchmark
==========

the benchmark explain how to replay the whold experiment, and collect what data.

mono test 
----------

mono test, let one vm have loads(mm_test_mono), others empty, to give free memory.
it is used to watch the basic work states for adjust program. ``mm_test_mono``
is a simple helper program, would allocate memory from a *low* value to *high*
value, then free memory from *high* value down to *low* value.

you just need watch adjust program's log to see, whether it is worked.

1.  do the steps in usage_
2.  run ``mm_test_mono low high`` on guest vm at usage::step.5 , <low> and <high> are numbers
    of memory, for example ``mm_test_mono 50M 400M`` 
3.  start mm_server at usage::step.6
4.  see the terminal output changes on host, you can see one domain `free`
    memory turn less, `total` turn more. that proves the adjust is working.

dacapo test
------------

dacapo test runs under 4 situations, **low load & no adjust**, **low load &
adjust**, **high load & no adjust**, **high load & adjust**. watch execution
time of dacapo. 

dacapo is used for single vm load test. that is , keep one vm run dacapo,
others empty to give memory. some times it is hard to use all memory, so you
need mm_test_static to *eat* some memory first.

high load is simulated by running load program ``mm_test_static <static>`` , it
would allocate <static> memory, and keep it(not like mm_test_mono), to avoid it
swaped into disk. it would write allocated memory non-stop.

if you need record swap usage, can run ``mm_util_swap`` at target vm, it would
produce a log file.

if need run dacapo test many times ( to work with `phoronix-test-suite test`_ )
use ``dacapo_test.sh`` script, for example:

.. code:: bash

    $ ./dacapo_test.sh h2 10  # run h2 10 times
    $ ./dacapo_test.sh all    # run all test once
    $ ./dacapo_test.sh 'h2 tradebeans' 5 # run h2 and tradebeans 5 times

phoronix-test-suite test
-------------------------

the test under multi vms is based on ``phoronix-test-suite`` test suites. test
suite means it is a test framework, you can select to run some of famous test
by it. use ``$ phoronix-test-suite list-tests``. use  ``$phoronix-test-suite
install`` to install one test.

that is, you need open many terminal and login into every vm in usage::step.5
by ``virsh console``.

notice phoronix-test-suite runs under interactive mode, which doesn't we want.
so we need use ``phoronix-test-suite batch-steup`` to finish steup, and use
``phoronix-test-suite batch-run`` to runs under automatic mode.

to increase a test running time(ensure all vms test runs enough long), need use
`TOTAL_LOOP_COUNT` environment. for example:

.. code:: bash

    $ export TOTAL_LOOP_COUNT=10
    $ phoronix-test-suite batch-run nginx

this would run nginx test for 10 times, because phoronix would give out every
run used time. the total run time could be estimated out. Also,
phoronix-test-suite couldn't use ``Ctrl+C`` to force stop during running. we
should ``touch ~/.phoronix-test-suite/halt-testing`` to stop after this run
end. And produce test result.

mono client test
-----------------

mono client test is a little like mono test. it is used to measure pure cpu performance decrease. 
it doesn't use mmclient as client, but mm_test_client as the client. this is a fake client program, means which doesn't report 'real' memory status to server. but generate a fake and rapid changed number to server. which makes server always do the adjust. then it would cause memory remap, then it makes cpu performance down, without introduce other noise (compare with run a real eat memory program). 

mm_test_client use two parameters, base memory and delta memory. it makes a mono increase on used memory from base memory to total memory . and then decrease it. with every second change delta memory. 

1.  run ``service mmclientd start`` on other domain
2.  run ``mm_test_client 150M 10M`` on target domain
3.  run ``renice -10 -p \`pidof mm_test_client\``` on target domain
4.  start server program
5.  run ``./dacapo_test 'sunflow luindex lusearch' 15`` on target domain

tools
------

because when doing experiments, need to open many console and run various test
program, some of them is main benchmark (dacapo/phoronix-test-suite), some of
them eat memory(mm_test_static), and some of them log useful
information(mm_util_swap). 

tmux is a terminal reuse tool, you can google it and learn what is it and how
to use. you would found it would help in experiment. some basic shortcut keys
are ``Ctrl+b [`` , ``Ctrl+b c`` , ``Ctrl+b n``. for example, i use tmux to
run 3 virtual tty::

   tty1: mm_test_static 100M (optional)
   tty2: mm_util_swap 
   tty3: phoronix-test-suite batch-run nginx or dacapo

data log
============

we provide the experiment log at `dropbox`_ please download
`xenmm-data.squashfs` and mount it, see README inside for detail

.. _dropbox: https://www.dropbox.com/sh/edg0nygofee8fua/AAAdSoGPkuAIPFka-bwLvCgRa?dl=0

dynamic display
================

dynamic display is view changes dynamicly in a webpage.

should open **SOCK** option when compile, then use ./start.sh under canvas
directory.

source directories
====================

+  build : cmake compile director
+  notebook : mathematica experiment notebook
+  script : client daemon script
+  src : source , include ``mmserver`` , ``mmclient`` program.
+  test : test program, include ``mm_test_static`` , ``mm_test_mono``
