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
#ifdef INCLUDE_AGINET_APP_V2
#include <os_msg.h>
#endif /*INCLUDE_AGINET_APP_V2*/
#ifdef INCLUDE_CLS_L1_IMDA_TPAPP
 #include "ssh.h"
#endif /* INCLUDE_CLS_L1_IMDA_TPAPP */

#define uchar unsigned char
#define uint unsigned int

#define SHA256_PLAIN_TEXT_LEN		(129)
#define SHA256_DIGEST_LEN			(32)
#define SVR_USERNAME_LEN			(65)
#define SVR_PASSWORD_LEN			(65)
#define SVR_SALT_LEN				(48)
#define SVR_ADMIN_LEN				(6)
#define BUF_LEN						(160)

#define SHELL_SH					"/bin/sh"
#define SHELL_CLI					"/bin/cli"

#define DROPBEAR_AUTH				"/var/tmp/dropbear/dropbearpwd"
#define DROPBEAR_AUTH_CONFIG		"/var/tmp/dropbear/config"
#define SFTP_CFG_FILE				"/var/tmp/dconf/sftpcfg"
#define SFTP_HOME_DIR				"/var/vsftp/root"

#define ACCESS_USER_PREFIX			"user="
#define ACCESS_APP_NAME				"dropbear"
#define ACCESS_VOICE_MASK			"admin"

#define M_FREE(x)      \
	{                  \
		if (x)         \
		{              \
			m_free(x); \
		}              \
	}

enum USER_TYPE
{
	USER_TYPE_ADMIN			= 1 << 0,
	USER_TYPE_USER			= 1 << 1,
	USER_TYPE_APP			= 1 << 2,
	USER_TYPE_SFTP			= 1 << 3,
	USER_TYPE_SSH			= 1 << 4,
	USER_TYPE_VOICE			= 1 << 5,
	USER_TYPE_CLOUD			= 1 << 6,
};

enum TETHER_LOGIN_MODE
{
	TETHER_LOGIN_MODE_OLD = 1,
	TETHER_LOGIN_MODE_NEW = 2
};

#ifdef INCLUDE_CLS_L1_IMDA_TPAPP
 #define TETHER_PORT		20001
#else /* ! INCLUDE_CLS_L1_IMDA_TPAPP */
 #define TETHER_PORT		22
#endif /* INCLUDE_CLS_L1_IMDA_TPAPP */
#define VOICE_PORT			22

#ifdef INCLUDE_AGINET_APP_V2
static int send_account_login_msg(const char *username);
#endif /* INCLUDE_AGINET_APP_V2 */
#ifdef INCLUDE_CLOUD_V2
static int send_cloudauth_msg_and_getresult(const char *email,const char *passwd);
static int username_is_valid_char(const char* name,unsigned char dot);
#endif /* INCLUDE_CLOUD_V2 */
static void cstr_strncpy(char *pDest, const char *pSrc, size_t n);

