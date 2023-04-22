#include "utils.h"
#include "consts.h"

pid_t curr_background_cmds[MAX_BACKGROUND_CMDS];
char bg_cmds_strings[MAX_BACKGROUND_CMDS][COMMAND_LINE_LENGTH];

int main() {
    char command[COMMAND_LINE_LENGTH];
    int exit_status = 0;
    initialize_bg_cmds();
    while(1) {

        printf("hw1shell$ ");
        fgets(command, COMMAND_LINE_LENGTH, stdin);
        int cmd_length = strlen(command);
        clean_command(&command[0], cmd_length);
        cmd_length = strlen(command);
        
        if (cmd_length <= 1)
            continue;

        exit_status = parse_operation(command);

        if (exit_status == 0)
            check_running_bg_cmds();
        else
            break;
    }
    return 0;
}