#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#ifdef TCSUPPORT_SYSLOG_ENHANCE
#include <syslog.h>
#endif

#include "mini-asp.h"
#include "grammar.h"
#include "http-get-utils.h"
#include "../boa.h"
#include "libtcapi.h"
#if defined(TCSUPPORT_CT)
#ifdef TCSUPPORT_SYSLOG
#include <syslog.h>
#endif
#endif

#if defined(TCSUPPORT_GENERAL_MULTILANGUAGE)
#include "parsers.h"
#endif
#if defined(TCSUPPORT_CT_JOYME2)
#include "ctc_auth_api.h"
#include "ecnt_json_api.h"
#endif
#if defined(TCSUPPORT_ECNT_MAP)
#include "mesh_getjson_api.h"
#endif 
#if defined(TCSUPPORT_CMCCV2) || defined(TCSUPPORT_ANDLINK)
#if defined(TCSUPPORT_ANDLINK)
#include "cmcc_np_api.h"
#else
#include "cmcc_itms_api.h"
#endif
#endif
#if defined(TCSUPPORT_CT_JOYME4)
#include <ecnt_event_global/ecnt_event_global.h>
#include <ecnt_event_global/ecnt_event_system.h>
#endif

#if defined(TCSUPPORT_CHARSET_CHANGE) 
#include <iconv.h>
#include <ecnt_utility.h>
#endif
#include "blapi_traffic.h"
#include <common/ecnt_global_macro.h>

/*krammer add for bug 1321*/
#define WAN_PVC "Wan_PVC"
#define MAX_PVC_NUMBER 8
#define DEFAULT_RT_ATTR_LENGTH 4
#define DEFAULT_RT "DEFAULTROUTE"
#define TO_ASCII_OFFSET 48
#if defined(TCSUPPORT_CT_E8GUI)
#define MAX_BUFF_SIZE 1024
#endif


static void get_post_multipart(request *req);
static void get_post(request *req);
static void get_query(request *req);
static int http_header();

void init_asp_funcs(void);
#ifndef TRENDCHIP
void init_asp_constants (void);
#endif
/*transfrom the number to ASCII*/
int decode_uri(char *uri);
static char hex_to_decimal(char char1,char char2);

static void asp_Write (asp_reent* reent, const asp_text* params,  asp_text* ret);
static void request_Form (asp_reent* reent, const asp_text* params,  asp_text* ret);
static void tcWebApi_Set (asp_reent* reent, const asp_text* params,  asp_text* ret);
static void tcWebApi_Get (asp_reent* reent, const asp_text* params,  asp_text* ret);
static void tcWebApi_Unset (asp_reent* reent, const asp_text* params,  asp_text* ret);
static void tcWebApi_Commit (asp_reent* reent, const asp_text* params,  asp_text* ret);
static void tcWebApi_Save (asp_reent* reent, const asp_text* params,  asp_text* ret);
static void tcWebApi_CurrentDefaultRoute(asp_reent * reent, const asp_text * params, asp_text * ret);
#if defined(TCSUPPORT_IMPROVE_GUI_PERFM)
static void tcWebApi_constSet (asp_reent* reent, const asp_text* params,  asp_text* ret);
#endif
static void tcWebApi_staticGet (asp_reent* reent, const asp_text* params,  asp_text* ret);
static void tcWebApi_CommitWithoutSave (asp_reent* reent, const asp_text* params,  asp_text* ret);
#if defined(TCSUPPORT_CT_JOYME2) || defined(TCSUPPORT_CMCCV2) || defined(TCSUPPORT_ANDLINK)
static void tcWebApi_JsonHook(asp_reent* reent, const asp_text* params, asp_text* ret);
#endif
#if defined(TCSUPPORT_ECNT_MAP)
static void tcWebApi_MeshJsonHook(asp_reent* reent, const asp_text* params, asp_text* ret);
#endif
static void tcWebApi_getbywifiid(asp_reent* reent, const asp_text* params, asp_text* ret);

#ifdef RA_PARENTALCONTROL
static void getClientMacAddr(char * ip_addr);
#endif/*RA_PARENTALCONTROL*/
#if defined(TCSUPPORT_WEBSERVER_SSL)
static request* temp_req = NULL;
#endif

int fd_out;
extern char cur_username[129];
#if defined(TCSUPPORT_WEB_SAVE)
extern int flag_save;
#endif

#if defined(TCSUPPORT_GENERAL_MULTILANGUAGE)
extern	int	getString(const char *pkey, char *value);
extern	int	initandparserfile(void);
extern	void	closefp(void);
extern	int	islangChanged(void);
#endif

#if defined(TCSUPPORT_CT_PON_C9)
int getMACfromNeigh(char *in_ipaddr, char *out_mac);
#endif

#if defined(TCSUPPORT_CT_JOYME2)
static const unsigned long crc_32_tab[] = { /* CRC polynomial 0xedb88320 */
0x00000000, 0x77073096, 0xee0e612c, 0x990951ba, 0x076dc419, 0x706af48f,
0xe963a535, 0x9e6495a3, 0x0edb8832, 0x79dcb8a4, 0xe0d5e91e, 0x97d2d988,
0x09b64c2b, 0x7eb17cbd, 0xe7b82d07, 0x90bf1d91, 0x1db71064, 0x6ab020f2,
0xf3b97148, 0x84be41de, 0x1adad47d, 0x6ddde4eb, 0xf4d4b551, 0x83d385c7,
0x136c9856, 0x646ba8c0, 0xfd62f97a, 0x8a65c9ec, 0x14015c4f, 0x63066cd9,
0xfa0f3d63, 0x8d080df5, 0x3b6e20c8, 0x4c69105e, 0xd56041e4, 0xa2677172,
0x3c03e4d1, 0x4b04d447, 0xd20d85fd, 0xa50ab56b, 0x35b5a8fa, 0x42b2986c,
0xdbbbc9d6, 0xacbcf940, 0x32d86ce3, 0x45df5c75, 0xdcd60dcf, 0xabd13d59,
0x26d930ac, 0x51de003a, 0xc8d75180, 0xbfd06116, 0x21b4f4b5, 0x56b3c423,
0xcfba9599, 0xb8bda50f, 0x2802b89e, 0x5f058808, 0xc60cd9b2, 0xb10be924,
0x2f6f7c87, 0x58684c11, 0xc1611dab, 0xb6662d3d, 0x76dc4190, 0x01db7106,
0x98d220bc, 0xefd5102a, 0x71b18589, 0x06b6b51f, 0x9fbfe4a5, 0xe8b8d433,
0x7807c9a2, 0x0f00f934, 0x9609a88e, 0xe10e9818, 0x7f6a0dbb, 0x086d3d2d,
0x91646c97, 0xe6635c01, 0x6b6b51f4, 0x1c6c6162, 0x856530d8, 0xf262004e,
0x6c0695ed, 0x1b01a57b, 0x8208f4c1, 0xf50fc457, 0x65b0d9c6, 0x12b7e950,
0x8bbeb8ea, 0xfcb9887c, 0x62dd1ddf, 0x15da2d49, 0x8cd37cf3, 0xfbd44c65,
0x4db26158, 0x3ab551ce, 0xa3bc0074, 0xd4bb30e2, 0x4adfa541, 0x3dd895d7,
0xa4d1c46d, 0xd3d6f4fb, 0x4369e96a, 0x346ed9fc, 0xad678846, 0xda60b8d0,
0x44042d73, 0x33031de5, 0xaa0a4c5f, 0xdd0d7cc9, 0x5005713c, 0x270241aa,
0xbe0b1010, 0xc90c2086, 0x5768b525, 0x206f85b3, 0xb966d409, 0xce61e49f,
0x5edef90e, 0x29d9c998, 0xb0d09822, 0xc7d7a8b4, 0x59b33d17, 0x2eb40d81,
0xb7bd5c3b, 0xc0ba6cad, 0xedb88320, 0x9abfb3b6, 0x03b6e20c, 0x74b1d29a,
0xead54739, 0x9dd277af, 0x04db2615, 0x73dc1683, 0xe3630b12, 0x94643b84,
0x0d6d6a3e, 0x7a6a5aa8, 0xe40ecf0b, 0x9309ff9d, 0x0a00ae27, 0x7d079eb1,
0xf00f9344, 0x8708a3d2, 0x1e01f268, 0x6906c2fe, 0xf762575d, 0x806567cb,
0x196c3671, 0x6e6b06e7, 0xfed41b76, 0x89d32be0, 0x10da7a5a, 0x67dd4acc,
0xf9b9df6f, 0x8ebeeff9, 0x17b7be43, 0x60b08ed5, 0xd6d6a3e8, 0xa1d1937e,
0x38d8c2c4, 0x4fdff252, 0xd1bb67f1, 0xa6bc5767, 0x3fb506dd, 0x48b2364b,
0xd80d2bda, 0xaf0a1b4c, 0x36034af6, 0x41047a60, 0xdf60efc3, 0xa867df55,
0x316e8eef, 0x4669be79, 0xcb61b38c, 0xbc66831a, 0x256fd2a0, 0x5268e236,
0xcc0c7795, 0xbb0b4703, 0x220216b9, 0x5505262f, 0xc5ba3bbe, 0xb2bd0b28,
0x2bb45a92, 0x5cb36a04, 0xc2d7ffa7, 0xb5d0cf31, 0x2cd99e8b, 0x5bdeae1d,
0x9b64c2b0, 0xec63f226, 0x756aa39c, 0x026d930a, 0x9c0906a9, 0xeb0e363f,
0x72076785, 0x05005713, 0x95bf4a82, 0xe2b87a14, 0x7bb12bae, 0x0cb61b38,
0x92d28e9b, 0xe5d5be0d, 0x7cdcefb7, 0x0bdbdf21, 0x86d3d2d4, 0xf1d4e242,
0x68ddb3f8, 0x1fda836e, 0x81be16cd, 0xf6b9265b, 0x6fb077e1, 0x18b74777,
0x88085ae6, 0xff0f6a70, 0x66063bca, 0x11010b5c, 0x8f659eff, 0xf862ae69,
0x616bffd3, 0x166ccf45, 0xa00ae278, 0xd70dd2ee, 0x4e048354, 0x3903b3c2,
0xa7672661, 0xd06016f7, 0x4969474d, 0x3e6e77db, 0xaed16a4a, 0xd9d65adc,
0x40df0b66, 0x37d83bf0, 0xa9bcae53, 0xdebb9ec5, 0x47b2cf7f, 0x30b5ffe9,
0xbdbdf21c, 0xcabac28a, 0x53b39330, 0x24b4a3a6, 0xbad03605, 0xcdd70693,
0x54de5729, 0x23d967bf, 0xb3667a2e, 0xc4614ab8, 0x5d681b02, 0x2a6f2b94,
0xb40bbe37, 0xc30c8ea1, 0x5a05df1b, 0x2d02ef8d
};

static unsigned int calc_crc32(const char *buf, unsigned int size)
{
	unsigned int i, crc;
	crc = 0xFFFFFFFF;
	
	for (i = 0; i < size; i++)
		crc = crc_32_tab[( crc ^ buf[i] ) & 0xFF] ^ (crc >> 8);
		
	return crc ^ 0xFFFFFFFF;
}

