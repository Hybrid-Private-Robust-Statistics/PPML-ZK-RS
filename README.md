## Use of EMP-ZK in our protocol
Our zero-knowledge (ZK) proofs are based on an sVOLE protocol \cite{yang_quicksilver_2021} implemented in the EMP-TOOL library \cite{emp-toolkit}. EMP-ZK also follows the offline-online paradigm.  We implement and measure the overhead introduced by multiplication proofs (to verify the parties'  inputs), range proofs (for verifying the input range, the correct computation of the matrix inversion using fixed-point arithmetic, the $q$-th quantile iterative computation, and the participation set), and the generation of privately authenticated values. %


Installation
=====
1. `wget https://raw.githubusercontent.com/emp-toolkit/emp-readme/master/scripts/install.py`
2. `python[3] install.py --deps --tool --ot --zk`
    1. By default it will build for Release. `-DCMAKE_BUILD_TYPE=[Release|Debug]` option is also available.
    2. No sudo? Change [`CMAKE_INSTALL_PREFIX`](https://cmake.org/cmake/help/v2.8.8/cmake.html#variable%3aCMAKE_INSTALL_PREFIX).

Test
=====

Testing on localhost
-----

   `./run ./bin/[binary]`

   
Testing on two
-----

1. Change the IP address in the test code 

2. run `./bin/[binary] 1 [port]` on one machine and 
  
   run `./bin/[binary] 2 [port]` on the other machine.

 
