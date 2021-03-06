
#include <ngadmin.h>

#include <nsdp/attr.h>
#include <nsdp/protocol.h>
#include <nsdp/str.h>

#include "lib.h"
#include "network.h"


static const char* const liberror_str_tab[] = {
	[-ERR_OK] = "no error",
	[-ERR_NET] = "network error",
	[-ERR_NOTLOG] "not logged",
	[-ERR_DENIED] = "access denied",
	[-ERR_BADPASS] = "bad password",
	[-ERR_BADID] = "bad id",
	[-ERR_INVARG] = "invalid argument",
	[-ERR_TIMEOUT] = "timeout",
	[-ERR_MEM] = "out of memory",
	[-ERR_NOTIMPL] = "not implemented",
	[-ERR_BADREPLY] = "bad reply",
	[-ERR_INVOP] = "invalid operation",
	[-ERR_UNKNOWN] = "unknown error",
	NULL
};


const char* ngadmin_errorStr (int error)
{
	return getValueStr(liberror_str_tab, -ERR_OK, -ERR_UNKNOWN, -error);
}


int ngadmin_setName (struct ngadmin *nga, const char *name)
{
	List *attr;
	struct attr *at;
	struct swi_attr *sa;
	int ret = ERR_OK;
	
	
	if (nga == NULL)
		return ERR_INVARG;
	
	sa = nga->current;
	if (sa == NULL)
		return ERR_NOTLOG;
	
	
	attr = createEmptyList();
	if (name == NULL)
		at = newEmptyAttr(ATTR_NAME);
	else
		at = newAttr(ATTR_NAME, strlen(name), strdup(name));
	pushBackList(attr, at);
	ret = writeRequest(nga, attr);
	if (ret != ERR_OK)
		goto end;
	 
	/* successful, also update local name */
	if (name == NULL)
		memset(sa->name, '\0', NAME_SIZE);
	else
		strncpy(sa->name, name, NAME_SIZE);
	
end:
	return ret;
}


int ngadmin_changePassword (struct ngadmin *nga, const char* pass)
{
	List *attr;
	struct attr *at;
	int ret = ERR_OK;
	
	
	if (nga == NULL || pass == NULL)
		return ERR_INVARG;
	else if (nga->current == NULL)
		return ERR_NOTLOG;
	
	
	attr = createEmptyList();
	at = newAttr(ATTR_NEW_PASSWORD, strlen(pass), strdup(pass));
	if (nga->encrypt_pass)
		passwordEndecode(at->data, at->size);
	pushBackList(attr, at);
	ret = writeRequest(nga, attr);
	if (ret != ERR_OK)
		goto end;
	
	/* successful, also update local password */
	strncpy(nga->password, pass, PASSWORD_MAX);
	
end:
	
	return ret;
}


int ngadmin_restart (struct ngadmin *nga)
{
	List *attr;
	
	
	attr = createEmptyList();
	pushBackList(attr, newByteAttr(ATTR_RESTART, 1));
	
	
	return writeRequest(nga, attr);
}


int ngadmin_defaults (struct ngadmin *nga)
{
	List *attr;
	int ret = ERR_OK;
	
	
	attr = createEmptyList();
	pushBackList(attr, newByteAttr(ATTR_DEFAULTS, 1));
	ret = writeRequest(nga, attr);
	if (ret != ERR_OK)
		goto end;
	
	
	/* successful: delog and clean list */
	free(nga->swi_tab);
	nga->swi_tab = NULL;
	nga->swi_count = 0;
	nga->current = NULL;
	
end:
	return ret;
}


