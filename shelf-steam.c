//Description:
/*
  This program implements a simple version of Steam for the Linux shell called shelf-steam.
  It provides capabilities for searching and running games from a specified repository.
  The program functions through an interactive cycle, handling user commands,
  carrying them out, and returning the output.
*/

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>

#define MAX_COMMAND_LENGTH 256
#define ERROR_MESSAGE "An error has occurred\n"

// Function to print error message
void print_error() {
    write(STDERR_FILENO, ERROR_MESSAGE, strlen(ERROR_MESSAGE));
    fflush(stderr);
}

// Function to check if path is a directory
int is_directory(const char *path) {
    struct stat statbuf;
    if (stat(path, &statbuf) != 0) {
        return 0;
    }
    return S_ISDIR(statbuf.st_mode);
}

// Function to get the game description by running with --help
char *get_game_description(const char *repo_path, const char *file_name) {
    char *description = malloc(1024 * sizeof(char));
    if (description == NULL) {
        return NULL;
    }
    
    // Build full path to the file
    char file_path[MAX_COMMAND_LENGTH];
    sprintf(file_path, "%s/%s", repo_path, file_name);
    
    // Check if the file is a directory
    struct stat statbuf;
    if (stat(file_path, &statbuf) == 0 && S_ISDIR(statbuf.st_mode)) {
        strcpy(description, "(empty)");
        return description;
    }
    
    // Check if the file is executable
    if (access(file_path, X_OK) != 0) {
        strcpy(description, "(empty)");
        return description;
    }
    
    description[0] = '\0'; // Initialize as empty string
    
    pid_t pid = fork();
    if (pid == 0) { // Child process
        // Create temporary file for output
        char temp_file[256];
        sprintf(temp_file, "/tmp/shelf-steam-desc-%d", getpid());
        int fd = open(temp_file, O_CREAT | O_WRONLY | O_TRUNC, 0644);
        if (fd == -1) {
            exit(1);
        }
        
        // Redirect stdout to the file
        dup2(fd, STDOUT_FILENO);
        close(fd);
        
        // Execute the file with --help
        execlp(file_path, file_name, "--help", NULL);
        exit(1); // If exec fails
    } else if (pid > 0) { // Parent process
        int status;
        waitpid(pid, &status, 0);
        
        // Read the description from temp file
        char temp_file[256];
        sprintf(temp_file, "/tmp/shelf-steam-desc-%d", pid);
        
        FILE *fp = fopen(temp_file, "r");
        if (fp) {
            size_t bytes_read = fread(description, 1, 1023, fp);
            description[bytes_read] = '\0';
            fclose(fp);
            unlink(temp_file); // Delete temp file
            
            // Remove trailing newlines
            char *newline = strchr(description, '\n');
            if (newline) {
                *newline = '\0';
            }
        }
        
        if (description[0] == '\0') {
            strcpy(description, "(empty)");
        }
    } else {
        print_error();
        free(description);
        return NULL;
    }
    
    return description;
}

// Built-in command: ls
void builtin_ls(char *repo_path) {
    DIR *dir = opendir(repo_path);
    if (!dir) {
        print_error();
        return;
    }
    
    // Get all entries in directory
    struct dirent *entry;
    char **files = NULL;
    int file_count = 0;
    
    while ((entry = readdir(dir)) != NULL) {
        // Skip "." and ".." entries
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }
        
        files = realloc(files, (file_count + 1) * sizeof(char *));
        if (files == NULL) {
            print_error();
            closedir(dir);
            return;
        }
        files[file_count] = strdup(entry->d_name);
        file_count++;
    }
    closedir(dir);
    
    // Sort files lexicographically
    for (int i = 0; i < file_count - 1; i++) {
        for (int j = i + 1; j < file_count; j++) {
            if (strcmp(files[i], files[j]) > 0) {
                char *temp = files[i];
                files[i] = files[j];
                files[j] = temp;
            }
        }
    }
    
    // Print files with descriptions
    for (int i = 0; i < file_count; i++) {
        char *description = get_game_description(repo_path, files[i]);
        printf("%s: %s\n", files[i], description);
        free(description);
    }
    
    // Clean up
    for (int i = 0; i < file_count; i++) {
        free(files[i]);
    }
    free(files);
}

// Built-in command: path
int builtin_path(char *new_path, char *repo_path) {
    if (!is_directory(new_path)) {
        print_error();
        return 0;
    }
    
    strcpy(repo_path, new_path);
    return 1;
}

