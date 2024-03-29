Script for running litefuzz
===========================  

Litefuzz
--------
Litefuzz is a program that creates fuzz tests for finding bugs and improving programs. It can be found in this  
repository:  
[litefuzz](https://github.com/sec-tools/litefuzz). For using it, it is required to have python3 installed.  
The repository litefuzz has a subdirectory named `setup`.
After cloning the repository, run the setup script in accordance to the OS being used. For example, for Linux, 
run 
`<litefuzz-directory>/setup/linux.sh`.
If necessary, make the setup file an executable with command `chmod +x <litefuzz-directory>/setup/<script-name>.sh`.
Now the script can be run without extra steps.

Disclaimer: litefuzz might not work properly for every single machine with a linux distro and its specific software configurations, being able to show some dependency issues and errors. For Ubuntu, lietfuzz was primarily tested on distro Ubuntu 20.04, so it is not fully guaranteed to work properly on other versions. Also, `setup/linux.sh` will install many different libraries, and there may be the case in which litefuzz only needs some of them. 

Usage of the script
-------------------

This script will invoke litefuzz to test a specific program for crashes. The usage of this script is the following:

```  
./runFuzz.sh [ -e path-to-executable ] [ -f path-to-litefuzz-dir ] [ -n number-of-iterations (default: 30000) ] [ -o output-dir (default: ./crashes) ] [ -i input-files ]  
```  

Here,
* `-e` is the path to the executable file of the program we intend to test with `litefuzz`, e.g., velocypack's vpack-to-json executable file. This is the executable of the program we want to test with the fuzz inputs generated by `litefuzz`.
* `-f` is the path to the `litefuzz` directory.
*  `-n` is the number of iterations that `litefuzz`will run for each input file with the program provided with `-e`. For each iteration, the input file will be edited with new fuzz, so the input file will be  different for each iteration. If not provided, the default number of iterations is 30000.
* `-o` is the output directory in which the results from the crashes encountered are saved.  If more than one input file is provided, the crashes for each input file would be saved inside this directory with the name "crashes" + a sequential number, e.g. if two input files are provided, their crashes results will be saved at `<directory-name>/crashes1` and `<directory-name>/crashes2`. If not provided, the default name is `./crashes`.
* `-i` are the input files. The user can provide one input file with the argument `-i <path-to-input-file>`, or also specify a directory, as in `-i <directory-name>`, or a list of files within a directory, as in `-i <directory-name>/*.json`. Also all the files in the directory can be provided as argument, but then it must be in quotes and the absolute path must be provided, e.g. `"/home/<username/<directory-name>/*"`. All these input values can be combined within the same command, but, for each separate argument, it must follow a `-i` option.
  Example of a run:
  `./runFuzz.sh -e ~/velocypack/build/tools/vpack-to-json -f ./ -n 10 -o outputDir -i ~/velocypack/tests/jsonSample/api-docs.json -i /home/<username>/velocypack/tests/jsonSample/commits.json -i "/home/<username>/velocypack/tests/jsonSample/*"`.
  The result for each of the input files is displayed like this:
```
----------------------------------------------------------------------------------------------------------------------------
Running test ~/velocypack/tests/jsonSample/api-docs.json with 10 iterations...
--========================--
--======| litefuzz |======--
--========================--

[STATS]
run id:     1563
cmdline:    ~/velocypack/build/tools/vpack-to-json FUZZ
crash dir:  ./crashes/crashes1
input:      ~/velocypack/tests/jsonSample/api-docs.json
inputs:     1
iterations: 10
mutator:    random(mutators)

@ 10/10 (0 crashes, 0 duplicates, ~0:00:00 remaining)

[RESULTS]
completed (10) iterations with no crashes found

----------------------------------------------------------------------------------------------------------------------------
```
When there are crashes, as mentioned, the information will be stored in the output directory provided, or in the directory `./crashes`. There can be more than one different kind of crash for each run and some duplicates.
In the specific directory for the run, as in the example `./crashes/crashes1`, thre would be the input file that caused an error for that specific iteration of the run, the `.diff` file showing the difference between the original input file provided for that run and the current input file that caused the error, because `litefuzz` edited the file with fuzz for testing, and the stack trace and other useful information about the error.
This is an example of what the crashes directory would look like for a run that had 2 different kinds of crashes:
```UNKNOWN_SIGABRT_7ffff7ab4476_33e9c012c38977e5001e6f3ee9e825bb9d33f74cc930a98fd6650d5d9596644f.diff
UNKNOWN_SIGABRT_7ffff7ab4476_33e9c012c38977e5001e6f3ee9e825bb9d33f74cc930a98fd6650d5d9596644f.diffs
UNKNOWN_SIGABRT_7ffff7ab4476_33e9c012c38977e5001e6f3ee9e825bb9d33f74cc930a98fd6650d5d9596644f.out
UNKNOWN_SIGABRT_7ffff7ab4476_33e9c012c38977e5001e6f3ee9e825bb9d33f74cc930a98fd6650d5d9596644f.txt
UNKNOWN_SIGABRT_7ffff7ab4476_33e9c012c38977e5001e6f3ee9e825bb9d33f74cc930a98fd6650d5d9596644f.zz
UNKNOWN_SIGSEGV_2208bd_cc4aa12819ae7b6cae75d35adc83b57891f5110d748941eb8c302ce3c313e9cc.diff
UNKNOWN_SIGSEGV_2208bd_cc4aa12819ae7b6cae75d35adc83b57891f5110d748941eb8c302ce3c313e9cc.diffs
UNKNOWN_SIGSEGV_2208bd_cc4aa12819ae7b6cae75d35adc83b57891f5110d748941eb8c302ce3c313e9cc.out
UNKNOWN_SIGSEGV_2208bd_cc4aa12819ae7b6cae75d35adc83b57891f5110d748941eb8c302ce3c313e9cc.txt
UNKNOWN_SIGSEGV_2208bd_cc4aa12819ae7b6cae75d35adc83b57891f5110d748941eb8c302ce3c313e9cc.zz
```

