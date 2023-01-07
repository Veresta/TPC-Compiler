# TPC-Compiler

Compiler for a sub-language of C, the TPC

## Guideline

To generate the compiler, execute the command-line :
```shell
make
```

The tpcc file will be generated in the bin folder. 
To compile a tpc project, use the following command : 
```shell
./bin/tpcc <your_source.tpc> [OPTIONS] [--]
```
or 
```shell
./bin/tpcc < your_source.tpc [OPTIONS] [--]
```
## OPTIONS

- -h/--help : to print how to use the options in the stdout
- -t/--tree : to print the tree in the stdout
- -s/--symtabs : to print all the symbol-tables
- -n/--noexec : to avoid generating the executable

## Contributors
- Ramaroson "Sacane" Johan
- Menaa "Veresta" Mathis
