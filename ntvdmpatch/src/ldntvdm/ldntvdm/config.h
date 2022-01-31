#pragma once
/* Possible tagets:
TARGET_WINXP	- Windows XP / Server 2003
TARGET_WIN7		- Windows 7 / Server 2008
TARGET_WIN80	- Windows 8 / Server 2012
TARGET_WIN10    - Windows 10
TARGET_WIN11    - Windows 11

Default is TARGET_WIN10, which is set when none of these are set
*/

#if !defined(TARGET_WINXP) && !defined(TARGET_WIN7) && !defined(TARGET_WIN80) && !defined(TARGET_WIN11)
#define TARGET_WIN10	// Default
#endif

// If set, we place some hooks for PrivateExtractIconsW so that 16bit icon support gets enabled again
// which was disabled starting with Windows 7
#ifndef TARGET_WINXP
#define EXTRACTICON_HOOK
#endif

// If set, we also service 16bit Windows applications via WOW32
#define WOW16_SUPPORT

// Enable debug tracing via Debug console. Recommended, as it helps diagnosing loader issues, but makes DLL bigger
// due to debug strings
#define TRACING
//#define TRACE_FILE

// This #define was an experiment to encrypt certain code sections of the loader to evade 
// false positive detection by stupid Antivirus software on x32. 
// However it didn't help at all, so it's deactivated. 
#ifndef _WIN64
//#define CRYPT_LDR
#endif

/* Normally, the loader gets injected with Appinit_DLLs
* On systems with SecureBoot on, this is not possible :(
*/
#ifdef TARGET_WIN11
#define NO_APPINIT_DLL
#endif

// Inject as AppCert DLL. This loads the DLL on first launch of another process,
// but due to process propagation code (CREATEPROCESS_HOOK), this is better than
// nothing to ensure injection
#ifdef NO_APPINIT_DLL
#define APPCERT_DLL
#endif

#ifdef NO_APPINIT_DLL
/* If this is set, CreateProcess() APIs are hooked. This is needed because
* the AppInit-Hooks don't invade console applications.
* If not set, ldntvdm, which gets injected into conhost.exe via AppInit,
* instead tries to inject itself into the target application from conhost.exe,
* which may be a more stable solution than hooking CreateProcess
* However AppInitDlls are only available with Secure Boot turned off.
*/
#define CREATEPROCESS_HOOK
#endif

#ifdef CREATEPROCESS_HOOK
/* If this is set, bit CreateProcess() APIs are hooked, but the depper
* NtCreateUserProcess call. Thsi has the advantage that a process newly
* invaded by APPCERT_DLL method can also inject into the first process
* that gets launched from it.
*/
#ifndef TARGET_WINXP
#define CREATEPROCESS_HOOKNTCREATE
#endif
#endif

/* This is an experimental patch to enforce ConhostV1 to also load on users
* who haven't installed NTVDMx64 themselves and therefore, ForceV2 key for
* console is still TRUE in their user accounts.
* It's pretty useless, though, because AppInit_DLLs get loaded after decision
* about console version was already made, therefore we come too late.
* Would need to patch condrv.sys in order to add -ForceV1 parameter, but it's
* kernel mode and therefore not possible.
*/
#define HOOK_CONHOSTV2

/* On Windows >=7, appinfo.dll needs to get patched inside svchost.exe netsvcs */
#if !defined(TARGET_WINXP) && defined(_WIN64) 
#define NEED_APPINFO
#endif

/* In Windows XP And Windows 7, all the stuff is internal in kernel32.dll :(
* Windows XP has its own xpcreateproc.c module, therefore doesn't have to lookup
* all these symbols.
* All Windows versions >= Windows 7 however have the appinfo.dll for "Run as...",
* implementation, therefore all of these need to have a symbol cache.
* Windows 7: We cannot use the symbol loader in the module loading routine, so
* a symbol cache in the registry needs to be built that can get accessed
* during module load
* Windows >=7: The appinfo.dll is used in the service where the symbol loader also
* doesn't seem to work, so a logged on user first needs to lookup the symbols and
* add them to the cache, so the service can read the cache and hook the function
* If CreateProcess hook is needed, the hooking is preferably done with METHOD_HOOKLDR,
* which also needs a symbol lookup, so symcache is also benificial there.
*/
#if defined(TARGET_WIN7) || defined(TARGET_WINXP) || defined(NEED_APPINFO) || defined(CREATEPROCESS_HOOK)
#define USE_SYMCACHE
#endif

/* In Windows XP, all the BaseVDM functions are not linked into the kernel32.dll,
* therefore we need to reimplement them all :(
*/
#ifdef TARGET_WINXP
#define NEED_BASEVDM
#endif

