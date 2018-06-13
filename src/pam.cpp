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
    retval = pam_start("ukui-screensaver", username, &conv, &pamh);
	if(retval == PAM_SUCCESS)
        qDebug("PAM started successfully.");
	else
        qDebug("PAM started unsuccessfully.");
    qDebug("Invoke pam authentication.");
	auth_status = pam_authenticate(pamh, 0);
    qDebug("Complete pam authentication.");
	if(pam_end(pamh, retval) != PAM_SUCCESS){
        qDebug("Failed to terminate PAM.");
		_exit(1);
	}
    qDebug("PAM ended successfully.");
	kill(getppid(), SIGUSR1);
	char buffer[16];
	sprintf(buffer, "%d", auth_status);
	PIPE_OPS_SAFE(
		write(toParent[1], buffer, strlen(buffer) + 1)
	);
    qDebug("Auth status has been written to pipe.");
	::close(toParent[1]);
	::close(toAuthChild[0]);
    qDebug("Authenticate child process now exits.");
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

    qDebug("Into pam_conversation");

	fd = (unsigned long)appdata_ptr;
	read_from_parent = (fd >> TO_AUTHCHILD_RD_SHIFT) & PIPE_FD_MASK;
	write_to_parent = (fd >> TO_PARENT_WR_SHIFT) & PIPE_FD_MASK;
	count = num_msg;
	password = (char *)malloc(MAX_PASSWORD_LENGTH);
	*resp = (struct pam_response *)malloc(num_msg *
						sizeof(struct pam_response));
	tmp_save = (struct pam_response *)(*resp);
	memset(*resp, 0, num_msg * sizeof(struct pam_response));
    qDebug("Resolve PAM messages.");
	while(count-- >= 1){
		struct pam_message_object pam_msg_obj;
		pam_msg_obj.msg_style = (*msg)->msg_style;
		strncpy(pam_msg_obj.msg, (*msg)->msg, MAX_MSG_LENGTH);
		PIPE_OPS_SAFE(
			write(write_to_parent, &pam_msg_obj, sizeof(pam_msg_obj));
		);
        qDebug("PAM message has been written to pipe.");
		kill(getppid(), SIGUSR1);

		if ((*msg)->msg_style == PAM_PROMPT_ECHO_OFF
				|| (*msg)->msg_style == PAM_PROMPT_ECHO_ON){
            int n;
			PIPE_OPS_SAFE(
                n = read(read_from_parent, password, MAX_PASSWORD_LENGTH);
			);
            qDebug("%d bytes response received from pipe.", n);
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

    qDebug("Out pam_conversation.");
	return PAM_SUCCESS;
}

