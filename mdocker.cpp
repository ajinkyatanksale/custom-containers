#include <iostream>
#include <unistd.h>
#include <bits/stdc++.h>
#include <sys/wait.h>
#include <sched.h>
#include <sys/mount.h>

using namespace std;

typedef struct args {
    int argc;
    char **cmdv;
} args;

typedef struct command {
    string cmd;
    char **cmds;
} command;


void change_root_fs () {
    if (chdir("/home/ajinkyatanksale/new/alpine/") < 0) {
        cout << "Change directory call failed" << endl;
        perror(0);
    }
    if (chroot(".") < 0) {
        cout << "Change root call failed" << endl;
        perror(0);
    }
}

void change_hostname (string new_host_name) {
    if (sethostname(new_host_name.c_str(), new_host_name.size()) < 0) {
        cout << "Change hostname call failed" << endl;
        perror(0);
    }
}

void mount_new_proc () {
    if (mount("proc", "/proc", "proc", 0, nullptr) < 0) {
        cout << "Mounting virtual proc failed" << endl;
        perror(0);        
    }
}

command* create_command (args *arguments) {
    command *new_cmd = (command *)malloc(sizeof(command));
    char *cmds[arguments->argc-1];
    
    new_cmd->cmds = (char**)malloc((arguments->argc-1) * sizeof(char));
    new_cmd->cmd = arguments->cmdv[2];
    for (int i = 2; i < arguments->argc; i++) {
        new_cmd->cmds[i-2] = (char*)malloc(10 * sizeof(char));
        memcpy(new_cmd->cmds[i-2], arguments->cmdv[i], sizeof(arguments->cmdv[i])/sizeof(char));
    }
    new_cmd->cmds[arguments->argc-2] = NULL;
    return new_cmd;
}

int docker_process (void *args) {
    struct args *argument = (struct args*) args;
    command *cmd = create_command(argument);
    change_hostname("Ajinkya");
    change_root_fs();
    mount_new_proc();
    if (execv("/bin/busybox", cmd->cmds) < 0) {
        cout << "Error executing command " << cmd << endl;
        perror(0);
    }
    umount("/proc");
    return 0;
}

void run_command (int argc, char *argv[]) {
    if (string(argv[1]) == string("run")) {
        void *child_stack = malloc(1024 * 1024);
        args new_args = {argc, argv};
        pid_t child_pid = clone(docker_process, child_stack + (1024 * 1024),  CLONE_NEWUTS | CLONE_NEWPID | CLONE_NEWNS | CLONE_NEWUSER | SIGCHLD , &new_args);
        if (child_pid < 0) {
            cout << "Error creating child process" << endl;
            perror(0);
        }
        else if (child_pid > 0) {
            waitpid(child_pid, nullptr, 0);
        }
    }
    else {
        cout << "Incorrect docker command" << endl;
    }
}

int main (int argc, char *argv[]) {
    run_command(argc, argv);
}

// void create_new_process (args *arguments) {
//     pid_t pid = fork();
//     if (pid == -1) {
//         cout << "Error!!";
//         exit(1);
//     }
//     else if (pid == 0) {
//         unshare(CLONE_NEWUTS | CLONE_NEWUSER | SIGCHLD);
        
//         command *cmd = create_command(arguments);
//         execv(("/bin/" + cmd->cmd).c_str(), cmd->cmds);
//         perror(0);
        
//     }
//     else {
//         waitpid(pid, NULL, 0);
//     }
// }