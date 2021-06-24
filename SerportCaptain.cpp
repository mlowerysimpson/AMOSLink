#include "pch.h"
#include "SerportCaptain.h"
#include "VideoLinkDlg.h"
#include <mmsystem.h>


SerportCaptain* g_sc = NULL;//global SerportCaptain object that gets created by calling program

extern "C" {
	__declspec (dllexport) int Serport_DeleteFiles(char* szFileNames, int nFilenamesSize, int nFileType) {
		if (!g_sc) {//must not be connected yet
			return ERROR_NOTCONNECTED;
		}
		return g_sc->DeleteFiles(szFileNames, nFilenamesSize, nFileType);
	}

	__declspec (dllexport) int TestSerportConnection(int nCOMPort, LPCTSTR sRootFolder) {
		//check to see if AMOS is available and can communicate over wireless serial link
		CString rootFolder = CString(sRootFolder);
		if (g_sc) {
			delete g_sc;
			g_sc = NULL;
		}
		g_sc = new SerportCaptain(nCOMPort, rootFolder.GetBuffer(rootFolder.GetLength()));
		if (g_sc->ConnectToBoat()) {
			return CONNECT_OK;
		}
		else {
			delete g_sc;
			g_sc = NULL;
		}
		return ERROR_CANNOT_CONNECT;
	}

	__declspec (dllexport) int Serport_StopRemoteProgram() {
		//send signal to stop remote program running on AMOS
		if (!g_sc) {//must not be connected yet
			return ERROR_NOTCONNECTED;
		}
		if (g_sc->QuitRemoteProgram()) {
			//need to delete the g_sc object since its connection is no longer valid
			delete g_sc;
			g_sc = NULL;
			return COMMAND_OK;
		}
		return ERROR_COMMAND_FAILED;
	}

	__declspec (dllexport) int Serport_GetGPSData(char* gpsBuf, int nSize) {
		//request GPS data from AMOS
		CString sGPSData = "";
		if (g_sc) {
			g_sc->RequestGPSPosition(sGPSData);
			if (sGPSData.GetLength() < nSize) {
				strcpy(gpsBuf, sGPSData.GetBuffer(sGPSData.GetLength()));
			}
			return 0;
		}
		return -1;
	}

	__declspec (dllexport) int Serport_GetCompassData(char* compassBuf, int nSize) {
		//request compass / inertial data from AMOS
		if (!g_sc) {//must not be connected yet
			return -1;
		}
		CString sCompassData = "";
		bool bGotCompassData = g_sc->RequestCompassData(sCompassData);
		if (!bGotCompassData) {
			return -2;
		}
		if (sCompassData.GetLength() < nSize) {
			strcpy(compassBuf, sCompassData.GetBuffer(sCompassData.GetLength()));
		}
		return 0;
	}

	__declspec (dllexport) int Serport_GetSensorData(char* sensorBuf, int nSize) {
		//request sensor and diagnostic data from AMOS
		if (!g_sc) {//must not be connected yet
			return -1;
		}
		CString sSensorData = "";
		bool bGotSensorData = g_sc->RequestSensorData(sSensorData);
		if (!bGotSensorData) {
			return -2;
		}
		if (sSensorData.GetLength() < nSize) {
			strcpy(sensorBuf, sSensorData.GetBuffer(sSensorData.GetLength()));
		}
		return 0;
	}

	__declspec (dllexport) int Serport_SendThrusterCommand(int nCommand) {
		//send command to move in a particular direction
		if (!g_sc) {//must not be connected yet
			return -1;
		}
		if (nCommand == THRUST_FORWARDS) {
			if (g_sc->ForwardHo()) {
				return 0;
			}
		}
		else if (nCommand == THRUST_LEFT) {
			if (g_sc->PortHo()) {
				return 0;
			}
		}
		else if (nCommand == THRUST_RIGHT) {
			if (g_sc->StarboardHo()) {
				return 0;
			}
		}
		else if (nCommand == THRUST_BACK) {
			if (g_sc->BackHo()) {
				return 0;
			}
		}
		else if (nCommand == THRUST_STOP) {
			if (g_sc->Stop()) {
				return 0;
			}
		}
		return -2;//unknown command or command failed
	}

	__declspec (dllexport) float Serport_GetRudderAngle() {
		//send command to get the current rudder angle
		if (!g_sc) {//must not be connected yet
			return 0;
		}
		PROPELLER_STATE* pState = g_sc->GetCurrentPropState();
		float fAngle = 0;
		if (pState) {
			fAngle = pState->fRudderAngle;
		}
		return fAngle;
	}

	__declspec (dllexport) float Serport_GetThrusterPower() {
		//send command to get the current thruster power
		if (!g_sc) {//must not be connected yet
			return 0;
		}
		PROPELLER_STATE* pState = g_sc->GetCurrentPropState();
		float fPower = 0;
		if (pState) {
			fPower = pState->fPropSpeed;
		}
		return fPower;
	}

	__declspec (dllexport) int Serport_GetRemoteScriptStatus(char* remoteScriptStatus, int nSize) {
		//request remote script status from AMOS
		CString sRemoteScriptStatus = "";
		if (g_sc) {
			g_sc->RequestRemoteScriptStatus(sRemoteScriptStatus);
			if (sRemoteScriptStatus.GetLength() < nSize) {
				strcpy(remoteScriptStatus, sRemoteScriptStatus.GetBuffer(sRemoteScriptStatus.GetLength()));
			}
			return 0;
		}
		return -1;
	}

	__declspec (dllexport) int Serport_RemoteScriptStepChange(int nStepChange) {
		//change step of currently running file script
		if (g_sc) {
			return g_sc->RemoteScriptStepChange(nStepChange);
		}
		return -1;
	}

	__declspec (dllexport) int Serport_GetRemoteFiles(char* remoteFiles, int nSize, int nFileType) {
		//get carriage return separated list of files of a certain type found on AMOS
		if (g_sc) {
			return g_sc->GetRemoteFiles(remoteFiles, nSize, nFileType);
		}
		return -1;
	}

	__declspec (dllexport) int Serport_UseRemoteScript(char* remoteFileName, int nSize) {
		//instruct AMOS to start using the specified remote filename script
		if (g_sc) {
			return g_sc->UseRemoteScript(remoteFileName, nSize);
		}
		return -1;
	}

	__declspec (dllexport) int Serport_SendFile(char* filename, int nFileNameSize, char* destPath, int nDestPathSize) {
		if (g_sc) {
			return g_sc->SendFile(filename, nFileNameSize, destPath, nDestPathSize);
		}
		return -1;
	}

	__declspec (dllexport) int Serport_GetFile(char* remoteFilename, char* destPath, char *bytesDownloaded, char *fileSize) {
		if (g_sc) {
			return g_sc->ReceiveFile(remoteFilename, destPath, bytesDownloaded, fileSize);
		}
		return -1;
	}

	__declspec (dllexport) int Serport_RefreshSettings(int nSettingsType) {
		if (g_sc) {
			return g_sc->RefreshSettings(nSettingsType);
		}
		return -1;
	}

	__declspec (dllexport) int Serport_GetBytesToDownload() {
		if (g_sc) {
			return g_sc->GetBytesToDownload();
		}
		return 0;
	}

	__declspec (dllexport) int Serport_GetBytesDownloaded() {
		if (g_sc) {
			return g_sc->GetBytesDownloaded();
		}
		return 0;
	}
}

UINT DelayedSendThread(LPVOID pParam) {//thread for sending out delayed data / commands over wireless serial link
	SerportCaptain *pCap = (SerportCaptain *)pParam;
	pCap->m_bDelayedSendThreadRunning = true;
	pCap->m_bStopDelayedSend = false;
	bool bSentData = false;
	REMOTE_COMMAND *pTempCommand = new REMOTE_COMMAND;
	memset(pTempCommand,0,sizeof(REMOTE_COMMAND));
	TRACE("Started delayed send thread.\n");
	while (!bSentData&&!pCap->m_bStopDelayedSend) {
		if (timeGetTime()>=pCap->m_dwDelayedSendTimeout) {
			//send out data
			CSingleLock sL2(&pCap->m_delayedDataMutex);
			sL2.Lock(1000);
			if (sL2.IsLocked()) {
				Captain::CopyCommand(pTempCommand,&pCap->m_delayedRemoteCommand);
			}
			else {
				delete pTempCommand;
				TRACE("Could not lock delayed data mutex.\n");
				return 1;
			}
			sL2.Unlock();
			CSingleLock sL(&pCap->m_sendMutex);
			sL.Lock(1000);
			if (sL.IsLocked()) {
				bSentData = pCap->SendSerialCommand(pTempCommand);
				sL.Unlock();
			}
		}
		Sleep(0);
	}
	if (pTempCommand) {
		if (pTempCommand->pDataBytes) {
			delete pTempCommand->pDataBytes;
			pTempCommand->pDataBytes = NULL;
		}
		delete pTempCommand;
	}
	pCap->m_bDelayedSendThreadRunning = false;	
	TRACE("Exit delayed send thread.\n");
	return 0;
}

