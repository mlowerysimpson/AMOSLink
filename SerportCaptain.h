#pragma once
#include <afxmt.h>
#include "captain.h"
#define MAX_SERPORT_NUM 255 //the maximum numbered serial port that can be found
#define DELAYED_SEND_TIME_MS 200 //delay time (in ms) before sending out a delayed serial command

//definitions for the various possible return values from the CheckPacket function
#define PACKET_OK 0 //the packet is fine without any data problems
#define REPEATED_CHUNK 1 //the packet is fine, but corresponds to an old (repeated) chunk of data that has already been received
#define BAD_CRC 2 //one or both of the CRC bytes are incorrect
#define NOT_ENOUGH_BYTES 3 //what has been received of the packet so far is ok, but there are not enough bytes present
#define NO_SYNC_BYTES 4 //no synchronization bytes could be found in the packet data
#define BAD_CHUNK_INDEX 5 //bytes for chunk index corresponded to a chunk that was too high
#define BAD_DATASIZE 6 //bytes for data size are wrong

#ifdef _AMOS_LINK
#define AMOSLINK_API _declspec (dllexport)
extern AFX_EXTENSION_MODULE AMOSLinkDLL;
#else
#define AMOSLINK_API _declspec (dllimport)
#endif

/**
 * @brief class is used for wireless serial communications with the boat
 * 
 */
class AMOSLINK_API SerportCaptain :
	public Captain
{
public:
	SerportCaptain(int nCOMPort, CString sRootFolder);
	~SerportCaptain(void);
	static int GetAvailableSerports(int *serportNums);//find available serial ports

	//functions
	int DeleteFiles(char *szFileNames, int nFilenamesSize, int nFileType);//delete one or more files of a certain type
	int GetBytesDownloaded();//get number of bytes of remote AMOS file that have been downloaded so far
	int GetBytesToDownload();//get number of bytes of remote AMOS file that is being downloaded
	int RefreshSettings(int nSettingsType);//send instruction to AMOS to refresh a particular settings type based on the current contents of the prefs.txt file
	int ReceiveFile(char *remoteFilename, char *destPath, char* bytesDownloaded, char* bytesToDownload);//receive a remote file on AMOS
	int SendFile(char* filename, int nFileNameSize, char* destPath, int nDestPathSize); //send data from a file out the serial port
	int UseRemoteScript(char* remoteFileName, int nSize);//instruct AMOS to start using a particular remote script filename
	int GetRemoteFiles(char *remoteFiles, int nBufSize, int nFileType);//get the names of remote files on AMOS of a particular file type
	void ExitDelayedSendThread();//stop the thread for sending delayed serial port data
	bool SendSerialCommand(REMOTE_COMMAND *pRC);//sends a remote command out over the serial port connection
	void StopDownloadThread();//stop thread that is downloading data
	int ReceiveLargeDataChunk(char * rxBytes, int nNumToReceive);//tries to receive a large chunk of data over network socket connection
	void FlushRXBuffer();//flush input buffer (typically called if some unexpected data arrives and communications need to get back on track)
	bool QuitRemoteProgram();//send command to quit the remote program running on AMOS
	bool RequestVideoImage(CImage *requestedImg, CDialog *pVideoDlg, int nFeatureThreshold);//request an image capture with feature markings in it from the boat 
	bool ConnectToBoat();
	bool ForwardHo();//accelerate forward
	bool StarboardHo();//turn to right at default speed
	bool PortHo();//turn to left at default speed
	bool BackHo();//slow down boat
	bool Stop();//stop propeller and return rudder to zero position
	int RemoteScriptStepChange(int nStepChange);//send command to change the step of the currently executing remote program
	bool RequestRemoteScriptStatus(CString& sRemoteScriptStatus);//query the boat as to the name of the remote script currently running (if any) and what step the script is currently on
	bool RequestGPSPosition(CString &sGPSPosition);//query the boat for its GPS position and return the corresponding GPS string in sGPSPosition, returns true if successful
	bool RequestCompassData(CString &sCompassData);//query the boat for its compass data (heading, roll, and pitch angles, as well as temperature). Returns true if successful
	bool RequestSensorData(CString &sSensorData);//query the boat for any sensor data that it might have (eg: water temperature, pH, turbidity, etc.)
	bool ReceiveBoatData();//receive data from boat over serial port connection
	bool IsConnected();//return true if currently connected to the boat
	bool IsDownloadRunning();//return true if a download is in progress

	//data
	bool m_bDelayedSendThreadRunning;//flag is true if the thread for sending delayed data / commands is currently running
	bool m_bStopDelayedSend;//flag is true if the thread for sending delayed data / command should be stopped
	DWORD m_dwDelayedSendTimeout;//timeout value in ms that corresponds to when the delayed data / command will be sent
	REMOTE_COMMAND m_delayedRemoteCommand;//data that will be sent after a delay of DELAYED_SEND_TIME_MS ms
	CMutex m_sendMutex;//control access to threads sending and receiving serial port data
	CMutex m_delayedDataMutex;//control access to structure that holds the delayed data
	CDialog *m_videoDlg;//pointer to CDialog object used for displaying the pictures
	CImage *m_requestedImg;//pointer to CImage object that holds image data downloaded from boat over wireless serial link
	bool m_bDownloadThreadRunning;//flag is true if a thread for downloading data is currently running
	bool m_bStopDownloadThread;//flag is true if the thread for downloading data should be stopped
	int m_nNumImageBytesToReceive;//the number of image bytes to receive over the serial port connection

private:
	//data
	char * m_bytesDownloaded;//the number of bytes downloaded 
	char * m_bytesToDownload;//the total number of bytes to download
	bool m_bAskedForSupportedSensors;//flag is true if we have asked the boat for which sensors it supports
	bool m_bConnected;//true if we have connected to the boat over the wireless serial link, otherwise false
	HANDLE m_hPort;//handle to serial port used for wireless communications

	//functions
	bool DownloadBytes(int nNumBytes, int nDataType);//download a largeish number of bytes at once
	void SendConfirmationBytes(HANDLE hPort,unsigned char byte1, unsigned char byte2);//send a couple of confirmation bytes to the boat
	void SendErrorBytes(HANDLE hPort);//send error bytes to the boat
	void SendCancelBytes();//send three 0x0a bytes to indicate that downloading is stopping
	void CopyPacketToBuf(unsigned char *inBuf, int nBufferStartIndex, DWORD dwBufSize, char *destBuf);//copy the data portion of inBuf to destBuf
	int CheckPacket(unsigned char *inBuf, DWORD dwBufSize, int nChunkIndex, DWORD &dwMoreBytesRequired, int &nPacketStartIndex);//check to see if a chunk of data is valid or not
	void StartDelayedSendThread();//starts a worker thread that will send out the data / command contained in m_delayedRemoteCommand once the system timer goes beyond m_dwDelayedSendTimeout ms
	double GetRXWirelessPower();//get the wireless received power level in dBm
	void cleanup();
	bool DelayedSendSerialCommand(REMOTE_COMMAND *pRC); //sends a remote command out over the serial port connection, after DELAYED_SEND_TIME_MS milliseconds. Can be useful when issuing large numbers of navigation commands in a short period of time (i.e. only need to send the most recent navigation command).
	bool FindSupportedSensors();//query supported data (i.e. find out what types of data the boat is capable of collecting)
	void StartImageDownloadThread();//downloading image might take a while and tie up main thread, so start a worker thread to do it
};
