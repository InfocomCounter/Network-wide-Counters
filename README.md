# Network-Wide Flow Accounting under Packet Reordering and Loss.

This repository contains the code to reproduce the experimental evaluation in the paper "Network-Wide Flow Accounting under Packet Reordering and Loss."

Evaluations are based on the YAPS simulator(https://github.com/NetSys/simulator).

## Compile: 

`CXX=g++ cmake . -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_FLAGS="-std=c++14"` then `make`. 

## Run:

`./simulator 1 conf_file.txt > output.ans`, where `conf_file.txt` is a configuration file 

Simulator will print to file `output.ans` all info about flows with positive R or L. Parts of the line consisting of `+`, `-` shows the existence of gamma span for different gamma under fixed values of `n` and `t`.(`+` - exist, `-` does not exist).


The line that starts with the word `Error` describes network-wide counter errors for a flow from the previous line.
## Parameters variation

* `speed` - correspond to beta, in evaluations we varied `speed` from 10 to 20.
* `queue_size` - buffers size in bytes.
* `flowlet_size` - value of FL.

File `conf_standart.txt` store configuration for the standard experiment.

