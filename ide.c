#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <errno.h>

#define MAX_FILES 10
#define FILE_NAME_LEN 256
#define PROJ_EXT ".proj"

struct Project {
    int num_files;
    char files[MAX_FILES][FILE_NAME_LEN];
};

int load_project(const char* proj_name, struct Project* proj) {
    int fd = open(proj_name, O_RDONLY);
    if (fd == -1) {
        if (errno == ENOENT) {
            proj->num_files = 0;
            return 0; // new project
        }
        perror("open");
        return -1;
    }
    if (read(fd, &proj->num_files, sizeof(int)) != sizeof(int)) {
        perror("read num_files");
        close(fd);
        return -1;
    }
    for (int i = 0; i < proj->num_files; i++) {
        if (read(fd, proj->files[i], FILE_NAME_LEN) != FILE_NAME_LEN) {
            perror("read file name");
            close(fd);
            return -1;
        }
    }
    close(fd);
    return 0;
}

void save_project(const char* proj_name, struct Project* proj) {
    int fd = open(proj_name, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd == -1) {
        perror("open for write");
        return;
    }
    if (write(fd, &proj->num_files, sizeof(int)) != sizeof(int)) {
        perror("write num_files");
        close(fd);
        return;
    }
    for (int i = 0; i < proj->num_files; i++) {
        if (write(fd, proj->files[i], FILE_NAME_LEN) != FILE_NAME_LEN) {
            perror("write file name");
            close(fd);
            return;
        }
    }
    close(fd);
}

int is_in_project(struct Project* proj, const char* fname) {
    for (int i = 0; i < proj->num_files; i++) {
        if (strcmp(proj->files[i], fname) == 0) {
            return i;
        }
    }
    return -1;
}

void new_file(struct Project* proj, const char* proj_name) {
    if (proj->num_files >= MAX_FILES) {
        printf("Maximum number of files reached.\n");
        return;
    }
    char fname[FILE_NAME_LEN];
    printf("Enter file name (.c or .h): ");
    scanf("%s", fname);
    if (strstr(fname, ".c") == NULL && strstr(fname, ".h") == NULL) {
        printf("File must be .c or .h\n");
        return;
    }
    if (is_in_project(proj, fname) != -1) {
        printf("File already exists in project.\n");
        return;
    }
    strcpy(proj->files[proj->num_files++], fname);
    save_project(proj_name, proj);
    pid_t pid = fork();
    if (pid == 0) {
        execlp("vim", "vim", fname, NULL);
        perror("execlp vim");
        exit(1);
    } else if (pid > 0) {
        wait(NULL);
    } else {
        perror("fork");
    }
}

void open_file(struct Project* proj) {
    char fname[FILE_NAME_LEN];
    printf("Enter file name: ");
    scanf("%s", fname);
    if (is_in_project(proj, fname) == -1) {
        printf("File not in project.\n");
        return;
    }
    pid_t pid = fork();
    if (pid == 0) {
        execlp("vim", "vim", fname, NULL);
        perror("execlp vim");
        exit(1);
    } else if (pid > 0) {
        wait(NULL);
    } else {
        perror("fork");
    }
}

void delete_file(struct Project* proj, const char* proj_name) {
    char fname[FILE_NAME_LEN];
    printf("Enter file name: ");
    scanf("%s", fname);
    int idx = is_in_project(proj, fname);
    if (idx == -1) {
        printf("File not in project.\n");
        return;
    }
    if (unlink(fname) == -1) {
        perror("unlink");
        return;
    }
    for (int i = idx; i < proj->num_files - 1; i++) {
        strcpy(proj->files[i], proj->files[i+1]);
    }
    proj->num_files--;
    save_project(proj_name, proj);
}

void build_project(struct Project* proj, const char* proj_name) {
    char exe_name[FILE_NAME_LEN];
    strcpy(exe_name, proj_name);
    exe_name[strlen(exe_name) - 5] = '\0'; // remove .proj
    pid_t pids[MAX_FILES];
    int num_c = 0;
    for (int i = 0; i < proj->num_files; i++) {
        if (strstr(proj->files[i], ".c")) {
            char obj[FILE_NAME_LEN];
            strcpy(obj, proj->files[i]);
            obj[strlen(obj) - 1] = 'o';
            pid_t pid = fork();
            if (pid == 0) {
                execlp("gcc", "gcc", "-c", proj->files[i], "-o", obj, NULL);
                perror("execlp gcc");
                exit(1);
            } else if (pid > 0) {
                pids[num_c++] = pid;
            } else {
                perror("fork");
            }
        }
    }
    int all_ok = 1;
    for (int i = 0; i < num_c; i++) {
        int status;
        waitpid(pids[i], &status, 0);
        if (!WIFEXITED(status) || WEXITSTATUS(status) != 0) {
            all_ok = 0;
        }
    }
    if (!all_ok) {
        printf("Compilation failed.\n");
        return;
    }
    // link
    char* args[MAX_FILES + 4];
    args[0] = "gcc";
    args[1] = "-o";
    args[2] = exe_name;
    int arg_idx = 3;
    for (int i = 0; i < proj->num_files; i++) {
        if (strstr(proj->files[i], ".c")) {
            char obj[FILE_NAME_LEN];
            strcpy(obj, proj->files[i]);
            obj[strlen(obj) - 1] = 'o';
            args[arg_idx++] = strdup(obj);
        }
    }
    args[arg_idx] = NULL;
    pid_t pid = fork();
    if (pid == 0) {
        execvp("gcc", args);
        perror("execvp gcc link");
        exit(1);
    } else if (pid > 0) {
        int status;
        waitpid(pid, &status, 0);
        if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
            printf("Build successful.\n");
        } else {
            printf("Linking failed.\n");
        }
    } else {
        perror("fork");
    }
    for (int i = 3; i < arg_idx; i++) {
        free(args[i]);
    }
}