UINT DownloadImageThread(LPVOID pParam) {//thread for downloading image data over the wireless serial link
	SerportCaptain *pCap = (SerportCaptain *)pParam;
	pCap->m_bDownloadThreadRunning = true;
	pCap->m_bStopDownloadThread = false;
	unsigned char *videoBytes = new unsigned char[pCap->m_nNumImageBytesToReceive];
	int nNumReceived = pCap->ReceiveLargeDataChunk((char *)videoBytes,pCap->m_nNumImageBytesToReceive);
	TRACE("Starting download image thread.\n");
	if (nNumReceived==pCap->m_nNumImageBytesToReceive) {//image bytes received successfully, save to temporary image file
		CString sTmpImgFilename="";
		CString sTmp = TMP_IMAGE_FILENAME;
		sTmpImgFilename.Format("%s\\%s",pCap->m_sRootFolder,sTmp);
		CFile imgFile;
		if (imgFile.Open(sTmpImgFilename,CFile::modeCreate|CFile::modeReadWrite|CFile::shareDenyNone)) {
			imgFile.Write(videoBytes,(UINT)nNumReceived);
			imgFile.Close();
		}
		pCap->CopyReceivedImage(pCap->m_requestedImg);
		if (pCap->m_videoDlg&&pCap->m_videoDlg->GetSafeHwnd()) {
			((CVideoLinkDlg *)pCap->m_videoDlg)->SetImage(pCap->m_requestedImg);
			((CVideoLinkDlg *)pCap->m_videoDlg)->PostMessage(WM_UPDATED_PICTURE,0,0);
		}
	}
	delete []videoBytes;
	pCap->m_bDownloadThreadRunning = false;
	TRACE("Ending DownloadImageThread.\n");
	return 0;
}


/**
 * @brief Construct a new SerportCaptain object
 * 
 * @param nCOMPort the number of the serial COM port used for communications over the wireless serial link
 * @param szRootFolder the name of the program folder where the BoatCaptain software is installed and runs from
 */
