#ifndef PAM_H
#define PAM_H

#include <errno.h>
#include <string.h>
#include <QDebug>

void authenticate(int toParent[2], int toAuthChild[2]);

int pam_conversation(int num_msg, const struct pam_message **msg,
				struct pam_response **resp, void *appdata_ptr);

#define MAX_MSG_LENGTH 1024
struct pam_message_object {
	int msg_style;
	char msg[MAX_MSG_LENGTH + 1];
} __attribute__((packed));

#define PIPE_OPS_SAFE(statement) do { \
	int return_value = statement; \
    if (return_value == -1)\
        qWarning("PIPE write/read error: %s", strerror(errno)); \
} while (0)

#endif // PAM_H
