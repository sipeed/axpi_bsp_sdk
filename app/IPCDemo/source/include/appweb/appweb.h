/*
    appweb.h -- Embedthis Appweb HTTP Web Server header

    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

#ifndef _h_APPWEB
#define _h_APPWEB 1

/********************************* Includes ***********************************/

#include    "osdep.h"
#include    "mpr.h"
#include    "http.h"

#ifdef __cplusplus
extern "C" {
#endif

/********************************* Tunables ***********************************/

#define MA_UNLOAD_TIMEOUT       "5mins"     /**< Default module inactivity timeout */

/********************************** Defines ***********************************/
/*
    Pack defaults
 */
#ifndef ME_COM_CGI
    #define ME_COM_CGI 0
#endif
#ifndef ME_COM_DIR
    #define ME_COM_DIR 0
#endif
#ifndef ME_COM_EJSCRIPT
    #define ME_COM_EJSCRIPT 0
#endif
#ifndef ME_COM_ESP
    #define ME_COM_ESP 0
#endif
#ifndef ME_COM_MDB
    #define ME_COM_MDB 0
#endif
#ifndef ME_COM_PHP
    #define ME_COM_PHP 0
#endif
#ifndef ME_COM_SDB
    #define ME_COM_SDB 0
#endif
#ifndef ME_COM_SSL
    #define ME_COM_SSL 0
#endif

/******************************************************************************/
/*
    State flags
 */
#define MA_PARSE_NON_SERVER     0x1     /**< Command file being parsed by a utility program */

/**
    Current configuration parse state
    @stability Evolving
    @defgroup MaState MaState
    @see MaDirective MaState maAddDirective maArchiveLog maPopState maPushState maTokenize
    @stability Evolving
 */
typedef struct MaState {
    HttpHost    *host;                  /**< Current host */
    HttpAuth    *auth;                  /**< Quick alias for route->auth */
    HttpRoute   *route;                 /**< Current route */
    MprFile     *file;                  /**< Config file handle */
    char        *key;                   /**< Current directive being parsed */
    char        *configDir;             /**< Directory containing config file */
    char        *filename;              /**< Config file name */
    char        *endpoints;             /**< Virtual host endpoints */
    char        *data;                  /**< Config data (managed) */
    int         lineNumber;             /**< Current line number */
    int         enabled;                /**< True if the current block is enabled */
    int         flags;                  /**< Parsing flags */
    struct MaState *prev;               /**< Previous (inherited) state */
    struct MaState *top;                /**< Top level state */
    struct MaState *current;            /**< Current state */
} MaState;

/**
    Appweb configuration file directive parsing callback function
    @description Directive callbacks are invoked to parse a directive. Directive callbacks are registered using
        #maAddDirective.
    @param state Current config parse state.
    @param key Directive key name
    @param value Directive key value
    @return Zero if successful, otherwise a negative Mpr error code. See the Appweb log for diagnostics.
    @ingroup MaState
    @stability Evolving
 */
typedef int (MaDirective)(MaState *state, cchar *key, cchar *value);

/**
    Define a new appweb configuration file directive
    @description The appweb configuration file parse is extensible. New directives can be registered by this call. When
        encountered in the config file, the given callback proc will be invoked to parse.
    @param directive Directive name
    @param proc Directive callback procedure of the type #MaDirective. 
    @ingroup MaState
    @stability Evolving
 */
PUBLIC void maAddDirective(cchar *directive, MaDirective proc);

/** 
    Configure a web server
    @description This will configure a web server based on either a configuration file or using the supplied
        IP address and port. 
    @param configFile File name of the Appweb configuration file (appweb.conf) that defines the web server configuration.
    @param home Admin directory for the server. This overrides the value in the config file.
    @param documents Default directory for web documents to serve. This overrides the value in the config file.
    @param ip IP address to listen on. This overrides the value specified in the config file.
    @param port Port address to listen on. This overrides the value specified in the config file.
    @return Zero if successful, otherwise a negative Mpr error code. See the Appweb log for diagnostics.
    @ingroup MaState
    @stability Evolving
 */
PUBLIC int maConfigureServer(cchar *configFile, cchar *home, cchar *documents, cchar *ip, int port);

/**
    Get the argument in a directive
    @description Break into arguments. Args may be quoted. An outer quoting of the entire arg is removed.
    @param s String to examine
    @param tok Next token reference
    @return Reference to the next token. (Not allocate
    @ingroup MaState
    @stability Evolving
*/
PUBLIC char *maGetNextArg(char *s, char **tok);

/**
    Load an appweb module
    @description Load an appweb module. If the module is already loaded, this call will return successfully without
        reloading. Modules can be dynamically loaded or may also be pre-loaded using static linking.
    @param name User name. Must be defined in the system password file.
    @param libname Library path name
    @return Zero if successful, otherwise a negative Mpr error code. See the Appweb log for diagnostics.
    @ingroup MaState
    @stability Evolving
 */
PUBLIC int maLoadModule(cchar *name, cchar *libname);

/**
    Load default modules
    @return Zero if successful, otherwise a negative Mpr error code. See the Appweb log for diagnostics.
    @ingroup MaState
    @stability Prototype
 */
PUBLIC int maLoadModules();

/**
    Parse an Appweb configuration file
    @description Parse the configuration file and configure the server. This creates a default host and route
        and then configures the server based on config file directives.
    @param path Configuration file pathname.
    @return Zero if successful, otherwise a negative Mpr error code. See the Appweb log for diagnostics.
    @ingroup MaState
    @stability Evolving
 */
PUBLIC int maParseConfig(cchar *path);

/**
    Parse a configuration file
    @param state Current state level object
    @param path Filename to parse
    @return Zero if successful, otherwise a negative Mpr error code. See the Appweb log for diagnostics.
    @ingroup MaState
    @stability Prototype
 */
PUBLIC int maParseFile(MaState *state, cchar *path);

/**
    Pop the state 
    @description This is used when parsing config files to handle nested include files and block level directives
    @param state Current state
    @return The next lower level state object
    @ingroup MaState
    @stability Evolving
 */
PUBLIC MaState *maPopState(MaState *state);

/**
    Push the state 
    @description This is used when parsing config files to handle nested include files and block level directives
    @param state Current state
    @return The state passed as a parameter which becomes the new top level state
    @ingroup MaState
    @stability Evolving
 */
PUBLIC MaState *maPushState(MaState *state);

/**
    Tokenize a string based on route data
    @description This is a utility routine to parse a string into tokens given a format specifier. 
    Mandatory tokens can be specified with "%" format specifier. Optional tokens are specified with "?" format. 
    Values wrapped in quotes will have the outermost quotes trimmed.
    @param state Current config parsing state
    @param str String to expand
    @param fmt Format string specifier
    Supported tokens:
    <ul>
    <li>%B - Boolean. Parses: on/off, true/false, yes/no.</li>
    <li>%N - Number. Parses numbers in base 10.</li>
    <li>%S - String. Removes quotes.</li>
    <li>%P - Path string. Removes quotes and expands ${PathVars}. Resolved relative to host->dir (ServerRoot).</li>
    <li>%W - Parse words into a list</li>
    <li>%! - Optional negate. Set value to HTTP_ROUTE_NOT present, otherwise zero.</li>
    </ul>
    @return True if the string can be successfully parsed.
    @ingroup MaState
    @stability Evolving
 */
PUBLIC bool maTokenize(MaState *state, cchar *str, cchar *fmt, ...);

/** 
    Create and run a simple web server listening on a single IP address.
    @description Create a simple web server without using a configuration file. The server is created to listen on
        the specified IP address and port. This routine provides a one-line embedding of Appweb. If you want to 
        use a config file, try the #maRunWebServer instead. 
    @param ip IP address on which to listen. Set to "0.0.0.0" to listen on all interfaces.
    @param port Port number to listen to
    @param home Home directory for the web server
    @param documents Directory containing the documents to serve.
    @return Zero if successful, otherwise a negative Mpr error code. See the Appweb log for diagnostics.
    @ingroup MaState
    @stability Evolving
 */
PUBLIC int maRunSimpleWebServer(cchar *ip, int port, cchar *home, cchar *documents);

/** 
    Create and run a web server based on a configuration file
    @description Create a web server configuration based on the supplied config file. This routine provides 
        a one-line embedding of Appweb. If you don't want to use a config file, try the #maRunSimpleWebServer 
        instead. 
    @param configFile File name of the Appweb configuration file (appweb.conf) that defines the web server configuration.
    @return Zero if successful, otherwise a negative Mpr error code. See the Appweb log for diagnostics.
    @ingroup MaState
    @stability Evolving
 */
PUBLIC int maRunWebServer(cchar *configFile);

/**
    Save the authorization configuration to a file
    AuthFile schema:
        User name password abilities...
        Role name abilities...
    @param auth Auth object allocated by #httpCreateAuth.
    @param path Path name of file
    @return "Zero" if successful, otherwise a negative MPR error code
    @ingroup HttpAuth
    @stability Internal
    @internal 
 */
PUBLIC int maWriteAuthFile(HttpAuth *auth, char *path);

/*
    Internal
 */
PUBLIC int httpCgiInit(Http *http, MprModule *mp);
PUBLIC int httpEspInit(Http *http, MprModule *mp);

#ifdef __cplusplus
} /* extern C */
#endif

/*
    Permit overrides
 */
#if ME_CUSTOMIZE
 #include "customize.h"
#endif

#endif /* _h_APPWEB */

/*
    Copyright (c) Embedthis Software. All Rights Reserved.
    This software is distributed under commercial and open source licenses.
    You may use the Embedthis Open Source license or you may acquire a 
    commercial license from Embedthis Software. You agree to be fully bound
    by the terms of either license. Consult the LICENSE.md distributed with
    this software for full details and other copyrights.
 */
