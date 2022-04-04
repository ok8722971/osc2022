#ifndef COMMAND_H
#define COMMAND_H

void error_buffer_overflow (char*);
void error_cmd_not_found(char*);

void cmd_help ();
void cmd_hello ();
void cmd_reboot ();

#endif
