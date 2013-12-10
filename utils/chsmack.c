/*
 * chsmack - Set smack attributes on files
 *
 * Copyright (C) 2011 Nokia Corporation.
 * Copyright (C) 2011, 2012, 2013 Intel Corporation
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 2.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
 * 02110-1301 USA
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/xattr.h>
#include <linux/xattr.h>
#include <sys/smack.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>

static const char usage[] =
	"Usage: %s [options] <path>\n"
	"options:\n"  
	" -a --access        set/remove "XATTR_NAME_SMACK"\n"  
	" -e --exec          set/remove "XATTR_NAME_SMACKEXEC"\n"  
	" -m --mmap          set/remove "XATTR_NAME_SMACKMMAP"\n"  
	" -t --transmute     set/remove "XATTR_NAME_SMACKTRANSMUTE"\n"
;

int main(int argc, char *argv[])
{
	static struct option options[] = {
		{"access", required_argument, 0, 'a'},
		{"exec", required_argument, 0, 'e'},
		{"mmap", required_argument, 0, 'm'},
		{"transmute", no_argument, 0, 't'},
		{NULL, 0, 0, 0}
	};

	/*  Buffers are zeroed automatically by keeping them static variables.
	 *  No separate memset is needed this way.
	 */
	static char access_buf[SMACK_LABEL_LEN + 1];
	static char exec_buf[SMACK_LABEL_LEN + 1];
	static char mmap_buf[SMACK_LABEL_LEN + 1];
	static int options_map[128];

	int transmute_flag = 0;
	int option_flag = 0;
	int rc;
	int c;
	int i;

	for (i = 0; options[i].name != NULL; i++)
		options_map[options[i].val] = i;

	while ((c = getopt_long(argc, argv, "a:e:m:t", options,
				NULL)) != -1) {
		if ((c == 'a' || c == 'e' || c == 'm')
		    && strnlen(optarg, SMACK_LABEL_LEN + 1)
		       == (SMACK_LABEL_LEN + 1)) {
			fprintf(stderr, "%s: %s: \"%s\" " \
					"exceeds %d characters.\n",
				basename(argv[0]), options[options_map[c]].name, optarg,
					 SMACK_LABEL_LEN);
			exit(1);
		}

		switch (c) {
			case 'a':
				strncpy(access_buf, optarg, SMACK_LABEL_LEN + 1);
				break;
			case 'e':
				strncpy(exec_buf, optarg, SMACK_LABEL_LEN + 1);
				break;
			case 'm':
				strncpy(mmap_buf, optarg, SMACK_LABEL_LEN + 1);
				break;
			case 't':
				transmute_flag = 1;
				break;
			default:
				printf(usage, basename(argv[0]));
				exit(1);
		}

		option_flag = 1;
	}

	for (i = optind; i < argc; i++) {
		if (option_flag) {
			if (strlen(access_buf) > 0) {
				rc = lsetxattr(argv[i], XATTR_NAME_SMACK,
					       access_buf, strlen(access_buf), 0);
				if (rc < 0)
					perror(argv[i]);
			}

			if (strlen(exec_buf) > 0) {
				rc = lsetxattr(argv[i], XATTR_NAME_SMACKEXEC,
					       exec_buf, strlen(exec_buf), 0);
				if (rc < 0)
					perror(argv[i]);
			}

			if (strlen(mmap_buf) > 0) {
				rc = lsetxattr(argv[i], XATTR_NAME_SMACKMMAP,
					       mmap_buf, strlen(mmap_buf), 0);
				if (rc < 0)
					perror(argv[i]);
			}

			if (transmute_flag) {
				rc = lsetxattr(argv[i], XATTR_NAME_SMACKTRANSMUTE,
					       "TRUE", 4, 0);
				if (rc < 0)
					perror(argv[i]);
			}
		} else {
			/* Print file path. */
			printf("%s", argv[i]);

			rc = lgetxattr(argv[i], XATTR_NAME_SMACK, access_buf,
				       SMACK_LABEL_LEN + 1);
			if (rc > 0) {
				access_buf[rc] = '\0';
				printf(" access=\"%s\"", access_buf);
			}

			rc = lgetxattr(argv[i], XATTR_NAME_SMACKEXEC, access_buf,
				       SMACK_LABEL_LEN + 1);
			if (rc > 0) {
				access_buf[rc] = '\0';
				printf(" execute=\"%s\"", access_buf);

			}
			rc = lgetxattr(argv[i], XATTR_NAME_SMACKMMAP, access_buf,
				       SMACK_LABEL_LEN + 1);
			if (rc > 0) {
				access_buf[rc] = '\0';
				printf(" mmap=\"%s\"", access_buf);
			}

			rc = lgetxattr(argv[i], XATTR_NAME_SMACKTRANSMUTE,
				       access_buf, SMACK_LABEL_LEN + 1);
			if (rc > 0) {
				access_buf[rc] = '\0';
				printf(" transmute=\"%s\"", access_buf);
			}

			printf("\n");
		}
	}

	exit(0);
}
