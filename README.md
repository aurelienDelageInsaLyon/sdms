SDM'Studio: The Reconstruction ToolKit
======================================


<!-- [![Build Status](https://travis-ci.com/hill-a/stable-baselines.svg?branch=master)](https://travis-ci.com/hill-a/stable-baselines) 
[![Documentation Status](https://readthedocs.org/projects/stable-baselines/badge/?version=master)](https://stable-baselines.readthedocs.io/en/master/?badge=master) 
[![Codacy Badge](https://api.codacy.com/project/badge/Grade/3bcb4cd6d76a4270acb16b5fe6dd9efa)](https://www.codacy.com/app/baselines_janitors/stable-baselines?utm_source=github.com&amp;utm_medium=referral&amp;utm_content=hill-a/stable-baselines&amp;utm_campaign=Badge_Grade) 
[![Codacy Badge](https://api.codacy.com/project/badge/Coverage/3bcb4cd6d76a4270acb16b5fe6dd9efa)](https://www.codacy.com/app/baselines_janitors/stable-baselines?utm_source=github.com&utm_medium=referral&utm_content=hill-a/stable-baselines&utm_campaign=Badge_Coverage)

[![GitHub release](https://img.shields.io/github/release/SimonRit/RTK.svg)](https://github.com/SimonRit/RTK/releases/latest) -->
<!-- [![PyPI](https://img.shields.io/pypi/v/itk-rtk.svg)](https://pypi.python.org/pypi/itk-rtk) -->
[![License](https://img.shields.io/badge/License-Apache%202.0-blue.svg)](https://gitlab.inria.fr/jdibango/sdms/-/blob/master/LICENSE)

- [1. Requirements](#1-requirements)
- [2. Installation](#2-installation-of-sdms)
- [3. Formaisms](#3-formalisms)
  - [3.1. Multi-agent](#31-multi-agent)
  - [3.2. Single-agent](#32-single-agent)
- [4. Algorithms](#4-algorithms)
- [5. Usage](#5-usage)


## 1. Requirements
  - c++		    version >= 5.4.0
  - clang++ 	version >= 3.8.0
  - boost 	  version >= 1.66
  - eigen 	  version >= 3.0.0
  - cplex 	  version >= 12.63
  - python    version >= 3.7
  - ncurses (for macOSx, we need to create a symlink: ln -s /usr/lib/libncurses.dylib /usr/local/lib/libncursesw.dylib) 


## 2. Installation of SDM'S

### 2.1. Install eigen3 library, the version in the repository should be enough (default installation prefix is /usr)
```bash
  sudo apt-get install libeigen3-dev
```

### 2.2. Install boost library, version 1.66.

  The current version in the repository is 1.66 => need to install from sources
	http://www.boost.org/doc/libs/1_60_0/more/getting_started/unix-variants.html
	Go to section 5.1 "Easy Build and Install" and follow the instructions on how to install boost.
  The installation prefix should be /usr/local

### 2.3. Install clang, the version in the repository should be enough
   - NOTE: the project compiles using clang++ and not gcc
```bash
  sudo apt-get install clang
```

### 2.4. Install pytorch CXX11 
```bash

wget https://download.pytorch.org/libtorch/cpu/libtorch-cxx11-abi-shared-with-deps-1.7.0%2Bcpu.zip
```

### 2.4. Get SDMS from sources
```bash
git clone https://github.com/blavad/sdms.git
cd sdms
git checkout develop
mkdir build
mkdir libs
mkdir libs/libsdm
cd libs
wget https://download.pytorch.org/libtorch/cpu/libtorch-cxx11-abi-shared-with-deps-1.7.0%2Bcpu.zip
unzip libtorchxxxxxxxxxxxxx.zip
cd ../build
cmake .. -DCMAKE_INSTALL_PREFIX=../libs/libsdm/ -DCMAKE_PREFIX_PATH=../libs/libtorch
make install
```

### 2.5. Install ILOG CPLEX (third party software)
   - The installation should be in /opt/ibm/ILOG/CPLEX_Studio1263 on Linux machines and /Applications/CPLEX_Studio1263 on MAC machines

## 3. Formalisms

### 3.1. Multi-agent
|        POSG        |     Dec-POMDP      |       ZSPOSG       |         SG         | Dec-MDP |
| :----------------: | :----------------: | :----------------: | :----------------: | :-----: |
| :heavy_check_mark: | :heavy_check_mark: | :heavy_check_mark: | :heavy_check_mark: |   :x:   |


### 3.2. Single-agent
|       POMDP        |        MDP         |
| :----------------: | :----------------: |
| :heavy_check_mark: | :heavy_check_mark: |

## 4. Algorithms

| HSVI  | Q-Learning | Value Iteration | Policy Iteration | JESP  |
| :---: | :--------: | :-------------: | :--------------: | :---: |
|  :x:  |    :x:     |       :x:       |       :x:        |  :x:  |


## 5. Usage

```cpp
#include <iostream>
#include <sdm/worlds.hpp>
#include <sdm/parser/parser.hpp>

int main(int argc, char **argv)
{
	sdm::DecPOMDP dpomdp_world= sdm::parser::parse_file("my_problem.dpomdp");
  
  std::cout << "Nb States : " << dpomdp_world.getNumStates() << std::endl;
	std::cout << "Nb Agents : " << dpomdp_world.getNumAgents() << std::endl;
	std::cout << "Nb Joint Actions : " << dpomdp_world.getNumJActions() << std::endl;
	std::cout << "Nb Joint Observations : " << dpomdp_world.getNumJObservations() << std::endl;

  return 0;
}
```
