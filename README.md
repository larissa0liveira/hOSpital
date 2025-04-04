# hOSpital - Operating Systems Project  

This repository contains the implementation of **hOSpital**, a simulation of hospital patient admission and consultation processes, developed as part of the **Operating Systems** course at ULisboa.  

### **Phase 1: Process Management and Communication**  
- Implementation of a multi-process system where **patients, receptionists, and doctors** interact.  
- Development of process creation and memory allocation mechanisms.  
- Use of **producer-consumer synchronization** to handle patient admissions.  

### **Phase 2: Synchronization, Logging, and Signals**  
- **Semaphore-based synchronization** to prevent race conditions.  
- **File-based configuration** instead of command-line arguments.  
- **Logging system** to track hospital activity.  
- **Signal handling** (e.g., SIGINT) for graceful termination.  
- **Alarm system** to periodically display patient status.  
- **Final statistics generation** including admission processing times.

## Compilation and Execution  
Compile the project using **Makefile**:  
```bash
make
```
Run the simulation with a configuration file:
```bash
./hOSpital config.txt
```

## Project Structure
```bash
/HOSPITAL  
 ├── bin/          # Executable files  
 ├── include/      # Header files (.h)  
 ├── obj/          # Compiled object files (.o)  
 ├── src/          # Source code files (.c)  
 ├── config.txt    # Configuration file  
 ├── makefile      # Compilation script  
```

## Authors
- João Rebelo
- Juliana Duarte
- Larissa Oliveira
