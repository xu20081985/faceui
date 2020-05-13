#ifndef WPA_CLI_H
#define WPA_CLI_H
extern "C"{

	int wpa_cli_cmd_function(int argc, char *argv[],char*buf);
	int wpa_cli_start(void);
	int wpa_cli_stop();
	int wpa_cli_pending();
	int wpa_cli_recv(char*dstbuf);
	int wpa_cli_cmd_function(int argc, char *argv[],char*buf);

}
#endif

