# <a name=""></a>Analysis

## <a name="nods"></a>In-built nodes

- Scene I/O
  - [Camera (Set)](#scm)
- Inputs
  - [Particle data](#ipd)
  - [System info](#isi)
- Modifiers
  - [Set Param](#msp)
- Generators
  - [Add Bonds](#gab)
  - [Trace Trajectory](#gtt)
- Miscallenous
  - [Plot graph](#mpg)
  - [Show Range](#msr)

## <a name="scrp"></a>Scripting guide

- [General](#sgn)
- [C++](#cpp)
- [Python](#py)
- [Fortran](#f90)


# Details

## In-built nodes


## Scripting guide

### <a name="sgn"></a>General

*This section describes common properties amongst languages. Please refer to the actual languages for examples.*

AViS Accepts these basic data types. They can be either appear single or as an array.

- short
- int
- double

These few things are parsed by the pre-compiler. They are all optional except when marked with *.

- Description
  - Beginning lines of the script, if any, will be parsed as the description of the node.
- Input variables
  - Global variables that are commented as `in` will be parsed as the inputs for the node.
- Output variables
  - Global variables that are commented as `out`, well, you get the idea.
- Progress variable
  - A variable with type `double` marked as `progress` will allow the node to display its progress (duh) on the UI.
- Entry point*
  - The function to be called when the node is executed. It must be a function marked as `entry` with no arguments and no return value.

"Marks" are the single API format for an AViS script. Each mark must immediately precede its target, with no extra spaces or newlines in between.
```
*mark*
variable or function as *mark*
```

### <a name="cpp"></a>C++

The general format of a parse-able script is as follows.

- Description
```cpp
1. ///This must come at the top of the script.
2. ///No new-lines must occur before or between these lines.
3. ///This is another line.
```

- Input variables
```cpp
//in
int numberOfFrames = 0;
```

- Output variables
```cpp
//out
int numberOfRedAtoms = 0;
```

- Progress variable
```cpp
//progress
double progressOfTheCode = 0;
```

- Entry point
```cpp
//entry
void DoSomething() {
    //do something fancy!
}
```

#### Arrays

C-like code uses raw-pointers as dynamic arrays, so you need to know the length of each dimension. They can be specified beside the `in` or `out` comment. Length variables can be either: an input or output variable; a variable declared as `var`; or a constant. All length variables must be of type `int`.

Example:
```cpp
//in cnt
short* takeAnArrayOfSizeCnt = 0;
//out cnt 3
double* andReturnAnArrayOfSizeCntX3 = 0;

//in
int numberOfBirds = 0;
//out numberOfBirds numberOfLegs
int* numberOfToes = 0;
//var
int numberOfLegs = 2;
```

For multi-dimensional arrays, the items are arranged row-major. That is, the right-est index advances the fastest.

Example:
```cpp
//in a b c
int* myArray = 0;

//The element at location [x][y][z] can be accessed as below.
//It is your responsibility not to overflow the indices!
int xyz = myArray[x*b*c + y*c + z];
```

**As this is C++, please use `new` and not `malloc` for arrays!**

Example:
```cpp
//out 100
double* myArray = 0;
void Alloc() {
    myArray = new double[100]{}; //good
    //myArray = (double*)calloc(100*sizeof(double)); //never ever ever do this!
}
```

### <a name="py"></a>Python


### <a name="f90"></a>Fortran

A fortran script should contain a primary module with the same name as the first module.

The general format of a parse-able script is as follows.

- Description
```f90
1. ! This must come at the top of the script.
2. ! No new-lines must occur before or between these lines.
3. ! A space must exist between the ! and the text.
```

- Input variables
```f90
!in 
INTEGER :: MYINT
```

- Output variables
```f90
!out
REAL*8 :: DOUBLEVAR
```

- Progress variable
```f90
!progress
REAL*8 :: PROGRESSMEOW
```

- Entry point
```f90
!entry
SUBROUTINE HELLO()
    !say hello!
end subroutine HELLO
```

#### Arrays

To allow for interoperability with other languages, arrays must be declared as `ALLOCATABLE TARGET`s.

Example:
```f90
REAL*8, ALLOCATABLE, TARGET :: SOMEARRAY (:,:)
```