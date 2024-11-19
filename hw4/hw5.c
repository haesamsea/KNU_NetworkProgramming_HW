#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <time.h>

int child_elapsed = 0;
int parent_elapsed = 0;

void child_handler(int sig) {
    child_elapsed += 5;
    printf("[Child] timeout: 5, elapsed time: %d seconds(%d)\n", child_elapsed, child_elapsed / 5);
    if (child_elapsed >= 25) {
        exit(5); // Child process exits after 25 seconds with return value 5
    }
}

void parent_handler(int sig) {
    parent_elapsed += 2;
    printf("<Parent> timeout: 2, elapsed time: %d seconds\n", parent_elapsed);
}

void sigchld_handler(int sig) {
    int status;
    waitpid(-1, &status, WNOHANG);
    if (WIFEXITED(status)) {
        printf("<Parent> Child process ended with return value: %d\n", WEXITSTATUS(status));
    }
    // Parent continues running after child exits
}

void sigint_handler(int sig) {
    char choice;
    printf("\nDo you want to exit (y or Y to exit)? ");
    scanf(" %c", &choice);
    if (choice == 'y' || choice == 'Y') {
        exit(0);
    }
}

int main() {
    struct sigaction sa_child, sa_parent, sa_sigchld, sa_sigint;
    
    pid_t pid = fork();
    
    if (pid < 0) {
        perror("fork failed");
        return 1;
    } else if (pid == 0) { // Child process
        sa_child.sa_handler = child_handler;
        sigemptyset(&sa_child.sa_mask);
        sa_child.sa_flags = 0;
        sigaction(SIGALRM, &sa_child, NULL);

        for (int i = 0; i < 5; i++) {
            alarm(5);
            pause(); // Wait for the signal
        }
       
    } else { // Parent process
        printf("Parent process created.\n");
        printf("Child process created.\n");

        sa_parent.sa_handler = parent_handler;
        sigemptyset(&sa_parent.sa_mask);
        sa_parent.sa_flags = 0;
        sigaction(SIGALRM, &sa_parent, NULL);

        sa_sigchld.sa_handler = sigchld_handler;
        sigemptyset(&sa_sigchld.sa_mask);
        sa_sigchld.sa_flags = 0;
        sigaction(SIGCHLD, &sa_sigchld, NULL);

        sa_sigint.sa_handler = sigint_handler;
        sigemptyset(&sa_sigint.sa_mask);
        sa_sigint.sa_flags = 0;
        sigaction(SIGINT, &sa_sigint, NULL);

        while (1) {
            alarm(2);
            pause(); 
        }
    }

    return 0;
}