SerportCaptain::SerportCaptain(int nCOMPort, CString sRootFolder) : Captain
(sRootFolder)
{
	m_bytesDownloaded = NULL;
	m_bytesToDownload = NULL;
	m_bAskedForSupportedSensors = false;
	char sComport[11];
	sprintf_s(sComport, "\\\\.\\com%d", nCOMPort);
	HANDLE hPort = CreateFile(sComport, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
	if (hPort != INVALID_HANDLE_VALUE)
	{ //check to see if port has a valid handle, i.e. if the port exists and is available
		m_hPort = hPort;
		//setup for baud rate, timeouts, etc.
		//timeouts
		COMMTIMEOUTS cto;
		cto.ReadIntervalTimeout = 3000;
		cto.ReadTotalTimeoutMultiplier = 1;
		cto.ReadTotalTimeoutConstant = 3000;
		cto.WriteTotalTimeoutConstant = 500;
		cto.WriteTotalTimeoutMultiplier = 1;
		SetCommTimeouts(m_hPort, &cto);
		//baud rate and configuration
		DCB dcb;
		GetCommState(m_hPort,&dcb);
		dcb.BaudRate = 38400;
		dcb.ByteSize = 8;
		dcb.DCBlength = sizeof(DCB);
		dcb.Parity = NOPARITY;
		dcb.StopBits = ONESTOPBIT;
		SetCommState(m_hPort,&dcb);

		//test
		//get comm properties
		COMMPROP comProp;
		if (GetCommProperties(m_hPort,&comProp)) {
			int a=0;
		}
		//end test

		//make sure RX and TX buffers are cleared 
		PurgeComm(m_hPort, PURGE_RXCLEAR | PURGE_TXCLEAR);
	}
	else m_hPort = NULL;
	memset(&m_delayedRemoteCommand,0,sizeof(REMOTE_COMMAND));
	m_bConnected=false;
	m_requestedImg = NULL;
	m_videoDlg = NULL;
	m_nNumImageBytesToReceive=0;
	m_bDownloadThreadRunning = false;
	m_bStopDownloadThread = false;
	m_dwDelayedSendTimeout = 0;
	m_bDelayedSendThreadRunning = false;
	m_bStopDelayedSend = false;
}

/**
 * @brief Destroy the SerportCaptain object
 * 
 */
SerportCaptain::~SerportCaptain(void)
{
	ExitDelayedSendThread();
	if (m_delayedRemoteCommand.pDataBytes) {
		delete m_delayedRemoteCommand.pDataBytes;
		m_delayedRemoteCommand.pDataBytes = NULL;
	}
	if (m_hPort != NULL) {
		CloseHandle(m_hPort);
		m_hPort = NULL;
	}
}

/**
 * @brief Find available serial ports on the PC that can be opened and used for serial communications.
 * 
 * @param serportNums an array of integers (should be of length MAX_SERPORT_NUM) that holds the returned serial port numbers.
 * @return int the number of serial ports found that could be successfully opened and used.
 */
int SerportCaptain::GetAvailableSerports(int *serportNums) {//find available serial ports
	int nNumPortsFound = 0;
	for (int i=1;i<=MAX_SERPORT_NUM;i++) {
		char sComport[11];
		sprintf_s(sComport,"\\\\.\\com%d",i);
		HANDLE hPort = CreateFile(sComport,GENERIC_READ|GENERIC_WRITE,0,NULL,OPEN_EXISTING,0,NULL);
		if (hPort != INVALID_HANDLE_VALUE) {//check to see if port has a valid handle, i.e. if the port exists and is available
			serportNums[nNumPortsFound]=i;
			nNumPortsFound++;
			CloseHandle(hPort);
		}	
	}
	return nNumPortsFound;
}

/**
 * @brief this function does not really "connect" with the boat in the same way that it does in the related NetworkCaptain class, but it does find out what sensor types the boat is able to support.
 * 
 * @return true if there is a valid serial port and the supported sensor types could be found
 * @return false if either there is no valid serial port or the supported sensor types could not be found
 */
bool SerportCaptain::ConnectToBoat() {
	if (!m_hPort) {
		return false;//no valid serial port
	}
	//query supported data (i.e. find out what types of data the boat is capable of collecting)
	if (!FindSupportedSensors()) {
		return false;//unable to find supported sensors
	}
	m_bConnected=true;
	return true;
}


/**
 * @brief accelerate boat forward
 * 
 * @return true if command to accelerate forward was sent successfully
 * @return false if there was a problem sending the command to accelerate forward
 */
bool SerportCaptain::ForwardHo() {//accelerate forward
	PROPELLER_STATE propState;
	if (!Captain::ForwardHo(&propState)) {
		return false;
	}
	REMOTE_COMMAND rc;
	rc.nCommand = THRUST_ON;
	rc.nNumDataBytes = sizeof(PROPELLER_STATE);
	rc.pDataBytes = (unsigned char *)&propState;
	return DelayedSendSerialCommand(&rc);
}

/**
 * @brief turn boat to right
 * 
 * @return true if the command to turn the boat to the right was sent successfully
 * @return false if there was a problem sending the command to turn to the right
 */
bool SerportCaptain::StarboardHo() {//turn to right
	PROPELLER_STATE propState;
	if (!Captain::StarboardHo(&propState)) {
		return false;
	}
	REMOTE_COMMAND rc;
	rc.nCommand = THRUST_ON;
	rc.nNumDataBytes = sizeof(PROPELLER_STATE);
	rc.pDataBytes = (unsigned char *)&propState;
	return DelayedSendSerialCommand(&rc);
}

/**
 * @brief turn boat to left
 * 
 * @return true if the command to turn the boat to the left was sent successfully
 * @return false if there was a problem sending the command to turn to the left
 */
bool SerportCaptain::PortHo() {//turn to left
	PROPELLER_STATE propState;
	if (!Captain::PortHo(&propState)) {
		return false;
	}
	REMOTE_COMMAND rc;
	rc.nCommand = THRUST_ON;
	rc.nNumDataBytes = sizeof(PROPELLER_STATE);
	rc.pDataBytes = (unsigned char *)&propState;
	return DelayedSendSerialCommand(&rc);
}

/**
 * @brief slow down boat
 * 
 * @return true if the command to slow down the boat was sent successfully
 * @return false if there was a problem sending the command to slow down the boat
 */
bool SerportCaptain::BackHo() {//slow down boat
	PROPELLER_STATE propState;
	if (!Captain::BackHo(&propState)) {
		return false;
	}
	REMOTE_COMMAND rc;
	rc.nCommand = THRUST_ON;
	rc.nNumDataBytes = sizeof(PROPELLER_STATE);
	rc.pDataBytes = (unsigned char *)&propState;
	return DelayedSendSerialCommand(&rc);
}

/**
 * @brief sends a remote command out over the serial port connection
 * 
 * @param pRC REMOTE_COMMAND structure describing the command to be sent to the boat
 * @return true if the command was sent successfully
 * @return false if there was a problem sending the command
 */
bool SerportCaptain::SendSerialCommand(REMOTE_COMMAND *pRC) {
	if (!m_hPort) {
		return false;//not connected to a serial port
	}
	//first purge serial buffer
	PurgeComm(m_hPort, PURGE_RXCLEAR|PURGE_TXCLEAR);
	//then send command bytes
	DWORD dwNumWritten = 0;
	unsigned char commandBytes[4];
	commandBytes[0] = (unsigned char)((pRC->nCommand&0xff000000)>>24);
	commandBytes[1] = (unsigned char)((pRC->nCommand&0x00ff0000)>>16);
	commandBytes[2] = (unsigned char)((pRC->nCommand&0x0000ff00)>>8);
	commandBytes[3] = (unsigned char)(pRC->nCommand&0x000000ff);
	if (!WriteFile(m_hPort, commandBytes, 4, &dwNumWritten, NULL)||dwNumWritten!=4) {
		//DisplayLastError("Error sending conmmand bytes.");
		TRACE("Error sending command bytes.\n");
		return false;
	}
	if (pRC->pDataBytes) {//need to send data along with command
		unsigned char dataSizeBytes[4];
		dataSizeBytes[0] = (unsigned char)((pRC->nNumDataBytes&0xff000000)>>24);
		dataSizeBytes[1] = (unsigned char)((pRC->nNumDataBytes&0x00ff0000)>>16);
		dataSizeBytes[2] = (unsigned char)((pRC->nNumDataBytes&0x0000ff00)>>8);
		dataSizeBytes[3] = (unsigned char)(pRC->nNumDataBytes&0x000000ff);
		if (!WriteFile(m_hPort, dataSizeBytes, 4, &dwNumWritten, NULL)||dwNumWritten!=4) {
			//DisplayLastError("Error sending data size bytes.");
			TRACE("Error sending data size bytes.");
			return false;
		}
		//send actual data
		//nNumSent = send(m_connectedSock,(char *)pRC->pDataBytes,pRC->nNumDataBytes,0);
		if (!WriteFile(m_hPort, pRC->pDataBytes, pRC->nNumDataBytes, &dwNumWritten, NULL)||dwNumWritten!=pRC->nNumDataBytes) {
			//DisplayLastError("Error sending data bytes.");
			TRACE("Error sending data bytes.");
			return false;
		}
	}
	//get confirmation from remote boat
	unsigned char inBuf[4];
	DWORD dwNumRead = 0;
	//first read until we get a starting zero byte (sometimes get stray bytes from wireless module)
	while (ReadFile(m_hPort,inBuf,1,&dwNumRead,0)&&dwNumRead==1&&inBuf[0]!=0) {
		TRACE("extra byte = %02x\n",inBuf[0]);
	}
	DWORD dwNumToRead = 3;
	if (dwNumRead==0||inBuf[0]!=0) {
		dwNumToRead=4;
	}
	if (!ReadFile(m_hPort,&inBuf[4-dwNumToRead],dwNumToRead,&dwNumRead,0)||dwNumRead!=dwNumToRead) {
		DWORD dwError = GetLastError();
		CString sError = "";
		sError.Format("Error %u trying to get confirmation from boat, %u of %u bytes read, command = %d\n.",dwError,dwNumRead,dwNumToRead,pRC->nCommand);
		TRACE(sError+"\n");
		//AfxMessageBox(sError,MB_ICONERROR);
		PurgeComm(m_hPort, PURGE_RXCLEAR);
		return false;
	}
	int nResponse = (inBuf[0]<<24) + (inBuf[1]<<16) + (inBuf[2]<<8) + inBuf[3];
	if (nResponse!=pRC->nCommand) {
		//AfxMessageBox("Error, invalid confirmation response from boat.");
		//error occurred, flush rx buffer
		TRACE("Error, invalid confirmation response from boat, nResponse = %d, expected = %d.\n",nResponse,pRC->nCommand);
		TRACE("inBuf[0] = %d, inBuf[1] = %d, inBuf[2] = %d, inBuf[3] = %d\n",(int)inBuf[0],(int)inBuf[1],(int)inBuf[2],(int)inBuf[3]);
		PurgeComm(m_hPort, PURGE_RXCLEAR);
		return false;
	}
	return true;//command was sent successfully
}

/**
 * @brief query the boat for its GPS position and return the corresponding GPS string in sGPSPosition, returns true if successful
 * 
 * @param sGPSPosition the GPS position text returned by the function
 * @return true if the function was able to acquire the GPS position info successfully from the boat
 * @return false if there was a problem acquiring GPS data from the boat
 */
bool SerportCaptain::RequestGPSPosition(CString &sGPSPosition) {
	sGPSPosition="";
	REMOTE_COMMAND rc;
	rc.nCommand = GPS_DATA_PACKET;
	rc.nNumDataBytes = 0;
	rc.pDataBytes = NULL;
	CSingleLock sL(&this->m_sendMutex);
	if (!sL.Lock(1000)) {
		return false;
	}
	if (SendSerialCommand(&rc)) {
		//test	
		TRACE("try to receive for gps...\n");
		//end test
		if (!ReceiveBoatData()) {
			sL.Unlock();
			return false;
		}
	}
	else {
		sL.Unlock();
		return false;
	}
	sL.Unlock();
	if (!m_gpsTime) {//no GPS readings obtained yet
		return true;
	}
	sGPSPosition = FormatGPSData();
	return true;
}

/**
 * @brief query the boat as to the name of the remote script currently running (if any) and what step the script is currently on
 *
 * @param sRemoteScriptStatus the status of the script that is currently running on the boat, ex: "MyAMOSScript.txt, 3 of 20". It indicates the name of the script currently running (if any), the current step number, and the total number of steps
 * @return true if the function was able to acquire the remote script status info successfully from the boat
 * @return false if there was a problem acquiring the remote script status
 */
bool SerportCaptain::RequestRemoteScriptStatus(CString &sRemoteScriptStatus) {//query the boat as to the name of the remote script currently running (if any) and what step the script is currently on
	sRemoteScriptStatus = "";
	REMOTE_COMMAND rc;
	const int MAX_TRIES = 3;
	rc.nCommand = SCRIPT_STATUS_PACKET;
	rc.nNumDataBytes = 0;
	rc.pDataBytes = NULL;
	CSingleLock sL(&this->m_sendMutex);
	if (!sL.Lock(1000)) {
		return false;
	}
	int i = 0;
	while (i < MAX_TRIES) {
		i++;
		if (SendSerialCommand(&rc)) {
			//test	
			TRACE("try to receive remote script status...\n");
			//end test
			if (!ReceiveBoatData()) {
				if (i < MAX_TRIES) {
					Sleep(1000);
					continue;
				}
				sL.Unlock();
				return false;
			}
		}
		else {
			if (i < MAX_TRIES) {
				Sleep(1000);
				continue;
			}
			sL.Unlock();
			return false;
		}
		break;
	}
	sL.Unlock();
	sRemoteScriptStatus = FormatRemoteScriptStatus();
	return true;
}


/**
 * @brief query the boat for its compass data (heading, roll, and pitch angles, as well as temperature). Returns true if successful
 * 
 * @param sCompassData the compass data returned by the function
 * @return true if the function was able to acquire the compass data successfully from the boat
 * @return false if there was a problem acquiring compass data from the boat
 */
bool SerportCaptain::RequestCompassData(CString &sCompassData) {
	sCompassData="";
	REMOTE_COMMAND rc;
	rc.nCommand = COMPASS_DATA_PACKET;
	rc.nNumDataBytes = 0;
	rc.pDataBytes = NULL;
	CSingleLock sL(&this->m_sendMutex);
	if (!sL.Lock(1000)) {
		return false;
	}
	if (SendSerialCommand(&rc)) {
		//test
		TRACE("try to receive compass...\n");
		//end test
		if (!ReceiveBoatData()) {
			sL.Unlock();
			return false;
		}
	}
	else {
		sL.Unlock();
		return false;
	}
	sL.Unlock();
	sCompassData = FormatCompassData();
	return true;
}


/**
 * @brief receive data from the boat over the serial port connection
 * 
 * @return true if boat data could be successfully received and processed
 * @return false if there was a problem getting data from the boat
 */
bool SerportCaptain::ReceiveBoatData() {//receive data from boat over network
	if (!m_hPort) {
		return false;//no valid serial port connection
	}
	int nNumBytesToReceive=2*sizeof(int);
	//first receive the data type from the boat and the number of data bytes
	int nDataType=0;
	int nDataSize=0;
	unsigned char *inBuf = new unsigned char[nNumBytesToReceive];
	DWORD dwNumReceived = 0;
	DWORD dwT1 = timeGetTime();
	BOOL bReadOK = ReadFile(m_hPort,inBuf,(DWORD)nNumBytesToReceive,&dwNumReceived,0);
	if (!bReadOK) {
		DWORD dwError = GetLastError();
		TRACE("Error %u trying to do serial port read.\n",dwError);
		delete []inBuf;
		PurgeComm(m_hPort, PURGE_RXCLEAR);
		return false;
	}
	else if (dwNumReceived!=nNumBytesToReceive) {
		delete []inBuf;
		//timed out trying to receive bytes
		PurgeComm(m_hPort, PURGE_RXCLEAR);
		TRACE("Read timeout, %u of %d bytes received.\n",dwNumReceived,nNumBytesToReceive);
		return false;//timeout or error trying to receive data
	}
	DWORD dwT2 = timeGetTime();
	TRACE("Test time = %d\n",(int)(dwT2 - dwT1));
	int nTest = sizeof(int);
	memcpy(&nDataType,inBuf,sizeof(int));
	memcpy(&nDataSize,&inBuf[sizeof(int)],sizeof(int));

	//create structure for receiving boat data
	BOAT_DATA *pBoatData = BoatCommand::CreateBoatData(nDataType);
	if (!pBoatData) {
		TRACE("Unable to create boat data.\n");//probably received some out of sync data
		Sleep(1000);
		//PurgeComm(m_hPort, PURGE_RXCLEAR);
		//test
		int nTestCount = 0;
		for (int i=0;i<8;i++) {
			TRACE("inBuf[%d] = %d\n",i,(int)inBuf[i]);
		}
		bReadOK = ReadFile(m_hPort,inBuf,1,&dwNumReceived,0);
		while (bReadOK&&dwNumReceived>0) {
			TRACE("test_byte = %d\n",(int)inBuf[0]);
			bReadOK = ReadFile(m_hPort,inBuf,1,&dwNumReceived,0);
		}
		//end test
		delete []inBuf;
		return false;//unable to create this data type
	}
	delete []inBuf;
	//special case for sensor types
	if (nDataType==SENSOR_TYPES_INFO) {
		pBoatData->nDataSize=m_nNumSensorsAvailable*sizeof(int);
		pBoatData->dataBytes = new unsigned char[pBoatData->nDataSize];
	}
	if (pBoatData->nDataSize!=nDataSize) {
		TRACE("Expected data size of %d, but got %d. nDataType = %d\n",nDataSize,pBoatData->nDataSize,nDataType);
		PurgeComm(m_hPort, PURGE_RXCLEAR);
		return false;//unexpected data size
	}
	nNumBytesToReceive = pBoatData->nDataSize+1;
	inBuf = new unsigned char[nNumBytesToReceive];
	//nNumReceived = recv(m_connectedSock,inBuf,nNumBytesToReceive,0);
	if (!ReadFile(m_hPort, inBuf, (DWORD)nNumBytesToReceive, &dwNumReceived,0)) {
		//timeout or other error trying to receive data
		BoatCommand::DeleteBoatData(pBoatData);
		PurgeComm(m_hPort, PURGE_RXCLEAR);
		delete []inBuf;
		TRACE("Timeout or other error trying to receive data.\n");
		return false;
	}
	memcpy(pBoatData->dataBytes,inBuf,pBoatData->nDataSize);
	pBoatData->checkSum = (unsigned char)inBuf[nNumBytesToReceive-1];
	//special case for video capture frame
	bool bSkipProcessData=false;
	if (pBoatData->nPacketType==VIDEO_DATA_PACKET) {
		bSkipProcessData=true;
		int nImageBytes = 0;
		memcpy(&nImageBytes,pBoatData->dataBytes,sizeof(int));
		//make sure image is not super large
		if (nImageBytes<10000000) {
			this->m_nNumImageBytesToReceive = nImageBytes;
			StartImageDownloadThread();//downloading image might take a while and tie up main thread, so start a worker thread to do it
			BoatCommand::DeleteBoatData(pBoatData);
			delete []inBuf;
			return true;
		}
	}
	else if (pBoatData->nPacketType == LIST_REMOTE_SCRIPTS) {
		bSkipProcessData = true;
		int nNumBytes = 0;
		memcpy(&nNumBytes, pBoatData->dataBytes, sizeof(int));
		//make sure number of bytes is not too large
		if (nNumBytes < 200000) {
			bool bRetval = DownloadBytes(nNumBytes, LIST_REMOTE_SCRIPTS);
			delete[]inBuf;
			return bRetval;
		}
	}
	else if (pBoatData->nPacketType == LIST_REMOTE_IMAGE) {
		bSkipProcessData = true;
		int nNumBytes = 0;
		memcpy(&nNumBytes, pBoatData->dataBytes, sizeof(int));
		//make sure number of bytes is not too large
		if (nNumBytes < 200000) {
			bool bRetval = DownloadBytes(nNumBytes, LIST_REMOTE_IMAGE);
			delete[]inBuf;
			return bRetval;
		}
	}
	else if (pBoatData->nPacketType == LIST_REMOTE_DATA) {
		bSkipProcessData = true;
		int nNumBytes = 0;
		memcpy(&nNumBytes, pBoatData->dataBytes, sizeof(int));
		//make sure number of bytes is not too large
		if (nNumBytes < 200000) {
			bool bRetval = DownloadBytes(nNumBytes, LIST_REMOTE_DATA);
			delete[]inBuf;
			return bRetval;
		}
	}
	else if (pBoatData->nPacketType == LIST_REMOTE_LOG) {
		bSkipProcessData = true;
		int nNumBytes = 0;
		memcpy(&nNumBytes, pBoatData->dataBytes, sizeof(int));
		//make sure number of bytes is not too large
		if (nNumBytes < 20000) {
			bool bRetval = DownloadBytes(nNumBytes, LIST_REMOTE_LOG);
			delete[]inBuf;
			return bRetval;
		}
	}
	else if (pBoatData->nPacketType == FILE_RECEIVE) {
		bSkipProcessData = true;
		int nNumBytes = 0;
		memcpy(&nNumBytes, pBoatData->dataBytes, sizeof(int));
		//make sure number of bytes is not too large
		if (nNumBytes < 100000000) {
			bool bRetval = DownloadBytes(nNumBytes, FILE_RECEIVE);
			delete[]inBuf;
			return bRetval;
		}
	}
	else if (pBoatData->nPacketType == DELETE_FILES) {
		bSkipProcessData = true;
		int nNumBytes = 0;
		memcpy(&nNumBytes, pBoatData->dataBytes, sizeof(int));
		//make sure number of bytes is not too large
		if (nNumBytes>0&&nNumBytes < 20000) {
			bool bRetval = DownloadBytes(nNumBytes, DELETE_FILES);
			delete[]inBuf;
			return bRetval;
		}
		else {
			m_sUnDeletedFiles = "";
			delete[]inBuf;
			return true;
		}
	}
	delete []inBuf;
	if (!bSkipProcessData&&!ProcessBoatData(pBoatData)) {
		BoatCommand::DeleteBoatData(pBoatData);
		TRACE("Error, could not process boat data.\n");
		PurgeComm(m_hPort, PURGE_RXCLEAR);
		return false;
	}
	BoatCommand::DeleteBoatData(pBoatData);
	return true;//command was sent successfully
}

/**
 * @brief stop propeller and return rudder angle to the zero position.
 * 
 * @return true if the propeller was properly stopped and the rudder was returned to the zero position.
 * @return false if there was a problem stopping the propeller or returning the rudder to the zero position.
 */
bool SerportCaptain::Stop() {
	PROPELLER_STATE propState;
	if (!Captain::Stop(&propState)) {
		return false;
	}
	REMOTE_COMMAND rc;
	rc.nCommand = THRUST_ON;
	rc.nNumDataBytes = sizeof(PROPELLER_STATE);
	rc.pDataBytes = (unsigned char *)&propState;
	CSingleLock sL(&this->m_sendMutex);
	if (!sL.Lock()) {
		return false;
	}
	bool bSentStopCommand = SendSerialCommand(&rc);
	sL.Unlock();
	return bSentStopCommand;
}


/**
 * @brief query supported data (i.e. find out what types of data the boat is capable of collecting).
 * 
 * @return true if a response from the boat was successfully obtained about what types of data it supported.
 * @return false if no valid response could be obtained from the boat.
 */
bool SerportCaptain::FindSupportedSensors() {
	REMOTE_COMMAND rc;
	rc.nCommand = SUPPORTED_SENSOR_DATA;
	rc.nNumDataBytes = 0;
	rc.pDataBytes = NULL;
	CSingleLock sL(&this->m_sendMutex);
	if (!sL.Lock(1000)) {
		return false;
	}
	if (SendSerialCommand(&rc)) {
		//test
		TRACE("try to receive supported sensors...\n");
		//end test
		if (!ReceiveBoatData()) {
			sL.Unlock();
			return false;
		}
		//receive info for the type of each sensor
		if (m_nNumSensorsAvailable>0) {
			//test
			TRACE("try to receive sensor types...\n");
			//end test
			if (!ReceiveBoatData()) {
				sL.Unlock();
				return false;
			}
		}	
	}
	else {
		sL.Unlock();
		return false;
	}
	sL.Unlock();
	return true;
}

/**
 * @brief query the boat for any sensor data that it might have (eg: water temperature, pH, turbidity, etc.). It also gets voltage and link quality data.
 * 
 * @param sSensorData holds the sensor data text returned by the function if it succeeds.
 * @return true if the request for sensor data from the boat was successful or at least partially successful (got data from at least one sensor).
 * @return false if there was a problem requesting sensor data from the boat.
 */
bool SerportCaptain::RequestSensorData(CString &sSensorData) {
	sSensorData="";
	bool bGotSomeData = false;
	if (!m_bAskedForSupportedSensors) {
		if (FindSupportedSensors()) {
			m_bAskedForSupportedSensors = true;
		}
	}
	CSingleLock sL(&this->m_sendMutex);
	if (!sL.Lock(1000)) {
		return false;
	}
	for (int i=0;i<this->m_nNumSensorsAvailable;i++) {
		if (Captain::isGraphableSensorType(m_sensorTypes[i])) {
			CString sVal = "";
			REMOTE_COMMAND rc;
			rc.nCommand = Captain::GetSensorPacketType(m_sensorTypes[i]);
			rc.nNumDataBytes = 0;
			rc.pDataBytes = NULL;
			if (SendSerialCommand(&rc)) {
				//test
				TRACE("try to receive %s data...\n", Captain::GetSensorTypeStr(m_sensorTypes[i]));
				//end test
				if (ReceiveBoatData()) {
					bGotSomeData = true;
				}
				sVal = FormatSensorVal(m_sensorTypes[i]);//format the received sensor data value
				sSensorData += (sVal + ", ");
			}
			else {
				continue;
			}
		}
	}
	sL.Unlock();
	int nSensorDataLength = sSensorData.GetLength();
	if (nSensorDataLength>0) {//get rid of trailing comma
		sSensorData = sSensorData.Left(nSensorDataLength-2);
	}
	return bGotSomeData;
}

/**
 * @brief request an image capture with feature markings in it from the boat. 
 * 
 * @param requestedImg pointer to CImage object that is used to store the returned image.
 * @pVideoDlg pointer to CDialog object that corresponds to the CVideoLinkDlg dialog window that is displaying the image from the video camera.
 * @param nFeatureThreshold feature threshold value from 0 to 255 that is used to control how feature markings are determined in the image. nFeatureThreshold==0 disables or removes feature markers from the image.
 * @return true if the video image could be properly obtained.
 * @return false if there was a problem getting the video image.
 */
bool SerportCaptain::RequestVideoImage(CImage *requestedImg, CDialog *pVideoDlg, int nFeatureThreshold) {
	if (!requestedImg) return false;
	m_bRequestingVideoImage = true;
	m_requestedImg = requestedImg;
	m_videoDlg = pVideoDlg;
	REMOTE_COMMAND rc;
	rc.nCommand = VIDEO_DATA_PACKET;
	rc.nNumDataBytes = sizeof(int);
	rc.pDataBytes = new unsigned char[rc.nNumDataBytes];
	//test
	nFeatureThreshold = 0x00020000;
	//end test
	//nFeatureThreshold = nFeatureThreshold | 0x00020000;//change to low quality image for slow serial port connection
	memcpy(rc.pDataBytes,&nFeatureThreshold,rc.nNumDataBytes);
	CSingleLock sL(&this->m_sendMutex);
	if (!sL.Lock(1000)) {
		return false;
	}
	if (SendSerialCommand(&rc)) {
		//test
		TRACE("try to receive video image...\n");
		//end test
		if (!ReceiveBoatData()) {
			TRACE("Could not receive video image.\n");
			delete []rc.pDataBytes;
			m_bRequestingVideoImage = false;
			sL.Unlock();
			return false;
		}
	}
	else {
		delete []rc.pDataBytes;
		m_bRequestingVideoImage = false;
		sL.Unlock();
		return false;
	}
	delete []rc.pDataBytes;
	sL.Unlock();
	//CopyReceivedImage(requestedImg);
	m_bRequestingVideoImage = false;
	return true;
}



/**
 * @brief tries to receive a large chunk of data over serial port connection.
 * 
 * @param rxBytes buffer used to store the received data.
 * @param nNumToReceive the number of bytes to receive over the serial port connection.
 * @return int the number of bytes successfully read (and stored in rxBytes) over the serial port connection.
 */
int SerportCaptain::ReceiveLargeDataChunk(char * rxBytes, int nNumToReceive) {
	const int MAX_ALLOWED_TIMEOUTS = 5;
	const int MAX_RECEIVE_ATTEMPT = 128;//maximum number of bytes to attempt receiving in a single ReadFile attempt
	const int NUM_PACKET_OVERHEAD_BYTES = 11;//the number of packet overhead bytes, sync bytes, id bytes, data size bytes, crc bytes, etc.
	unsigned char outBuf[3] = {0,0,0};//output buffer for sending confirmation bytes over serial port link
	unsigned char inBuf[1024];//temporary buffer for collecting incoming data
	int nNumRemaining = nNumToReceive;
	int nNumReceived=0;
	int nTimeoutCount=0;
	int nChunkIndex = 0;
	//test
	TRACE("About to receive %d data bytes.\n",nNumToReceive);
	//end test
	if (!m_hPort) return 0;
	do {
		DWORD dwNumReceived = 0;
		DWORD dwNumToReceive = (DWORD)min(nNumRemaining+NUM_PACKET_OVERHEAD_BYTES, MAX_RECEIVE_ATTEMPT);
		if (!ReadFile(m_hPort,inBuf,dwNumToReceive,&dwNumReceived,0)) {
			//timeout or error occurred
			nTimeoutCount++;
		}
		else {
			if (dwNumReceived>0) {
				DWORD dwMoreBytesRequired = 0;//will be non-zero if we need to receive more bytes in order to make a valid packet of data
				int nPacketStartIndex=0;
				int nValidPacket = CheckPacket(inBuf,dwNumReceived,nChunkIndex,dwMoreBytesRequired,nPacketStartIndex);
				if (nValidPacket==PACKET_OK) {
					CopyPacketToBuf(inBuf,nPacketStartIndex,dwNumReceived,&rxBytes[nNumReceived]);
					nNumRemaining-=((int)(dwNumReceived - NUM_PACKET_OVERHEAD_BYTES));
					nNumReceived+=((int)(dwNumReceived - NUM_PACKET_OVERHEAD_BYTES));
					//send confirmation (CRC) bytes back to boat
					TRACE("data received ok, sending confirmation.\n");
					nChunkIndex++;
					SendConfirmationBytes(m_hPort,inBuf[dwNumReceived-2],inBuf[dwNumReceived-1]);
					nTimeoutCount=0;
				}
				else if (nValidPacket==REPEATED_CHUNK) {
					//just send confirmation (CRC) bytes back to boat
					//send confirmation (CRC) bytes back to boat
					TRACE("received repeated chunk.\n");
					SendConfirmationBytes(m_hPort,inBuf[dwNumReceived-2],inBuf[dwNumReceived-1]);
					nTimeoutCount=0;
				}
				else if (nValidPacket==NOT_ENOUGH_BYTES) {//need to receive more bytes of data
					//dwMoreBytesRequired should be non-zero
					//try reading in more bytes
					DWORD dwMoreBytesRead = 0;
					ReadFile(m_hPort,&inBuf[dwNumReceived],dwMoreBytesRequired,&dwMoreBytesRead,0);
					if (dwMoreBytesRead==0) {
						TRACE("not enough bytes received\n");
						SendErrorBytes(m_hPort);
					}
					else {//got at least some more bytes, check again to see if packet is valid
						dwNumReceived+=dwMoreBytesRead;
						nValidPacket = CheckPacket(inBuf,dwNumReceived,nChunkIndex,dwMoreBytesRequired,nPacketStartIndex);
						if (nValidPacket==PACKET_OK) {
							CopyPacketToBuf(inBuf,nPacketStartIndex,dwNumReceived,&rxBytes[nNumReceived]);
							nNumRemaining-=((int)(dwNumReceived - NUM_PACKET_OVERHEAD_BYTES));
							nNumReceived+=((int)(dwNumReceived - NUM_PACKET_OVERHEAD_BYTES));
							//send confirmation (CRC) bytes back to boat
							TRACE("data received ok, sending confirmation.\n");
							nChunkIndex++;
							SendConfirmationBytes(m_hPort,inBuf[dwNumReceived-2],inBuf[dwNumReceived-1]);
							nTimeoutCount=0;
						}
						else if (nValidPacket==REPEATED_CHUNK) {
							//just send confirmation (CRC) bytes back to boat
							//send confirmation (CRC) bytes back to boat
							TRACE("received repeated chunk.\n");
							SendConfirmationBytes(m_hPort,inBuf[dwNumReceived-2],inBuf[dwNumReceived-1]);
							nTimeoutCount=0;
						}
						else {//packet is not valid
							TRACE("invalid packet.\n");
							SendErrorBytes(m_hPort);
						}
					}
				}
				else {
					TRACE("data got garbled\n");
					SendErrorBytes(m_hPort);
				}
				TRACE("nNumReceived = %d\n",nNumReceived);
			}
			else {//timeout occurred
				nTimeoutCount++;
			}
			if (this->m_bStopDownloadThread) {
				SendCancelBytes();//send three 0x0a bytes to indicate that downloading is stopping
				//outBuf[0] = 0x0a; outBuf[1] = 0x0a; outBuf[2] = 0x0a;
				//WriteFile(m_hPort, outBuf, 3, &dwNumWritten,NULL);
				return 0;
			}
		}
		if (m_bytesDownloaded) {
			sprintf(m_bytesDownloaded, "%d", nNumReceived);
		}
	} while (nNumRemaining>0&&nTimeoutCount<MAX_ALLOWED_TIMEOUTS&&!this->m_bStopDownloadThread);
	if (m_bStopDownloadThread&&nNumReceived<nNumToReceive) {
		SendCancelBytes();		
		return 0;
	}
	return nNumReceived;	
}

/**
 * @brief return true if currently connected to the boat over a wireless serial link
 * 
 * @return true if currently connected to the boat over a wireless serial link
 * @return false if not connected to the boat over a wireless serial link
 */
bool SerportCaptain::IsConnected() {
	return this->m_bConnected;
}

void SerportCaptain::StartImageDownloadThread() {//downloading image might take a while and tie up main thread, so start a worker thread to do it
	if (!this->m_bDownloadThreadRunning) {
		AfxBeginThread(DownloadImageThread, (LPVOID)this);
	}
}

void SerportCaptain::StopDownloadThread() {//stop thread that is downloading data
	this->m_bStopDownloadThread = true;
	DWORD dwTimeoutTime = timeGetTime() + 5000;
	while (m_bDownloadThreadRunning&&timeGetTime()<dwTimeoutTime) {
		Sleep(100);
	}
}

bool SerportCaptain::IsDownloadRunning() {//return true if a download is in progress
	return this->m_bDownloadThreadRunning;
}

double SerportCaptain::GetRXWirelessPower() {//get the wireless received power level in dBm
	const DWORD TIMEOUT_INTERVAL_MS = 2000;//timeout in ms when waiting for a response
	if (!m_hPort) {
		return 0.0;
	}
	PurgeComm(m_hPort,PURGE_RXCLEAR|PURGE_TXCLEAR);
	unsigned char outBuf[7];
	outBuf[0] = 0x0d;//send 1st carriage return to make receiving local radio unit ready for a new line of input
	outBuf[1] = 'p';
	outBuf[2] = 'o';
	outBuf[3] = 'w';
	outBuf[4] = 'e';
	outBuf[5] = 'r';
	outBuf[6] = 0x0d;
	//send 'w' followed by carriage return to request power level from local radio
	DWORD dwNumWritten = 0;
	WriteFile(m_hPort, outBuf, 7, &dwNumWritten,NULL);
	char resp[256];
	//read in response text
	int i=0;
	DWORD dwNumReceived = 0;
	BOOL bRead = ReadFile(m_hPort, &resp[i], 1, &dwNumReceived, NULL);
	DWORD dwTimeoutTime = timeGetTime() + TIMEOUT_INTERVAL_MS;
	while (bRead&&(timeGetTime()<dwTimeoutTime)&&resp[i]!=0x0a) {
		if (dwNumReceived>0) {
			i++;
		}
		bRead = ReadFile(m_hPort, &resp[i], 1, &dwNumReceived, NULL);
	}
	if (i>0) {
		resp[i] = 0;//replace end of text with null termination
		CString sReceivedText = CString(resp);
		if (sReceivedText.Find("N.A.")>=0) {//link info not available
			return 0.0;
		}
		double dLinkPower = 0.0;//link power in dBm
		if (sscanf_s(sReceivedText,"Link: %lf", &dLinkPower)>0) {
			return dLinkPower;
		}
	}
	return 0;//link info could not be received or parsed
}

bool SerportCaptain::DelayedSendSerialCommand(REMOTE_COMMAND *pRC) { //sends a remote command out over the serial port connection, after DELAYED_SEND_TIME_MS milliseconds. Can be useful when issuing large numbers of navigation commands in a short period of time (i.e. only need to send the most recent navigation command).
	//multiple calls to this function update a timeout value in a separate thread, which once it times out, will just send the most recent REMOTE_COMMAND data.
	//this has the effect of reducing the amount of communications traffic occurring over a wireless link.
	if (!m_hPort) {
		return false;//not connected to a serial port
	}
	m_dwDelayedSendTimeout = timeGetTime() + DELAYED_SEND_TIME_MS;
	CSingleLock sL(&this->m_delayedDataMutex);
	if (!sL.Lock(1000)) {
		return false;
	}
	Captain::CopyCommand(&this->m_delayedRemoteCommand, pRC);
	sL.Unlock();
	if (!m_bDelayedSendThreadRunning) {
		StartDelayedSendThread();
	}
	return true;
}

void SerportCaptain::StartDelayedSendThread() {//starts a worker thread that will send out the data / command contained in m_delayedRemoteCommand once the system timer goes beyond m_dwDelayedSendTimeout ms
	if (m_bDelayedSendThreadRunning) {
		return;//thread for sending delayed data / commands is already running
	}
	AfxBeginThread(DelayedSendThread,(LPVOID)this);
}


void SerportCaptain::ExitDelayedSendThread() {//stop the thread for sending delayed serial port data
	const int TIMEOUT_VAL = 5000;//length of time to wait in ms
	DWORD dwTimeout = timeGetTime() + TIMEOUT_VAL;
	this->m_bStopDelayedSend = true;
	while (timeGetTime()<dwTimeout&&this->m_bDelayedSendThreadRunning) {
		Sleep(100);
	}
}

/**
 * @brief send command to quit the remote program running on AMOS
 * 
 * @return true if the command to quit the remote program was received successfully
 */
bool SerportCaptain::QuitRemoteProgram() {
	REMOTE_COMMAND rc;
	rc.nCommand = QUIT_PROGRAM;
	rc.nNumDataBytes = 0;
	rc.pDataBytes = NULL;
	CSingleLock sL(&this->m_sendMutex);
	if (!sL.Lock()) {
		return false;
	}
	bool bSentQuitCommand = SendSerialCommand(&rc);
	sL.Unlock();
	return bSentQuitCommand;	
}

int SerportCaptain::CheckPacket(unsigned char *inBuf, DWORD dwBufSize, int nChunkIndex, DWORD &dwMoreBytesRequired, int &nPacketStartIndex) {//check to see if a chunk of data is valid or not
	//returns one of the following:
	//PACKET_OK 0 //the packet is fine without any data problems
	//REPEATED_CHUNK 1 //the packet is fine, but corresponds to an old (repeated) chunk of data that has already been received
	//BAD_CRC 2 //one or both of the CRC bytes are incorrect
	//NOT_ENOUGH_BYTES 3 //what has been received of the packet so far is ok, but there are not enough bytes present
	//NO_SYNC_BYTES 4 //no synchronization bytes could be found in the packet data
	//BAD_CHUNK_INDEX 5 //bytes for chunk index corresponded to a chunk that was too high
	//BAD_DATASIZE 6 //bytes for data size are wrong
	//test
	const int MAX_DATA_CHUNKSIZE = 117;//the maximum number of allowed data bytes in the packet
	dwMoreBytesRequired = 0;
	nPacketStartIndex = -1;
	TRACE("checking %u bytes at %u\n",dwBufSize,timeGetTime());
	//end test
	if (dwBufSize<=11) {
		TRACE("not enough data present.\n");
		return NOT_ENOUGH_BYTES;//not enough data present
	}
	//look for sync bytes (i.e. "AMOS" text)
	int nSyncIndex = -1;
	for (DWORD i = 0;i<(dwBufSize-4);i++) {
		if (inBuf[i]=='A'&&inBuf[i+1]=='M'&&inBuf[i+2]=='O'&&inBuf[i+3]=='S') {
			//found sync bytes
			nSyncIndex = i;
			break;
		}
	}
	if (nSyncIndex<0) {
		TRACE("no sync bytes could be found\n");
		return NO_SYNC_BYTES;
	}
	//check chunk value
	int nChunkVal = (inBuf[nSyncIndex+4]<<16) + (inBuf[nSyncIndex+5]<<8) + inBuf[nSyncIndex+6];
	if (nChunkVal>nChunkIndex) {//receiving an unexpected chunk of data, data for chunk index must have gotten garbled
		TRACE("bytes for chunk index are wrong.\n");
		return BAD_CHUNK_INDEX;
	}
	int nDataSize = (inBuf[nSyncIndex+7]<<8) + inBuf[nSyncIndex+8];
	if (nDataSize>MAX_DATA_CHUNKSIZE) {
		TRACE("bytes for data size are wrong.\n");
		return BAD_DATASIZE;
	}
	if ((nDataSize+11)>dwBufSize) {//not enough data available
		TRACE("not enough data available.\n");
		return NOT_ENOUGH_BYTES;
	}
	//add up checksum
	int nChecksum = 0;
	for (int i=0;i<nDataSize;i++) {
		nChecksum+=((int)(inBuf[nSyncIndex+9+i]));
	}
	int nTestChecksum = ((int)inBuf[nSyncIndex+9+nDataSize])*256 + inBuf[nSyncIndex+9+nDataSize+1];
	bool bValidChecksum = (nChecksum==nTestChecksum);
	if (!bValidChecksum) {
		TRACE("invalid checksum, crc_byte1 = %d, crc_byte2 = %d, nChecksum = %d, nTestChecksum = %d.\n",(int)inBuf[nSyncIndex+9+nDataSize],(int)inBuf[nSyncIndex+9+nDataSize+1],
			nChecksum,nTestChecksum);
		return BAD_CRC;
	}
	nPacketStartIndex = nSyncIndex;
	if (nChunkVal<nChunkIndex) {
		//everything was ok, but it is for old data that has already been received
		TRACE("repeated chunk of data.\n");
		return REPEATED_CHUNK;
	}
	return PACKET_OK;
}

void SerportCaptain::FlushRXBuffer() {//flush input buffer (typically called if some unexpected data arrives and communications need to get back on track)
	if (!m_hPort) return;
	PurgeComm(m_hPort, PURGE_RXCLEAR);
}

void SerportCaptain::CopyPacketToBuf(unsigned char *inBuf, int nBufferStartIndex, DWORD dwBufSize, char *destBuf) {//copy the data portion of inBuf to destBuf
	int nDataSize = (inBuf[nBufferStartIndex+7]<<8) + inBuf[nBufferStartIndex+8];
	if (nDataSize>dwBufSize) {
		return;//data size too big? should not happen
	}
	memcpy(destBuf,&inBuf[9+nBufferStartIndex],nDataSize);
}

void SerportCaptain::SendConfirmationBytes(HANDLE hPort,unsigned char byte1, unsigned char byte2) {//send a couple of confirmation bytes to the boat
	unsigned char outBuf[4];
	TRACE("sending confirmation bytes %d and %d\n",(int)byte1,(int)byte2);
	outBuf[0] = 0;
	outBuf[1] = 0;
	outBuf[2] = byte1;
	outBuf[3] = byte2;
	DWORD dwNumWritten = 0;
	WriteFile(hPort,outBuf,4,&dwNumWritten,NULL);
}

void SerportCaptain::SendErrorBytes(HANDLE hPort) {//send bytes that correspond to 
	unsigned char outBuf[4] = {0xff, 0xff, 0xff, 0xff};
	DWORD dwNumWritten = 0;
	if (!hPort) return;
	PurgeComm(m_hPort, PURGE_RXCLEAR | PURGE_TXCLEAR);
	WriteFile(hPort,outBuf,4,&dwNumWritten,NULL);
}

void SerportCaptain::SendCancelBytes() {//send four 0x0a bytes to indicate that downloading is stopping
	if (!m_hPort) return;
	unsigned char outBuf[4] = {0x0a, 0x0a, 0x0a, 0x0a};
	DWORD dwNumWritten = 0;
	WriteFile(m_hPort,outBuf,4,&dwNumWritten,NULL);
}

/**
 * @brief send command to change the step of the currently executing remote program
 *
 * @param nStepChange the number of steps to advance (positive) or go back (negative) in the current remote script
 * @return 0 if the command was sent successfully sent, or negative if an error occurred.
 */
int SerportCaptain::RemoteScriptStepChange(int nStepChange) {//send command to change the step of the currently executing remote program
	REMOTE_COMMAND rc;
	rc.nCommand = SCRIPT_STEP_CHANGE;
	rc.nNumDataBytes = sizeof(int);
	rc.pDataBytes = new unsigned char[rc.nNumDataBytes];
	
	memcpy(rc.pDataBytes, &nStepChange, rc.nNumDataBytes);
	CSingleLock sL(&this->m_sendMutex);
	if (!sL.Lock(1000)) {
		delete[]rc.pDataBytes;
		return false;
	}
	int nRetval = 0;
	if (!SendSerialCommand(&rc)) {
		nRetval = -1;
	}
	delete[]rc.pDataBytes;
	sL.Unlock();
	return nRetval;
}

/**
 * @brief get the names of remote files on AMOS of a particular file type
 *
 * @param remoteFiles pointer to a char buffer that holds the names of the returned filenames. Each filename is separated by a "\n".
 * @param nBufSize the size of the buffer pointed to by remoteFiles.
 * @param nFileType one of the defined filetypes listed in the Captain.h file.
 * @return 0 if the command was executed successfully, or negative if an error occurred.
 */
int SerportCaptain::GetRemoteFiles(char* remoteFiles, int nBufSize, int nFileType) {//get the names of remote files on AMOS of a particular file type
	REMOTE_COMMAND rc;
	rc.nCommand = LIST_REMOTE_SCRIPTS;
	if (nFileType == REMOTE_DATA_FILES) {
		rc.nCommand = LIST_REMOTE_DATA;
	}
	else if (nFileType == REMOTE_LOG_FILES) {
		rc.nCommand = LIST_REMOTE_LOG;
	}
	else if (nFileType == REMOTE_IMAGE_FILES) {
		rc.nCommand = LIST_REMOTE_IMAGE;
	}
	rc.nNumDataBytes = 0;
	rc.pDataBytes = NULL;

	CSingleLock sL(&this->m_sendMutex);
	if (!sL.Lock(10000)) {
		return -2;
	}
	int nRetval = 0;
	if (!SendSerialCommand(&rc)) {
		nRetval = -3;
	}
	else {
		if (!ReceiveBoatData()) {
			nRetval = -4;
		}
		else {
			//filenames received successfully
			if (nFileType == REMOTE_SCRIPT_FILES) {
				strcpy(remoteFiles, m_sRemoteScriptsAvailable.GetBuffer(m_sRemoteScriptsAvailable.GetLength()));
			}
			else if (nFileType == REMOTE_DATA_FILES) {
				strcpy(remoteFiles, m_sRemoteDataAvailable.GetBuffer(m_sRemoteDataAvailable.GetLength()));
			}
			else if (nFileType == REMOTE_LOG_FILES) {
				strcpy(remoteFiles, m_sRemoteLogsAvailable.GetBuffer(m_sRemoteLogsAvailable.GetLength()));
			}
			else if (nFileType == REMOTE_IMAGE_FILES) {
				strcpy(remoteFiles, m_sRemoteRemoteImageFilesAvailable.GetBuffer(m_sRemoteRemoteImageFilesAvailable.GetLength()));
			}
		}
	}
	delete[]rc.pDataBytes;
	sL.Unlock();
	return nRetval;
}

bool SerportCaptain::DownloadBytes(int nNumBytes, int nDataType) {//download a largeish number of bytes at once
	bool bRetval = false;
	if (nDataType == LIST_REMOTE_SCRIPTS) {
		char* fileNameBytes = new char[nNumBytes+1];
		int nNumReceived = ReceiveLargeDataChunk(fileNameBytes, nNumBytes);
		if (nNumReceived==nNumBytes) {//bytes received successfully, save to m_sRemoteScriptsAvailable
			fileNameBytes[nNumReceived] = 0;//null terminate
			m_sRemoteScriptsAvailable = CString(fileNameBytes);
			bRetval = true;
		}
		else {
			m_sRemoteScriptsAvailable = "";
		}
		delete[]fileNameBytes;
	}
	else if (nDataType == LIST_REMOTE_DATA) {
		char* fileNameBytes = new char[nNumBytes + 1];
		int nNumReceived = ReceiveLargeDataChunk(fileNameBytes, nNumBytes);
		if (nNumReceived == nNumBytes) {//bytes received successfully, save to m_sRemoteDataAvailable
			fileNameBytes[nNumReceived] = 0;//null terminate
			m_sRemoteDataAvailable = CString(fileNameBytes);
			bRetval = true;
		}
		else {
			m_sRemoteDataAvailable = "";
		}
		delete[]fileNameBytes;
	}
	else if (nDataType == LIST_REMOTE_IMAGE) {
		char* fileNameBytes = new char[nNumBytes + 1];
		int nNumReceived = ReceiveLargeDataChunk(fileNameBytes, nNumBytes);
		if (nNumReceived == nNumBytes) {//bytes received successfully, save to m_sRemoteDataAvailable
			fileNameBytes[nNumReceived] = 0;//null terminate
			m_sRemoteRemoteImageFilesAvailable = CString(fileNameBytes);
			bRetval = true;
		}
		else {
			m_sRemoteRemoteImageFilesAvailable = "";
		}
		delete[]fileNameBytes;
	}
	else if (nDataType == LIST_REMOTE_LOG) {
		char* fileNameBytes = new char[nNumBytes + 1];
		int nNumReceived = ReceiveLargeDataChunk(fileNameBytes, nNumBytes);
		if (nNumReceived == nNumBytes) {//bytes received successfully, save to m_sRemoteLogsAvailable
			fileNameBytes[nNumReceived] = 0;//null terminate
			m_sRemoteLogsAvailable = CString(fileNameBytes);
			bRetval = true;
		}
		else {
			m_sRemoteLogsAvailable = "";
		}
		delete[]fileNameBytes;
	}
	else if (nDataType == FILE_RECEIVE) {//receiving a file from AMOS
		if (!m_fileDestPath) {
			return false;
		}
		unsigned char* fileBytes = new unsigned char[nNumBytes + 1];
		if (m_bytesToDownload) {
			sprintf(m_bytesToDownload, "%d", nNumBytes);
		}
		int nNumReceived = ReceiveLargeDataChunk((char *)fileBytes, nNumBytes);
		if (nNumReceived == nNumBytes) {//bytes received successfully, save to m_fileDestPath
			CFile destFile;
			BOOL bOpened = destFile.Open(m_fileDestPath, CFile::modeCreate | CFile::shareDenyNone | CFile::modeReadWrite);
			if (!bOpened) {
				bRetval = false;
			}
			else {
				destFile.Write(fileBytes, nNumReceived);
				bRetval = true;
			}
		}
		delete [] fileBytes;
	}
	else if (nDataType == DELETE_FILES) {//receiving a list of files from AMOS of files that could not be deleted 
		if (nNumBytes == 0) {
			m_sUnDeletedFiles = "";
			bRetval = true;
		}
		else {
			char* fileNameBytes = new char[nNumBytes + 1];
			int nNumReceived = ReceiveLargeDataChunk(fileNameBytes, nNumBytes);
			if (nNumReceived == nNumBytes) {//bytes received successfully, save to m_sU
				fileNameBytes[nNumReceived] = 0;//null terminate
				m_sRemoteDataAvailable = CString(fileNameBytes);
				bRetval = true;
			}
			delete[] fileNameBytes;
		}
	}
	return bRetval;
}

/**
 * @brief instruct AMOS to start using a particular remote script filename. 
 *
 * @param remoteFileName pointer to a char buffer that holds the name of the remote script file.
 * @param nSize the size of the buffer pointed to by remoteFileName.
 * @return 0 if the command was executed successfully, or negative if an error occurred.
 */
int SerportCaptain::UseRemoteScript(char* remoteFileName, int nSize) {
	REMOTE_COMMAND rc;
	rc.nCommand = USE_REMOTE_SCRIPT;
	rc.nNumDataBytes = sizeof(int)+strlen(remoteFileName);
	rc.pDataBytes = new unsigned char[rc.nNumDataBytes];
	memcpy(rc.pDataBytes, &nSize, sizeof(int));
	int nRemoteFilenameLength = strlen(remoteFileName);
	if (nRemoteFilenameLength > 0) {
		memcpy(&rc.pDataBytes[sizeof(int)], remoteFileName, nRemoteFilenameLength);
	}

	CSingleLock sL(&this->m_sendMutex);
	if (!sL.Lock(1000)) {
		delete []rc.pDataBytes;
		return -2;
	}
	int nRetval = 0;
	if (!SendSerialCommand(&rc)) {
		nRetval = -3;
	}
	delete[]rc.pDataBytes;
	sL.Unlock();
	return nRetval;
}

/**
 * @brief send data from a file out the serial port
 *
 * @param filename pointer to a char buffer that holds the name of the file to be uploaded
 * @param nFileNameSize the size of the buffer pointed to by filename
 * @param destPath the destination path on the remote device, where the contents of the file will be copied to (cannot be NULL)
 * @param nDestPathSize the size of the buffer pointed to by destPath (in bytes)
 * @return 0 if the command was executed successfully, or negative if an error occurred.
 */
int SerportCaptain::SendFile(char* filename, int nFileNameSize, char* destPath, int nDestPathSize) {
	if (!filename||nFileNameSize<=0||!destPath) {
		return -1;
	}
	CFile fileToSend;
	BOOL bOpened = fileToSend.Open(filename, CFile::modeRead | CFile::shareDenyNone);
	if (!bOpened) {
		return -2;
	}
	int nFileLength = (int)fileToSend.GetLength();
	if (nFileLength <= 0) {
		return -3;//file is either way too big or empty!
	}
	int nNumDataBytesToSend = sizeof(int) + nFileLength + nDestPathSize;//the total size of data being sent (integer corresponding to nDestPathSize, the actual file contents, and then the destination output path)
	unsigned char* fileBytes = new unsigned char[nNumDataBytesToSend];
	fileBytes[0] = (unsigned char)((nDestPathSize & 0xff000000) >> 24);
	fileBytes[1] = (unsigned char)((nDestPathSize & 0x00ff0000) >> 16);
	fileBytes[2] = (unsigned char)((nDestPathSize & 0x0000ff00) >> 8);
	fileBytes[3] = (unsigned char)(nDestPathSize & 0x000000ff);
	int nNumRead = (int)fileToSend.Read(&fileBytes[4], nFileLength);
	if (nNumRead < nFileLength) {
		delete[]fileBytes;
		return -6;
	}
	memcpy(&fileBytes[4 + nFileLength], destPath, nDestPathSize);
	REMOTE_COMMAND rc;
	rc.nCommand = FILE_TRANSFER;
	rc.nNumDataBytes = 4;
	rc.pDataBytes = new unsigned char[4];
	unsigned char dataSizeBytes[4];
	dataSizeBytes[0] = (unsigned char)((nNumDataBytesToSend & 0xff000000) >> 24);
	dataSizeBytes[1] = (unsigned char)((nNumDataBytesToSend & 0x00ff0000) >> 16);
	dataSizeBytes[2] = (unsigned char)((nNumDataBytesToSend & 0x0000ff00) >> 8);
	dataSizeBytes[3] = (unsigned char)(nNumDataBytesToSend & 0x000000ff);
	memcpy(rc.pDataBytes, dataSizeBytes, 4);//send info pertaining to the size of the next block of data to come
		
	CSingleLock sL(&this->m_sendMutex);
	if (!sL.Lock(1000)) {
		delete []fileBytes;
		delete[]rc.pDataBytes;
		return -4;
	}
	int nRetval = 0;
	if (!SendSerialCommand(&rc)) {
		nRetval = -5;
	}
	else {
		//now send a larger block of data that corresponds to the actual file contents and destination path
		if (!BoatCommand::SendLargeSerialData((int)m_hPort, fileBytes, nNumDataBytesToSend, NULL)) {//send large amount of data out serial port, need to get confirmation after sending each chunk
			nRetval = -7;
		}
	}
	delete[]rc.pDataBytes;
	delete[]fileBytes;
	sL.Unlock();
	return nRetval;
}

/**
 * @brief receive a copy of a remote file on AMOS
 *
 * @param remoteFilename the path to the remote filename on AMOS that we want to download.
 * @param destPath the local path of the copied file that was downloaded (should get downloaded to the current folder)
 * @param bytesDownloaded a char pointer to the number of bytes downloaded (updated by this function as it downloads the data)
 * @param bytesToDownload a char pointer to the number of bytes that need to be downloaded (updated by this function)
  * @return 0 if the command was executed successfully, or negative if an error occurred.
 */
int SerportCaptain::ReceiveFile(char* remoteFilename, char* destPath, char *bytesDownloaded, char *bytesToDownload) {//receive a remote file on AMOS
	if (!remoteFilename || !destPath) {//invalid parameter
		return -2;
	}
	int nRemoteFilenameLength = strlen(remoteFilename);
	REMOTE_COMMAND rc;
	rc.nCommand = FILE_RECEIVE;
	rc.nNumDataBytes = 4;
	rc.pDataBytes = new unsigned char[4];
	unsigned char dataSizeBytes[4];
	dataSizeBytes[0] = (unsigned char)((nRemoteFilenameLength & 0xff000000) >> 24);
	dataSizeBytes[1] = (unsigned char)((nRemoteFilenameLength & 0x00ff0000) >> 16);
	dataSizeBytes[2] = (unsigned char)((nRemoteFilenameLength & 0x0000ff00) >> 8);
	dataSizeBytes[3] = (unsigned char)(nRemoteFilenameLength & 0x000000ff);
	memcpy(rc.pDataBytes, dataSizeBytes, 4);//send info pertaining to the size of the next block of data to come

	CSingleLock sL(&this->m_sendMutex);
	if (!sL.Lock(1000)) {
		delete[]rc.pDataBytes;
		return -3;
	}
	int nRetval = 0;
	if (!SendSerialCommand(&rc)) {
		nRetval = -4;
	}
	else {
		//now send a block of data that corresponds to the path of the remote file
		DWORD dwNumWritten = 0;
		if (!WriteFile(m_hPort, remoteFilename, (DWORD)nRemoteFilenameLength, &dwNumWritten, NULL)) {
			nRetval = -5;
		}
		//copy pointer to file destination path
		m_fileDestPath = destPath;
		//try to receive data from boat
		m_bytesDownloaded = bytesDownloaded;
		m_bytesToDownload = bytesToDownload;
		if (!ReceiveBoatData()) {
			nRetval = -6;
		}
		m_bytesDownloaded = NULL;
		m_bytesToDownload = NULL;
	}
	delete[]rc.pDataBytes;
	sL.Unlock();
	return nRetval;
}

/**
 * @brief send instruction to AMOS to refresh a particular settings type based on the current contents of the prefs.txt file
 *
 * @param nSettingsType the particular type of settings that we want to refresh (see CommandList.h for list of possible settings types)
 * @return 0 if the command was executed successfully, or negative if an error occurred.
 */
int SerportCaptain::RefreshSettings(int nSettingsType) {//send instruction to AMOS to refresh a particular settings type based on the current contents of the prefs.txt file
	REMOTE_COMMAND rc;
	rc.nCommand = REFRESH_SETTINGS;
	rc.nNumDataBytes = 4;
	rc.pDataBytes = new unsigned char[4];
	unsigned char dataSizeBytes[4];
	dataSizeBytes[0] = (unsigned char)((nSettingsType & 0xff000000) >> 24);
	dataSizeBytes[1] = (unsigned char)((nSettingsType & 0x00ff0000) >> 16);
	dataSizeBytes[2] = (unsigned char)((nSettingsType & 0x0000ff00) >> 8);
	dataSizeBytes[3] = (unsigned char)(nSettingsType & 0x000000ff);
	memcpy(rc.pDataBytes, dataSizeBytes, 4);//send info pertaining to the size of the next block of data to come
	CSingleLock sL(&this->m_sendMutex);
	if (!sL.Lock(1000)) {
		delete[]rc.pDataBytes;
		return false;
	}
	int nRetval = 0;
	if (!SendSerialCommand(&rc)) {
		nRetval = -1;
	}
	delete[]rc.pDataBytes;
	sL.Unlock();
	return nRetval;
}

int SerportCaptain::GetBytesToDownload() {//get number of bytes of remote AMOS file that is being downloaded
	int nBytesToDownload = 0;
	if (this->m_bytesToDownload) {
		sscanf(m_bytesToDownload, "%d", &nBytesToDownload);
	}
	return nBytesToDownload;
}

int SerportCaptain::GetBytesDownloaded() {//get number of bytes of remote AMOS file that have been downloaded so far
	int nBytesDownloaded = 0;
	if (m_bytesDownloaded) {
		sscanf(m_bytesDownloaded, "%d", &nBytesDownloaded);
	}
	return nBytesDownloaded;
}

int SerportCaptain::DeleteFiles(char* szFileNames, int nFilenamesSize, int nFileType) {//delete one or more files of a certain type
	//szFileNames = list of filenames to delete, each filename separated by a "\n" character
	//nFilenamesSize = the size of the szFileNames buffer
	//nFileType = one of REMOTE_DATA_FILES, REMOTE_LOG_FILES, or REMOTE_IMAGE_FILES
	REMOTE_COMMAND rc;
	rc.nCommand = DELETE_FILES;
	rc.nNumDataBytes = 5;
	rc.pDataBytes = new unsigned char[5];
	unsigned char dataSizeBytes[4];
	dataSizeBytes[0] = (unsigned char)((nFilenamesSize & 0xff000000) >> 24);
	dataSizeBytes[1] = (unsigned char)((nFilenamesSize & 0x00ff0000) >> 16);
	dataSizeBytes[2] = (unsigned char)((nFilenamesSize & 0x0000ff00) >> 8);
	dataSizeBytes[3] = (unsigned char)(nFilenamesSize & 0x000000ff);
	memcpy(rc.pDataBytes, dataSizeBytes, 4);//send info pertaining to the size of the next block of data to come
	rc.pDataBytes[4] = (unsigned char)nFileType;

	CSingleLock sL(&this->m_sendMutex);
	if (!sL.Lock(1000)) {
		delete[]rc.pDataBytes;
		return -4;
	}
	int nRetval = 0;
	if (!SendSerialCommand(&rc)) {
		nRetval = -5;
	}
	else {
		//now send a larger block of data that corresponds to the filenames to be deleted
		if (!BoatCommand::SendLargeSerialData((int)m_hPort, (unsigned char *)szFileNames, nFilenamesSize, NULL)) {//send large amount of data out serial port, need to get confirmation after sending each chunk
			nRetval = -7;
		}
	}
	delete[]rc.pDataBytes;
	m_sUnDeletedFiles = "";
	if (ReceiveBoatData()) {
		nRetval = 1;
		//copy over any filenames that could not be deleted
		strcpy(szFileNames, m_sUnDeletedFiles.GetBuffer(m_sUnDeletedFiles.GetLength()));
	}
	sL.Unlock();
	return nRetval;
}