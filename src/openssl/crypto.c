/** 
 * XMLSec library
 *
 * See Copyright for the status of this software.
 * 
 * Author: Aleksey Sanin <aleksey@aleksey.com>
 */
#include "globals.h"

#include <string.h>

#include <openssl/evp.h>
#include <openssl/rand.h>

#include <xmlsec/xmlsec.h>
#include <xmlsec/keys.h>
#include <xmlsec/transforms.h>
#include <xmlsec/transformsInternal.h>
#include <xmlsec/errors.h>

#include <xmlsec/openssl/crypto.h>
#include <xmlsec/openssl/x509.h>

static int 		xmlSecOpenSSLErrorsInit			(void);
static int		xmlSecOpenSSLKeysInit			(void);
static int		xmlSecOpenSSLTransformsInit		(void);

/**
 * xmlSecOpenSSLInit:
 * 
 * XMLSec library specific crypto engine initialization. 
 *
 * Returns 0 on success or a negative value otherwise.
 */
int 
xmlSecOpenSSLInit (void)  {
    if(xmlSecOpenSSLErrorsInit() < 0) {
	xmlSecError(XMLSEC_ERRORS_HERE,
		    NULL,
		    "xmlSecOpenSSLErrorsInit",
		    XMLSEC_ERRORS_R_XMLSEC_FAILED,
		    XMLSEC_ERRORS_NO_MESSAGE);
	return(-1);
    }
    if(xmlSecOpenSSLKeysInit() < 0) {
	xmlSecError(XMLSEC_ERRORS_HERE,
		    NULL,
		    "xmlSecOpenSSLKeysInit",
		    XMLSEC_ERRORS_R_XMLSEC_FAILED,
		    XMLSEC_ERRORS_NO_MESSAGE);
	return(-1);
    }
    if(xmlSecOpenSSLTransformsInit() < 0) {
	xmlSecError(XMLSEC_ERRORS_HERE,
		    NULL,
		    "xmlSecOpenSSLTransformsInit",
		    XMLSEC_ERRORS_R_XMLSEC_FAILED,
		    XMLSEC_ERRORS_NO_MESSAGE);
	return(-1);
    }
    
    return(0);
}

/**
 * xmlSecOpenSSLShutdown:
 * 
 * XMLSec library specific crypto engine shutdown. 
 *
 * Returns 0 on success or a negative value otherwise.
 */
int 
xmlSecOpenSSLShutdown(void) {

    return(0);
}

int
xmlSecOpenSSLGenerateRandom(xmlSecBufferPtr buffer, size_t size) {	
    int ret;
    
    xmlSecAssert2(buffer != NULL, -1);
    xmlSecAssert2(size > 0, -1);

    ret = xmlSecBufferSetSize(buffer, size);
    if(ret < 0) {
	xmlSecError(XMLSEC_ERRORS_HERE, 
		    NULL,
		    "xmlSecBufferSetSize",
		    XMLSEC_ERRORS_R_MALLOC_FAILED,
		    "%d", size);
	return(-1);
    }
        
    /* get random data */
    ret = RAND_bytes((unsigned char*)xmlSecBufferGetData(buffer), size);
    if(ret != 1) {
	xmlSecError(XMLSEC_ERRORS_HERE, 
		    NULL,
		    "RAND_bytes",
		    XMLSEC_ERRORS_R_CRYPTO_FAILED,
		    "%d", size);
	return(-1);    
    }	
    return(0);
}

void 
xmlSecOpenSSLErrorsDefaultCallback(const char* file, int line, const char* func,
				const char* errorObject, const char* errorSubject,
				int reason, const char* msg) {

    ERR_put_error(XMLSEC_OPENSSL_ERRORS_LIB, 
		XMLSEC_OPENSSL_ERRORS_FUNCTION, 
		reason, file, line);
    xmlSecErrorsDefaultCallback(file, line, func, 
		errorObject, errorSubject, 
		reason, msg);
}

