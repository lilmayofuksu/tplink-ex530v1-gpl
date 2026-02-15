/*
 * Dropbear - a SSH2 server
 * 
 * Copyright (c) 2002,2003 Matt Johnston
 * All rights reserved.
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE. */

/* Validates a user password */

#include "includes.h"
#include "session.h"
#include "buffer.h"
#include "dbutil.h"
#include "auth.h"
#include "runopts.h"
#include "md5.h"
#include "md5_interface.h"

#define SVR_USERNAME_LEN 65
enum TETHER_LOGIN_MODE
{
	TETHER_LOGIN_MODE_OLD = 1,
	TETHER_LOGIN_MODE_NEW = 2
};

#define TETHER_PORT	22
#define VOICE_PORT	22

#ifdef ENABLE_SVR_PASSWORD_AUTH

/* not constant time when strings are differing lengths. 
 string content isn't leaked, and crypt hashes are predictable length. */
static int constant_time_strcmp(const char* a, const char* b) {
	size_t la = strlen(a);
	size_t lb = strlen(b);

	if (la != lb) {
		return 1;
	}

	return constant_time_memcmp(a, b, la);
}

/* Process a password auth request, sending success or failure messages as
 * appropriate */
void svr_auth_password() {
	
	char * passwdcrypt = NULL; /* the crypt from /etc/passwd or /etc/shadow */
	char * testcrypt = NULL; /* crypt generated from the user's password sent */
	char * password;
	unsigned int passwordlen;

	unsigned int changepw;

	passwdcrypt = ses.authstate.pw_passwd;

#ifdef DEBUG_HACKCRYPT
	/* debugging crypt for non-root testing with shadows */
	passwdcrypt = DEBUG_HACKCRYPT;
#endif

	/* check if client wants to change password */
	changepw = buf_getbool(ses.payload);
	if (changepw) {
		/* not implemented by this server */
		send_msg_userauth_failure(0, 1);
		return;
	}

	password = buf_getstring(ses.payload, &passwordlen);

	/* the first bytes of passwdcrypt are the salt */
	testcrypt = crypt(password, passwdcrypt);
	m_burn(password, passwordlen);
	m_free(password);

	if (testcrypt == NULL) {
		/* crypt() with an invalid salt like "!!" */
		dropbear_log(LOG_WARNING, "User account '%s' is locked",
				ses.authstate.pw_name);
		send_msg_userauth_failure(0, 1);
		return;
	}

	/* check for empty password */
	if (passwdcrypt[0] == '\0') {
		dropbear_log(LOG_WARNING, "User '%s' has blank password, rejected",
				ses.authstate.pw_name);
		send_msg_userauth_failure(0, 1);
		return;
	}

	if (constant_time_strcmp(testcrypt, passwdcrypt) == 0) {
		/* successful authentication */
		dropbear_log(LOG_NOTICE, 
				"Password auth succeeded for '%s' from %s",
				ses.authstate.pw_name,
				svr_ses.addrstring);
		send_msg_userauth_success();
	} else {
		dropbear_log(LOG_WARNING,
				"Bad password attempt for '%s' from %s",
				ses.authstate.pw_name,
				svr_ses.addrstring);
		send_msg_userauth_failure(0, 1);
	}
}


#if DROPBEAR_PWD

//输入字符串格式为"00010A..."，每两个数字一起共32字节
/* !!! Can NOT be static, why??? */
void hexString2Md5Digest(unsigned char *md5Pswd)
{
	unsigned int index = 0;
	unsigned int val = 0;
	unsigned char md5_password[MD5_DIGEST_LEN] = {0};
	
	for (index = 0; index < MD5_DIGEST_LEN; ++index)
	{
		/* sscanf requires md5Pswd be '\0' terminated */
		sscanf(md5Pswd + 2 * index, "%02x", &val);
		*(md5_password + index) = val;
	}
	
	memcpy(md5Pswd, md5_password, MD5_DIGEST_LEN);
}

