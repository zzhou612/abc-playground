# abc-playground

Personal playground for ABC (System for Sequential Logic Synthesis and Formal Verification)

## Prerequisites
### macOS

The `gcc` alias on macOS is actually `clang`. For compatibility, GNU `gcc`, instead of `clang` provided by macOS, is taken into use.

First, install `gcc` using `homebrew`.

```bash
brew install gcc@8
```

Install `boost` via `homebrew`. Make sure that `--cc=gcc-8` flag is added for the installation command. 
Otherwise `clang` compiled version will be installed and mistakes will happen during linking.

```bash
brew install boost --cc=gcc-8
```

To prevent `boost` from being updated/upgraded to `clang` compiled version, it needs to be pinned.

```bash
brew pin boost
```

### Ubuntu 18.04

Install the latest version of `boost` library.
```bash
sudo apt install libboost-all-dev
```

## Getting Started

### Project Structure


- ectl: ECTL library (under development)
- abc: source codes of Berkeley ABC
- playground: demos

### Install ABC

ABC has already provided `CMakeLists.txt` to support the use of CLion. Just clone the official ABC GitHub repository to the local directory.
```bash
git clone https://github.com/berkeley-abc/abc.git
```

Then add the following lines to the `CMakeLists.txt` of the project to ensure that ABC is compiled as C++ code with namespaces.

```cmake
set(ABC_USE_NAMESPACE abc)
set(abc_include ${PROJECT_SOURCE_DIR}/abc/src)
add_subdirectory(abc EXCLUDE_FROM_ALL)
```

Finally, link some target to `libabc`, like `abc_playground`, CMake will build `libabc` and use it automatically.

### ABC Basics

In ECTL, ABC is used as a static library. The basic interface and external declarations of ABC can be found in `base/abc/abc.h`. ABC is written in C. ABC supports a wide range of representations of circuits. In general, gate and circuit are abstracted as object and network: two basic data structures, `struct Abc_Obj_t` and `struct Abc_Ntk_t`, are used to store circuits.


#### Abc_Obj_t

`struct Abc_Obj_t` is used to store the objects in the network. In different representations of circuits, the term `object` can represent different entities. The object type is represented by a 4-bit unsigned integer, which is shown in the enumerate variable Abc_ObjType_t:

```c
typedef enum { 
    ...
    ABC_OBJ_CONST1,     //  1:  constant 1 node (AIG only)
    ABC_OBJ_PI,         //  2:  primary input terminal
    ABC_OBJ_PO,         //  3:  primary output terminal
    ...
    ABC_OBJ_NET,        //  6:  net
    ABC_OBJ_NODE,       //  7:  node
    ...
} Abc_ObjType_t;
```

Take a glimpse at `struct Abc_Obj_t`:

```c
struct Abc_Obj_t_
{
    Abc_Ntk_t *       pNtk;          // the host network
    Abc_Obj_t *       pNext;         // the next pointer in the hash table
    int               Id;            // the object ID
    unsigned          Type    :  4;  // the object type
    ...
    Vec_Int_t         vFanins;       // the array of fanins
    Vec_Int_t         vFanouts;      // the array of fanouts
    union { void *    pData;         // the network specific data
      int             iData; };      // (SOP, BDD, gate, equiv class, etc)
    union { void *    pTemp;         // temporary store for user's data
      Abc_Obj_t *     pCopy;         // the copy of this object
      int             iTemp;
      float           dTemp; };
};
```

There ary many members in `struct Abc_Obj_t`. It should be noticed that each object has a unique `ID`. And `vFanins`, `vFanouts` are the vectors storing the pointers that point to fan-in objects and fan-out objects respectively.

`union` is used to store different data types in the same memory location. It is used to store different types of network-specific functionalities and multiple types of temporary values. 

For example, the functionality of an object in a logic network can be stored in SOP (Sum of Product) form. And `iTemp` (temporary integer) can be used to store the output of a gate in the implementation of logic simulator.



#### Abc_Ntk_t

Network is the abstraction of circuit used by ABC. `struct Abc_Ntk_t` is the basic data structure to store a network:


