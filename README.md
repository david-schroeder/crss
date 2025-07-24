# CRSS - The RedSynth Framework Server

CRSS (C Redstone Speedup Server) is a Minecraft Server written to compile redstone to an optimized Graph, which yields HDL code using RedSynth, in order to run redstone circuits at speeds millions of times higher than vanilla Minecraft.

## Table of Contents
1. The Project
    1. Project Structure
    2. Working with the Source Code
        1. Function Identifiers
    3. Roadmap
    4. About
2. User Guide
    1. GUI Documentation
    2. Console Commands
    3. Configuration Options
    4. In-Game Features
        1. Plots
3. Reference Guide
    1. RedSynth implementation
        1. Optimizer passes
    2. Server implementation
    3. QoL Server Features
    4. Miscellaneous
        1. Logger
4. Performance
    1. FPGA QoR and Usage
    2. FPGA Speed
    3. Game Speed

## 1. The Project
### 1.1. Project Structure
CRSS is organized hierarchically into components. The toplevel components, and several lower-level components are bound to individual threads:
```
CRSS Master
├── RedSynth Backend
│   ├── Redstone compiler
│   └── Graph optimization passes
│       └── Clock detection
├── Minecraft Server
│   ├── Networking
│   └── Game Logic
├── Logger
└── GUI
    └── Graph Renderer
```

### 1.2. Working with the Source Code
Most components and functions in the CRSS/RedSynth source code are well-documented
and straightforward. There's a few things that should still be addressed:

#### 1.2.1. Function Identifiers
Many functions take an initial argument `fnpath`, which is typically a `char *` or
a `const char *` and is used for logging. This string practically contains the call
stack in dot notation; if a Minecraft client using version 1.18.2 was disconnected 
from the game during login, the disconnect function may internally assume the value
`network.game.dispatch.758.login.disconnect` for its `fnpath` variable. The caller,
i.e. the (failed) login function, would then pass `network.game.dispatch.758.login`
as an argument to the function.

There's a few guidelines to be aware of that pertain to this common function
parameter:
- The *callee* function is responsible for appending its own identifier string to
the end of the passed `fnpath` value
- For this, the function `get_fn_path` ([utils.h](lib/utils.h)) is recommended,
which allocates a new string containing the correct value for the callee's
`fnpath` variable
- Because the parameter is uniformly named `fnpath` in most cases, the most standard
use case for `get_fn_path` is wrapped into the macro `FUNCPATH(funcname)`
([utils.h](lib/utils.h)), which expands to `fnpath = get_fn_path(fnpath, funcname)`.
Note that the resulting string in `fnpath` must still be freed, even if its
declaration is hidden by macro
- Certain functions, especially (GTK) callback functions or thread functions, cannot
(or should not) accept an `fnpath` parameter. For these functions, `fnpath` should
be statically declared as a `const char *` at the top of the function. There may
be a macro for this in the future.

### 1.3. Roadmap
Next:
- plots
- specifically opengl support
- graph rendering in 3d ogl

Somewhere down the line:
- Packaging as application

## 2. User Guide
### 2.1. GUI Documentation
The GUI is split up into multiple components:
- Graph view
- Server console + Administration
- Runtime metrics + Configuration
- Hardware + FPGA State overview
- Generated HDL, reports etc. viewport

### 2.4. In-Game Features

#### 2.4.1 Plots

Plot sizes are dynamic, and the suggested way to track plots is by using labels. This results in some interesting implications for creating and using plots:

- Plots must be created with a name via `/plot new <plotname> [<size>]`, where `<size>` is the side length of the plot in units of 16 blocks.
- If you wish to select a certain plot position, please use the extended syntax `/plot new <plotname> <size> <x> <z>`.
- Since plots can have dynamic sizes, they are layed out in a *virtual* grid, where each grid field is one plot. Traveling between plots by going over the edge is thus possible, but not the recommended way of transport, which is `/plot visit <plotname>`.

# License
CRSS is licensed under CC BY-NC-SA 4.0.

# Copyright
Copyright 2025 [David Schröder](mailto:post@schroederdavid.de). All rights reserved.
