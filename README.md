About:
-------
Test 1

Contains two interacting components: 
- GUI application `sdetector`;
- Server application `scannerd`.

Also there are three test applications. 

Depends:
-------
Qt 5  
CMake 2.8 or later  
OS: Windows/Linux  

Build:
-------
`$git clone https://github.com/leonid-belyasnik/sdetector.git`  
`$cd sdetector`    

**MSVS2015:**  
`>build64.bat ` 
* Use IDE for build applications ...

**Linux:**  
`$build.sh`  
* If successful, executables must appear in the `./bin` directory.  

* If use CLion IDE or Qt Creator IDE:  

`$mkdir build`   
`$cd build`  
`$cmake ..`  

* Open in IDE CMakeLists.txt as project.  

Usage:
-------
```
Usage server: scannerd [ _option_ ]  
Options:  
 Without option -       Start as application.  
 --help(-h)             This description.  
 --stop(-x)             Stop server.  
 --daemon(-d)           Start as daemon.  
```
note:   
In OS Windows server working only as application.  
For exit from console, enter `:quit`  

**Possible simple test algorithm:**

**Windows**  
- run test in console: `>test_list.exe`
- run test in console: `>test_seeker.exe`
- run server in console: `>scannerd.exe`
- run test in console: `>test_client.exe`
- run GUI application: push button `Find >>>`
- finish GUI application
- finish server: `>:quit`

**Linux**  
- run test in terminale: `$./test_list`
- run test in terminale: `$./test_seeker`
- run daemon in terminale: `$./scannerd -d`
- run test in terminale: `$./test_client`
- run GUI application: push button `Browse >>>` -> to choose `/tmp`
- GUI application: push button `Find >>>`
- finish GUI application
- finish daemon: `$./scannerd -x`

