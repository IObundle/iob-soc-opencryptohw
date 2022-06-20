# SpinalHDL
This directory contains the sources needed to build the custom Versat
functional units for the SHA application using SpinalHDL.

- [SpinalHDL Repository](https://github.com/SpinalHDL/SpinalHDL)
- [SpinalHDL Documentation](https://spinalhdl.github.io/SpinalDoc-RTD/dev/index.html)

## Spinal Project Structure
- From the SpinalHDL Template examples, the common sbt projects have the
  following minimal working structure:
  - build.sbt: sbt settings
  - src/main/scala/units/: scala sources
    - notice that

  - other generated folders: 
    - project/build.properties: (generated project files)
    - target/ (generated folder)
    - rtl/ (generated verilog sources)

## Setup
SpinalHDL is a library written in Scala. Scala is a language that compiles to
the Java virtual machine (JVM).

To generate the verilog FUs we need to install:
- Java Development Kit (JDK)
- Scala distribution
- Scala Build Tool (SBT)

### Update repository cache
```
sudo apt update
```
### Install JDK
```
sudo apt install openjdk-8-jdk
```
### Install Scala Distribution
```
sudo apt install scala
```
### Install Scala Build Tool
```
# SBT 
echo "deb https://repo.scala-sbt.org/scalasbt/debian all main" | sudo tee /etc/apt/sources.list.d/sbt.list
echo "deb https://repo.scala-sbt.org/scalasbt/debian /" | sudo tee /etc/apt/sources.list.d/sbt_old.list
curl -sL "https://keyserver.ubuntu.com/pks/lookup?op=get&search=0x2EE0EA64E40A89B84B2DF73499E82A75642AC823" | sudo apt-key add
sudo apt-get update
sudo apt-get install sbt
```
### Setup Alternative: Automated Script
Alternatively run the equivalent `setup_spinal.sh` script with `root`
permissions:
```
sudo ./setup_spinal.sh
```


- Install/Run Example for SpinalHDL:
    ```
    git clone https://github.com/SpinalHDL/SpinalTemplateSbt.git SpinalTemplateSbt
    cd SpinalTemplateSbt
    sbt run # select mylib.MyTopLevelVhdl in the menu
    ls MyTopLevelVhdl.vhd
    ```
    - SBT is the scala build tool
        - it automatically(?) finds and compiles the files in the repository
    - for this repository, the program is in: 
        `./src/main/scala/mylib/MyTopLevel.scala`
    - When you run `sbt run` you open an interactive execution of `sbt`
        - sbt will find all instances of `main` functions and ask for which one 
        we want to run
        - this happens because the file has multiple `main` functions defined
    - If you want to generate the code from the command line:
    ```
    sbt "runMain mylib.MyTopLevelVerilog"
    ```
    - Run `sbt clean` to remove the `./target` folder
    - You still have to manually remove the `./project/target` folder

- Check more examples with 
    [Workshop repository](https://github.com/SpinalHDL/SpinalWorkshop):
    - check this clone
    - each workshop lab is in `./src/main/scala/workshop/<lab>` folder
    - the `spec.md` is the assignment
    - the `assets` folder has some solution and other auxilar things
    - change the `<Lab>Main.scala` to generate verilog instead of vhdl
    - tryed to do Counter and Uart
        - Counter is just a simple conter, somehow I think that my io.full
          solution is simpler and generates less trash in verilog
        - Counter does not pass verilog test (maybe problem on my end?)
        - Uart I could not understant the hardware architecture and just copied
          the solution
        - generate verilog with:
        ```
        sbt "runMain workshop.counter.CounterMain"
        sbt "runMain workshop.uart.UartCtrlRxMain"
        ```
        - check generated modules in `rtl/`