static int 
xmlSecOpenSSLErrorsInit(void) {
    static ERR_STRING_DATA xmlSecOpenSSLStrReasons[XMLSEC_ERRORS_MAX_NUMBER + 1];
    static ERR_STRING_DATA xmlSecOpenSSLStrLib[]= {
	{ ERR_PACK(XMLSEC_OPENSSL_ERRORS_LIB,0,0),	"xmlsec routines"},
	{ 0,     					NULL}
    }; 
    static ERR_STRING_DATA xmlSecOpenSSLStrDefReason[]= {
	{ XMLSEC_OPENSSL_ERRORS_LIB,			"xmlsec lib"},
        { 0,						NULL}
    };
    size_t pos;

    /* initialize reasons array */
    memset(xmlSecOpenSSLStrReasons, 0, sizeof(xmlSecOpenSSLStrReasons));
    for(pos = 0; (pos < XMLSEC_ERRORS_MAX_NUMBER) && (xmlSecErrorsGetMsg(pos) != NULL); ++pos) {
	xmlSecOpenSSLStrReasons[pos].error  = xmlSecErrorsGetCode(pos);
	xmlSecOpenSSLStrReasons[pos].string = xmlSecErrorsGetMsg(pos);
    }
    
    /* finally load xmlsec strings in OpenSSL */
    ERR_load_strings(XMLSEC_OPENSSL_ERRORS_LIB, xmlSecOpenSSLStrLib); /* define xmlsec lib name */
    ERR_load_strings(XMLSEC_OPENSSL_ERRORS_LIB, xmlSecOpenSSLStrDefReason); /* define default reason */
    ERR_load_strings(XMLSEC_OPENSSL_ERRORS_LIB, xmlSecOpenSSLStrReasons);     
    
    /* and set default errors callback for xmlsec to us */
    xmlSecErrorsSetCallback(xmlSecOpenSSLErrorsDefaultCallback);
    
    return(0);
}

