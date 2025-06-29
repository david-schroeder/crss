# CRSS - The RedSynth Framework Server

CRSS (C Redstone Speedup Server) is a Minecraft Server written to compile redstone to an optimized Graph, which yields HDL code using RedSynth, in order to run redstone circuits at speeds millions of times higher than vanilla Minecraft.

## Table of Contents
1. The Project
    1. Project Structure
    2. Roadmap
    3. About
2. User Guide
    1. GUI Documentation
    2. Console Commands
    3. Configuration Options
3. Reference Guide
    1. RedSynth implementation
        1. Optimizer passes
    2. Server implementation
    3. QoL Server Features
4. Performance
    1. FPGA QoR and Usage
    2. FPGA Speed
    3. Game Speed

## 1. The Project
### 1.1 Project Structure
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
├── Command Line
└── GUI
    └── Graph Renderer
```

### 1.2. Roadmap
Next:
- proper gui setup
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

# License
CRSS is licensed under CC BY-NC-SA 4.0.

# Copyright
Copyright 2025 [David Schröder](mailto:post@schroederdavid.de). All rights reserved.
