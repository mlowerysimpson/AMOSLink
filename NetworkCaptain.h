#pragma once
#include "Captain.h"
#include <afxwin.h>
#include <WinSock2.h>
#include <ws2tcpip.h>

#define PASSCODE_TEXT "AMOS2018" //password used for connecting to AMOS

#ifdef _AMOS_LINK
#define AMOSLINK_API _declspec (dllexport)
extern AFX_EXTENSION_MODULE AMOSLinkDLL;
#else
#define AMOSLINK_API _declspec (dllimport)
#endif

class AMOSLINK_API NetworkCaptain :
	public Captain
{
public:
	NetworkCaptain(CString sRootFolder);
	~NetworkCaptain(void);

	//functions
	int SendRTKData(unsigned char* rtkBuf, int nRTKBufSize); //send RTK correction data to AMOS(used for making differential GPS measurements, i.e.getting improved accuracyand precision)
	int RefreshSettings(int nSettingsType);//send instruction to AMOS to refresh a particular settings type based on the current contents of the prefs.txt file
	int ReceiveFile(char* remoteFilename, char* destPath);//receive a remote file on AMOS
	int SendFile(char* filename, int nFileNameSize, char* destPath, int nDestPathSize);//send data from a file over the network connection
	int UseRemoteScript(char* remoteFileName, int nSize);//instruct AMOS to start using a particular remote filename
	int GetRemoteFiles(char* remoteFiles, int nBufSize, int nFileType);//get the names of remote files on AMOS of a particular file type
	void FlushRXBuffer(SOCKET sock);//read receive socket until no more data can be read (typically called if some unexpected data arrives and communications need to get back on track)
	bool RequestVideoImage(CImage *requestedImg, int nFeatureThreshold);//request an image capture with feature markings in it from the boat 
	bool QuitRemoteProgram();//send command to quit the remote program running on AMOS
	bool ConnectToBoat(CString sIPAddr, int nPortNumber, int &nSockError);
	bool ForwardHo();//accelerate forward
	bool StarboardHo();//turn to right at default speed
	bool PortHo();//turn to left at default speed
	bool BackHo();//slow down boat
	bool Stop();//stop thrusters
	CString GetIPAddr();//return the IP address of the remote boat that we are connected to
	int RemoteScriptStepChange(int nStepChange);//send command to change the step of the currently executing remote program
	bool RequestRemoteScriptStatus(CString& sRemoteScriptStatus);//query the boat as to the name of the remote script currently running (if any) and what step the script is currently on
	bool RequestGPSPosition(CString &sGPSPosition);//query the boat for its GPS position and return the corresponding GPS string in sGPSPosition, returns true if successful
	bool RequestCompassData(CString &sCompassData);//query the boat for its compass data (heading, roll, and pitch angles, as well as temperature). Returns true if successful
	bool RequestSensorData(CString &sSensorData);//query the boat for any sensor data that it might have (eg: water temperature, pH, turbidity, etc.)
	bool ReceiveBoatData();//receive data from boat over network
	bool IsConnected();//return true if currently connected to the boat server

private:
	//data
	SOCKET m_connectedSock;//socket connection to the remote boat server
	bool m_bCleanedUp;
	bool m_bConnected;
	CString m_sConnectedIPAddr;

	//functions
	bool DownloadBytes(SOCKET sock, int nNumBytes, int nDataType);//download a largeish number of bytes at once
	void cleanup();
	int ReceiveLargeDataChunk(SOCKET sock, char * rxBytes, int nNumToReceive);//tries to receive a large chunk of data over network socket connection
	bool SendNetworkCommand(REMOTE_COMMAND *pRC);//sends a remote command out over the network
	bool FindSupportedSensors();//query supported data (i.e. find out what types of data the boat is capable of collecting)
};
