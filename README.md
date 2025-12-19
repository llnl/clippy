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
There are three ways to use the Python code:
1. Install from PyPI:
```console
$ pip install llnl-clippy
```

2. Install from the cloned repository:
```console
$ cd py/src && pip install .
```

3. Via `PYTHONPATH` (see below)

## Building C++ Examples

```console
$ cd cpp && mkdir build && cd build
$ cmake ..
$ make
```

## Running Current C++ Examples (after building)
### From the repository root (using `PYTHONPATH`):
```python
$ PYTHONPATH=py/src:$PYTHONPATH CLIPPY_BACKEND_PATH=$(pwd)/cpp/build/examples ipython

In [1]: from clippy import ExampleBag

In [2]: b = ExampleBag()

In [3]: b.insert(5).insert(6).insert(7)    # can chain methods
Out[3]: <clippy.backends.fs.ExampleBag at 0x107d50830>

In [4]: b.insert(5).insert(8)
Out[4]: <clippy.backends.fs.ExampleBag at 0x107d50830>

In [5]: b.size()
Out[5]: 5

In [6]: b.remove_if(b.value > 6)  # removes 2 elements
Out[6]: <clippy.backends.fs.ExampleBag at 0x107d50830>

In [7]: b.size()
Out[7]: 3

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
