// dllmain.cpp : Defines the initialization routines for the DLL.
//

#include "pch.h"
#include "framework.h"
#include "SerportCaptain.h"
#include "NetworkCaptain.h"
#include <afxwin.h>
#include <afxdllx.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

extern AFX_EXTENSION_MODULE AMOSLinkDLL = { NULL,NULL };

extern SerportCaptain *g_sc;
extern NetworkCaptain *g_nc;

extern "C" int APIENTRY
DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved)
{
	// Remove this if you use lpReserved
	UNREFERENCED_PARAMETER(lpReserved);

	if (dwReason == DLL_PROCESS_ATTACH)
	{
		TRACE0("AMOSLink.DLL Initializing!\n");

		// Extension DLL one-time initialization
		if (!AfxInitExtensionModule(AMOSLinkDLL, hInstance))
			return 0;

		// Insert this DLL into the resource chain
		// NOTE: If this Extension DLL is being implicitly linked to by
		//  an MFC Regular DLL (such as an ActiveX Control)
		//  instead of an MFC application, then you will want to
		//  remove this line from DllMain and put it in a separate
		//  function exported from this Extension DLL.  The Regular DLL
		//  that uses this Extension DLL should then explicitly call that
		//  function to initialize this Extension DLL.  Otherwise,
		//  the CDynLinkLibrary object will not be attached to the
		//  Regular DLL's resource chain, and serious problems will
		//  result.

		new CDynLinkLibrary(AMOSLinkDLL);

		// Sockets initialization
		// NOTE: If this Extension DLL is being implicitly linked to by
		//  an MFC Regular DLL (such as an ActiveX Control)
		//  instead of an MFC application, then you will want to
		//  remove the following lines from DllMain and put them in a separate
		//  function exported from this Extension DLL.  The Regular DLL
		//  that uses this Extension DLL should then explicitly call that
		//  function to initialize this Extension DLL.
		if (!AfxSocketInit())
		{
			return FALSE;
		}

	}
	else if (dwReason == DLL_PROCESS_DETACH)
	{
		TRACE0("AMOSLink.DLL Terminating!\n");
		if (g_nc!=NULL) {
			delete g_nc;
			g_nc = NULL;
		}
		if (g_sc!=NULL) {
			delete g_sc;
			g_sc = NULL;
		}
		// Terminate the library before destructors are called
		AfxTermExtensionModule(AMOSLinkDLL);
	}
	return 1;   // ok
}