static int		
xmlSecOpenSSLKeysInit(void) {
#ifndef XMLSEC_NO_AES    
#ifndef XMLSEC_OPENSSL_096
    if(xmlSecKeyDataIdsRegister(xmlSecOpenSSLKeyDataAesId) < 0) {
	xmlSecError(XMLSEC_ERRORS_HERE,
		    xmlSecKeyDataKlassGetName(xmlSecOpenSSLKeyDataAesId),
		    "xmlSecKeyDataIdsRegister",
		    XMLSEC_ERRORS_R_XMLSEC_FAILED,
		    XMLSEC_ERRORS_NO_MESSAGE);
	return(-1);
    }
#endif /* XMLSEC_OPENSSL_096 */    
#endif /* XMLSEC_NO_AES */

#ifndef XMLSEC_NO_DES    
    if(xmlSecKeyDataIdsRegister(xmlSecOpenSSLKeyDataDesId) < 0) {
	xmlSecError(XMLSEC_ERRORS_HERE,
		    xmlSecKeyDataKlassGetName(xmlSecOpenSSLKeyDataDesId),
		    "xmlSecKeyDataIdsRegister",
		    XMLSEC_ERRORS_R_XMLSEC_FAILED,
		    XMLSEC_ERRORS_NO_MESSAGE);
	return(-1);
    }
#endif /* XMLSEC_NO_DES */

#ifndef XMLSEC_NO_DSA
    if(xmlSecKeyDataIdsRegister(xmlSecOpenSSLKeyDataDsaId) < 0) {
	xmlSecError(XMLSEC_ERRORS_HERE,
		    xmlSecKeyDataKlassGetName(xmlSecOpenSSLKeyDataDsaId),
		    "xmlSecKeyDataIdsRegister",
		    XMLSEC_ERRORS_R_XMLSEC_FAILED,
		    XMLSEC_ERRORS_NO_MESSAGE);
	return(-1);
    }
#endif /* XMLSEC_NO_DSA */    

#ifndef XMLSEC_NO_HMAC  
    if(xmlSecKeyDataIdsRegister(xmlSecOpenSSLKeyDataHmacId) < 0) {
	xmlSecError(XMLSEC_ERRORS_HERE,
		    xmlSecKeyDataKlassGetName(xmlSecOpenSSLKeyDataHmacId),
		    "xmlSecKeyDataIdsRegister",
		    XMLSEC_ERRORS_R_XMLSEC_FAILED,
		    XMLSEC_ERRORS_NO_MESSAGE);
	return(-1);
    }
#endif /* XMLSEC_NO_HMAC */    

#ifndef XMLSEC_NO_RSA
    if(xmlSecKeyDataIdsRegister(xmlSecOpenSSLKeyDataRsaId) < 0) {
	xmlSecError(XMLSEC_ERRORS_HERE,
		    xmlSecKeyDataKlassGetName(xmlSecOpenSSLKeyDataRsaId),
		    "xmlSecKeyDataIdsRegister",
		    XMLSEC_ERRORS_R_XMLSEC_FAILED,
		    XMLSEC_ERRORS_NO_MESSAGE);
	return(-1);
    }
#endif /* XMLSEC_NO_RSA */

#ifndef XMLSEC_NO_X509
    if(xmlSecKeyDataIdsRegister(xmlSecOpenSSLKeyDataX509Id) < 0) {
	xmlSecError(XMLSEC_ERRORS_HERE,
		    xmlSecKeyDataKlassGetName(xmlSecOpenSSLKeyDataX509Id),
		    "xmlSecKeyDataIdsRegister",
		    XMLSEC_ERRORS_R_XMLSEC_FAILED,
		    XMLSEC_ERRORS_NO_MESSAGE);
	return(-1);
    }

    if(xmlSecKeyDataIdsRegister(xmlSecOpenSSLKeyDataRawX509CertId) < 0) {
	xmlSecError(XMLSEC_ERRORS_HERE,
		    xmlSecKeyDataKlassGetName(xmlSecOpenSSLKeyDataRawX509CertId),
		    "xmlSecKeyDataIdsRegister",
		    XMLSEC_ERRORS_R_XMLSEC_FAILED,
		    XMLSEC_ERRORS_NO_MESSAGE);
	return(-1);
    }
#endif /* XMLSEC_NO_X509 */

    return(0);
}