void clean_project(struct Project* proj, const char* proj_name) {
    char exe_name[FILE_NAME_LEN];
    strcpy(exe_name, proj_name);
    exe_name[strlen(exe_name) - 5] = '\0';
    unlink(exe_name);
    for (int i = 0; i < proj->num_files; i++) {
        if (strstr(proj->files[i], ".c")) {
            char obj[FILE_NAME_LEN];
            strcpy(obj, proj->files[i]);
            obj[strlen(obj) - 1] = 'o';
            unlink(obj);
        }
    }
    printf("Cleaned.\n");
}

void run_project(const char* proj_name) {
    char exe_name[FILE_NAME_LEN];
    strcpy(exe_name, proj_name);
    exe_name[strlen(exe_name) - 5] = '\0';
    if (access(exe_name, F_OK) == -1) {
        printf("Executable not found. Build first.\n");
        return;
    }
    printf("Enter command line arguments: ");
    char line[1024];
    getchar(); // consume newline
    fgets(line, sizeof(line), stdin);
    line[strcspn(line, "\n")] = 0; // remove newline
    char* args[100];
    args[0] = exe_name;
    int arg_count = 1;
    char* token = strtok(line, " ");
    while (token && arg_count < 99) {
        args[arg_count++] = token;
        token = strtok(NULL, " ");
    }
    args[arg_count] = NULL;
    pid_t pid = fork();
    if (pid == 0) {
        execvp(exe_name, args);
        perror("execvp run");
        exit(1);
    } else if (pid > 0) {
        wait(NULL);
    } else {
        perror("fork");
    }
}

void debug_project(const char* proj_name) {
    char exe_name[FILE_NAME_LEN];
    strcpy(exe_name, proj_name);
    exe_name[strlen(exe_name) - 5] = '\0';
    if (access(exe_name, F_OK) == -1) {
        printf("Executable not found. Build first.\n");
        return;
    }
    pid_t pid = fork();
    if (pid == 0) {
        execlp("gdb", "gdb", exe_name, NULL);
        perror("execlp gdb");
        exit(1);
    } else if (pid > 0) {
        wait(NULL);
    } else {
        perror("fork");
    }
}

void check_memory_leakage(const char* proj_name) {
    char exe_name[FILE_NAME_LEN];
    strcpy(exe_name, proj_name);
    exe_name[strlen(exe_name) - 5] = '\0';
    if (access(exe_name, F_OK) == -1) {
        printf("Executable not found. Build first.\n");
        return;
    }
    printf("Enter command line arguments: ");
    char line[1024];
    getchar(); // consume newline
    fgets(line, sizeof(line), stdin);
    line[strcspn(line, "\n")] = 0;
    char* args[101];
    args[0] = "valgrind";
    args[1] = "--leak-check=full";
    args[2] = exe_name;
    int arg_count = 3;
    char* token = strtok(line, " ");
    while (token && arg_count < 100) {
        args[arg_count++] = token;
        token = strtok(NULL, " ");
    }
    args[arg_count] = NULL;
    pid_t pid = fork();
    if (pid == 0) {
        execvp("valgrind", args);
        perror("execvp valgrind");
        exit(1);
    } else if (pid > 0) {
        wait(NULL);
    } else {
        perror("fork");
    }
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <project_dirpath> <project_name>\n", argv[0]);
        return 1;
    }
    char* proj_dir = argv[1];
    char* proj_name = argv[2];
    if (strstr(proj_name, PROJ_EXT) == NULL || strcmp(proj_name + strlen(proj_name) - 5, PROJ_EXT) != 0) {
        fprintf(stderr, "Project name must end with .proj\n");
        return 1;
    }
    struct stat st;
    if (stat(proj_dir, &st) == -1) {
        if (mkdir(proj_dir, 0755) == -1) {
            perror("mkdir");
            return 1;
        }
    }
    if (chdir(proj_dir) == -1) {
        perror("chdir");
        return 1;
    }
    struct Project proj;
    if (load_project(proj_name, &proj) == -1) {
        return 1;
    }
    while (1) {
        printf("\nMenu:\n");
        printf("1. New File\n");
        printf("2. Open File\n");
        printf("3. Delete File\n");
        printf("4. Build Project\n");
        printf("5. Clean Project\n");
        printf("6. Run Project\n");
        printf("7. Debug Project\n");
        printf("8. Check Memory Leakage\n");
        printf("9. Exit\n");
        printf("Enter choice: ");
        int choice;
        scanf("%d", &choice);
        switch (choice) {
            case 1:
                new_file(&proj, proj_name);
                break;
            case 2:
                open_file(&proj);
                break;
            case 3:
                delete_file(&proj, proj_name);
                break;
            case 4:
                build_project(&proj, proj_name);
                break;
            case 5:
                clean_project(&proj, proj_name);
                break;
            case 6:
                run_project(proj_name);
                break;
            case 7:
                debug_project(proj_name);
                break;
            case 8:
                check_memory_leakage(proj_name);
                break;
            case 9:
                return 0;
            default:
                printf("Invalid choice.\n");
        }
    }
    return 0;
}