void svr_chk_pwd(const char* username, const char* pwdfile)
{
	char isFactoryDefault = 'n';
	char svr_username[SVR_USERNAME_LEN] = {0};
	unsigned char svr_password[MD5_DIGEST_LEN*2 + 1] = {0};
	unsigned char * password;
	unsigned int passwordlen;
	unsigned int changepw;
	unsigned int loginMode = 0;
	/* dingcheng:add for service auth */
#if ALLOW_SERVICE_AUTH
	char servicename[ALLOW_SERVICE_TYPENUM][SVR_USERNAME_LEN] = {{0}};
	char servicepwd[ALLOW_SERVICE_TYPENUM][MD5_DIGEST_LEN*2 + 1] = {{0}};
	int index = 0;
	int i = 0;
#endif/* ALLOW_SERVICE_AUTH */
	/* end added */

	//add by chenming for sftp 2016-6-8
	char sftp_homedir[MAX_SFTP_USER_NUM][MAXPATHLEN] = {{0}};
	char sftp_usrname[MAX_SFTP_USER_NUM][SVR_USERNAME_LEN] = {{0}};
	char sftp_pwd[MAX_SFTP_USER_NUM][2 * MD5_DIGEST_LEN + 1] = {{0}};
	int sftp_port = TETHER_PORT;	//default sftp port
	int sftp_ideltm = 0;
	int sftp_usrindex = 0;

	int local_port = 22;

	FILE   *auth_file = NULL;
	char   *szLine = NULL;
	int len = 0;
	char *szPos = NULL;

	TRACE(("enter svr_chk_pwd"))
	
	if (!(auth_file = fopen(pwdfile, "r")))
               return ;
	
	while (getline(&szLine, &len, auth_file) != -1)
	{
		if(szLine[strlen(szLine) - 1] == '\n')
		{
			szLine[strlen(szLine) - 1] = '\0';
		}
		if (strncmp(SFTP_USR_PREFIX, szLine, strlen(SFTP_USR_PREFIX)) == 0)
		{
			if (3 != sscanf(szLine, SFTP_USR_PREFIX"%[^:]:%[^:]:%[^:]", 
				sftp_usrname[sftp_usrindex], sftp_pwd[sftp_usrindex], sftp_homedir[sftp_usrindex]))
			{
				TRACE(("get buf with %s format invalid!", szLine))
			}
			else
			{
				sftp_usrindex++;	
			}
			continue;			
		}
		else if (strncmp(SFTP_PORT_PREFIX, szLine, strlen(SFTP_PORT_PREFIX)) == 0)
		{
			if (1 != sscanf(szLine, SFTP_PORT_PREFIX"%d", &sftp_port))
			{
				TRACE(("get buf with %s format invalid, fail get port!", szLine))
			}
			continue;
			
		}
		else if (strncmp(SFTP_IDEL_TIMEOUT, szLine, strlen(SFTP_IDEL_TIMEOUT)) == 0)
		{
			if (1 != sscanf(szLine, SFTP_IDEL_TIMEOUT"%d", &sftp_ideltm))
			{
				TRACE(("get buf with %s format invalid, fail get idel time out!", szLine))
			}
			continue;
		}
		szPos = strchr(szLine, ':');
		if( szPos != NULL)
		{
			szPos++;
			if(strncmp( "username", szLine, strlen("username")) == 0)
			{
				strncpy(svr_username, szPos, SVR_USERNAME_LEN);
				svr_username[strlen(szPos)] = '\0';
			}
			else if(strncmp( "password", szLine, strlen("password")) == 0)
			{
				strncpy(svr_password, szPos, MD5_DIGEST_LEN*2);
				svr_password[strlen(szPos)] = '\0';
				hexString2Md5Digest(svr_password);
			}			
			else if (strncmp("isFactoryDefault", szLine, strlen("isFactoryDefault")) == 0)
			{
				memcpy(&isFactoryDefault, szPos, 1);
			}
			else if (strncmp("loginMode", szLine, strlen("loginMode")) == 0)
			{
				sscanf(szLine, "loginMode:%u\n", &loginMode);
			}
			/* dingcheng:add for service auth */
#if ALLOW_SERVICE_AUTH
			else if(strncmp( "servicename", szLine, strlen("servicename")) == 0)
			{
				strncpy(servicename[index], szPos, SVR_USERNAME_LEN);
				servicename[index][strlen(szPos)] = '\0';
			}
			else if(strncmp( "servicepwd", szLine, strlen("servicepwd")) == 0)
			{
				strncpy(servicepwd[index], szPos, MD5_DIGEST_LEN*2);
				servicepwd[index][strlen(szPos)] = '\0';
				hexString2Md5Digest(servicepwd[index]);
				if (index < ALLOW_SERVICE_TYPENUM - 1)
				{
					index ++;
				}
			}	
#endif/* ALLOW_SERVICE_AUTH */
			/* end added */
			
		}
    }
	fclose(auth_file);

	/* check if client wants to change password */
	changepw = buf_getbool(ses.payload);
	if (changepw) {
		/* not implemented by this server */
		send_msg_userauth_failure(0, 1);
		return;
	}
	
	password = buf_getstring(ses.payload, &passwordlen);		


	// for sftp CM 2016-6-8
	sscanf(svr_ses.l_addrstring, "%*[^:]:%d", &local_port);

	TRACE(("local port:%d, sftp_port:%d", local_port, sftp_port))

	if (local_port != sftp_port)	//not for sftp
	{
		CLR_SFTP_AUTH(ses.authstate.srv_type);
	}
	else 
	{
		TRACE(("sftp_usrindex:%d", sftp_usrindex))
		for( i = 0; i < sftp_usrindex; i++)
		{
			TRACE(("usrname:%s, sftp usrname[%d]:%s, sftp pwd:%s, passwd:%s", 
				username, i, sftp_usrname[i], sftp_pwd[i], password))
			hexString2Md5Digest(sftp_pwd[i]);
			if ((strlen(username) == strlen(sftp_usrname[i])) &&
				strncmp(username, sftp_usrname[i], strlen(username)) == 0 &&
				md5_verify_digest(sftp_pwd[i], password, strlen(password)))
			{
				SET_SFTP_AUTH(ses.authstate.srv_type);	
				//here we need resetting the password and username
				if (ses.authstate.pw_dir)
					m_free(ses.authstate.pw_dir);
				if (ses.authstate.pw_name)
					m_free(ses.authstate.pw_name);
				if (ses.authstate.pw_passwd)
					m_free(ses.authstate.pw_passwd);
				if (ses.authstate.pw_shell)
					m_free(ses.authstate.pw_shell);

				ses.authstate.pw_dir = m_strdup(sftp_homedir[i]);
				ses.authstate.pw_passwd = m_strdup(password);
				ses.authstate.pw_name = m_strdup(username);
				ses.authstate.pw_shell = m_strdup("/bin/sh");
				ses.authstate.pw_gid = ses.authstate.pw_uid = 500;	//here need we do?	
				opts.idle_timeout_secs = sftp_ideltm;
			}
		}
	}

	
	/* dingcheng:add for service auth */
#if ALLOW_SERVICE_AUTH
	if (local_port != VOICE_PORT)
	{
		CLR_VOICE_AUTH(ses.authstate.srv_type);
	}
	else
	{
		for (i = 0;i <= index;i++)
		{
			TRACE(("username %s servicename %s password %s",username,servicename[i],
				password))
			if (strncmp(username, servicename[i], strlen(username)) == 0 && 
				md5_verify_digest( servicepwd[i], password, strlen(password)))
			{
				TRACE(("svr_chk_pwd send_msg_userauth_success"))
				dropbear_log(LOG_NOTICE, 
						"Password auth succeeded for '%s' from %s",
						username,
						svr_ses.addrstring);
				strncpy(ses.mark,servicename[i],SVR_USERNAME_LEN);
				ses.mark[strlen(servicename[i])] = '\0';
				SET_VOICE_AUTH(ses.authstate.srv_type);
				break;
			}
		}
	}
#endif/* ALLOW_SERVICE_AUTH */
	/* end added */

	if (local_port != TETHER_PORT)
	{
		CLR_TETHER_AUTH(ses.authstate.srv_type);
	}	
	else if ((loginMode == TETHER_LOGIN_MODE_NEW) && (isFactoryDefault == 'y'))
	{
		TRACE(("svr_chk_pwd send_msg_userauth_success"))
#if ALLOW_SERVICE_AUTH
		if (!CHK_VOICE_AUTH(ses.authstate.srv_type))
		{
			//voice app check header is preferred 
			strncpy(ses.mark, "admin", 5);
			ses.mark[5] = '\0';
		}
#endif/* ALLOW_SERVICE_AUTH */
		SET_TETHER_AUTH(ses.authstate.srv_type);
	}
	else if ((loginMode == TETHER_LOGIN_MODE_OLD) || (isFactoryDefault == 'n'))
	{
		TRACE(("loginMode = %d, isFactoryDefault = %x,username = %s", loginMode, isFactoryDefault,username))
		TRACE(("svr_username = %s, svr_password = %s, password = %s,result = %d",svr_username,svr_password, password,md5_verify_digest(svr_password, password, strlen(password))))
		if( (strlen(username) == strlen(svr_username)) && 
	     	strncasecmp(username, svr_username, strlen(username)) == 0 &&
			md5_verify_digest( svr_password, password, strlen(password) ) )
		{
			TRACE(("svr_chk_pwd send_msg_userauth_success"))
			dropbear_log(LOG_NOTICE, 
					"Password auth succeeded for '%s' from %s",
					username,
					svr_ses.addrstring);
			/* dingcheng:add for service auth */
#if ALLOW_SERVICE_AUTH
			if (!CHK_VOICE_AUTH(ses.authstate.srv_type))
			{
				strncpy(ses.mark,"admin",5);
				ses.mark[5] = '\0';
			}
#endif/* ALLOW_SERVICE_AUTH */
			/* end added */	
			SET_TETHER_AUTH(ses.authstate.srv_type);
		}	
	}

	if (CHK_SFTP_AUTH(ses.authstate.srv_type) 
		|| CHK_TETHER_AUTH(ses.authstate.srv_type)
		|| CHK_VOICE_AUTH(ses.authstate.srv_type))
	{
		TRACE(("svr_chk_pwd send_msg_userauth_success"))
		dropbear_log(LOG_NOTICE, 
				"Password auth succeeded for '%s' from %s",
				username,
				svr_ses.addrstring);
		send_msg_userauth_success();
	}
	else
	{
		TRACE(("svr_chk_pwd send_msg_userauth_failure"))
		dropbear_log(LOG_WARNING,
				"Bad password attempt for '%s' from %s",
				username,
				svr_ses.addrstring);
		send_msg_userauth_failure(0, 1);
	}

out:
	m_burn(password, passwordlen);
	m_free(password);
}

#endif

#endif