static int 
xmlSecOpenSSLTransformsInit(void) {
#ifndef XMLSEC_NO_SHA1    
    if(xmlSecTransformRegister(xmlSecOpenSSLTransformSha1Id) < 0) {
	xmlSecError(XMLSEC_ERRORS_HERE,
		    xmlSecTransformKlassGetName(xmlSecOpenSSLTransformSha1Id),
		    "xmlSecTransformRegister",
		    XMLSEC_ERRORS_R_XMLSEC_FAILED,
		    XMLSEC_ERRORS_NO_MESSAGE);
	return(-1);
    }
#endif /* XMLSEC_NO_SHA1 */

#ifndef XMLSEC_NO_RIPEMD160
    if(xmlSecTransformRegister(xmlSecOpenSSLTransformRipemd160Id) < 0) {
	xmlSecError(XMLSEC_ERRORS_HERE,
		    xmlSecTransformKlassGetName(xmlSecOpenSSLTransformRipemd160Id),
		    "xmlSecTransformRegister",
		    XMLSEC_ERRORS_R_XMLSEC_FAILED,
		    XMLSEC_ERRORS_NO_MESSAGE);
	return(-1);
    }
#endif /* XMLSEC_NO_RIPEMD160 */

#ifndef XMLSEC_NO_HMAC
    if(xmlSecTransformRegister(xmlSecOpenSSLTransformHmacSha1Id) < 0) {
	xmlSecError(XMLSEC_ERRORS_HERE,
		    xmlSecTransformKlassGetName(xmlSecOpenSSLTransformHmacSha1Id),
		    "xmlSecTransformRegister",
		    XMLSEC_ERRORS_R_XMLSEC_FAILED,
		    XMLSEC_ERRORS_NO_MESSAGE);
	return(-1);
    }
    if(xmlSecTransformRegister(xmlSecOpenSSLTransformHmacRipemd160Id) < 0) {
	xmlSecError(XMLSEC_ERRORS_HERE,
		    xmlSecTransformKlassGetName(xmlSecOpenSSLTransformHmacRipemd160Id),
		    "xmlSecTransformRegister",
		    XMLSEC_ERRORS_R_XMLSEC_FAILED,
		    XMLSEC_ERRORS_NO_MESSAGE);
	return(-1);
    }
    if(xmlSecTransformRegister(xmlSecOpenSSLTransformHmacMd5Id) < 0) {
	xmlSecError(XMLSEC_ERRORS_HERE,
		    xmlSecTransformKlassGetName(xmlSecOpenSSLTransformHmacMd5Id),
		    "xmlSecTransformRegister",
		    XMLSEC_ERRORS_R_XMLSEC_FAILED,
		    XMLSEC_ERRORS_NO_MESSAGE);
	return(-1);
    }
#endif /* XMLSEC_NO_HMAC */

#ifndef XMLSEC_NO_DSA
    if(xmlSecTransformRegister(xmlSecOpenSSLTransformDsaSha1Id) < 0) {
	xmlSecError(XMLSEC_ERRORS_HERE,
		    xmlSecTransformKlassGetName(xmlSecOpenSSLTransformDsaSha1Id),
		    "xmlSecTransformRegister",
		    XMLSEC_ERRORS_R_XMLSEC_FAILED,
		    XMLSEC_ERRORS_NO_MESSAGE);
	return(-1);
    }
#endif /* XMLSEC_NO_DSA */    

#ifndef XMLSEC_NO_RSA
    if(xmlSecTransformRegister(xmlSecOpenSSLTransformRsaSha1Id) < 0) {
	xmlSecError(XMLSEC_ERRORS_HERE,
		    xmlSecTransformKlassGetName(xmlSecOpenSSLTransformRsaSha1Id),
		    "xmlSecTransformRegister",
		    XMLSEC_ERRORS_R_XMLSEC_FAILED,
		    XMLSEC_ERRORS_NO_MESSAGE);
	return(-1);
    }
    if(xmlSecTransformRegister(xmlSecOpenSSLTransformRsaPkcs1Id) < 0) {
	xmlSecError(XMLSEC_ERRORS_HERE,
		    xmlSecTransformKlassGetName(xmlSecOpenSSLTransformRsaPkcs1Id),
		    "xmlSecTransformRegister",
		    XMLSEC_ERRORS_R_XMLSEC_FAILED,
		    XMLSEC_ERRORS_NO_MESSAGE);
	return(-1);
    }
    if(xmlSecTransformRegister(xmlSecOpenSSLTransformRsaOaepId) < 0) {
	xmlSecError(XMLSEC_ERRORS_HERE,
		    xmlSecTransformKlassGetName(xmlSecOpenSSLTransformRsaOaepId),
		    "xmlSecTransformRegister",
		    XMLSEC_ERRORS_R_XMLSEC_FAILED,
		    XMLSEC_ERRORS_NO_MESSAGE);
	return(-1);
    }
#endif /* XMLSEC_NO_RSA */
    
#ifndef XMLSEC_NO_DES    
    if(xmlSecTransformRegister(xmlSecOpenSSLTransformDes3CbcId) < 0) {
	xmlSecError(XMLSEC_ERRORS_HERE,
		    xmlSecTransformKlassGetName(xmlSecOpenSSLTransformDes3CbcId),
		    "xmlSecTransformRegister",
		    XMLSEC_ERRORS_R_XMLSEC_FAILED,
		    XMLSEC_ERRORS_NO_MESSAGE);
	return(-1);
    }
    if(xmlSecTransformRegister(xmlSecOpenSSLTransformKWDes3Id) < 0) {
	xmlSecError(XMLSEC_ERRORS_HERE,
		    xmlSecTransformKlassGetName(xmlSecOpenSSLTransformKWDes3Id),
		    "xmlSecTransformRegister",
		    XMLSEC_ERRORS_R_XMLSEC_FAILED,
		    XMLSEC_ERRORS_NO_MESSAGE);
	return(-1);
    }
#endif /* XMLSEC_NO_DES */

#ifndef XMLSEC_NO_AES    
#ifndef XMLSEC_OPENSSL_096
    if(xmlSecTransformRegister(xmlSecOpenSSLTransformAes128CbcId) < 0) {
	xmlSecError(XMLSEC_ERRORS_HERE,
		    xmlSecTransformKlassGetName(xmlSecOpenSSLTransformAes128CbcId),
		    "xmlSecTransformRegister",
		    XMLSEC_ERRORS_R_XMLSEC_FAILED,
		    XMLSEC_ERRORS_NO_MESSAGE);
	return(-1);
    }
    if(xmlSecTransformRegister(xmlSecOpenSSLTransformAes192CbcId) < 0) {
	xmlSecError(XMLSEC_ERRORS_HERE,
		    xmlSecTransformKlassGetName(xmlSecOpenSSLTransformAes192CbcId),
		    "xmlSecTransformRegister",
		    XMLSEC_ERRORS_R_XMLSEC_FAILED,
		    XMLSEC_ERRORS_NO_MESSAGE);
	return(-1);
    }
    if(xmlSecTransformRegister(xmlSecOpenSSLTransformAes256CbcId) < 0) {
	xmlSecError(XMLSEC_ERRORS_HERE,
		    xmlSecTransformKlassGetName(xmlSecOpenSSLTransformAes256CbcId),
		    "xmlSecTransformRegister",
		    XMLSEC_ERRORS_R_XMLSEC_FAILED,
		    XMLSEC_ERRORS_NO_MESSAGE);
	return(-1);
    }

    if(xmlSecTransformRegister(xmlSecOpenSSLTransformKWAes128Id) < 0) {
	xmlSecError(XMLSEC_ERRORS_HERE,
		    xmlSecTransformKlassGetName(xmlSecOpenSSLTransformKWAes128Id),
		    "xmlSecTransformRegister",
		    XMLSEC_ERRORS_R_XMLSEC_FAILED,
		    XMLSEC_ERRORS_NO_MESSAGE);
	return(-1);
    }
    if(xmlSecTransformRegister(xmlSecOpenSSLTransformKWAes192Id) < 0) {
	xmlSecError(XMLSEC_ERRORS_HERE,
		    xmlSecTransformKlassGetName(xmlSecOpenSSLTransformKWAes192Id),
		    "xmlSecTransformRegister",
		    XMLSEC_ERRORS_R_XMLSEC_FAILED,
		    XMLSEC_ERRORS_NO_MESSAGE);
	return(-1);
    }
    if(xmlSecTransformRegister(xmlSecOpenSSLTransformKWAes256Id) < 0) {
	xmlSecError(XMLSEC_ERRORS_HERE,
		    xmlSecTransformKlassGetName(xmlSecOpenSSLTransformKWAes256Id),
		    "xmlSecTransformRegister",
		    XMLSEC_ERRORS_R_XMLSEC_FAILED,
		    XMLSEC_ERRORS_NO_MESSAGE);
	return(-1);
    }
#endif /* XMLSEC_OPENSSL_096 */    
#endif /* XMLSEC_NO_AES */

    return(0);
}

