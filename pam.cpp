#include "pam.h"

extern "C" {
	#include <security/pam_appl.h>
	#include <security/pam_misc.h>
	#include <signal.h>
}

#define TO_AUTHCHILD_RD_SHIFT 16
#define TO_PARENT_WR_SHIFT 0
#define PIPE_FD_MASK ((1 << TO_AUTHCHILD_RD_SHIFT) - 1)

void authenticate(int toParent[2], int toAuthChild[2])
{
	pam_handle_t *pamh = NULL;
	char *username;
	int retval;
	int auth_status;
	struct pam_conv conv;
	unsigned long fd = 0;
	fd = fd | toAuthChild[0] << TO_AUTHCHILD_RD_SHIFT
		| toParent[1] << TO_PARENT_WR_SHIFT;
	/* authentication child process */
	username=getenv("USER");
	conv.conv = pam_conversation;
	conv.appdata_ptr = (void *)fd;
	retval = pam_start("screensaver", username, &conv, &pamh);
	if(retval == PAM_SUCCESS)
		printf("PAM started successfully.\n");
	else
		printf("PAM started unsuccessfully.\n");
	printf("Invoke pam authentication.\n");
	auth_status = pam_authenticate(pamh, 0);
	printf("Complete pam authentication.\n");
	if(pam_end(pamh, retval) != PAM_SUCCESS){
		printf("Failed to terminate PAM.\n");
		_exit(1);
	}
	printf("PAM ended successfully.\n");
	kill(getppid(), SIGUSR1);
	char buffer[2];
	sprintf(buffer, "%d", auth_status);
	PIPE_OPS_SAFE(
		write(toParent[1], buffer, strlen(buffer))
	);
	printf("Auth status has been written to pipe.\n");
	::close(toParent[1]);
	::close(toAuthChild[0]);
	printf("Authenticate child process now exits.\n");
	_exit(0);
}

#define MAX_PASSWORD_LENGTH 1024
int pam_conversation(int num_msg, const struct pam_message **msg,
				struct pam_response **resp, void *appdata_ptr)
{
	unsigned long fd;
	int read_from_parent;
	int write_to_parent;
	int count;
	char *password;
	struct pam_response *tmp_save;

	printf("Into pam_conversation\n");

	fd = (unsigned long)appdata_ptr;
	read_from_parent = (fd >> TO_AUTHCHILD_RD_SHIFT) & PIPE_FD_MASK;
	write_to_parent = (fd >> TO_PARENT_WR_SHIFT) & PIPE_FD_MASK;
	count = num_msg;
	password = (char *)malloc(MAX_PASSWORD_LENGTH);
	*resp = (struct pam_response *)malloc(num_msg *
						sizeof(struct pam_response));
	tmp_save = (struct pam_response *)(*resp);
	memset(*resp, 0, num_msg * sizeof(struct pam_response));
	printf("Resolve PAM messages.\n");
	while(count-- >= 1){
		struct pam_message_object pam_msg_obj;
		pam_msg_obj.msg_style = (*msg)->msg_style;
		strncpy(pam_msg_obj.msg, (*msg)->msg, MAX_MSG_LENGTH);
		PIPE_OPS_SAFE(
			write(write_to_parent, &pam_msg_obj, sizeof(pam_msg_obj));
		);
		printf("PAM message has been written to pipe.\n");
		kill(getppid(), SIGUSR1);

		if ((*msg)->msg_style == PAM_PROMPT_ECHO_OFF
				|| (*msg)->msg_style == PAM_PROMPT_ECHO_ON){
			PIPE_OPS_SAFE(
				read(read_from_parent, password, MAX_PASSWORD_LENGTH);
			);
			printf("Password has been read from pipe.\n");
			(*resp)->resp = password;
			(*resp)->resp_retcode = 0;
		} else {
			;
		}
		if(count != 0){
			msg++;
			(*resp)++;
		}
	}

	(*resp) = tmp_save;

	printf("Out pam_conversation.\n");
	return PAM_SUCCESS;
}

