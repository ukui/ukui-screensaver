/*
 * Copyright (C) 2018 Tianjin KYLIN Information Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 *
**/
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