extern char cookie_crc[32];
#endif 
int asp_handler(request * req)
{
	int ret, i;
	char nodename[15], username[129];
#if defined(TCSUPPORT_NP_CMCC)
	char active[8]={0};
#endif
#if defined(TCSUPPORT_C2_TRUE)
	char cmd[8] = {0};
	FILE *fp;
#endif
#if defined(TCSUPPORT_ACCOUNT_ACL)
	char acc_id[8];
#endif
#if defined(TCSUPPORT_CT_JOYME2)
	int k = 0, buf_len = 0;
	char svr_crc[32] = {0};
	char *checkBuf = NULL;
	unsigned int result = 0;
#endif
	fprintf(stderr,"%s mothed:%d %s \n",__FUNCTION__,req->method,req->pathname);
	for(i = 0 ; i < 3; i++){
		sprintf(nodename, "Account_Entry%d", i);
#if defined(TCSUPPORT_NP_CMCC)		
		tcapi_get(nodename, "Active", active);
		if(0 == strcmp("No", active))
			continue;
#endif
		ret = tcapi_get(nodename, "username", username);
		if(ret < 0){ /*Fail to acquire username from romfile*/
			fprintf(stderr, "Fail to acquire username from romfile: ret = %d\r\n", ret);
			return -1;
		}
		if(!strcmp(cur_username, username)){
			sprintf(nodename, "%d", i);
#if defined(TCSUPPORT_ANDLINK)
			ret = tcapi_set("WebCurSet_Entry", "CurrentAccess", "0");
#else
			ret = tcapi_set("WebCurSet_Entry", "CurrentAccess", nodename);
#endif
			if(ret < 0){ /*Fail to set current username*/
				fprintf(stderr, "Fail to set current username: ret = %d\r\n", ret);
				return -1;
			}
#if defined(TCSUPPORT_ANDLINK)
			snprintf(username, sizeof(username), "%d", i);
#else
			tcapi_get("WebCurSet_Entry", "CurrentAccess", username);
#endif

			break;
		}
	}
	

	g_var_post = NULL;
#if defined(TCSUPPORT_WEBSERVER_SSL)
	temp_req = req;
	dontDoAsp = 0;
#endif
	fd_out = req->fd;
	if(http_header() == -1)
	{
		close(fd_out);
		req->status = DEAD;
		req->keepalive = KA_INACTIVE;//Ian_20070326
		return 0;//Ian_20070326
	}
	if((req->method == M_POST) && req->content_length){
		if(req->content_type == NULL)
		{
			get_post(req);
		}
		/*add support to parse multipart. shnwind 2009.4.8*/
		else if(strstr(req->content_type,"multipart") == NULL){
			get_post(req);
		}else{
			get_post_multipart(req);
		}
	}else if((req->method == M_GET) && req->query_string)
		get_query(req);

#if defined(TCSUPPORT_CT_JOYME2) && !defined(TCSUPPORT_CUC)
	if( req->method == M_POST ){
		/* post content value check */
		if ( 0 != strcmp(req->pathname, "/boaroot/cgi-bin/login.cgi") 
			&& 0 != strcmp(req->pathname, "/boaroot/cgi-bin/login.asp")
			&& 0 != strcmp(req->pathname, "/boaroot/cgi-bin/check_auth.json")
			&& 0 != strcmp(req->pathname,"/boaroot/cgi-bin/redirect.asp")
			&& 0 != strcmp(cookie_crc, "ecntBaorga") ) {
			checkBuf = (char *)malloc(SINGLE_POST_LIMIT_DEFAULT);
			if ( checkBuf ){				
				memset(checkBuf, 0 , SINGLE_POST_LIMIT_DEFAULT);
			for (k = 0; g_var_post && g_var_post[k] 
				&& g_var_post[k]->name && 0 != g_var_post[k]->name[0] 
				&& 0 != strcmp(g_var_post[k]->name, "psd")
				&& 0 != strcmp(g_var_post[k]->name, "password"); k++) {
					buf_len += snprintf(checkBuf + buf_len, SINGLE_POST_LIMIT_DEFAULT - buf_len, "%s%s", g_var_post[k]->name, g_var_post[k]->value);
					if ( buf_len > SINGLE_POST_LIMIT_DEFAULT )
					{
						send_r_bad_request(req);
						return 0;
					}
			}
			decode_uri(checkBuf);
			result = calc_crc32(checkBuf, strlen(checkBuf));
				free(checkBuf);
			snprintf(svr_crc, sizeof(svr_crc), "%x", result);
			if ( result > 0 && 0 != strcmp(cookie_crc, svr_crc) )
			{
				send_r_bad_request(req);
				return 0;
			}
		}
	}
	}
#endif

//	init_asp_funcs ();
//    init_asp_constants();
#ifdef RA_PARENTALCONTROL
	/* Modify for taking CGI environment to ASP handler by richard.lai */
	do_asp (req, req->pathname); 
#else
#if defined(TCSUPPORT_CT_PON_C9) || defined(TCSUPPORT_CT_JOYME2)
	do_asp (req, req->pathname);
#else
	do_asp (NULL, req->pathname);
#endif
#endif/*RA_PARENTALCONTROL*/
#if defined(TCSUPPORT_WEB_SAVE)
		if(flag_save == 1){
			flag_save = 0;
			
			tcapi_save();
		}
#endif
	#ifdef TRENDCHIP
	if(dontDoAsp !=0){
		dontDoAsp=0;
	}
	#endif
	free_param_line(g_var_post);
	req->cgi_status = CGI_DONE;
	req->status = DONE;
	close(fd_out);
	fprintf(stderr,"%s END \n",__FUNCTION__);
	req->keepalive = KA_INACTIVE;//Ian_20070326
	return 0;//Ian_20070326
}

static void get_post_multipart(request *req){

	//char boundary[64];
	char boundary[256] = {0}; 
	char *c_ret = NULL, *post = NULL;
	int fd = -1, content_len;
	struct stat buf_st;
	int i = 0, len_bound = 0, count = 0;


	/*get boundary*/
	c_ret = strstr(req->content_type,"boundary=");
	if(c_ret != NULL){
		len_bound = strlen(c_ret);
		if(len_bound <= sizeof(boundary)){
			for(i = strlen("boundary=") ; i < len_bound ;i++){
				/*do not need "-------"*/
				if(c_ret[i] != '-'){
					break;
				}
			}
			strncpy(boundary,c_ret+i, sizeof(boundary) - 1);
			//tcdbg_printf("bound = %s\n",boundary);
		}else{
			tcdbg_printf("boundary too long!!!upgrade fail\n");
			return;
		}
	}
	/*paser all item information*/
	multipart_parser(req, boundary);
	/*get all item information*/
	if(stat(MULTI_TO_NORMAL_TEMP, &buf_st) != 0){
		return ;
	}
	content_len = buf_st.st_size;
	post = (char*)malloc(content_len+1);
	if(post == NULL){
		return;
	}
	fd = open(MULTI_TO_NORMAL_TEMP,O_RDONLY);
	if(fd < 0){
		free(post);
		return;
	}
	if((count = read(fd,post,content_len)) <= 0){
		free(post);
		close(fd);
		return;
	}
	close(fd);
	post[content_len] = '\0';
	g_var_post = parse_param_line (post);
	free(post);
	//move to cfg_manager
	//unlink(MULTI_TO_NORMAL_TEMP);
}


static void get_post(request *req)
{
	char *post;
	int ret,len;

	len = atoi(req->content_length);
	post = (char*)malloc(len+1);
	if(post == NULL){
		return;
	}
	memset(post,0,len+1);

	lseek(req->post_data_fd, 0,SEEK_SET);
	ret = read(req->post_data_fd,post,len);
	post[len] = '\0';
	if(ret<=0)
		goto exit;

	//fprintf(stderr,"post:%s %s\n",post,req->content_length);
	g_var_post = parse_param_line (post);
exit:
	#if 0  //lee 2006-11-27
	close(req->post_data_fd);
	req->post_data_fd = 0;
	#endif
	free(post);
}

static void get_query(request *req)
{
#if 0
	char *query;
	int len;

	len = strlen(req->query_string);
	query =(char*) malloc(len+1);
	memcpy(query,req->query_string,len);
	//fprintf(stderr,"query:%s %d\n",query,len);
	g_var_post = parse_param_line (query);
	free(query);
#endif
	g_var_post = parse_param_line (req->query_string);
}

static int http_header()
{
	char HTTP_OK[] = "HTTP/1.0 200 OK\r\n";
	char x_opt1[] = "X-Frame-Options: SAMEORIGIN\r\n";
/*pork 20090309 added*/
#if 0
	char CONTENT_TYPE[] = "Content-type: text/html;charset=GB2312\r\n\r\n";
#else
	char CONTENT_TYPE[64];
	if(charset){
		sprintf(CONTENT_TYPE,"Content-type: text/html;charset=%s\r\n\r\n",charset);
	}
	else{
		/*default char set: ISO-8859-1*/
		sprintf(CONTENT_TYPE,"Content-type: text/html;charset=ISO-8859-1\r\n\r\n");
	}
#endif
/*pork 20090309 added over*/

	if(asp_send_response (NULL, HTTP_OK, strlen(HTTP_OK)) == -1)
		return -1;
#if defined(TCSUPPORT_CT_JOYME2)
	if(asp_send_response (NULL, x_opt1, strlen(x_opt1)) ==-1)
		return -1;
#endif

	if(asp_send_response (NULL, CONTENT_TYPE, strlen(CONTENT_TYPE)) ==-1)
		return -1;

	return 0;
}
#ifndef TRENDCHIP
static void Request_Form(asp_reent* reent, int id, const asp_text* params,  asp_text* ret)
{
	char *val,*tmp;

	//fprintf(stderr, "(%s ID: %d ", __FUNCTION__, id);
    ret->str ="";
    ret->len = 0;
    if (params->str && params->len)
    {
    	//fwrite(params->str, params->len, 1, stderr);

    	tmp = (char*)asp_alloc(reent,params->len+1);
    	memset(tmp,0,params->len+1);
    	memcpy(tmp,params->str,params->len);
    	val = get_param(g_var_post,tmp);
    	if(val ==NULL)
    		return;

		ret->str = val;
		ret->len = strlen(val);
    }
    //else
    //	fprintf (stderr, "No Params )");
    //fprintf (stderr, "\n");

}

extern asp_reent *my_reent;
static void TcWebApi_set (asp_reent* reent, int id, const asp_text* params,  asp_text* ret)
{
	char *tmp;

    //fprintf(stderr, "(%s ID: %d ", __FUNCTION__, id);
    if (params->str && params->len)
    {
      //fwrite(params->str, params->len, 1, stderr);

      tmp = (char*)asp_alloc(reent,params->len+1);
      memset(tmp,0,params->len+1);
      memcpy(tmp,params->str,params->len);
      my_reent = reent;
      asp_TCWebApi_set(id,tmp);
    }
    //else
    //	fprintf (stderr, "No Params )");
    //fprintf (stderr, "\n");

}


static void TcWebApi_get (asp_reent* reent, int id, const asp_text* params,  asp_text* ret)
{
	char *val;

	#if 0
    fprintf(stderr, "(%s ID: %d ", __FUNCTION__, id);

    if (params->str && params->len)
      fwrite(params->str, params->len, 1, stderr);
    else
      fprintf (stderr, "No Params )");
    fprintf (stderr, "\n");
    #endif

    my_reent = reent;
    val = asp_TCWebApi_get(id);
    if(val == NULL)
    {
    	ret->str = "";
    	ret->len = 0;
    }
    else
    {
    	ret->str = val;
		ret->len = strlen(val);
    }

}

static void TcWebApi_execute (asp_reent* reent, int id, const asp_text* params,  asp_text* ret)
{

    //fprintf(stderr, "%s ID: %d \n", __FUNCTION__, id);
    my_reent = reent;
    asp_TCWebApi_execute(id);
}
#endif
void init_asp_funcs(void)
{
    #ifndef TRENDCHIP
    append_asp_func ("Request.Form", Request_Form);
    append_asp_func ("TcWebApi_set", TcWebApi_set);
    append_asp_func ("TcWebApi_get", TcWebApi_get);
    append_asp_func ("TcWebApi_execute", TcWebApi_execute);
    #else
    /*new tcWebApi*/
    append_asp_func ("asp_Write", asp_Write);
    append_asp_func ("request_Form", request_Form);
    append_asp_func ("tcWebApi_Set", tcWebApi_Set);
    append_asp_func ("tcWebApi_Get", tcWebApi_Get);
    append_asp_func ("tcWebApi_Unset", tcWebApi_Unset);
    append_asp_func ("tcWebApi_Commit", tcWebApi_Commit);
    append_asp_func ("tcWebApi_Save", tcWebApi_Save);
/*krammer add for bug 1321*/
    append_asp_func("tcWebApi_CurrentDefaultRoute", tcWebApi_CurrentDefaultRoute);
#if defined(TCSUPPORT_IMPROVE_GUI_PERFM)
    append_asp_func ("tcWebApi_constSet", tcWebApi_constSet);
#endif
    append_asp_func ("tcWebApi_staticGet", tcWebApi_staticGet);

    append_asp_func ("tcWebApi_CommitWithoutSave", tcWebApi_CommitWithoutSave);
#if defined(TCSUPPORT_CT_JOYME2) || defined(TCSUPPORT_CMCCV2) || defined(TCSUPPORT_ANDLINK)
	append_asp_func("tcWebApi_JsonHook", tcWebApi_JsonHook);
#endif
#if defined(TCSUPPORT_ECNT_MAP)
    append_asp_func("tcWebApi_MeshJsonHook", tcWebApi_MeshJsonHook);
#endif
	append_asp_func("tcWebApi_getbywifiid", tcWebApi_getbywifiid);
    #endif
}

