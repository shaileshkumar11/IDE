
# Console-Based C Programming IDE
[![Platform](https://img.shields.io/badge/Platform-Linux%20(Ubuntu)-blue)](#)
[![Language](https://img.shields.io/badge/Language-C%20Programming-green)](#)
[![Concepts](https://img.shields.io/badge/Concepts-Process%20%26%20File%20System%20Calls-orange)](#)

This project is a **console-based C programming IDE** designed for Linux environments. It allows users to **create, edit, compile, run, debug, and manage multi-file C projects** directly through the terminal—without any external IDE.

The project uses **system calls** from the Linux OS such as `fork()`, `exec()`, `open()`, `read()`, `write()`, and `unlink()` to implement all features manually.

---

## 🔥 Overview

This IDE works using a `.proj` file that stores all project file names.  
Once launched, it provides a **menu-based interface** for performing operations like:

- Creating new `.c` or `.h` files
- Editing using `vim`
- Compiling all `.c` files concurrently
- Linking object files into a final executable
- Running with command-line arguments
- Debugging with `gdb`
- Checking memory leaks using `valgrind`

---

## ✅ Features

| Feature | Description |
|--------|-------------|
| New File | Create `.c` or `.h` source files and open with `vim` |
| Open File | Edit any project file with `vim` |
| Delete File | Remove file + update `.proj` metadata |
| Build Project | Compile each `.c` using **fork()** and link to executable |
| Clean Project | Remove all `.o` files and the executable |
| Run Project | Execute binary with user-provided CLI arguments |
| Debug Project | Launch executable in **GDB** |
| Check Memory Leakage | Run executable under **Valgrind** |

---

## 🛠️ Technologies Used

### **Operating System Concepts**
- Process creation (`fork()`)
- Program execution (`execvp()`, `execlp()`)
- Waiting for child processes (`wait()`, `waitpid()`)
- File handling (`open()`, `read()`, `write()`, `lseek()`, `unlink()`)
- Directory handling (`stat()`, `mkdir()`, `chdir()`)

### **Tools**
| Tool | Purpose |
|------|---------|
| GCC | Compiling C files |
| Vim | Source code editing |
| GDB | Debugging |
| Valgrind | Memory leak detection |

---

## 📂 Running the Project

### **Compile the IDE**
```bash
gcc ide.c -o ide
```

### **Create or Load a Project**
```bash
./ide <project_directory> <project_name>.proj
```

### **Example**
```bash
./ide workspace myproject.proj
```

---

## 🧭 Menu Example
```
Menu:
1. New File
2. Open File
3. Delete File
4. Build Project
5. Clean Project
6. Run Project
7. Debug Project
8. Check Memory Leakage
9. Exit
Enter choice:
```

---

## 🎯 Purpose

This project is implemented under **Hands-On OS & Linux Programming** for understanding:

- System call level programming
- Process management
- Manual build automation
- Project metadata handling

---

## 👨‍💻 Author

**ShaileshKumar Zanjare**  
PG-DESD | (sunbeeam infotech)CDAC  
