# Introduction

---

## What is Plumber?

Plumber is a modular, event-driven, high-performance framework for pipeline applications, especially 
for server softwares. Unlike traditional framework, Plumber adopots the desgin pattern which assemble 
different software components with pipes. Similar to pipe mechanism in UNIX-like operating systems, 
Plumber allows developer creates simple and well-defined components and combines different components into
a complicated system. Different from UNIX pipes, the Plumber framework provides a pipelining environment
with many enhancements. It's desgined to build complicated software with the pipeline desgin pattern.

## Getting Started

Before we actually expolorer the new world of Plumber, let's start with the environment setup.

First of all, you need to install the dependencies:

- CMake
- libuuid
- OpenSSL
- Doxygen
- Python-2.7
- libreadline
- Z Shell

For Ubuntu, use the following command to install all the dependencies

```
apt-get install git cmake gcc g++ uuid-dev libssl-dev doxygen pkg-config python2.7 libpython2.7-dev libreadline-dev zsh
```

In order to try all the examples in this tour, you need to fetch the Plumber Tutourial repo as
following.

```
git clone --recursive https://github.com/38/plumber-tutorial.git 
```

After fetching the code, run `setup.sh` to initialize the environment. 

Each of the step is a different branch, under each of the branch, run `build.sh` to build the example code
use `run.sh` to run the code.