// Run a game from the repository
void run_game(char *repo_path, char **args, int arg_count, char *input_file) {
    pid_t pid = fork();
    
    if (pid == 0) { // Child process
        // Handle input redirection
        if (input_file != NULL) {
            int fd = open(input_file, O_RDONLY);
            if (fd == -1) {
                print_error();
                exit(1);
            }
            dup2(fd, STDIN_FILENO);
            close(fd);
        }
        
        // Build path to the game
        char game_path[MAX_COMMAND_LENGTH];
        sprintf(game_path, "%s/%s", repo_path, args[0]);
        
        // Prepare args for execvp
        char *exec_args[arg_count + 1];
        for (int i = 0; i < arg_count; i++) {
            exec_args[i] = args[i];
        }
        exec_args[arg_count] = NULL;
        
        // Execute the game
        execvp(game_path, exec_args);
        
        // If exec fails
        print_error();
        exit(1);
    } else if (pid < 0) { // Fork failed
        print_error();
    } else { // Parent process
        int status;
        waitpid(pid, &status, 0);
    }
}

int main(int argc, char *argv[]) {
    // Check command line arguments
    if (argc != 2) {
        print_error();
        return 1;
    }
    
    // Check if the argument is a directory
    if (!is_directory(argv[1])) {
        print_error();
        return 1;
    }
    
    char repo_path[MAX_COMMAND_LENGTH];
    strcpy(repo_path, argv[1]);
    
    char *line = NULL;
    size_t line_size = 0;
    ssize_t line_length;
    
    // Main loop
    while (1) {
        printf("shelf-steam> ");
        fflush(stdout);
        
        // Read a line
        line_length = getline(&line, &line_size, stdin);
        if (line_length == -1) {
            break; // EOF
        }
        
        // Remove trailing newline
        if (line[line_length - 1] == '\n') {
            line[line_length - 1] = '\0';
        }
        
        // Skip empty commands
        if (strlen(line) == 0) {
            continue;
        }
        
        // Parse the command and arguments
        char *token;
        char *cmd_copy = strdup(line);
        char *saveptr;
        
        char *args[MAX_COMMAND_LENGTH];
        int arg_count = 0;
        char *input_file = NULL;
        int redirect = 0;
        
        token = strtok_r(cmd_copy, " \t", &saveptr);
        while (token != NULL) {
            if (strcmp(token, "<") == 0) {
                redirect = 1;
                token = strtok_r(NULL, " \t", &saveptr);
                if (token != NULL) {
                    input_file = strdup(token);
                    token = strtok_r(NULL, " \t", &saveptr);
                    if (token != NULL) {
                        // Error: Multiple arguments after redirection
                        print_error();
                        free(input_file);
                        input_file = NULL;
                        arg_count = -1; // Signal an error
                        break;
                    }
                } else {
                    // Error: No file after redirection
                    print_error();
                    arg_count = -1; // Signal an error
                    break;
                }
            } else if (redirect) {
                // Error: Multiple redirection operators
                print_error();
                free(input_file);
                input_file = NULL;
                arg_count = -1; // Signal an error
                break;
            } else {
                args[arg_count++] = strdup(token);
            }
            token = strtok_r(NULL, " \t", &saveptr);
        }
        
        free(cmd_copy);
        
        // Skip if parsing error
        if (arg_count == -1) {
            continue;
        }
        
        // Handle built-in commands
        if (strcmp(args[0], "exit") == 0) {
            if (arg_count > 1) {
                print_error();
            } else {
                // Free memory before exiting
                for (int i = 0; i < arg_count; i++) {
                    free(args[i]);
                }
                if (input_file) free(input_file);
                if (line) free(line);
                exit(0);
            }
        } else if (strcmp(args[0], "ls") == 0) {
            if (arg_count > 1) {
                print_error();
            } else {
                builtin_ls(repo_path);
            }
        } else if (strcmp(args[0], "path") == 0) {
            if (arg_count != 2) {
                print_error();
            } else {
                builtin_path(args[1], repo_path);
            }
        } else {
            // Run a game
            run_game(repo_path, args, arg_count, input_file);
        }
        
        // Clean up
        for (int i = 0; i < arg_count; i++) {
            free(args[i]);
        }
        if (input_file) free(input_file);
    }
    
    // Clean up
    if (line) free(line);
    
    return 0;
}
