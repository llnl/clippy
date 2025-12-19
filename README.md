# CLIPPy - Command Line Interface Plus Python
 ````
 ╭────────────────────────────────────╮
 │ It looks like you want to use HPC. │ 
 │ Would you like help with that?     │
 ╰────────────────────────────────────╯
  ╲
   ╲
    ╭──╮  
    ⊙ ⊙│╭
    ││ ││
    │╰─╯│
    ╰───╯
````

## Overview

Clippy (CLI + PYthon) is a Python language interface to HPC resources. Precompiled binaries
that execute on HPC systems are exposed as methods to a dynamically-created `Clippy` Python
object, where they present a familiar interface to researchers, data scientists, and others.
Clippy allows these users to interact with HPC resources in an easy, straightforward
environment – at the REPL, for example, or within a notebook – without the need to learn
complex HPC behavior and arcane job submission commands.

## Installation of Python Code
```console
$ pip install .
```

## Building C++ Examples on LC

```console
$ . /usr/workspace/llamag/spack/share/spack/setup-env.sh
$ git clone https://github.com/LLNL/clippy-cpp.git
$ spack load gcc
$ spack load boost
$ cd clippy-cpp
$ mkdir build
$ cd build
$ cmake ../
$ make
$ cd ../.. #back to root project directory 
```

## Running Current C++ Examples
```python
$ CLIPPY_BACKEND_PATH=/path/to/binaries ipython3

In [1]: from clippy import *

 ╭────────────────────────────────────╮
 │ It looks like you want to use HPC. │ 
 │ Would you like help with that?     │
 ╰────────────────────────────────────╯
  ╲
   ╲
    ╭──╮  
    ⊙ ⊙│╭
    ││ ││
    │╰─╯│
    ╰───╯

In [2]: c = ExampleBag()  # creates a bag datastructure.

In [3]: c.insert("foo").insert("bar")  # mutating methods can be chained
       
Out[3]: foo bar

In [4]: c.size()  # nonmutating methods return the appropriate output.
Out[4]: 2

```
## Authors
- Seth Bromberger (seth at llnl dot gov)
- Roger Pearce (rpearce at llnl dot gov)


## License
CLIPPy is distributed under the MIT license.

See [LICENSE-MIT](LICENSE-MIT), [NOTICE](NOTICE), and [COPYRIGHT](COPYRIGHT) for
details.

SPDX-License-Identifier: MIT

## Release
LLNL-CODE-818157