static void wait_fd (int fd)
{
	fd_set block_write_fdset,block_read_fdset;
	#ifdef TRENDCHIP
	struct timeval timeout={FAIL_SESSION_TIMEOUT,0};
	int ret=0;
	#endif

	while (1)
	{
		FD_ZERO(&block_write_fdset);
		FD_SET (fd, &block_write_fdset);
		FD_ZERO(&block_read_fdset);
		FD_SET (fd, &block_read_fdset);
	 	#ifdef TRENDCHIP
                /*
                   krammer change for bug 1094
                   we make select to have a timeout, if timeout, that means the session has some
                   thing wrong, so we break wait fd and dont do any asp until new session.
                */
            	if ((ret=select(fd + 1, &block_read_fdset,
                   &block_write_fdset, NULL,
                   &timeout)) <0 ) {
		#else
		if (select(fd + 1, &block_read_fdset,
                   &block_write_fdset, NULL,
                   0) <=0 ) {
              #endif
            /* what is the appropriate thing to do here on EBADF */
            if (errno == EINTR)
            {
                continue;   /* while(1) */
            }
            else if (errno != EBADF) {
                fprintf(stderr,"select error \n");
            }
        }
		#ifdef TRENDCHIP
		if(ret==0){/*timeout*/
			dontDoAsp=1;
			break;
		}
		#endif
		if (FD_ISSET(fd, &block_read_fdset))
			break;

		if (FD_ISSET(fd, &block_write_fdset))
			break;
	}

}


int asp_send_response (asp_reent* reent, const void* buffer, size_t len)
{

	int res=0;
	#ifdef TRENDCHIP
	 /*
           krammer add for bug 1094, this flag can block asp_send_response when
           wait_fd is timeout and change_ip flag is set.
        */
	if(dontDoAsp){
		return 0;
	}
	#endif

loop_write:

	len =len-res;
	buffer = buffer+res;

	wait_fd (fd_out);
#if defined(TCSUPPORT_WEBSERVER_SSL)
	if(temp_req->ssl == NULL)
#endif
	{
	res = write(fd_out,buffer,len);
	}
#if defined(TCSUPPORT_WEBSERVER_SSL)
	else{
		int retCode = 0; 
		if(len <=0)
			return 0; 
		res = boa_sslWrite(temp_req->ssl, buffer,len,&retCode);
		if(res<0)
		{
			dontDoAsp = 1;
		}
	}
#endif

  if(res <0 )
  {
  	return -1;
  }
  if(res<len)
  	goto loop_write;

  return 0;



}

int asp_send_format_response (const char *format, ...)
{
	static char buffer[1024] = {0};
	int res=0, len = 0;
	char *ptr = NULL;
	#ifdef TRENDCHIP
	 /*
           krammer add for bug 1094, this flag can block asp_send_test_response when
           wait_fd is timeout and change_ip flag is set.
        */
	if(dontDoAsp){
		return 0;
	}
	#endif
	memset(buffer, 0, sizeof(buffer));
	va_list args;
	va_start(args, format);
	vsprintf(buffer, format, args);
	ptr = buffer;
	len = strlen(buffer);
loop_write:

	len =len-res;
	ptr = ptr+res;

	wait_fd (fd_out);
	res = write(fd_out,buffer,len);

  if(res <0 )
  {
  	va_end(args);
  	return -1;
  }
  if(res<len)
  	goto loop_write;

  va_end(args);
  return 0;
}

#ifndef TRENDCHIP
void init_asp_constants (void)
{
  //sample append_asp_constant ("WAN_SETTING",					0);

}
#endif
#ifdef TRENDCHIP
/*____________________________________________________________________*
**	function name: asp_Write
**
**	description:
*     Output the string to web page.
*
**	parameters:
*     reent:not use
*     id:not use
*     params:params[0] will save the string user input
*     ret:not use
**	global:
*     none
**	return:
*     0:successful
*     -1:fail
**	call:
*     none
**	revision:
*     1.shnwind
* _______________________________________________________________*/

static void
asp_Write (asp_reent* reent, const asp_text* params,  asp_text* ret)
{

    asp_send_response (NULL,params[0].str,params[0].len);

}
/*____________________________________________________________________*
**	function name: request_Form
**
**	description:
*     get the value of item on page.
*
**	parameters:
*     reent:not use
*     id:not use
*     params:params[0] item name.
*     ret:not use
*
**	global:
*     none
**	return:
*     none
**	call:
*     none
**	revision:
*     1.shnwind
* _____________________________________________________________________*/
static void
request_Form (asp_reent* reent, const asp_text* params,  asp_text* ret)
{

    char *val,*tmp;

    /*ret->str = "");
    ret->len = 0;*/

    if (params[0].str && params[0].len)
    {
    	tmp = (char*)asp_alloc(reent,params[0].len+1);
    	memset(tmp,0,params[0].len+1);
    	memcpy(tmp,params[0].str,params[0].len);
    	val = get_param(g_var_post,tmp);

	//tcdbg_printf("request name %s value %s\n",tmp,val);
      if(val ==NULL)
    		return;
		decode_uri(val);
		ret->str = val;
		ret->len = strlen(val);
    }

}

/*[OSBNB00040562]ADD by peter.jiang@20141015, enable password encryption*/
#if defined(TCSUPPORT_CT_VOIP_CRYPT)
/*____________________________________________________________________*
**      function name: privSymmCrypt
**
**      description:
*       private symmetrical crypt.
*
**      parameters:
*     text: text input, may be encoded or decodec.
*     crypt_out: text out after crypting
*     
*     ret:crypt_out
*
**      global:
*     none
**      return:
*     none
**      call:
*     tcWebApi_set
**      revision:
* _____________________________________________________________________*/
static char*
privSymmCrypt (const char* text, char* crypt_out)
{
    char PRIVATE_KEY[2];
    int i = 0;
    int len = 0;

    if (text == NULL || strlen(text) == 0 || crypt_out == NULL) {
        tcdbg_printf ("[ASP] privSymmCrypt: invalid input\n");
        return NULL;
    }

    /*In order to make crypt_out characters printable(<0x80), private key should be <0x80,
    **and not be confict with text, e.g., 'Z'^'Z' = 0, this will truncate crypt_out,
    **set private keys as 0x12, 0x13(<0x20). 
    */
    PRIVATE_KEY[0] = 0x12;
    PRIVATE_KEY[1] = 0x13;

    len = strlen(text);
    if (len%2)
        len--;

    for (i = 0; i < len; i+=2) {
        crypt_out[i] = text[i] ^ PRIVATE_KEY[0];
        crypt_out[i+1] = text[i+1] ^ PRIVATE_KEY[1];
    }

    if (strlen(text)%2) { /*the last byte*/
        crypt_out[i] = text[i] ^ PRIVATE_KEY[0];
    }
/*
    tcdbg_printf("\n+++++++++++++++(ASP text)++++++++++++++++++\n");
    for (i = 0; i < strlen(text); i++)
        tcdbg_printf("%2x ", text[i]);
    tcdbg_printf("\n+++++++++++++(ASP crypt_out)+++++++++++++++\n");
    for (i = 0; i < strlen(crypt_out); i++)
        tcdbg_printf("%2x ", crypt_out[i]);
    tcdbg_printf("\n++++++++++++++++++++++++++++++++++++++++++\n");
*/
    return crypt_out;
}
#endif

/*____________________________________________________________________*
**	function name: tcWebApi _set
**
**	description:
*     write the value which is user input to cfg_manager.
*
**	parameters:
*     reent:not use
*     id:not use
*     params:params[0] node name.
*            params[1] attribute name.
*            params[2] value
*     ret:not use
*
**	global:
*     none
**	return:
*     none
**	call:
*     none
**	revision:
*     1.shnwind
* _____________________________________________________________________*/
static void
tcWebApi_Set (asp_reent* reent, const asp_text* params,  asp_text* ret)
{

    char *node,*attr,*value,*v_tmp;
    int r_val, i = 0;
	char *directSet[] = {"CurPage", ""};

    node = (char*)asp_alloc(reent,params[0].len+1);
    attr = (char*)asp_alloc(reent,params[1].len+1);
    value = (char*)asp_alloc(reent,params[2].len+1);
    memset(node,0,params[0].len+1);
    memset(attr,0,params[1].len+1);
    memset(value,0,params[2].len+1);
    memcpy(node,params[0].str,params[0].len);
    memcpy(attr,params[1].str,params[1].len);
    memcpy(value,params[2].str,params[2].len);
    node[params[0].len]='\0';
    attr[params[1].len]='\0';
    value[params[2].len]='\0';

    if(params[2].len == 0){
      return;
    }

    v_tmp = get_param(g_var_post,value);//get_param on web page
    //tcdbg_printf("set node = %s attr %s value %s\n",node,attr,v_tmp);
    if(v_tmp != NULL){
      decode_uri(v_tmp);

/*[OSBNB00040562]ADD by peter.jiang@20141015, enable password encryption*/
#if defined(TCSUPPORT_CT_VOIP_CRYPT)
    {
        char crypt[64] = {0};
        if (!strncmp (node, "VoIPBasic_Entry", strlen("VoIPBasic_Entry"))
          && !strncmp (attr, "SIPPassword", strlen("SIPPassword"))){
            privSymmCrypt(v_tmp, crypt);
            memcpy (v_tmp, crypt, strlen(v_tmp)+1);
        }
    }
#endif

      r_val=tcapi_set(node, attr,v_tmp);
    }
	else{//If the value is not the name of an attribute, regard it as the value to be assigned
		while(strlen(directSet[i]))
			if(!strcmp(attr, directSet[i++]))
				r_val=tcapi_set(node, attr, value);
	}
		

}
void strQuotConvertHTML(char *oriStr,char *desStr)  {
    int i;
    int j = 0;
    for(i = 0;i < strlen(oriStr);i++){
    	if(oriStr[i] == '"'){
    		strcpy(&(desStr[j]),"&quot;");
    		j+=6;
    	}
		else if(oriStr[i] == '&'){
    		strcpy(&(desStr[j]),"&amp;");
    		j+=5;
    	}
		else if(oriStr[i] == '<'){
    		strcpy(&(desStr[j]),"&lt;");
    		j+=4;
    	}
		else if(oriStr[i] == '>'){
    		strcpy(&(desStr[j]),"&gt;");
    		j+=4;
    	}
    	else{
    		desStr[j] = oriStr[i];
    		j++;
    	}
    }
    desStr[j] = '\0';
}
#if defined(TCSUPPORT_CGNX)
char captchaArray[30][6] = {0};
int captchaValuecheck( char *node,char *attr, char val[MAX_BUFF_SIZE]  ){
	char captcha_url[32]={0};
	char capt_str[6]={0};
	int i ;
	int captchaindex;
	char *capt_idx;
	char *capt_val;

	if((!strcmp(node, "WebCurSet_Entry")) && (!strcmp(attr, "Captchaurl")))
	{
		for(i = 0;i<30;i++)
		{
				if(captchaArray[i][0]=='\0')
				{
					Captchalogin(i,capt_str);
					strncpy(captchaArray[i],capt_str,sizeof(captchaArray[i]));
					snprintf(captcha_url,sizeof(captcha_url),"captcha_%d",i);
					memset(val, 0, sizeof(val));
					strncpy(val,captcha_url,11);
					break ;
				}
				if(i==29){	
					memset(captchaArray, 0, sizeof(captchaArray));
					Captchalogin(0,capt_str);
					strncpy(captchaArray[0],capt_str,sizeof(captchaArray[0]));
					snprintf(captcha_url,sizeof(captcha_url),"captcha_%d",0);
					memset(val, 0, sizeof(val));
					strncpy(val,captcha_url,11);		
				}
			
		}
	}
	if((!strcmp(node, "WebCurSet_Entry")) && (!strcmp(attr, "CaptchaOK")))
	{	
		capt_idx = get_param(g_var_post,"captcha_url");
		capt_val = get_param(g_var_post,"validateCode");

			if(capt_val != NULL && capt_idx !=NULL){
					decode_uri(capt_idx);
					decode_uri(capt_val);
					captchaindex =atoi(capt_idx);
				if( strcmp(captchaArray[captchaindex],capt_val)==0)
				{	
					return 1;
				}
				return 0;
			}
			
	}
	return 0;
}
#endif
/*____________________________________________________________________*
**	function name: tcWebApi _get
**
**	description:
*     get the attribute value of specific node.
*
**	parameters:
*     reent:not use
*     id:not use
*     params:params[0] node name.
*            params[1] attribute name.
*            params[2] show or hide
*     ret:if params[2] == hide, use ret to return value.
*         if params[2] == show, use asp_send_response() to output string on page.
*
**	global:
*     none
**	return:
*     none
**	call:
*     none
**	revision:
*     1.shnwind
* _____________________________________________________________________*/
static void
tcWebApi_Get (asp_reent* reent, const asp_text* params,  asp_text* ret)
{
#if defined(TCSUPPORT_CT)
#if defined(TCSUPPORT_CT_E8GUI)
	static char val[MAX_BUFF_SIZE];
#else
	static char val[129];
#endif
	char *node,*attr,*show;
#if defined(TCSUPPORT_CT_E8GUI)
	static char retVal[MAX_BUFF_SIZE];
#else
	char retVal[129];
#endif
	int r_val, maxcnt = 6;
	char data[64] = {0};
#if defined(TCSUPPORT_CHARSET_CHANGE) 
	char utfValue[MAX_BUFF_SIZE] = {0};
	int inlen = 0;
	int outlen = MAX_BUFF_SIZE;
#endif


    node = (char*)asp_alloc(reent,params[0].len+1);
    attr = (char*)asp_alloc(reent,params[1].len+1);
    show = (char*)asp_alloc(reent,params[2].len+1);
    memset(node,0,params[0].len+1);
    memset(attr,0,params[1].len+1);
    memset(show,0,params[2].len+1);
    memcpy(node,params[0].str,params[0].len);
    memcpy(attr,params[1].str,params[1].len);
    memcpy(show,params[2].str,params[2].len);
    node[params[0].len]='\0';
    attr[params[1].len]='\0';
    show[params[2].len]='\0';

#if defined(TCSUPPORT_GENERAL_MULTILANGUAGE)
	if (!strcmp(node, "String_Entry"))
	{
		if(0 == getString(attr, val))
		{
			strncpy(val, "N/A", sizeof(val)-1);
		}	
	}
	else
	{
#endif
#if defined(TCSUPPORT_CT_JOYME4)
	/*
	Dbus info get failed under non-root mode.
	Excute "/usr/bin/gdbus call -y -d com.ctc.appframework1 -o /com/ctc/appframework1 -m com.ctc.appframework1.AppAgent.List"
	Print error message: Exhausted all available authentication mechanisms
	This issue only exists in EN7526, All chip made this modification in order to unity.
	*/
	if ( 0 == strcmp(node, "Plugin_Common")
		&& 0 == strcmp(attr, "PluginName") )
	{
		tcapi_set("WebCurSet_Entry", "PluginState", "Wait");
		r_val = ecnt_event_send(ECNT_EVENT_SYSTEM, ECNT_EVENT_PLUGIN_INFO, data, 0);
		strcpy(retVal, "0");
		asp_send_response (NULL, retVal, strlen(retVal));
		usleep( 200 * 1000 );
		/* will check update done */
		while ( maxcnt )
		{
			bzero(data, sizeof(data));
			if ( 0 == tcapi_get("WebCurSet_Entry", "PluginState", data)
				&& 0 == strcmp(data, "Done") )
			{
				break;
			}
			maxcnt --;
			usleep( 500 * 1000 );
		}

		return;
	}
#endif

    r_val = tcapi_get(node, attr, val);
		if(r_val < 0)
		{
      strcpy(val,"N/A");
    }
#if defined(TCSUPPORT_GENERAL_MULTILANGUAGE)
	}
#endif


#if defined(TCSUPPORT_CT_PON_C9)
	if( 0 == strcmp(node, "PC")
		&& 0 == strcmp(attr, "PCMAC") )
		getMACfromNeigh(((request *)reent->server_env)->remote_ip_addr, val);
#endif

/*[OSBNB00040562]ADD by peter.jiang@20141015, enable password encryption*/
#if defined(TCSUPPORT_CT_VOIP_CRYPT)
    {
        char crypt[64] = {0};
        if (!strncmp (node, "VoIPBasic_Entry", strlen("VoIPBasic_Entry"))
         && !strncmp (attr, "SIPPassword", strlen("SIPPassword"))){
            privSymmCrypt(val, crypt);
            memcpy (val, crypt, strlen(val)+1);
        }
    }
#endif
#if defined(TCSUPPORT_CHARSET_CHANGE) 
	if( 0 == strcmp(node, "Info_Ether")
		&& 0 == strcmp(attr, "HostName") )
	{

		inlen = strlen(val);
		if ( 0 == charsetconv("utf-8", "gb2312", &inlen, &outlen, val, utfValue) )
			strncpy(val, utfValue, sizeof(val) - 1);
	}
#endif
#if defined(TCSUPPORT_CGNX)
			captchaValuecheck(node,attr,val);
#endif
    if(!strcmp(show,"s")){//show -> s
#if defined(TCSUPPORT_CT_E8GUI)
    memset(retVal, 0, MAX_BUFF_SIZE);
#endif
    	strQuotConvertHTML(val,retVal);
      asp_send_response (NULL,retVal,strlen(retVal));
    }
    else if(!strcmp(show,"h")){//hide -> h
      if(strlen(val))
    	  ret->str = val;
        ret->len = strlen(val);
    }
#else
#if defined(TCSUPPORT_GUI_STRING_CONFIG) || defined(TCSUPPORT_GENERAL_MULTILANGUAGE)
#if defined(TCSUPPORT_GENERAL_MULTILANGUAGE)
	static char val[MAX_VALUE];
#else
	static char val[580];
#endif
#else
	static char val[129];
#endif
	char *node,*attr,*show;
#if defined(TCSUPPORT_GUI_STRING_CONFIG) || defined(TCSUPPORT_GENERAL_MULTILANGUAGE)
#if defined(TCSUPPORT_GENERAL_MULTILANGUAGE)
	static char retVal[MAX_STREAM];
#else
	char retVal[640];
#endif
#else
	//char retVal[129];
	char retVal[385];
#endif
	int r_val;

	node = (char*)asp_alloc(reent,params[0].len+1);
	attr = (char*)asp_alloc(reent,params[1].len+1);
	show = (char*)asp_alloc(reent,params[2].len+1);
	memset(node,0,params[0].len+1);
	memset(attr,0,params[1].len+1);
	memset(show,0,params[2].len+1);
	memcpy(node,params[0].str,params[0].len);
	memcpy(attr,params[1].str,params[1].len);
	memcpy(show,params[2].str,params[2].len);
	node[params[0].len]='\0';
	attr[params[1].len]='\0';
	show[params[2].len]='\0';
#ifdef RA_PARENTALCONTROL
	if(strcmp(node, "Parental") == 0 && strcmp(attr, "BrowserMAC") == 0){
		getClientMacAddr(reent->server_env->remote_ip_addr);
	} else {
#endif/*RA_PARENTALCONTROL*/

#if defined(TCSUPPORT_GENERAL_MULTILANGUAGE)
		if (!strcmp(node, "String_Entry"))
		{
			if(0 == getString(attr, val))
				strcpy(val, "N/A");
		}
		else {
#endif
		r_val = tcapi_get(node, attr, val);
		if(r_val < 0){
			strcpy(val,"N/A");
		}
#if defined(TCSUPPORT_GENERAL_MULTILANGUAGE)
		}
#endif
		if(!strcmp(show,"s")){//show -> s
#if defined(TCSUPPORT_GUI_STRING_CONFIG) || defined(TCSUPPORT_GENERAL_MULTILANGUAGE)
	if(strcmp(node,"String_Entry"))          //not from String_Entry
    	{    		
    		strQuotConvertHTML(val,retVal);
    	}
	else
	{
		memset(retVal,0,572);
		strcpy(retVal,val);
	}
#else
			strQuotConvertHTML(val,retVal);
#endif
			asp_send_response (NULL,retVal,strlen(retVal));
		}
		else if(!strcmp(show,"h")){//hide -> h
			if(strlen(val))
				ret->str = val;
			ret->len = strlen(val);
		}
#ifdef RA_PARENTALCONTROL
	}
#endif/*RA_PARENTALCONTROL*/
#endif
}

#if defined(TCSUPPORT_CT_JOYME2) || defined(TCSUPPORT_CMCCV2) || defined(TCSUPPORT_ANDLINK)
#define MAC_STR_LEN				12
#define	MAC_STR_DOT_LEN			17
#define SEC_PER_DAY 		86400 /*60*60*24  for ppp link-up time*/
#define SEC_PER_HOUR 		3600 /*60*60         shnwind 2008.4.14*/
#define SEC_PER_MIN 		60
#define DHCPLEASE_PATH 		"/etc/udhcp_lease"

#define	MAX_URLFILTER_NUM	100
#if defined(TCSUPPORT_ANDLINK)
#define MAX_MACFILTER_NUM	32
#define MAX_IPMACFILTER_RULE 120
#define MIN_IPMACFILTER_RULE 40
#define	MAX_PORTFILTER_NUM	40
#else

#if defined(TCSUPPORT_CT_JOYME4)
#define	MAX_PORTFILTER_NUM	40
#else
#define	MAX_PORTFILTER_NUM	100
#endif

#define	MAX_MACFILTER_NUM	100

#define MAX_IPMACFILTER_RULE (2 * MAX_PORTFILTER_NUM + MAX_MACFILTER_NUM)
#endif


#if defined(TCSUPPORT_CMCCV2) || defined(TCSUPPORT_ANDLINK)
int get_macfilter_info(asp_reent* reent, const asp_text* params, char *p_action, char *p_area, char **result)
{
	char nodeName[32] = {0}, totalInfo[128 * MAX_MACFILTER_NUM] = {0};
	char tmpbuf[32] = {0}, MacName[32] = {0}, MacAddr[20] = {0}, Active[4] = {0};
	int i = 0, len = 0, count = 0;

	len += snprintf(totalInfo + len, sizeof(totalInfo) - len, "{\"data\":[");
	for( i = 0; i < MAX_MACFILTER_NUM; i++ )
	{
		snprintf(nodeName, sizeof(nodeName), "IpMacFilter_Entry%d", i);
		if(tcapi_get(nodeName, "Active", Active) == 0)
		{
			tcapi_get(nodeName, "MacName", MacName);
			tcapi_get(nodeName, "MacAddr", MacAddr);

			len += snprintf(totalInfo + len, sizeof(totalInfo) - len, "{\"Index\":\"%d\",\"MacName\":\"%s\",\"MacAddr\":\"%s\",\"Active\":\"%s\"},", i, MacName, MacAddr, Active);
			count++;
		}
	}

	if( count )
		len += snprintf(totalInfo + len - 1, sizeof(totalInfo) - len + 1, "]}");
	else
		len += snprintf(totalInfo + len, sizeof(totalInfo) - len, "]}");
	
	asprintf(result, "%s", totalInfo);
	
	return 0;
}
int convertFormat(char *source, char *dest, char *target, char *replace)
{
	char *p = NULL, *p1 = NULL, *p2 = NULL;
	
	p1 = dest;
	p2 = source;
	while( p = strstr(p2, target) )
	{
		strncpy(p1, p2, p - p2);
		p1 += p - p2;
		strncpy(p1, replace, strlen(replace));
		p1 += strlen(replace);
		p2 = p + 1;
	}
	
	strncpy(p1, p2, strlen(p2));
	return 0;
}
int get_ssidname_info(int radio, char *totalBuf, int maxlen)
{
	char tbuf[256] = {0}, bssidnum[8] = {0}, nodeName[32] = {0};
	char value[256] = {0}, tmp[256] = {0};
	char nodeprefix[32] = {0}, dest[256] = {0};
	int inlen = 0, outlen = 0, maxloop = 0, i = 0, len = 0;
	
	if( 0 == radio )
	{
		tcapi_get("WLan_Common", "BssidNum", bssidnum);
		snprintf(nodeprefix, sizeof(nodeprefix), "WLan_Entry");
	}
	else
	{
		tcapi_get("WLan11ac_Common", "BssidNum", bssidnum);
		snprintf(nodeprefix, sizeof(nodeprefix), "WLan11ac_Entry");
	}

	maxloop = atoi(bssidnum);
	for( i = 0; i < maxloop; i++ )
	{
		memset(value, 0, sizeof(value));
		memset(tbuf, 0, sizeof(tbuf));
		memset(dest, 0, sizeof(dest));
		memset(nodeName, 0, sizeof(nodeName));
		snprintf(nodeName, sizeof(nodeName), "%s%d", nodeprefix, i);
		tcapi_get(nodeName, "SSID", value);
		
		convertFormat(value, dest, "\\", "\\\\");
		snprintf(tmp, sizeof(tmp), "%s", dest);
		memset(dest, 0, sizeof(dest));
		convertFormat(tmp, dest, "\"", "\\\"");
		
		inlen = strlen(dest);		
		outlen = sizeof(tbuf);
#if defined(TCSUPPORT_CHARSET_CHANGE) 
		charsetconv("utf-8", "gb2312", &inlen, &outlen, dest, tbuf);
#else
		snprintf(tbuf, sizeof(tbuf), "%s", dest);
#endif
		len += snprintf(totalBuf + len, maxlen - len, "\"%s\",", tbuf);
	}
	totalBuf[len -1] = '\0';

	return 0;
}

int get_wlan_ssid_info(asp_reent* reent, const asp_text* params, char *p_name, char *p_area, char **result)
{
	char totalBuf[1024] = {0};

	get_ssidname_info(0, totalBuf, sizeof(totalBuf));
	asprintf(result, "%s", totalBuf);

	return 0;
}

int get_wlanac_ssid_info(asp_reent* reent, const asp_text* params, char *p_name, char *p_area, char **result)
{
	char totalBuf[1024] = {0};

	get_ssidname_info(1, totalBuf, sizeof(totalBuf));
	asprintf(result, "%s", totalBuf);

	return 0;
}

int set_ssidname_info(asp_reent* reent, const asp_text* params, char *p_radio, char *p_val, char **result)
{
	char tbuf[256] = {0}, wlanid[8] = {0}, nodeName[32] = {0};
	char currRadio[8] = {0};
	int inlen = 0, outlen = 0;
	
	tcapi_get("WebCurSet_Entry", "currRadio", currRadio);
	if( !strcmp(currRadio, "0") )
	{
		tcapi_get("WebCurSet_Entry", "wlan_id", wlanid);
		snprintf(nodeName, sizeof(nodeName), "WLan_Entry%s", wlanid);
	}
	else
	{
		tcapi_get("WebCurSet_Entry", "wlanac_id", wlanid);
		snprintf(nodeName, sizeof(nodeName), "WLan11ac_Entry%s", wlanid);
	}
	
	memset(tbuf, 0, sizeof(tbuf));
	inlen = strlen(p_val);		
	outlen = sizeof(tbuf);
#if defined(TCSUPPORT_CHARSET_CHANGE) 
	charsetconv("gb2312", "utf-8", &inlen, &outlen, p_val, tbuf);
#else
	snprintf(tbuf, sizeof(tbuf), "%s", p_val);
#endif
	tcapi_set(nodeName, "SSID", tbuf);
	
	return 0;
}

int get_apclient_ssid_info(asp_reent* reent, const asp_text* params, char *p_name, char *p_area, char **result)
{
	char tbuf[256] = {0}, nodeName[32] = {0};
	char value[256] = {0}, totalBuf[128] = {0};
	int inlen = 0, outlen = 0, maxloop = 0, i = 0, len = 0;
	char tmp[256] = {0}, dest[256] = {0};
	
	for( i = 0; i < 2; i++ )
	{
		memset(value, 0, sizeof(value));
		memset(tbuf, 0, sizeof(tbuf));
		memset(dest, 0, sizeof(dest));
		memset(nodeName, 0, sizeof(nodeName));
		snprintf(nodeName, sizeof(nodeName), "APCli_Entry%d", i);
		tcapi_get(nodeName, "SSID", value);
		
		convertFormat(value, dest, "\\", "\\\\");
		snprintf(tmp, sizeof(tmp), "%s", dest);
		memset(dest, 0, sizeof(dest));
		convertFormat(tmp, dest, "\"", "\\\"");
		
		inlen = strlen(dest);		
		outlen = sizeof(tbuf);
#if defined(TCSUPPORT_CHARSET_CHANGE) 
		charsetconv("utf-8", "gb2312", &inlen, &outlen, dest, tbuf);
#else
		snprintf(tbuf, sizeof(tbuf), "%s", dest);
#endif
		len += snprintf(totalBuf + len, sizeof(totalBuf) - len, "\"%s\",", tbuf);
	}
	totalBuf[len -1] = '\0';

	asprintf(result, "%s", totalBuf);
	return 0;
}

int set_apclient_ssid_info(asp_reent* reent, const asp_text* params, char *p_radio, char *p_val, char **result)
{
	char tbuf[256] = {0}, wlanid[8] = {0}, nodeName[32] = {0};
	char currRadio[8] = {0};
	int inlen = 0, outlen = 0;

	snprintf(nodeName, sizeof(nodeName), "APCli_Entry%s", p_radio);

	memset(tbuf, 0, sizeof(tbuf));
	inlen = strlen(p_val);		
	outlen = sizeof(tbuf);
#if defined(TCSUPPORT_CHARSET_CHANGE)
	charsetconv("gb2312", "utf-8", &inlen, &outlen, p_val, tbuf);
#else
	snprintf(tbuf, sizeof(tbuf), "%s", p_val);
#endif
	tcapi_set(nodeName, "SSID", tbuf);
	
	return 0;
}
#else
int mac_add_dot(char *old_mac, char *new_mac)
{
	int i = 0, j = 0;

	if (old_mac == NULL || new_mac == NULL)
		return -1;

	for (i = 0; i < MAC_STR_LEN; i += 2) {
		new_mac[j] = old_mac[i];
		new_mac[j + 1] = old_mac[i + 1];
		if (j + 2 < MAC_STR_DOT_LEN)
			new_mac[j + 2] = ':';
		j += 3;
	}
	new_mac[MAC_STR_DOT_LEN] = '\0';
	
	return 0;
}

int boa_get_dhcpLease_status(char *strIP, char *pTime, 
									char *phostName, int hostNameLen)
{
	char buf[160] 		= {0};
	char mac[17]  		= {0}; 
	char ip[16]   		= {0};
	char expire[10]		= {0};
	char hostname[128]	= {0};
	//time_t curTime = time(0);
	time_t curTime;
	struct timespec curtime;
	int timeLeft;
	int day,hour,min,sec;
	FILE *fp = NULL;
	fp=fopen(DHCPLEASE_PATH, "r");
	
	if(fp == NULL){
		return 0;
	}

	clock_gettime(CLOCK_MONOTONIC,&curtime);
	curTime = curtime.tv_sec;

	while (fgets(buf, 160, fp)){
		sscanf(buf, "%s %s %s %s", mac, ip, expire, hostname);
		timeLeft = atoi(expire) - curTime;
		if(timeLeft > 0){
			trimIP(strIP);
			trimIP(ip);
			if(!strcmp(strIP,ip)){
				day = timeLeft/SEC_PER_DAY;
                hour = (timeLeft - SEC_PER_DAY * day) / SEC_PER_HOUR;
                min = (timeLeft - SEC_PER_DAY * day - SEC_PER_HOUR * hour) / SEC_PER_MIN;
                sec = timeLeft-SEC_PER_DAY * day - SEC_PER_HOUR * hour - SEC_PER_MIN * min;
				memset(buf, 0, sizeof(buf));
				sprintf(buf, "%d:%d:%d:%d", day, hour, min, sec);
				strcpy(pTime, buf);
				fclose(fp);
				snprintf(phostName, hostNameLen, "%s", hostname);
				return 1;
			}
		}	
		memset(buf, 0, sizeof(buf));
	}
	fclose(fp);

	return 0;	
}

int get_lanhost_info(asp_reent* reent, const asp_text* params, char *p_index, char *p_val, struct ctc_au_param *au_param, char **result)
{	
	char nodeName[32] = {0};	
	char IPv6Addr[128] = {0}, IPAddr[32] = {0};
	char MacTmp[32] = {0}, MacAddr[32] = {0}, Active[4] = {0};
	char dhcpstat[4] = {0}, expireTime[32] = {0}, hostname[128] = {0};
	char Model[64] = {0}, Brand[64] = {0}, Os[64] = {0}, rxBytes[64] = {0}, txBytes[64] = {0};
	char strbuf[128] = {0}, strgbkbuf[128] = {0}, gbkModel[64] = {0}, gbkBrand[64] = {0}, gbkOs[64] = {0};
	int i = 0, count = 0, inlen = 0, outlen = 0;

	char conntype[4] = {0}, port[6] = {0};
	asp_send_format_response("{\"data\":[");
	
	for( i = 0; i < MAX_LANHOST2_ENTRY_NUM; i++ )
	{
		memset(Active, 0, sizeof(Active));
		memset(MacTmp, 0, sizeof(MacTmp));
		memset(MacAddr, 0, sizeof(MacAddr));
		memset(IPAddr, 0, sizeof(IPAddr));
		memset(IPv6Addr, 0, sizeof(IPv6Addr));
		memset(expireTime, 0, sizeof(expireTime));
		memset(hostname, 0, sizeof(hostname));
		memset(Model, 0, sizeof(Model));
		memset(Brand, 0, sizeof(Brand));
		memset(Os, 0, sizeof(Os));
		memset(rxBytes, 0, sizeof(rxBytes));
		memset(txBytes, 0, sizeof(txBytes));
		memset(dhcpstat, 0, sizeof(dhcpstat));
		memset(conntype, 0, sizeof(conntype));
		memset(port, 0, sizeof(port));
		snprintf(nodeName, sizeof(nodeName), "LANHost2_Entry%d", i);
		
		if(tcapi_get(nodeName, "Active", Active) == 0 && !strcmp(Active, "1"))		
		{			
			if( tcapi_get(nodeName, "MAC", MacTmp) == 0 && 0 != MacTmp[0] 
				&& tcapi_get(nodeName, "IP", IPAddr) == 0 && 0 != IPAddr[0])
			{
#if defined(TCSUPPORT_CT_JOYME4)
				if( tcapi_get(nodeName, "ConnectionType", conntype) == 0 && !strcmp(conntype, "1") )
				{
					if( tcapi_get(nodeName, "Port", port) == 0 && 0 != strcmp(port, "1") &&  0 != strcmp(port, "9"))
					{
						continue;
					}

				}
#endif
				mac_add_dot(MacTmp, MacAddr);
				if ( 0 != tcapi_get(nodeName, "g_IPv6_1", IPv6Addr)
					|| 0 == IPv6Addr[0] )
				tcapi_get(nodeName, "l_IPv6", IPv6Addr);

				if (!boa_get_dhcpLease_status(IPAddr, expireTime, hostname, sizeof(hostname)))
				{
					strncpy(dhcpstat, "0",sizeof(dhcpstat)-1);
				}
				else
				{
					strncpy(dhcpstat, "1",sizeof(dhcpstat)-1);
				}
				
				memset(strbuf, 0 , sizeof(strbuf));
				tcapi_get(nodeName, "HostName", strbuf);
				if ( 0 != strbuf[0] )
				{
					memset(hostname, 0, sizeof(hostname));
					snprintf(hostname, sizeof(hostname), "%s", strbuf);
				}

				memset(strgbkbuf, 0, sizeof(strgbkbuf));
				inlen = strlen(hostname);
				outlen = sizeof(strgbkbuf);
				/* "" when fail to conv */
				charsetconv("utf-8", "gb2312", &inlen, &outlen, hostname, strgbkbuf);
				
				if( strlen(expireTime) == 0)
				{
					snprintf(expireTime, sizeof(expireTime), "0");
				}

				memset(gbkModel, 0, sizeof(gbkModel));
				if( 0 == tcapi_get(nodeName, "Model", Model) && 0 != Model[0] )
				{
					inlen = strlen(Model);
					outlen = sizeof(gbkModel);
					charsetconv("utf-8", "gb2312", &inlen, &outlen, Model, gbkModel);
				}
				else
					strncpy(gbkModel, "unknown", sizeof(gbkModel) - 1);

				memset(gbkBrand, 0, sizeof(gbkBrand));
				if( 0 == tcapi_get(nodeName, "Brand", Brand) && 0 != Brand[0] )
				{
					inlen = strlen(Brand);
					outlen = sizeof(gbkBrand);
					charsetconv("utf-8", "gb2312", &inlen, &outlen, Brand, gbkBrand);
				}
				else
					strncpy(gbkBrand, "unknown", sizeof(gbkBrand) - 1);

				memset(gbkOs, 0, sizeof(gbkOs));
				if( 0 == tcapi_get(nodeName, "OS", Os) && 0 != Os[0] )
				{
					inlen = strlen(Os);
					outlen = sizeof(gbkOs);
					charsetconv("utf-8", "gb2312", &inlen, &outlen, Os, gbkOs);				
				}
				else
					strncpy(gbkOs, "unknown", sizeof(gbkOs) - 1);

				if( 0 != tcapi_get(nodeName, "RxBytes", rxBytes) || 0 == rxBytes[0] )
				{
					strncpy(rxBytes, "0", sizeof(rxBytes) - 1);
				}

				if( 0 != tcapi_get(nodeName, "TxBytes", txBytes) || 0 == txBytes[0] )
				{
					strncpy(txBytes, "0", sizeof(txBytes) - 1);
				}

				if( 0 != count )
				{
					asp_send_format_response(",");
				}
				
				asp_send_format_response("{\"MAC\":\"%s\",\"IP\":\"%s\",\"IPv6\":\"%s\",\"DhcpStatus\":\"%s\",\"HostName\":\"%s\",\"ExpireTime\":\"%s\",\"EntryIndex\":\"%d\",\"Model\":\"%s\",\"Brand\":\"%s\",\"OS\":\"%s\",\"RxBytes\":\"%s\",\"TxBytes\":\"%s\"}",
					MacAddr, IPAddr, IPv6Addr, dhcpstat, strgbkbuf, expireTime, i, gbkModel, gbkBrand, gbkOs, rxBytes, txBytes);
				
				count++;
			}
		}	
	}	
	
	asp_send_format_response("]}");

	return 0;
}

#if defined(TCSUPPORT_CT_JOYME4)
int get_urlfilter_info(asp_reent* reent, const asp_text* params, char *p_index, char *p_val, struct ctc_au_param *au_param, char **result)
{	
	char nodeName[32] = {0}, url[64] = {0}, Active[4] = {0};
	char attrName[16] = {0}, url_num[16] = {0};
	int i = 0, count = 0;

	asp_send_format_response("{\"data\":[");

	memset(nodeName, 0, sizeof(nodeName));
	snprintf(nodeName, sizeof(nodeName), "UrlFilter_Entry0");
	for( i = 0; i < MAX_URLFILTER_NUM; i++ )
	{
		memset(attrName, 0, sizeof(attrName));
		memset(Active, 0, sizeof(Active));
		memset(url, 0, sizeof(url));
		
		snprintf(attrName, sizeof(attrName), "URL%d", i);

		if(tcapi_get(nodeName, "Activate", Active) == 0)
		{	
			tcapi_get(nodeName, attrName, url);
			if(url[0] == 0)
				continue;

			if( 0 != count )
			{
				asp_send_format_response(",");
			}

			asp_send_format_response("{\"Index\":\"%d\",\"URL\":\"%s\", \"Active\":\"%s\"}", i, url, Active);
			
			count++;
		}
	}

	memset(url, 0, sizeof(url));
	tcapi_get(nodeName, "URL0", url);
	if(url[0] != 0)
		count--;
	snprintf(url_num, sizeof(count), "%d", count);
	tcapi_set(nodeName, "url_num", url_num);

	asp_send_format_response("]}");
	
	return 0;
}
#else
int get_urlfilter_info(asp_reent* reent, const asp_text* params, char *p_index, char *p_val, struct ctc_au_param *au_param, char **result)
{	
	char nodeName[32] = {0}, url[64] = {0}, Active[4] = {0};
	int i = 0, count = 0;
	
	asp_send_format_response("{\"data\":[");

	for( i = 0; i < MAX_URLFILTER_NUM; i++ )
	{
		memset(nodeName, 0, sizeof(nodeName));
		memset(Active, 0, sizeof(Active));
		memset(url, 0, sizeof(url));
		
		snprintf(nodeName, sizeof(nodeName), "UrlFilter_Entry%d", i);
		if(tcapi_get(nodeName, "Activate", Active) == 0)
		{
			tcapi_get(nodeName, "URL", url);

			if( 0 != count )
			{
				asp_send_format_response(",");
			}

			asp_send_format_response("{\"Index\":\"%d\",\"URL\":\"%s\", \"Active\":\"%s\"}", i, url, Active);
			
			count++;
		}
	}
	
	asp_send_format_response("]}");
	
	return 0;
}
#endif

int get_macfilter_info(asp_reent* reent, const asp_text* params, char *p_index, char *p_val, struct ctc_au_param *au_param, char **result)
{	
	char nodeName[32] = {0}, MacName[32] = {0}, MacAddr[20] = {0}, Active[4] = {0};
	int i = 0, count = 0;
	
	asp_send_format_response("{\"data\":[");

	for( i = 0; i < MAX_MACFILTER_NUM; i++ )
	{
		memset(nodeName, 0, sizeof(nodeName));
		memset(Active, 0, sizeof(Active));
		memset(MacName, 0, sizeof(MacName));
		memset(MacAddr, 0, sizeof(MacAddr));
		
		snprintf(nodeName, sizeof(nodeName), "IpMacFilter_Entry%d", i);
		if(tcapi_get(nodeName, "Active", Active) == 0)
		{
			tcapi_get(nodeName, "MacName", MacName);
			tcapi_get(nodeName, "MacAddr", MacAddr);

			if( 0 != count )
			{
				asp_send_format_response(",");
			}

			asp_send_format_response("{\"Index\":\"%d\",\"MacName\":\"%s\",\"MacAddr\":\"%s\",\"Active\":\"%s\"}", i, MacName, MacAddr, Active);
			
			count++;
		}
	}
	
	asp_send_format_response("]}");
	
	return 0;
}

unsigned long long get_LANTxByte(int port)
{
	unsigned long long LANTXByte = 0; 
	char node_name[32] = {0}, entry_name[32] = {0};
	char invalid_lan_port[32] = {0};
	int lan_port1=0, lan_port2=0;

	if ( blapi_traffic_get_port_tx_cnt(port, &LANTXByte) < 0 )
		tcdbg_printf("%s blapi_traffic_get_port_tx_cnt failed[%d].\n", __FUNCTION__, port);

	snprintf(node_name, sizeof(node_name), "Lan_Entry");
	snprintf(entry_name, sizeof(entry_name), "invalid_lan_port");
	tcapi_get(node_name, entry_name, invalid_lan_port);
	sscanf(invalid_lan_port, "%d,%d", &lan_port1, &lan_port2);

	if((lan_port1 == port+1) || (lan_port2 == port+1))
		LANTXByte = 0;

	return LANTXByte;
}

unsigned long long get_LANRxByte(int port)
{
	unsigned long long LANRXByte = 0; 
	char node_name[32] = {0}, entry_name[32] = {0};
	char invalid_lan_port[32] = {0};
	int lan_port1=0, lan_port2=0;

	if ( blapi_traffic_get_port_rx_cnt(port, &LANRXByte) < 0 )
		tcdbg_printf("%s blapi_traffic_get_port_rx_cnt failed[%d].\n", __FUNCTION__, port);

	snprintf(node_name, sizeof(node_name), "Lan_Entry");
	snprintf(entry_name, sizeof(entry_name), "invalid_lan_port");
	tcapi_get(node_name, entry_name, invalid_lan_port);
	sscanf(invalid_lan_port, "%d,%d", &lan_port1, &lan_port2);

	if((lan_port1 == port+1) || (lan_port2 == port+1))
		LANRXByte = 0;

	return LANRXByte;
}

int get_LANState(int port)
{
	char node_name[32] = {0}, lan_port[32] = {0};
	char lan_status[32] = {0}, entry_name[32] = {0};
	char invalid_lan_port[32] = {0};
	int lan_port1=0, lan_port2=0;

	snprintf(node_name, sizeof(node_name), "Sys_Entry");
	snprintf(lan_port, sizeof(lan_port), "status_lan%d", port);
	tcapi_get(node_name, lan_port, lan_status);

	snprintf(node_name, sizeof(node_name), "Lan_Entry");
	snprintf(entry_name, sizeof(entry_name), "invalid_lan_port");
	tcapi_get(node_name, entry_name, invalid_lan_port);
	sscanf(invalid_lan_port, "%d,%d", &lan_port1, &lan_port2);

	if((lan_port1 == port) || (lan_port2 == port))
		return 2;
	else
		return atoi(lan_status);
}

#define LANPORTNUM 4

int get_lancnt_info(asp_reent* reent, const asp_text* params, char *p_index, char *p_val, struct ctc_au_param *au_param, char **result)
{	
	FILE *fp1 = NULL, *fp2 = NULL;
	char nodeName[32] = {0}, buf[128] = {0};
	int i = 0, count = 0, port = 0;
	char negoration[LANPORTNUM][16] = {0}, mode[LANPORTNUM][16] = {0};
	char port_buf[64] = {0}, link_rate[64] = {0}, real_rate[64] = {0};
	int port_line = 0, lanPort[4], switchPort[4];

	fp2 = fopen("/proc/tc3162/eth_portmap", "r");
	if (fp2== NULL)
		return 0;
	bzero(buf, sizeof(buf));
	while (fgets(buf, sizeof(buf), fp2))
	{
		if (strstr(buf, "lan_port_map") == NULL) {
			continue;
		} else {
			fgets(buf, sizeof(buf), fp2);
			sscanf(buf, "%d %d", &lanPort[0], &switchPort[0]);
			fgets(buf, sizeof(buf), fp2);  
			sscanf(buf, "%d %d", &lanPort[1], &switchPort[1]);
			fgets(buf, sizeof(buf), fp2);  
			sscanf(buf, "%d %d", &lanPort[2], &switchPort[2]);
			fgets(buf, sizeof(buf), fp2);  
			sscanf(buf, "%d %d", &lanPort[3], &switchPort[3]);
			break;
		}
	}
	fclose(fp2);
	
	fp1 = fopen("/proc/tc3162/gsw_link_st", "r");
	if (fp1 == NULL)
		return 0;

	bzero(buf, sizeof(buf));
	while (fgets(buf, sizeof(buf), fp1))
	{
		if (strstr(buf, "Port") == NULL)
			continue;

		if (port > 0 && port <= LANPORTNUM)
		{
			sscanf(buf, "%s : %s", port_buf, link_rate);
			sscanf(link_rate, "%[^/]", real_rate);
	
			if (!strcmp(real_rate, "10M") || !strcmp(real_rate, "100M") || !strcmp(real_rate, "1000M"))
	{
				snprintf(negoration[switchPort[port - 1] - 1], sizeof(negoration[switchPort[port - 1] - 1]), real_rate);
				if(strstr(link_rate, "Full"))
					snprintf(mode[switchPort[port - 1] - 1], sizeof(mode[switchPort[port - 1] - 1]), "Full");
				else
					snprintf(mode[switchPort[port - 1] - 1], sizeof(mode[switchPort[port - 1] - 1]), "Half");	
			}
			else
			{
				snprintf(negoration[switchPort[port - 1] - 1], sizeof(negoration[switchPort[port - 1] - 1]), "-");
				snprintf(mode[switchPort[port - 1] - 1], sizeof(mode[switchPort[port - 1] - 1]), "-");		
			}
	}
		port++;
		
		if (port > LANPORTNUM)
			break;
	}
	fclose(fp1);

	asp_send_format_response("{\"data\":[");

	for( i = 1; i <= LANPORTNUM; i++ )
	{
		if( 0 != count )
		{
			asp_send_format_response(",");
		}

		asp_send_format_response("{\"Port\":\"%d\", \"LanState\":\"%d\", \"TxBytes\":\"%llu\", \"RxBytes\":\"%llu\", \"Negoration\":\"%s\", \"Mode\":\"%s\"}", 
			i, get_LANState(i), get_LANTxByte(i - 1), get_LANRxByte(i - 1), negoration[i - 1], mode[i - 1]);

		count++;
	}
	
	asp_send_format_response("]}");
	
	return 0;
}

int get_devname_info(asp_reent* reent, const asp_text* params, char *p_index, char *p_val, struct ctc_au_param *au_param, char **result)
{	
	char nodeName[32] = {0}, devName[128] = {0}, strgbkbuf[128] = {0};
	int i = 0, count = 0, negoration[LANPORTNUM] = {1000, 1000, 1000, 1000};
	int inlen = 0, outlen = 0;

	asp_send_format_response("{\"devName\":\"");

	memset(devName, 0, sizeof(devName));

	if(0 == tcapi_get("Sys_Entry", "DevName", devName))
	{
		memset(strgbkbuf, 0, sizeof(strgbkbuf));
		inlen = strlen(devName);
		outlen = sizeof(strgbkbuf);

		charsetconv("utf-8", "gb2312", &inlen, &outlen, devName, strgbkbuf);
		
		asp_send_format_response(strgbkbuf);

	}

	asp_send_format_response("\"}");
	
	return 0;
}

#endif

char ipmac_attribute[][32]=
{
	{"Active"},
	{"Interface"},
	{"IPName"},
	{"Protocol"},
	{"SrcIPAddr"},
	{"SrcIPMask"},
	{"SrcPort"},
	{"DesIPAddr"},
	{"DesIPMask"},
	{"DesPort"},
	{""},
};

int get_portfilterOut_info(asp_reent* reent, const asp_text* params, char *p_index, char *p_val, struct ctc_au_param *au_param, char **result)
{	
	char nodeName[32] = {0},  Active[4] = {0};
	int i, j, count = 0, min = 0, max = 0;
	char attr_value[10][64] = {0};	
	
	asp_send_format_response("{\"data\":[");
	
#if defined(TCSUPPORT_ANDLINK)
	min = MIN_IPMACFILTER_RULE;
	max = MIN_IPMACFILTER_RULE+MAX_PORTFILTER_NUM;
#else
	min = MAX_MACFILTER_NUM;
	max = MAX_MACFILTER_NUM+MAX_PORTFILTER_NUM;
#endif

	for( i = min; i < max; i++ )
	{
		memset(nodeName, 0, sizeof(nodeName));
		memset(Active, 0, sizeof(Active));
		memset(attr_value, 0, sizeof(attr_value));
		
		snprintf(nodeName, sizeof(nodeName), "IpMacFilter_Entry%d", i);
		if(tcapi_get(nodeName, "Active", Active) == 0)
		{
			for(j= 0; j<10; j++){
				tcapi_get(nodeName, ipmac_attribute[j],  attr_value[j]);
			}

			if( 0 != count )
			{
				asp_send_format_response(",");
			}

			asp_send_format_response("{\"Index\":\"%d\", \"Active\":\"%s\", \"Interface\":\"%s\", \"IPName\":\"%s\", \"Protocol\":\"%s\", "
				"\"SrcIPAddr\":\"%s\",	\"SrcIPMask\":\"%s\", \"SrcPort\":\"%s\", \"DesIPAddr\":\"%s\", \"DesIPMask\":\"%s\", \"DesPort\":\"%s\"}", 
				i , attr_value[0], attr_value[1], attr_value[2], attr_value[3], attr_value[4], attr_value[5], attr_value[6], attr_value[7], attr_value[8], attr_value[9]);
			count++;
		}
	}
	
	asp_send_format_response("]}");
	
	return 0;
}

int get_portfilterIn_info(asp_reent* reent, const asp_text* params, char *p_index, char *p_val, struct ctc_au_param *au_param, char **result)
{	
	char nodeName[32] = {0},  Active[4] = {0};
	int i, j, count = 0, min = 0, max = 0;
	char attr_value[10][64] = {0};	
	
	asp_send_format_response("{\"data\":[");
	
#if defined(TCSUPPORT_ANDLINK)
	min = MIN_IPMACFILTER_RULE+MAX_PORTFILTER_NUM;
#else
	min = MAX_MACFILTER_NUM+MAX_PORTFILTER_NUM;
#endif
	max = MAX_IPMACFILTER_RULE;
	
	for( i = min; i < max; i++ )
	{
		memset(nodeName, 0, sizeof(nodeName));
		memset(Active, 0, sizeof(Active));
		memset(attr_value, 0, sizeof(attr_value));
		
		snprintf(nodeName, sizeof(nodeName), "IpMacFilter_Entry%d", i);
		if(tcapi_get(nodeName, "Active", Active) == 0)
		{
			for(j= 0; j<10; j++){
				tcapi_get(nodeName, ipmac_attribute[j],  attr_value[j]);
			}

			if( 0 != count )
			{
				asp_send_format_response(",");
			}

			asp_send_format_response("{\"Index\":\"%d\", \"Active\":\"%s\", \"Interface\":\"%s\", \"IPName\":\"%s\", \"Protocol\":\"%s\", "
				"\"SrcIPAddr\":\"%s\",\"SrcIPMask\":\"%s\", \"SrcPort\":\"%s\", \"DesIPAddr\":\"%s\", \"DesIPMask\":\"%s\", \"DesPort\":\"%s\"}", 
				i , attr_value[0], attr_value[1], attr_value[2], attr_value[3], attr_value[4], attr_value[5], attr_value[6], attr_value[7], attr_value[8], attr_value[9]);

			count++;
		}
	}
	
	asp_send_format_response("]}");
	
	return 0;
}

int get_aclfilter_info(asp_reent* reent, const asp_text* params, char *p_index, char *p_val, struct ctc_au_param *au_param, char **result)
{	
	char nodeName[32] = {0},  Active[4] = {0};
	int i = 0, j = 0, count = 0;
	char attr_value[6][64] = {0};	
	
	char acl_attribute[][32]=
	{
		{"Activate"},
		{"ACLName"},
		{"ScrIPAddrBegin"},
		{"ScrIPAddrEnd"},
		{"Interface"},
		{"Application"},
		{""},
	};

	asp_send_format_response("{\"data\":[");
		
	for( i = 0; i < 16; i++ )
	{
		memset(nodeName, 0, sizeof(nodeName));
		memset(Active, 0, sizeof(Active));
		memset(attr_value, 0, sizeof(attr_value));
		
		snprintf(nodeName, sizeof(nodeName), "ACL_Entry%d", i);
		if(tcapi_get(nodeName, "Activate", Active) == 0)
		{
			for(j = 0; j < 6; j++)
			{
				tcapi_get(nodeName, acl_attribute[j],  attr_value[j]);
			}

			if( 0 != count )
			{
				asp_send_format_response(",");
			}

			asp_send_format_response("{\"Index\":\"%d\", \"Active\":\"%s\", \"ACLName\":\"%s\", \"ScrIPAddrBegin\":\"%s\", \"ScrIPAddrEnd\":\"%s\", "
				"\"Interface\":\"%s\",\"Application\":\"%s\"}", 
				i , attr_value[0], attr_value[1], attr_value[2], attr_value[3], attr_value[4], attr_value[5]);

			count++;
		}
	}
	
	asp_send_format_response("]}");
	
	return 0;
}

int get_aclfilter_entry_info(asp_reent* reent, const asp_text* params, char *p_index, char *p_val, struct ctc_au_param *au_param, char **result)
{	
	char nodeName[32] = {0},  Active[4] = {0};
	int i = 0, AclEditNumber = -1;
	char attr_value[6][64] = {0};

	if(p_index == NULL)
		AclEditNumber = -1;
	else
		AclEditNumber = atoi(p_index);
		
	tcdbg_printf("\r\n[%s]%d: AclEditNumber = %d\n", __FUNCTION__, __LINE__, AclEditNumber);
	char acl_attribute[][32]=
	{
		{"Activate"},
		{"ACLName"},
		{"ScrIPAddrBegin"},
		{"ScrIPAddrEnd"},
		{"Interface"},
		{"Application"},
		{""},
	};

	asp_send_format_response("{\"data\":[");
		
	memset(nodeName, 0, sizeof(nodeName));
	memset(Active, 0, sizeof(Active));
	memset(attr_value, 0, sizeof(attr_value));

	if(AclEditNumber != -1)
	{
		snprintf(nodeName, sizeof(nodeName), "ACL_Entry%d", AclEditNumber);
		if(tcapi_get(nodeName, "Activate", Active) == 0)
		{
			for(i = 0; i < 6; i++)
			{
				tcapi_get(nodeName, acl_attribute[i],  attr_value[i]);
			}

			asp_send_format_response("{\"Index\":\"%d\", \"Active\":\"%s\", \"ACLName\":\"%s\", \"ScrIPAddrBegin\":\"%s\", \"ScrIPAddrEnd\":\"%s\", "
				"\"Interface\":\"%s\",\"Application\":\"%s\"}", 
				AclEditNumber , attr_value[0], attr_value[1], attr_value[2], attr_value[3], attr_value[4], attr_value[5]);

		}
	}
	else
		asp_send_format_response("{\"Index\":\"%d\"}", AclEditNumber);

	asp_send_format_response("]}");
	
	return 0;
}

typedef struct
{
	char *funname;
	int (*json_hook)(asp_reent* reent, const asp_text* params, char *param1, char *param2
#if !defined(TCSUPPORT_CMCCV2) && !defined(TCSUPPORT_ANDLINK)
		, struct ctc_au_param *au_param
#endif
		, char **result);
}HOOK_TABLE;

HOOK_TABLE G_JSON_TABLE[] = 
{
#if defined(TCSUPPORT_CMCCV2) || defined(TCSUPPORT_ANDLINK)
	{"cmcc_itms_auth", cmcc_itms_auth},
	{"get_macfilter_info", get_macfilter_info},
	{"get_wlan_ssid_info", get_wlan_ssid_info},
	{"get_wlanac_ssid_info", get_wlanac_ssid_info},
	{"set_ssidname_info", set_ssidname_info},
	{"get_apclient_ssid_info", get_apclient_ssid_info},
	{"set_apclient_ssid_info", set_apclient_ssid_info},
#else
	{"ctc_check_auth", ctc_check_auth},
	{"ctc_luci_check_auth", ctc_check_auth},
	{"get_lanhost_info", get_lanhost_info},
	{"set_lanhost_hostName", set_lanhost_hostName},
	{"set_lanhost_hostNamelist", set_lanhost_hostNamelist},
#if defined(TCSUPPORT_CT_JOYME4)	
	{"set_urlfilter", set_urlfilter},
#endif
	{"get_urlfilter_info", get_urlfilter_info},
	{"get_macfilter_info", get_macfilter_info},
	{"get_lancnt_info", get_lancnt_info},
	{"get_devname_info", get_devname_info},
	{"ecnt_json_get_portal_info", ecnt_json_get_portal_info},
	{"ecnt_json_get_wandata_info", ecnt_json_get_wandata_info},
#if defined(TCSUPPORT_CUC)
	{"wan_interface_name", ecnt_get_wan_interface_name},
	{"wan_internet_ppp_waninfo", ecnt_get_internet_ppp_waninfo},
	{"set_wan_internet_ppp_waninfo", ecnt_set_internet_ppp_waninfo},
	{"get_ssid1_info", ecnt_get_ssid1_info},
	{"get_lanhost2_info", ecnt_get_lanhost2_info},
#endif
#endif
	{"get_portfilterOut_info", get_portfilterOut_info},
	{"get_portfilterIn_info", get_portfilterIn_info},
	{"get_aclfilter_info", get_aclfilter_info},
	{"get_aclfilter_entry_info", get_aclfilter_entry_info},
	{NULL, NULL}
};


static void 
tcWebApi_JsonHook(asp_reent* reent, const asp_text* params, asp_text* ret)
{
	char *funname = NULL, *param1 = NULL, *param2 = NULL;
	char *pval_1 = NULL, *pval_2 = NULL;
	char *json_result = NULL;
	int idx = 0;
#if !defined(TCSUPPORT_CMCCV2) && !defined(TCSUPPORT_ANDLINK)
	struct ctc_au_param au_param;

	bzero(&au_param, sizeof(au_param));
#endif
	/* current: only support two parameters. */
	funname = (char*)asp_alloc(reent, params[0].len+1);
	param1 = (char*)asp_alloc(reent, params[1].len+1);
	param2 = (char*)asp_alloc(reent, params[2].len+1);
	bzero(funname, params[0].len+1);
	bzero(param1, params[1].len+1);
	bzero(param2, params[2].len+1);
	memcpy(funname, params[0].str, params[0].len);
	memcpy(param1, params[1].str, params[1].len);
	memcpy(param2, params[2].str, params[2].len);
	funname[params[0].len] = 0;
	param1[params[1].len] = 0;
	param2[params[2].len] = 0;

	for ( idx = 0; NULL != G_JSON_TABLE[idx].funname; idx ++ )
	{
		if ( 0 == strcmp(funname, G_JSON_TABLE[idx].funname) )
		{
			pval_1 = get_param(g_var_post, param1);
			pval_2 = get_param(g_var_post, param2);
			if ( pval_1 )
				decode_uri(pval_1);
			if ( pval_2 )
				decode_uri(pval_2);
#if !defined(TCSUPPORT_CMCCV2) && !defined(TCSUPPORT_ANDLINK)
			if ( 0 == strcmp(funname, "ctc_luci_check_auth") )
				au_param.login_via_luci = 1;
#endif

			G_JSON_TABLE[idx].json_hook(reent, params, pval_1, pval_2
#if !defined(TCSUPPORT_CMCCV2) && !defined(TCSUPPORT_ANDLINK)
			, &au_param
#endif
			, &json_result);
			break;
		}
	}

	if ( json_result )
	{
		asp_send_response (NULL, json_result, strlen(json_result));
		free(json_result);
	}

}
#endif

#if defined(TCSUPPORT_ECNT_MAP)
typedef struct
{
	char *funname;
	int (*json_hook)(asp_reent* reent, char **result);
}MESH_HOOK_TABLE;

MESH_HOOK_TABLE G_MESH_JSON_TABLE[] = 
{
	{"mesh_get_run_time_topology", mesh_get_run_time_topology_fun},
	{"mesh_get_ap_bh_inf_list", mesh_get_ap_bh_inf_list_fun},
	{"mesh_get_ap_fh_inf_list", mesh_get_ap_fh_inf_list_fun},
	{"mesh_get_client_capabilities", mesh_get_client_capabilities_fun},
	{"mesh_get_sta_steering_progress", mesh_get_sta_steering_progress_fun},
	{"mesh_get_sta_bh_interface", mesh_get_sta_bh_interface_fun},
	{NULL, NULL}
};

static void 
tcWebApi_MeshJsonHook(asp_reent* reent, const asp_text* params, asp_text* ret)
{
	char *funname = NULL;
	char *json_result = NULL;
	int idx = 0;

	funname = (char*)asp_alloc(reent, params[0].len+1); 
	bzero(funname, params[0].len+1);
	memcpy(funname, params[0].str, params[0].len);
 	funname[params[0].len] = 0;
 
	for ( idx = 0; NULL != G_MESH_JSON_TABLE[idx].funname; idx ++ )
	{
		if ( 0 == strcmp(funname, G_MESH_JSON_TABLE[idx].funname) )
		{
			G_MESH_JSON_TABLE[idx].json_hook(reent, &json_result);
			break;
		}
	}

	if ( json_result )
	{
		asp_send_response (NULL, json_result, strlen(json_result));
		free(json_result);
	}
}
#endif

static void 
tcWebApi_getbywifiid(asp_reent* reent, const asp_text* params, asp_text* ret)
{
	char *noneName = NULL, *attr1 = NULL, *attr2 = NULL;
	char attr[128] = {0};
	char value[128] = {0};
	char attr2New[128] = {0};

	noneName = (char*)asp_alloc(reent, params[0].len+1); 
	attr1 = (char*)asp_alloc(reent, params[1].len+1); 
	attr2 = (char*)asp_alloc(reent, params[2].len+1); 
	bzero(noneName, params[0].len+1);
	memcpy(noneName, params[0].str, params[0].len);
	noneName[params[0].len] = 0;

	bzero(attr1, params[1].len+1);
	memcpy(attr1, params[1].str, params[1].len);
	attr1[params[1].len] = 0;

	bzero(attr2, params[2].len+1);
	memcpy(attr2, params[2].str, params[0].len);
	attr2[params[2].len] = 0;

	if (0 == strcmp(attr2, "wlan_id") || 0 == strcmp(attr2, "wlan_ac_id"))
	{
		tcapi_get("WebCurSet_Entry", attr2, attr2New);
	}

	snprintf(attr, sizeof(attr), "%s%s", attr1, attr2New);
	tcapi_get(noneName, attr, value);
	asp_send_response (NULL, value, strlen(value));
}

/*_____________________________________________________________________________*
**	function name: tcWebApi _unset
**
**	description:
*     delete the all attribute value of specific node.
*
**	parameters:
*     reent:not use
*     id:not use
*     params:params[0] node name.
*     ret:not use
*
**	global:
*     none
**	return:
*     0:successful
*     -1:fail
**	call:
*     none
**	revision:
*     1.shnwind
* ___________________________________________________________________________*/

static void
tcWebApi_Unset (asp_reent* reent, const asp_text* params,  asp_text* ret)
{

    char *node;
    int r_val;

    node = (char*)asp_alloc(reent,params[0].len+1);
    memset(node,0,params[0].len+1);
    memcpy(node,params[0].str,params[0].len);

    r_val=tcapi_unset(node);

}
/*___________________________________________________________________*
**	function name: tcWebApi_commit
**
**	description:
*     Write the value of specific attribute to file system.
*
**	parameters:
*     reent:not use
*     id:not use
*     params:params[0] node name.
*     ret:not use
*
**	global:
*     none
**	return:
*     0:successful
*     -1:fail
**	call:
*     none
**	revision:
*     1.shnwind
* __________________________________________________________________*/

static void
tcWebApi_Commit (asp_reent* reent, const asp_text* params,  asp_text* ret)
{
	  char *node;
    int r_val;
 #if defined(TCSUPPORT_WEB_SAVE)
    char bootType[16] = {0};
 #endif
 	int syslog_wait = 0, max_wait = 6;
 	char clearLogV[12] = {0};
#if defined(TCSUPPORT_CT)
#ifdef TCSUPPORT_SYSLOG	
	char loginfo_array[][2][64]=
	{
		{{"WanInfo"},{"Wan Configuration changed\n"}},
		{{"Lan"},{"Lan Configuration changed\n"}},
		{{"Dhcpd_Common"},{"Lan dhcp server Configuration changed\n"}},
		{{"Dhcpd_Option60"},{"Lan dhcp server option60 Configuration changed\n"}},
		{{"Radvd_Common"},{"Lan IPv6 radvd Configuration changed\n"}},
		{{"Dhcp6s_Common"},{"Lan IPv6 dhcp Configuration changed\n"}},
		{{"WLan_Entry"},{"WLan 2.4G Configuration changed\n"}},
		{{"WLan11ac_Entry"},{"WLan 5G Configuration changed\n"}},
		{{"Cwmp_Entry"},{"TR069 Configuration changed\n"}},
		{{"Ddns"},{"DDNS Configuration changed\n"}},
		{{"Account_Entry"},{"Login password changed\n"}},
		{{""},{""}}
	};
	char logContent[64] = {0};
	int i=0;
#endif
#endif
#if defined(TCSUPPORT_CMCCV2)
	char LinkMode[10] = {0};
	char mac_num[8] = {0};
	char nodeName[32] = {0};
	char MacAddr[32] = {0};
	char LoginFailTime[8] = {0};
	char LoginCondition[2] = {0};
	int macnum = 0, j = 0, LoginFailTimenum = 0;
#endif

    node = (char*)asp_alloc(reent,params[0].len+1);
    memset(node,0,params[0].len+1);
    memcpy(node,params[0].str,params[0].len);

#if defined(TCSUPPORT_WAN_ATM) || defined(TCSUPPORT_WAN_PTM) || defined(TCSUPPORT_WAN_ETHER)
//when someone uses webpage to commit wan, let CC have one more chance to do WAN_STOP or WAN_START for ATM/PTM.
	if(strstr(node,"Wan_PVC")){
		system("echo 1 > /tmp/commitByWeb");
	}
#endif
	if ( 0 == strcmp(node, "SysLog_Entry") )
	{
		if ( 0 == tcapi_get(node, "clearLog", clearLogV)
			&& 0 == strcmp(clearLogV, "Yes") )
			syslog_wait = 1;
	}

    r_val=tcapi_commit(node);
    //tcdbg_printf("commit ret = %d node %s\n",r_val,node);

	if ( syslog_wait )
	{
		while ( max_wait )
		{
			if ( 0 == tcapi_get(node, "clearLog", clearLogV)
				&& 0 == clearLogV[0] )
				break;
			usleep( 300 * 1000 );
			max_wait --;
		}
	}

#if defined(TCSUPPORT_GENERAL_MULTILANGUAGE)
	if (!strcmp(node, "LanguageSwitch_Entry"))
	{
		if (1 == islangChanged()) {
			initandparserfile();
		}
	}
#endif

#if defined(TCSUPPORT_WEB_SAVE)
    flag_save = 1;
    if(strcmp(node, SYSTEM_NODE) == 0) {
	tcapi_get(SYSTEM_NODE, REBOOT_TYPE, bootType);
	if(atoi(bootType) != NO_BOOT)
	     flag_save = 0;//If do system reboot or reset default, should not do save
    }
#endif

#ifdef TCSUPPORT_SYSLOG_ENHANCE
   	openlog("TCSysLog WEB", 0, LOG_LOCAL1);
   	syslog(LOG_INFO, "Configuration changed\n");
	closelog();
#endif
#if defined(TCSUPPORT_CT)
#ifdef TCSUPPORT_SYSLOG	
   	openlog("TCSysLog WEB", 0, LOG_LOCAL1);
	strcpy(logContent,"Other Configuration changed\n");
	for(i=0; strlen(loginfo_array[i][0])!=0; i++){
		if(strcmp(node,loginfo_array[i][0]) == 0){
			memset(logContent,0,sizeof(logContent));
			strcpy(logContent,loginfo_array[i][1]);
			break;
		}
	}
   	syslog(LOG_INFO, logContent);
	closelog();
#endif
#endif
#if defined(TCSUPPORT_CMCCV2)
	if(strcmp(node,"WanInfo_WanPVC") == 0)
	{
		tcapi_get("WanInfo_WanIF","LinkMode",LinkMode);
		if(strcmp(LinkMode,"linkPPP") == 0)
		{
			openlog("TCSysLog WEB", 0, LOG_LOCAL1);
			syslog(LOG_INFO, "Reset pppoe dial\n");
			closelog();
		}
		tcdbg_printf("LinkMode:%s\n",LinkMode);
	}
	else if(strcmp(node,"IpMacFilter_Entry") == 0)
	{
		tcapi_get("IpMacFilter","mac_num",mac_num);
		macnum = atoi(mac_num);
		for(j = 0; j < macnum; j++)
		{
			memset(nodeName,0,sizeof(nodeName));
			snprintf(nodeName,sizeof(nodeName),"IpMacFilter_Entry%d",j);
			tcapi_get(nodeName,"MacAddr",MacAddr);
			openlog("TCSysLog WEB", 0, LOG_LOCAL1);
			syslog(LOG_INFO, "mac:%s\n",MacAddr);
			closelog();
		}
	}
	else if(strcmp(node,"DeviceAlarm_Common") == 0)
	{
		tcapi_get(node,"LoginFailTime",LoginFailTime);
		tcapi_get("WebCurSet_Entry","LoginCondition",LoginCondition);
		LoginFailTimenum = atoi(LoginFailTime);
		if((LoginFailTimenum >= 10) && (strcmp(LoginCondition,"0") == 0))
		{
			openlog("TCSysLog alarm", 0, LOG_LOCAL2);
			syslog(LOG_ALERT, "id:104032 login failed 10 times\n");
			closelog();
		}
	}
#endif

}

static void
tcWebApi_CommitWithoutSave (asp_reent* reent, const asp_text* params,  asp_text* ret)
{
	  char *node;
    int r_val;

    node = (char*)asp_alloc(reent,params[0].len+1);
    memset(node,0,params[0].len+1);
    memcpy(node,params[0].str,params[0].len);

    r_val=tcapi_commit(node);
    //tcdbg_printf("commit ret = %d node %s\n",r_val,node);

}

/*____________________________________________________________________*
**	function name: tcWebApi _save
**
**	description:
*     Write all system parameters to flash
*
**	parameters:
*     reent:not use
*     id:not use
*     params:not use
*     ret:not use
*
**	global:
*     none
**	return:
*     none
**	call:
*     none
**	revision:
*     1.shnwind
* _____________________________________________________________________*/

static void
tcWebApi_Save (asp_reent* reent, const asp_text* params,  asp_text* ret)
{
#if defined(TCSUPPORT_C7) || defined(TCSUPPORT_CT_PROLINE_SUPPORT) || defined(TCSUPPORT_CT_PON_SK)
	char resetflag[8];
	tcapi_get("SysInfo_Entry", "ResetFlag", resetflag);
	if(!strcmp(resetflag, "0")){
	    tcapi_set("SysInfo_Entry", "ResetFlag", "1");
	    tcapi_commit("SysInfo");
        }
#endif
#if defined(TCSUPPORT_CT_SIMCARD_SEPARATION)
	char cardExist[16] = {0};
	char simAuthRet[16] = {0};
	char CurrentAccess[16] = {0};

	//only do write sim file when adminstrator login
	if( tcapi_get("WebCurSet_Entry", "CurrentAccess",CurrentAccess) == 0 && strcmp(CurrentAccess,"0") == 0 
		&& tcapi_get("SimCard_Entry", "cardExist",cardExist) == 0 && strcmp(cardExist,"1") == 0 
		&& tcapi_get("SimCard_Entry", "simAuthRet",simAuthRet) == 0 && strcmp(simAuthRet,"1") == 0 ){
		tcapi_set("SimCard_Entry","needWriteSim","1");
	}
#endif

#if defined(TCSUPPORT_WEB_SAVE)
	flag_save = 0;
#endif
    tcapi_save();
}

static char
hex_to_decimal(char char1,char char2)
{
	return (((char1 >= 'A') ? (((char1 & 0xdf) - 'A') + 10) : (char1 - '0')) * 16) + (((char2 >= 'A') ? (((char2 & 0xdf) - 'A') + 10) : (char2 - '0')));
}

int
decode_uri(char *uri)
{
	char c, d;
  char *uri_old;
  int count = 0;

  uri_old = uri;
  while ((c = *uri_old)) {
    if(count++ == SINGLE_POST_LIMIT_DEFAULT)
	{/*max run 128 cycle*/
      return 0;
    }
    if (c == '%') {
        uri_old++;
        if ((c = *uri_old++) && (d = *uri_old++))
          *uri++ = hex_to_decimal(c, d);
        else
          return 0;       /* NULL in chars to be decoded */
    }else{
			*uri++ = c;
      uri_old++;
    }
  }
  *uri = '\0';
  return 1;
}

/*krammer add for bug 1321*/
/*____________________________________________________________________*
**	function name: tcWebApi_CurrentDefaultRoute
**
**	description:
*     get the current route pvc number
*
**	parameters:
*     reent:not use
*     id:not use
*     params:not use
*     ret:not use
*
**	global:
*     none
**	return:
*     current default route pvc number
**	call:tcapi_get, asp_send_response
*     none
**	revision:
*     1.krammer
* _____________________________________________________________________*/

static void
tcWebApi_CurrentDefaultRoute(asp_reent* reent, const asp_text* params,  asp_text* ret)
{
	char i, defaultRT;
	int r_val;
	char val[DEFAULT_RT_ATTR_LENGTH]={0};
	char wanPvc[16]={0};

	for(i=0; i<MAX_PVC_NUMBER; i++){
		sprintf(wanPvc,"%s%d",WAN_PVC,i);
		r_val=tcapi_get(wanPvc, DEFAULT_RT, val);
		if(r_val<0){
			continue;
		}
		if(!strcmp(val, "Yes")){
			defaultRT=i+TO_ASCII_OFFSET;
			asp_send_response (NULL,&defaultRT,1);
			return;
		}
	}
	/*can't fine default route, we use pvc 0 to be the default route*/
	defaultRT=0+TO_ASCII_OFFSET;
	asp_send_response (NULL,&defaultRT,1);
}

#endif

#if defined(TCSUPPORT_IMPROVE_GUI_PERFM)
/*____________________________________________________________________*
**      function name: tcWebApi_constSet
**
**      description:
*     write the const value which is user input to cfg_manager.
*
**      parameters:
*     reent:not use
*     id:not use
*     params:params[0] node name.
*            params[1] attribute name.
*            params[2] value
*     ret:not use
*
**      global:
*     none
**      return:
*     none
**      call:
*     none
**      revision:
*     1.shnwind
* _____________________________________________________________________*/
static void
tcWebApi_constSet (asp_reent* reent, const asp_text* params,  asp_text* ret)
{
        char *node,*attr,*value;
        int r_val;
        
        node = (char*)asp_alloc(reent,params[0].len+1);
        attr = (char*)asp_alloc(reent,params[1].len+1);
        value = (char*)asp_alloc(reent,params[2].len+1);
        memset(node,0,params[0].len+1);
        memset(attr,0,params[1].len+1);
        memset(value,0,params[2].len+1);
        memcpy(node,params[0].str,params[0].len);
        memcpy(attr,params[1].str,params[1].len);
        memcpy(value,params[2].str,params[2].len);
        node[params[0].len]='\0';
        attr[params[1].len]='\0';
        value[params[2].len]='\0';

        // can alow set NULL
	/*
        if(params[2].len == 0)
        {
                return;
        }
	*/
        
        r_val=tcapi_set(node, attr, value);		
#if defined(TCSUPPORT_CMCCV2)
		if((strcmp(node,"WebCurSet_Entry") == 0) && (strcmp(attr,"LoginCondition") == 0))
		{
			if(strcmp(value,"1") == 0)
			{
				tcdbg_printf("[%s]%d:login success\n", __FUNCTION__, __LINE__);
				openlog("TCSysLog WEB", 0, LOG_LOCAL1);
				syslog(LOG_INFO, "login success\n");
				closelog();
			}
			else
			{
				tcdbg_printf("[%s]%d:LoginCondition:%s\n", __FUNCTION__, __LINE__, value);
				openlog("TCSysLog WEB", 0, LOG_LOCAL1);
				syslog(LOG_INFO, "login fail\n");
				closelog();
			}
		}
#endif
}
#endif
/*____________________________________________________________________*
**      function name: tcWebApi_staticGet
**
**      description:
*     get the attribute value of specific node.
*
**      parameters:
*     reent:not use
*     id:not use
*     params:params[0] node name.
*            params[1] attribute name.
*            params[2] show or hide
*     ret:if params[2] == hide, use ret to return value.
*         if params[2] == show, use asp_send_response() to output string on page.
*
**      global:
*     none
**      return:
*     none
**      call:
*     none
**      revision:
*     1.shnwind
* _____________________________________________________________________*/
static void
tcWebApi_staticGet (asp_reent* reent, const asp_text* params,  asp_text* ret)
{
        static char val[129];
        char *node,*attr,*show;
        char retVal[129];
        int r_val;
        
        node = (char*)asp_alloc(reent,params[0].len+1);
        attr = (char*)asp_alloc(reent,params[1].len+1);
        show = (char*)asp_alloc(reent,params[2].len+1);
        memset(node,0,params[0].len+1);
        memset(attr,0,params[1].len+1);
        memset(show,0,params[2].len+1);
        memcpy(node,params[0].str,params[0].len);
        memcpy(attr,params[1].str,params[1].len);
        memcpy(show,params[2].str,params[2].len);
        node[params[0].len]='\0';
        attr[params[1].len]='\0';
        show[params[2].len]='\0';
        r_val = tcapi_staticGet(node, attr, val);
        if(r_val < 0)
        {
                strcpy(val,"N/A");
        }
        
        //show -> s
        if(!strcmp(show,"s"))
        {
                strQuotConvertHTML(val,retVal);
                asp_send_response (NULL,retVal,strlen(retVal));
        }
        //hide -> h
        else if(!strcmp(show,"h"))
        {
                if(strlen(val))
                        ret->str = val;
                ret->len = strlen(val);
        }

}


#ifdef RA_PARENTALCONTROL
/* Add for Time Restrict by richard.lai */
/*____________________________________________________________________*
**	function name: getClientMacAddr
**
**	description:
*     Get Client Mac Address.
*
**	parameters:
*     reent:not use
*     id:not use
*     params:not use
*     ret:not use
*
**	global:
*     none
**	return:
*     none
**	call:
*     none
**	revision:
*     1.shnwind
* _____________________________________________________________________*/
#define PROC_ARP "/proc/net/arp"
static void
getClientMacAddr (char * ip_addr) {

	static char val[129];
	char retVal[129];
	FILE *fp;
	char buf[80];
	char ipaddr[16], hwtype[16], flags[16], hwaddr[20], mask[16], device[16];

	fp = fopen(PROC_ARP, "r");
	if(fp) {
		fgets(buf, 80, fp);
		while(fscanf(fp, "%s %s %s %s %s %s\n", ipaddr, hwtype, flags, hwaddr, mask, device) > 3) {
			if(strcmp(ipaddr, ip_addr) == 0) {
				strcpy(val, hwaddr);
				strQuotConvertHTML(val, retVal);
				asp_send_response (NULL, retVal, strlen(retVal));
			}
		}
		fclose(fp);
	}
}
#endif/*RA_PARENTALCONTROL*/

#if defined(TCSUPPORT_CT_PON_C9)
#define IP_NEIGH_TEMP_PATH		"/tmp/ip_neigh"
int getMACfromNeigh(char *in_ipaddr, char *out_mac)
{
	char cmdbuf[128] = {0};
	char buf[120] = {0};
	char ipaddr[64] = {0}, devs[16]={0}, devvalue[16]={0}, addype[16]={0}, hwaddr[20] = {0}, state[16] = {0};
	FILE *fp = NULL;

	if ( !in_ipaddr || !out_mac )
		return -1;

	memset(cmdbuf, 0, sizeof(cmdbuf));
	sprintf(cmdbuf, "/usr/bin/ip neigh > %s", IP_NEIGH_TEMP_PATH);
	system(cmdbuf);

	fp = fopen(IP_NEIGH_TEMP_PATH, "r");
	if ( !fp )
		return -2;

	while ( NULL != fgets(buf, sizeof(buf), fp) )
	{
		sscanf(buf, "%s %s %s %s %s %s\n", ipaddr, devs, devvalue, addype, hwaddr, state);
		if( strcmp(ipaddr, in_ipaddr) == 0)
		{
			strcpy(out_mac, hwaddr);
			if( strcmp("REACHABLE", state) == 0)
				break;
		}
	}

	fclose(fp);
	unlink(IP_NEIGH_TEMP_PATH);

	return 0;
}
#endif