```c
struct Abc_Ntk_t_ 
{
    // general information 
    Abc_NtkType_t     ntkType;       // type of the network
    Abc_NtkFunc_t     ntkFunc;       // functionality of the network
    char *            pName;         // the network name
    ...
    // components of the network
    Vec_Ptr_t *       vObjs;         // the array of all objects (net, nodes, latches, etc)
    Vec_Ptr_t *       vPis;          // the array of primary inputs
    Vec_Ptr_t *       vPos;          // the array of primary outputs
    ...
};
```

It should be noticed that every object in the network only has one instance. These instances can be accessed using the pointers stored in `Vec_Ptr_t` members.

`struct Abc_Ntk_t` supports various network types and functionalities, which are shown in `Abc_NtkType_t` and `Abc_NtkFunc_t` respectively:

```c
typedef enum { 
    ...
    ABC_NTK_NETLIST,    // 1:  network with PIs/POs, latches, nodes, and nets
    ABC_NTK_LOGIC,      // 2:  network with PIs/POs, latches, and nodes
    ABC_NTK_STRASH,     // 3:  structurally hashed AIG (two input AND gates with c-attributes on edges)
    ...
} Abc_NtkType_t;

typedef enum { 
    ...
    ABC_FUNC_SOP,       // 1:  sum-of-products
    ABC_FUNC_BDD,       // 2:  binary decision diagrams
    ABC_FUNC_AIG,       // 3:  and-inverter graphs
    ABC_FUNC_MAP,       // 4:  standard cell library
    ...
} Abc_NtkFunc_t;
```
Supported type and functionality combinations are shown in the following table:

|           |  SOP  |  BDD  |  AIG  |  Map  |
|-----------|-------|-------|-------|-------|
|  Netlist  |   x   |       |   x   |   x   |
|  Logic    |   x   |   x   |   x   |   x   |
|  Strash   |       |       |   x   |       |

In the current version of ECTL, logic network with SOP functionality is used.

####  Macros, Iterators and Functions

Useful macros, iterators and functions can be found in `base/abc/abc.h`. Most of them are named properly, and their functionalities can be easily guessed out with their names and comments.

For example, to iterate through all the objects in the target network, we can use:

```c
#define Abc_NtkForEachObj( pNtk, pObj, i ) \
    ...
```
To create a network based on the maximum fanout free cone of the target node, we can use:
```c
extern ABC_DLL Abc_Ntk_t * Abc_NtkCreateMffc( Abc_Ntk_t * pNtk, Abc_Obj_t * pNode, char * pNodeName );
```

### ECTL (Under Development)

The motivation of developing ECTL library is to create a interface of ABC using modern C++. Instead of C-style structures and functions, ECTL library uses `class Network` and `class Object`. The key idea of ECTL library is to provide an object-oriented, easy-to-use framework for users to implement and test their ideas on the area of approximate logic synthesis.

Currently, ECTL library is still under development. The current major issue is that `class Network` and `class Object` are constructed based on `struct Abc_Ntk_t` and `struct Abc_Obj_t`. When methods, like approximate local changes, can result in changes in network topology, the topology of `class Network` also need to be renewed. Reconstructing the whole network based on the changed `struct Abc_Ntk_t` is too inefficient. Local changes on the topology of `class Network` need to be implemented to resolve this issue.

### Playground

The demo program in `playground` directory is a simple example of approximate substitution. The signal G23gat is replaced with the inverted signal G19gat. The error rate of the resulted approximate circuit is obtained through logic simulations. Then the approximate network is recovered to its original topology.

```cpp
auto origin_ntk = std::make_shared<Network>();
origin_ntk->ReadBlifLogic(benchmark_path.string());
auto approx_ntk = origin_ntk->Duplicate();

auto target_node     = approx_ntk->GetNodebyName("G23gat");
auto target_node_bak = origin_ntk->GetObjbyID(target_node->GetID());
auto sub_node        = approx_ntk->GetNodebyName("G19gat");

auto sub_inv = approx_ntk->CreateInverter(sub_node);

approx_ntk->ReplaceObj(target_node, sub_inv);

std::cout << SimER(origin_ntk, approx_ntk, true);

approx_ntk->DeleteObj(sub_inv);
approx_ntk->RecoverObjFrom(target_node_bak);

std::cout << SimER(origin_ntk, approx_ntk, true);
```

## Reference

Berkeley Logic Synthesis and Verification Group, ABC: A System for Sequential Synthesis and Verification. http://www.eecs.berkeley.edu/~alanmi/abc/
