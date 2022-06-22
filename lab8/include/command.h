#ifndef COMMAND_H
#define COMMAND_H

void error_buffer_overflow (char*);
void error_cmd_not_found(char*);

void cmd_help ();
void cmd_hello ();
void cmd_reboot ();
void cmd_ls ();
void cmd_cat ();
void cmd_test_malloc();
void cmd_test();
void cmd_exec();
void cmd_timer();
void cmd_timeout(char *s, unsigned int timeout);

#endif