#if DROPBEAR_SVR_PASSWORD_AUTH

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
void svr_auth_password(int valid_user) {
	
	char * passwdcrypt = NULL; /* the crypt from /etc/passwd or /etc/shadow */
	char * testcrypt = NULL; /* crypt generated from the user's password sent */
	char * password = NULL;
	unsigned int passwordlen;
	unsigned int changepw;

	/* check if client wants to change password */
	changepw = buf_getbool(ses.payload);
	if (changepw) {
		/* not implemented by this server */
		send_msg_userauth_failure(0, 1);
		return;
	}

	password = buf_getstring(ses.payload, &passwordlen);
	if (valid_user && passwordlen <= DROPBEAR_MAX_PASSWORD_LEN) {
		/* the first bytes of passwdcrypt are the salt */
		passwdcrypt = ses.authstate.pw_passwd;
		testcrypt = crypt(password, passwdcrypt);
	}
	m_burn(password, passwordlen);
	m_free(password);

	/* After we have got the payload contents we can exit if the username
	is invalid. Invalid users have already been logged. */
	if (!valid_user) {
		send_msg_userauth_failure(0, 1);
		return;
	}

	if (passwordlen > DROPBEAR_MAX_PASSWORD_LEN) {
		dropbear_log(LOG_WARNING,
				"Too-long password attempt for '%s' from %s",
				ses.authstate.pw_name,
				svr_ses.addrstring);
		send_msg_userauth_failure(0, 1);
		return;
	}

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

/* 
 * fn			static void cstr_strncpy(char *pDest, const char *pSrc, size_t n)
 * brief		copy a string
 *
 * param[out]	pDest  - destination string
 * param[in]	pSrc - source string
 * param[in]	n - not more than "n" bytes of source string are copied
 *
 * return		N/A.	
 *
 * note			1.If there is no null byte among the first n bytes of src,
 *				this function will set the Nth byte to '\0'. 
 *				2.You must ensure that dest is large enough.
 */
static void cstr_strncpy(char *pDest, const char *pSrc, size_t n)
{
	dropbear_assert((pDest != NULL) && (pSrc != NULL));
	dropbear_assert(n > 0);

	/* copy first (n - 1) bytes */ 
	strncpy(pDest, pSrc, (n - 1));
	/* set the Nth byte to '\0' */
	pDest[n - 1] = '\0';

	return;
}

int cen_sha256MakeDigestStr(char *res, const char *src, size_t n)
{
	hash_state md;
    int index = 0;
	uchar str[SHA256_DIGEST_LEN * 2 + 1] = {0}; /* dbs: add +1 to avoid array overflow 29Apr15 */

    // sha256 计算
	sha256_init(&md);
	sha256_process(&md, src, strlen(src));
	sha256_done(&md, str);
	
    // 转换为字符串
	for (index = 0; index < SHA256_DIGEST_LEN; ++index)
	{
		snprintf(res + 2 * index, sizeof(str), "%02x", str[index]);
	}
	res[SHA256_DIGEST_LEN * 2] = 0;

	return 0;
}

int cen_md5MakeDigestStr(char *res, const char *src, size_t n)
{
    MD5_CTX ctx;
    int index = 0;
    uchar str[MD5_DIGEST_LEN * 2 + 1] = {0};
	
	MD5_Init(&ctx);
	MD5_Update(&ctx, src, strlen(src));
	MD5_Final(str, &ctx);

    // 转换为字符串
	for (index = 0; index < MD5_DIGEST_LEN; ++index)
	{
		snprintf(res + 2 * index, sizeof(str), "%02x", str[index]);
	}
	res[MD5_DIGEST_LEN * 2] = 0;

	return 0;
}

uint ipStr2Val(char *str)
{
	uint tmp[4] = {0};

	sscanf(str, "%u.%u.%u.%u", &tmp[0], &tmp[1], &tmp[2], &tmp[3]);

	return (tmp[0] << 24) | (tmp[1] << 16) | (tmp[2] << 8) | tmp[3];
}

/* 获取形如 key:%d 的参数 */
int access_get_param(char *pFileName, char *_fsp, void *v)
{
	FILE *pFile = NULL;
    char *str = NULL;
    char *fsp = NULL;
	char _str[BUF_LEN] = {0};
    size_t len = 0;

	if (_fsp == NULL || v == NULL || !(pFile = fopen(pFileName, "r")))
	{
		return -1;
	}

    while (NULL != fgets(_str, sizeof(_str), pFile))
    {
		str = _str;
        fsp = _fsp;
        // 该函数仅在本地使用， fsp 一定包含结束符
        while (*fsp && *str == *fsp)
        {
            str++;
            fsp++;
        }

        if (*fsp != '%')
        {
            continue;
        }

        if (1 != sscanf(str, fsp, v))
        {
            TRACE(("get buf with %s format invalid!", _fsp))
        }

        break;
    }

	fclose(pFile);

    return 0;
}

int is_lan_ip(char *ipStr)
{
	uint ip = 0;
	uint lan1_enable = 0;
	uint lan1_ip = 0;
	uint lan1_mask = 0;
	uint lan2_enable = 0;
	uint lan2_ip = 0;
	uint lan2_mask = 0;

	ip = ipStr2Val(ipStr);

	access_get_param(DROPBEAR_AUTH_CONFIG, "lan1Enable=%u", &lan1_enable);
	if(lan1_enable)
	{
		access_get_param(DROPBEAR_AUTH_CONFIG, "lan1IpAddr=%u", &lan1_ip);
		access_get_param(DROPBEAR_AUTH_CONFIG, "lan1IpMask=%u", &lan1_mask);
		if (0 == lan1_ip || 0 == lan1_mask)
		{
			return 0;
		}
		if ((ip & lan1_mask) == (lan1_ip & lan1_mask))
		{
			return 1;
		}
	}

	access_get_param(DROPBEAR_AUTH_CONFIG, "lan2Enable=%u", &lan2_enable);
	if(lan2_enable)
	{
		access_get_param(DROPBEAR_AUTH_CONFIG, "lan2IpAddr=%u", &lan2_ip);
		access_get_param(DROPBEAR_AUTH_CONFIG, "lan2IpMask=%u", &lan2_mask);
		if (0 == lan2_ip || 0 == lan2_mask)
		{
			return 0;
		}
		if ((ip & lan2_mask) == (lan2_ip & lan2_mask))
		{

			return 1;
		}
	}

	return 0;
}

int check_port_ssh(uint port, int isLan)
{
	uint lPort = 0;
	uint rPort = 0;
	int lEnable = 0;
	int rEnable = 0;

	if (isLan)
	{
		access_get_param(DROPBEAR_AUTH_CONFIG, "sshLocalEnable=%u", &lEnable);
		access_get_param(DROPBEAR_AUTH_CONFIG, "sshLocalPort=%u", &lPort);
		if (lEnable && port == lPort)
		{
			return 1;
		}
	}
	else
	{
		access_get_param(DROPBEAR_AUTH_CONFIG, "sshRemoteEnable=%u", &rEnable);
		access_get_param(DROPBEAR_AUTH_CONFIG, "sshRemotePort=%u", &rPort);
		if (rEnable && port == rPort)
		{
			return 1;
		}
	}

	return 0;
}

uint access_auth_account(char *pFileName, char *pInputName, char *pInputPwd)
{
	FILE *pFile = NULL;
	char userName[SVR_USERNAME_LEN] = {0};
	char userPwd[SVR_PASSWORD_LEN] = {0};
	char userSalt[SVR_SALT_LEN] = {0};
	char pwd[SVR_PASSWORD_LEN] = {0};
	char buf[BUF_LEN] = {0}; // 长度设置的足够大
	uint uflag = 0;
	uint ret = 0;

	if (!(pFile = fopen(pFileName, "r")))
	{
		return 0;
	}

	while (NULL != fgets(buf, sizeof(buf), pFile))
	{
		userSalt[0] = '\0';
		if (0 != strncmp(buf, ACCESS_USER_PREFIX, sizeof(ACCESS_USER_PREFIX) - 1) || 3 > sscanf(buf, ACCESS_USER_PREFIX "%u:%[^:]:%[^:]:%s", &uflag, userName, userPwd, userSalt))
		{
			continue;
		}

		/* 遗留问题，APP不输入用户名时，使用dropbear作为用户名，因此需要特殊处理 */
		if ((uflag & USER_TYPE_APP) && 0 == strncmp(pInputName, ACCESS_APP_NAME, sizeof(ACCESS_APP_NAME)))
		{
			uflag = USER_TYPE_APP;
		}

		if (USER_TYPE_APP != uflag && 0 != strncasecmp(pInputName, userName, sizeof(userName)))
		{
			continue;
		}

#ifdef INCLUDE_SAVE_KEY_AS_HASH
        if ('\0' != userSalt[0])
        {
			snprintf(buf, sizeof(buf), "%s%s%s", userName, userSalt, pInputPwd);
			cen_sha256MakeDigestStr(pwd, buf, sizeof(pwd));
        }
        else
#endif /* INCLUDE_SAVE_KEY_AS_HASH */
        {
			cen_md5MakeDigestStr(pwd, pInputPwd, sizeof(pwd));
        }

        if (0 == strncmp(pwd, userPwd, strlen(pwd)))
        {
			ret = uflag;
			break;
        }
	}

	fclose(pFile);

	return ret;
}

#ifdef INCLUDE_CLS_L1_IMDA_TPAPP
int clear_fail_count()
{
	FILE *count_file = NULL;
	int count = 0;
	int offset = 0;
	int ret = 0;
	long start_lock_time = 0;
	char mac[SSH2_MAX_MAC_LEN] = {0};
	char mac_in_count_file[SSH2_MAX_MAC_LEN] = {0};

	count_file = fopen(DROPBEAR_FAIL_COUNT, "r+");
	if (count_file)
	{
		ret = get_mac_from_arptable(mac, SSH2_MAX_MAC_LEN);
		if (ret == DROPBEAR_FAILURE)
		{
			fclose(count_file);
			return ret;
		}

		do
		{
			memset(mac_in_count_file, '\0', SSH2_MAX_MAC_LEN);
			offset = ftell(count_file);
			ret = fscanf(count_file, "%s %d %ld\n", mac_in_count_file, &count, &start_lock_time);
			if (ret == EOF)
			{
				fclose(count_file);
				return ret;
			}
			else if (ret != 3) /*3 is the number of read args */
			{
				continue;
			}

			if (!strncmp(mac_in_count_file, mac, SSH2_MAX_MAC_LEN))
			{
				fseek(count_file, offset, SEEK_SET); /* find the location of begin of target line */
				break;
			}

		} while (ret != EOF);

		if (count < SSH2_PASSWD_FAILURE_MAX_TIMES)
		{

			fprintf(count_file, "%s %02d %012ld\n", mac, 0, 0);
		}

		fclose(count_file);
	}
	return ret;
}
#endif /* INCLUDE_CLS_L1_IMDA_TPAPP */

void svr_chk_pwd(const char* username, const char* pwdfile)
{
	uchar *password = NULL;
	uint passwordlen = 0;
	uint uflag = 0;
	uint isFactoryDefault = 0;
	uint loginMode = 0;
	uint local_port = TETHER_PORT;
	uint sftp_port = TETHER_PORT;
	uint voice_port = TETHER_PORT;
	uint islan = 0;

	TRACE(("enter svr_chk_pwd"))

	/* check if client wants to change password */
	if (buf_getbool(ses.payload))
	{
		/* not implemented by this server */
		send_msg_userauth_failure(0, 1);
		return;
	}

	sscanf(svr_ses.l_addrstring, "%*[^:]:%d", &local_port);

	password = buf_getstring(ses.payload, &passwordlen);

	islan = is_lan_ip(svr_ses.addrstring);
	uflag |= access_auth_account(DROPBEAR_AUTH, username, password);
	uflag |= access_auth_account(SFTP_CFG_FILE, username, password);

	dropbear_log(LOG_NOTICE,
					 "uflag: %u",
					 uflag);

#ifdef ALLOW_SERVICE_AUTH
	if ((uflag & USER_TYPE_VOICE) && (local_port == voice_port))
	{
		SET_VOICE_AUTH(ses.authstate.srv_type);

		TRACE(("svr_chk_pwd send_msg_userauth_success"))
		dropbear_log(LOG_NOTICE,
					 "Password auth succeeded for '%s' from %s",
					 username,
					 svr_ses.addrstring);
		cstr_strncpy(ses.mark, username, sizeof(ses.mark));
	}
#endif /* ALLOW_SERVICE_AUTH */

	access_get_param(DROPBEAR_AUTH_CONFIG, "loginMode=%u", &loginMode);
	access_get_param(DROPBEAR_AUTH_CONFIG, "isFactoryDefault=%u", &isFactoryDefault);
	if ((loginMode == TETHER_LOGIN_MODE_NEW) && (isFactoryDefault == 1))
    {
        TRACE(("svr_chk_pwd send_msg_userauth_success"))
 #if ALLOW_SERVICE_AUTH
        if (!CHK_VOICE_AUTH(ses.authstate.srv_type))
        {
            // voice app check header is preferred
            cstr_strncpy(ses.mark, ACCESS_VOICE_MASK, sizeof(ses.mark));
        }
 #endif /* ALLOW_SERVICE_AUTH */
        SET_TETHER_AUTH(ses.authstate.srv_type);
    }
    else if ((loginMode == TETHER_LOGIN_MODE_OLD) || (isFactoryDefault == 0))
    {
        TRACE(("loginMode = %d, isFactoryDefault = %x,username = %s", loginMode, isFactoryDefault, username))
        if (uflag & USER_TYPE_APP)
        {
            SET_TETHER_AUTH(ses.authstate.srv_type);
            TRACE(("svr_chk_pwd send_msg_userauth_success"))
            dropbear_log(LOG_NOTICE,
                         "Password auth succeeded for '%s' from %s",
                         username,
                         svr_ses.addrstring);
 #ifdef INCLUDE_CLS_L1_IMDA_TPAPP
            clear_fail_count();
 #endif /* INCLUDE_CLS_L1_IMDA_TPAPP */

 #if ALLOW_SERVICE_AUTH
            if (!CHK_VOICE_AUTH(ses.authstate.srv_type))
            {
                cstr_strncpy(ses.mark, ACCESS_VOICE_MASK, sizeof(ses.mark));
            }
 #endif /* ALLOW_SERVICE_AUTH */
        }
        else /* if (uflag & USER_TYPE_CLOUD) */ /* 部分机型没有把云账号存在本地，不需要在本地验证云账号*/
        {
            /*if local auth failed,do cloud login .
            no deal with username entered is error while auth mode is username and password.
            maybe need check char '@' to seperate username from email.but '@' also is not a sufficient condition*/
 #ifdef INCLUDE_AGINET_APP_V2
  #ifdef INCLUDE_CLOUD_V2
            /*do cloud login check.*/
            TRACE(("svr_chk_pwd: do cloud login check"))
            if (svr_chk_cloud_username_is_valid(username))
            {
                svr_chk_cloud_username_and_pwd(username, password, passwordlen);
                goto out;
            }
            else
            {
                TRACE(("svr_chk_pwd: invalid cloud_username,auth failed!"))
            }
  #endif /*INCLUDE_CLOUD_V2*/
 #endif /*INCLUDE_AGINET_APP_V2*/
        }
    }

#ifdef INCLUDE_SSH_ACCESS
	if ((uflag & USER_TYPE_SSH) && check_port_ssh(local_port, islan))
	{
		SET_SSH_AUTH(ses.authstate.srv_type);

		M_FREE(ses.authstate.cli_username);

		ses.authstate.cli_username = strdup(username);

 #if 0  /* do not change shell to cli, change at function ptycommand instead. */
		if (ses.authstate.pw_shell)
		{
			m_free(ses.authstate.pw_shell);
		}
		ses.authstate.pw_shell = strdup(SHELL_CLI);
 #endif /* if 0 */

 #ifdef INCLUDE_SSH_ACCESS_LIFETIME
		access_get_param(DROPBEAR_AUTH_CONFIG, "sessionLifeTime=%u", &(opts.idle_timeout_secs));
 #endif /* INCLUDE_SSH_ACCESS_LIFETIME */
	}
#endif /* INCLUDE_SSH_ACCESS */

//#ifdef INCLUDE_USB_SFTP_SERVER
#if 1
	access_get_param(SFTP_CFG_FILE, "sftpport=%u", &sftp_port);
	if ((uflag & USER_TYPE_SFTP) && (local_port == sftp_port))
	{
		SET_SFTP_AUTH(ses.authstate.srv_type);

		// here we need resetting the password and username

		M_FREE(ses.authstate.pw_dir);
		M_FREE(ses.authstate.pw_name);
		M_FREE(ses.authstate.pw_passwd);

		ses.authstate.pw_dir		= m_strdup(SFTP_HOME_DIR);
		ses.authstate.pw_passwd		= m_strdup(password);
		ses.authstate.pw_name		= m_strdup(username);
		ses.authstate.pw_gid		= ses.authstate.pw_uid = 500; // here need we do?

		access_get_param(SFTP_CFG_FILE, "sftpideltm=%u", &(opts.idle_timeout_secs));
	}
#endif /* INCLUDE_USB_SFTP_SERVER */

	if (CHK_SSH_AUTH(ses.authstate.srv_type)
		|| CHK_SFTP_AUTH(ses.authstate.srv_type) 
		|| CHK_TETHER_AUTH(ses.authstate.srv_type)
		|| CHK_VOICE_AUTH(ses.authstate.srv_type))
	{
		M_FREE(ses.authstate.username);

		ses.authstate.username = strdup(ACCESS_APP_NAME);

		TRACE(("svr_chk_pwd send_msg_userauth_success"))
		dropbear_log(LOG_NOTICE, 
				"Password auth succeeded for '%s' from %s",
				username,
				svr_ses.addrstring);
		send_msg_userauth_success();

#ifdef INCLUDE_AGINET_APP_V2
		/* Send a message(username) to COS, for check login user role */
		if(-1 == send_account_login_msg(username))
		{
			dropbear_log(LOG_ERR, "send_account_login_msg failed");
		}
#endif
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
#ifdef INCLUDE_CLOUD_V2
static int username_is_valid_char(const char* name,unsigned char dot)
{
	int i = 0;
	int len = 0;

	if(NULL == name)
	{
		return 0;
	}
	len = strlen(name);
	while(*name != '\0')
	{
		if(('a' <= *name && *name <= 'z')
			|| ('A' <= *name && *name <= 'Z') 
			|| ('0' <= *name && *name <= '9')
			|| (*name == '_')
			|| (*name == '.')
			|| (dot && (*name == '-')))
		{
			i++;
			name++;
		}
		else
		{
			break;
		}
	}
	if(i == 0 || i != len)
	{
		return 0;
	}
	else
	{
		return 1;
	}

}
/*check the entered username is valid email string.*/
int svr_chk_cloud_username_is_valid(const char* username)
{
	char *p = NULL;
	char *q = NULL;
	char name[SVR_USERNAME_LEN] = {0};
	if(NULL == username || strlen(username) > (SVR_USERNAME_LEN-1))
	{
		return 0;
	}
	cstr_strncpy(name,username,SVR_USERNAME_LEN);
	p = strchr(name,'@');
	q = strrchr(name,'.');
	/*must contain '@' and '.',and '@' is in front of the last '.' */
	if(NULL == p  || NULL == q  || p > q )
	{
		TRACE(("username not contain @ or . ."))
		return 0;
	}
	*p = '\0';
	p++;
	if(0 == username_is_valid_char(name,0))
	{
		TRACE(("username contains invalid char befor @."))
		return 0;
	}
	if(0 == username_is_valid_char(p,1))
	{
		TRACE(("username contains invalid char after @."))
		return 0;
	}
	return 1;
}

#define CLOUD_AUTH_TIMEOUT_MAX 10
/*auth with cloud server and waiting for reply,timeout is 10s,
keep the same with cloud auth time.*/
static int send_cloudauth_msg_and_getresult(const char *email,const char *passwd)
{

	CMSG_FD 	msgFd;
	CMSG_BUFF 	msgBuff;
	CLOUD_APP_LOGIN_MSG authMsg;

	
	TRACE(("start cloud auth."))

	if(NULL == email || NULL == passwd)
	{
		return -1;
	}
	memset(&msgFd, 0 , sizeof(CMSG_FD));
	memset(&authMsg,0,sizeof(authMsg));
	memset(&msgBuff,0,sizeof(msgBuff));

	cstr_strncpy(authMsg.email, email, 65);
	cstr_strncpy(authMsg.passwd, passwd, 33);	
	msgBuff.priv = 0;//1:means need rebind
	msgBuff.type = CMSG_CLOUD_ACCOUNT_LOGIN;
	memcpy(msgBuff.content,&authMsg,sizeof(CLOUD_APP_LOGIN_MSG));

	if (0 != msg_init(&msgFd))
	{
		TRACE(("msg_init failed."))
		return -1;
	}
	
	if (0 != msg_connSrv(CMSG_ID_CLOUD_CLIENT, &msgFd))
	{
		msg_cleanup(&msgFd);
		TRACE(("msg_connSrv failed."))
		return -1;
	}


	if (-1 == msg_sendAndGetReplyWithTimeout(&msgFd, &msgBuff, CLOUD_AUTH_TIMEOUT_MAX))
	{
		dropbear_log(LOG_WARNING,"msg_sendAndGetReplyWithTimeout failed!\n");
		msg_cleanup(&msgFd);
		return -1;
	}

	msg_cleanup(&msgFd);

	dropbear_log(LOG_WARNING,"cloud auth  result %s!\n",msgBuff.priv == 1 ? "SUCCESS" : "FAILED");

	return (msgBuff.priv == 1 ? 0 : -1);

}
/*do cloud auth:check the entered username and password with cloud server.*/
void svr_chk_cloud_username_and_pwd(const char* username, unsigned char * password, unsigned int passwordlen)
{
	char passwd[SVR_PASSWORD_LEN] = {0};

	if(passwordlen > (33-1))
	{		
		TRACE(("cloud auth:cloud password is too long!\n"))
		send_msg_userauth_failure(0, 1);
		return;
	}
	cstr_strncpy(passwd,password,SVR_PASSWORD_LEN);
	TRACE(("cloud auth:svr_username = %s, password = %s\n",username, passwd))
	
	if(0 != send_cloudauth_msg_and_getresult(username,passwd))
	{
		
		TRACE(("svr_chk_cloud_username_and_pwd send_msg_userauth_failure"))
		send_msg_userauth_failure(0, 1);
	}
	else
	{		
		TRACE(("svr_chk_cloud_username_and_pwd send_msg_cloudauth_success"))
#if ALLOW_SERVICE_AUTH
		if (!CHK_VOICE_AUTH(ses.authstate.srv_type))
		{
			cstr_strncpy(ses.mark, ACCESS_VOICE_MASK, SVR_ADMIN_LEN);
		}
#endif/* ALLOW_SERVICE_AUTH */	
		SET_TETHER_AUTH(ses.authstate.srv_type);
		send_msg_userauth_success();
	}

	return;
}
#endif /*INCLUDE_CLOUD_V2*/

#ifdef INCLUDE_AGINET_APP_V2
static int send_account_login_msg(const char *username)
{
	CMSG_FD		msgFd;
	CMSG_BUFF	msgBuff;
	ACCOUNT_LOGIN_MSG	*pAccountLoginMsg = NULL;

	TRACE(("start send login message."))

	if (NULL == username)
	{
		dropbear_log(LOG_ERR, "Username is NULL.");
		return -1;
	}

	memset(&msgFd, 0 , sizeof(CMSG_FD));
	memset(&msgBuff, 0, sizeof(CMSG_BUFF));

	pAccountLoginMsg = (ACCOUNT_LOGIN_MSG *)(msgBuff.content);
	cstr_strncpy(pAccountLoginMsg->loginName, username, 16);

	TRACE(("loginName is %s.", pAccountLoginMsg->loginName))
	msgBuff.type = CMSG_ACCOUNT_LOGIN;

	if (msg_connCliAndSend(CMSG_ID_COS, &msgFd, &msgBuff) < 0)
	{
		dropbear_log(LOG_ERR, "Send msg failed.");
		return -1;
	}
	return 0;
}
#endif /*INCLUDE_AGINET_APP_V2*/

#endif
#endif
