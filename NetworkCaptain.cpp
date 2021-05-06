#include "pch.h"
#include "NetworkCaptain.h"
#include <mmsystem.h>

NetworkCaptain* g_nc = NULL; //global NetworkCaptain object that gets created by calling program

extern "C" {
	__declspec (dllexport) int TestNetworkConnection(LPCTSTR sIPAddr, int nNetworkPortNumber, LPCTSTR sRootFolder) {
		//check to see if AMOS is available and can communicate over wireless serial link
		CString ipaddr = CString(sIPAddr);
		CString rootFolder = CString(sRootFolder);
		if (g_nc) {
			delete g_nc;
			g_nc = NULL;
		}
		g_nc = new NetworkCaptain(rootFolder);
		int nWinsockError = 0;
		if (g_nc->ConnectToBoat(ipaddr, nNetworkPortNumber, nWinsockError)) {
			return CONNECT_OK;
		}
		else {
			delete g_nc;
			g_nc = NULL;
		}
		return nWinsockError;
	}

	__declspec (dllexport) int Network_StopRemoteProgram() {
		//send signal to stop remote program running on AMOS
		if (!g_nc) {
			//not connected yet
			return ERROR_NOTCONNECTED;
		}
		if (g_nc->QuitRemoteProgram()) {
			//need to delete the g_nc object since its connection is no longer valid
			delete g_nc;
			g_nc = NULL;
			return COMMAND_OK;
		}
		return ERROR_COMMAND_FAILED;
	}

	__declspec (dllexport) int Network_GetGPSData(char* gpsBuf, int nSize) {
		//request GPS data from AMOS
		CString sGPSData = "";
		if (g_nc) {
			g_nc->RequestGPSPosition(sGPSData);
			if (sGPSData.GetLength() < nSize) {
				strcpy(gpsBuf, sGPSData.GetBuffer(sGPSData.GetLength()));
			}
			return 0;
		}
		return -1;
	}

	__declspec (dllexport) int Network_GetCompassData(char* compassBuf, int nSize) {
		//request compass / inertial data from AMOS
		if (!g_nc) {//must not be connected yet
			return -1;
		}
		CString sCompassData = "";
		bool bGotCompassData = g_nc->RequestCompassData(sCompassData);
		if (!bGotCompassData) {
			return -2;
		}
		if (sCompassData.GetLength() < nSize) {
			strcpy(compassBuf, sCompassData.GetBuffer(sCompassData.GetLength()));
		}
		return 0;
	}

	__declspec (dllexport) int Network_GetSensorData(char *sensorBuf, int nSize) {
		//request sensor and diagnostic data from AMOS
		if (!g_nc) {//must not be connected yet
			return -1;
		}
		CString sSensorData = "";
		bool bGotSensorData = g_nc->RequestSensorData(sSensorData);
		if (!bGotSensorData) {
			return -2;
		}
		if (sSensorData.GetLength() < nSize) {
			strcpy(sensorBuf, sSensorData.GetBuffer(sSensorData.GetLength()));
		}
		return 0;
	}

	__declspec (dllexport) int Network_SendThrusterCommand(int nCommand) {
		//send command to move in a particular direction
		if (!g_nc) {//must not be connected yet
			return -1;
		}
		if (nCommand == THRUST_FORWARDS) {
			if (g_nc->ForwardHo()) {
				return 0;
			}
		}
		else if (nCommand == THRUST_LEFT) {
			if (g_nc->PortHo()) {
				return 0;
			}
		}
		else if (nCommand == THRUST_RIGHT) {
			if (g_nc->StarboardHo()) {
				return 0;
			}
		}
		else if (nCommand == THRUST_BACK) {
			if (g_nc->BackHo()) {
				return 0;
			}
		}
		else if (nCommand == THRUST_STOP) {
			if (g_nc->Stop()) {
				return 0;
			}
		}
		return -2;//unknown command or command failed
	}

	__declspec (dllexport) float Network_GetRudderAngle() {
		//send command to get the current rudder angle
		if (!g_nc) {//must not be connected yet
			return 0;
		}
		PROPELLER_STATE* pState = g_nc->GetCurrentPropState();
		float fAngle = 0;
		if (pState) {
			fAngle = pState->fRudderAngle;
		}
		return fAngle;
	}

	__declspec (dllexport) float Network_GetThrusterPower() {
		//send command to get the current thruster power
		if (!g_nc) {//must not be connected yet
			return 0;
		}
		PROPELLER_STATE* pState = g_nc->GetCurrentPropState();
		float fPower = 0;
		if (pState) {
			fPower = pState->fPropSpeed;
		}
		return fPower;
	}

	__declspec (dllexport) int Network_GetRemoteScriptStatus(char* remoteScriptStatus, int nSize) {
		//request remote script status from AMOS
		CString sRemoteScriptStatus = "";
		if (g_nc) {
			g_nc->RequestRemoteScriptStatus(sRemoteScriptStatus);
			if (sRemoteScriptStatus.GetLength() < nSize) {
				strcpy(remoteScriptStatus, sRemoteScriptStatus.GetBuffer(sRemoteScriptStatus.GetLength()));
			}
			return 0;
		}
		return -1;
	}

	__declspec (dllexport) int Network_GetRemoteFiles(char* remoteFiles, int nSize, int nFileType) {
		//get carriage return separated list of files of a certain type found on AMOS
		if (g_nc) {
			return g_nc->GetRemoteFiles(remoteFiles, nSize, nFileType);
		}
		return -1;
	}

	__declspec (dllexport) int Network_UseRemoteScript(char* remoteFileName, int nSize) {
		//instruct AMOS to start using the specified remote filename script
		if (g_nc) {
			return g_nc->UseRemoteScript(remoteFileName, nSize);
		}
		return -1;
	}

	__declspec (dllexport) int Network_SendFile(char* filename, int nFileNameSize, char* destPath, int nDestPathSize) {
		if (g_nc) {
			return g_nc->SendFile(filename, nFileNameSize, destPath, nDestPathSize);
		}
		return -1;
	}

	__declspec (dllexport) int Network_GetFile(char* remoteFilename, char* destPath) {
		if (g_nc) {
			return g_nc->ReceiveFile(remoteFilename, destPath);
		}
		return -1;
	}

	__declspec (dllexport) int Network_RemoteScriptStepChange(int nStepChange) {
		//change step of currently running file script
		if (g_nc) {
			return g_nc->RemoteScriptStepChange(nStepChange);
		}
		return -1;
	}

	__declspec (dllexport) int Network_SendRTKData(unsigned char* rtkBuf, int nRTKBufSize) {
		if (g_nc) {
			return g_nc->SendRTKData(rtkBuf, nRTKBufSize);
		}
		return -1;
	}
}

NetworkCaptain::NetworkCaptain(CString sRootFolder) : Captain(sRootFolder)
{
	m_connectedSock = INVALID_SOCKET;
	WSADATA wsaData;
    // Initialize Winsock
	m_bCleanedUp=false;
    int iResult = WSAStartup(MAKEWORD(2,2), &wsaData);
    if (iResult != 0) {
		CString sErr;
		sErr.Format("WSAStartup failed with error: %d\n", iResult);
        AfxMessageBox(sErr);
    }
	m_sConnectedIPAddr="";
	m_bConnected=false;
}

NetworkCaptain::~NetworkCaptain(void)
{
	if (m_connectedSock!=INVALID_SOCKET) {
		closesocket(m_connectedSock);
	}
	//clean up windows sockets
	WSACleanup();
}

bool NetworkCaptain::ConnectToBoat(CString sIPAddr, int nPortNumber, int &nSockError) {
	int iResult;
	nSockError = 0;
    m_connectedSock = INVALID_SOCKET;
	m_sConnectedIPAddr="";
    struct addrinfo *result = NULL,
                    *ptr = NULL,
                    hints;
    ZeroMemory( &hints, sizeof(hints) );
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    // Resolve the server address and port
	int nRemotePortNum = nPortNumber;
	CString sRemotePortNum="";
	sRemotePortNum.Format("%d",nRemotePortNum);
    iResult = getaddrinfo(sIPAddr, sRemotePortNum, &hints, &result);
    if ( iResult != 0 ) {
		nSockError = WSAGetLastError();
		CString sErr="";
		sErr.Format("getaddrinfo failed with error: %d\n", iResult);
		cleanup();
        WSACleanup();
        return false;
    }
    // Attempt to connect to an address until one succeeds
    for(ptr=result; ptr != NULL ;ptr=ptr->ai_next) {
        // Create a SOCKET for connecting to server
        m_connectedSock = socket(ptr->ai_family, ptr->ai_socktype, 
            ptr->ai_protocol);
        if (m_connectedSock == INVALID_SOCKET) {
			//CString sErr;
			//sErr.Format("Socket failed with error: %ld\n", WSAGetLastError());
            //WSACleanup();
            continue;
        }
        // Connect to server.
        iResult = connect(m_connectedSock, ptr->ai_addr, (int)ptr->ai_addrlen);
        if (iResult == SOCKET_ERROR) {
			nSockError = WSAGetLastError();
            closesocket(m_connectedSock);
            m_connectedSock = INVALID_SOCKET;
            continue;
        }
        break;
    }
    freeaddrinfo(result);

    if (m_connectedSock == INVALID_SOCKET) {
		nSockError = WSAGetLastError();
		CString sErr="Unable to connect to server!\n";
		cleanup();
        return false;
    }
	m_bConnected=true;
	nSockError = 0;
	m_sConnectedIPAddr = sIPAddr;
	//set receive timeout for socket
	DWORD dwReceiveTimeout = 3000;
	setsockopt(m_connectedSock,SOL_SOCKET,SO_RCVTIMEO,(char *)&dwReceiveTimeout,sizeof(DWORD));
	//send passcode to AMOS
	char *passcodeBuf = PASSCODE_TEXT;
	int nNumPassCodeBytes = strlen(passcodeBuf);
	Sleep(1000);
	if (send(m_connectedSock, passcodeBuf,nNumPassCodeBytes,0)!=nNumPassCodeBytes) {
		//error sending passcode
		nSockError = WSAGetLastError();
		cleanup();
		return false;
	}

	//query supported data (i.e. find out what types of data the boat is capable of collecting)
	FindSupportedSensors();
	return true;
}

void NetworkCaptain::cleanup() {
	if (m_bCleanedUp) {
		return;
	}
	WSACleanup();
	m_bCleanedUp=true;
}

CString NetworkCaptain::GetIPAddr() {//return the IP address of the remote boat that we are connected to
	return m_sConnectedIPAddr;
}

bool NetworkCaptain::ForwardHo() {//accelerate forward
	PROPELLER_STATE propState;
	if (!Captain::ForwardHo(&propState)) {
		return false;
	}
	REMOTE_COMMAND rc;
	rc.nCommand = THRUST_ON;
	rc.nNumDataBytes = sizeof(PROPELLER_STATE);
	rc.pDataBytes = (unsigned char *)&propState;
	return SendNetworkCommand(&rc);
}

bool NetworkCaptain::StarboardHo() {//turn to right
	PROPELLER_STATE propState;
	if (!Captain::StarboardHo(&propState)) {
		return false;
	}
	REMOTE_COMMAND rc;
	rc.nCommand = THRUST_ON;
	rc.nNumDataBytes = sizeof(PROPELLER_STATE);
	rc.pDataBytes = (unsigned char *)&propState;
	return SendNetworkCommand(&rc);
}

bool NetworkCaptain::PortHo() {//turn to left
	PROPELLER_STATE propState;
	if (!Captain::PortHo(&propState)) {
		return false;
	}
	REMOTE_COMMAND rc;
	rc.nCommand = THRUST_ON;
	rc.nNumDataBytes = sizeof(PROPELLER_STATE);
	rc.pDataBytes = (unsigned char *)&propState;
	return SendNetworkCommand(&rc);
}

bool NetworkCaptain::BackHo() {//slow down boat
	PROPELLER_STATE propState;
	if (!Captain::BackHo(&propState)) {
		return false;
	}
	REMOTE_COMMAND rc;
	rc.nCommand = THRUST_ON;
	rc.nNumDataBytes = sizeof(PROPELLER_STATE);
	rc.pDataBytes = (unsigned char *)&propState;
	return SendNetworkCommand(&rc);
}

//SendNetworkCommand: sends a remote command out over the network
//pRC = REMOTE_COMMAND structure describing the command to be sent to the boat
//returns true if command was successfully sent, false otherwise
bool NetworkCaptain::SendNetworkCommand(REMOTE_COMMAND *pRC) {
	if (m_connectedSock == INVALID_SOCKET) {
		return false;//not connected yet
	}
	//first send command bytes
	unsigned char commandBytes[4];
	commandBytes[0] = (unsigned char)((pRC->nCommand&0xff000000)>>24);
	commandBytes[1] = (unsigned char)((pRC->nCommand&0x00ff0000)>>16);
	commandBytes[2] = (unsigned char)((pRC->nCommand&0x0000ff00)>>8);
	commandBytes[3] = (unsigned char)(pRC->nCommand&0x000000ff);
	int nNumSent = send(m_connectedSock,(char *)commandBytes,4,0);
	if (nNumSent!=4) {
		DisplayLastSockError();
		return false;
	}
	if (pRC->pDataBytes) {//need to send data along with command
		unsigned char dataSizeBytes[4];
		dataSizeBytes[0] = (unsigned char)((pRC->nNumDataBytes&0xff000000)>>24);
		dataSizeBytes[1] = (unsigned char)((pRC->nNumDataBytes&0x00ff0000)>>16);
		dataSizeBytes[2] = (unsigned char)((pRC->nNumDataBytes&0x0000ff00)>>8);
		dataSizeBytes[3] = (unsigned char)(pRC->nNumDataBytes&0x000000ff);
		nNumSent = send(m_connectedSock,(char *)dataSizeBytes,4,0);
		if (nNumSent!=4) {
			DisplayLastSockError();
			return false;
		}
		//send actual data
		nNumSent = send(m_connectedSock,(char *)pRC->pDataBytes,pRC->nNumDataBytes,0);
		if (nNumSent!=pRC->nNumDataBytes) {
			DisplayLastSockError();
			return false;
		}
	}
	if (pRC->nCommand != RTK_CORRECTION) {//most commands (except RTK_CORRECTION) require confirmation from remote device
		//get confirmation from remote boat
		unsigned char inBuf[4];
		int nNumReceived = recv(m_connectedSock, (char*)inBuf, 4, 0);
		if (nNumReceived == 0) {
			AfxMessageBox("Socket connection has been closed.");
			return false;
		}
		else if (nNumReceived == SOCKET_ERROR) {
			//DisplayLastSockError();
			//error occurred, flusn rx buffer
			int nLastError = WSAGetLastError();
			CString sError = "";
			sError.Format("Socket error: %d\n", nLastError);
			TRACE(sError);
			this->FlushRXBuffer(m_connectedSock);
			return false;
		}
		else if (nNumReceived < 4) {
			CString sErrMsg;
			sErrMsg.Format("Only %d of 4 bytes received from boat.", nNumReceived);
			AfxMessageBox(sErrMsg);
			return false;
		}
		int nResponse = (inBuf[0] << 24) + (inBuf[1] << 16) + (inBuf[2] << 8) + inBuf[3];
		if (nResponse != pRC->nCommand) {
			//AfxMessageBox("Error, invalid confirmation response from boat.");
			//error occurred, flusn rx buffer
			this->FlushRXBuffer(m_connectedSock);
			return false;
		}
	}
	return true;//command was sent successfully
}

bool NetworkCaptain::RequestGPSPosition(CString &sGPSPosition) {//query the boat for its GPS position and return the corresponding GPS string in sGPSPosition, returns true if successful
	sGPSPosition="";
	REMOTE_COMMAND rc;
	rc.nCommand = GPS_DATA_PACKET;
	rc.nNumDataBytes = 0;
	rc.pDataBytes = NULL;
	if (SendNetworkCommand(&rc)) {
		if (!ReceiveBoatData()) {
			return false;
		}
	}
	else {
		return false;
	}
	if (!m_gpsTime) {//no GPS readings obtained yet
		return true;
	}
	sGPSPosition = FormatGPSData();
	return true;
}

bool NetworkCaptain::RequestCompassData(CString &sCompassData) {//query the boat for its compass data (heading, roll, and pitch angles, as well as temperature). Returns true if successful
	sCompassData="";
	REMOTE_COMMAND rc;
	rc.nCommand = COMPASS_DATA_PACKET;
	rc.nNumDataBytes = 0;
	rc.pDataBytes = NULL;
	if (SendNetworkCommand(&rc)) {
		if (!ReceiveBoatData()) {
			return false;
		}
	}
	else {
		return false;
	}
	sCompassData = FormatCompassData();
	return true;
}

//ReceiveBoatData: receive network data from the boat
//returns true if boat data could be successfully received and processed
bool NetworkCaptain::ReceiveBoatData() {//receive data from boat over network
	if (m_connectedSock == INVALID_SOCKET) {
		return false;//not connected yet
	}
	int nNumBytesToReceive=2*sizeof(int);
	//first receive the data type from the boat and the number of data bytes
	int nDataType=0;
	int nDataSize=0;
	char *inBuf = new char[nNumBytesToReceive];
	int nNumReceived = recv(m_connectedSock,inBuf,nNumBytesToReceive,0);
	if (nNumReceived==0) {
		delete []inBuf;
		//error occurred, flush rx buffer
		this->FlushRXBuffer(m_connectedSock);
		return false;//timeout trying to receive data
	}
	else if (nNumReceived==SOCKET_ERROR) {
		//DisplayLastSockError();
		delete []inBuf;
		//error occurred, flush rx buffer
		this->FlushRXBuffer(m_connectedSock);
		return false;
	}
	else if (nNumReceived<nNumBytesToReceive) {
		//CString sErrMsg;
		//sErrMsg.Format("Only %d of %d bytes received from boat.",nNumReceived,nNumBytesToReceive);
		//AfxMessageBox(sErrMsg);
		delete []inBuf;
		//error occurred, flush rx buffer
		this->FlushRXBuffer(m_connectedSock);
		return false;
	}
	memcpy(&nDataType,inBuf,sizeof(int));
	memcpy(&nDataSize,&inBuf[sizeof(int)],sizeof(int));
	delete []inBuf;
	//create structure for receiving boat data
	BOAT_DATA *pBoatData = BoatCommand::CreateBoatData(nDataType);
	if (!pBoatData) {
		return false;//unable to create this data type
	}
	//special case for sensor types
	if (nDataType==SENSOR_TYPES_INFO) {
		pBoatData->nDataSize=m_nNumSensorsAvailable*sizeof(int);
		pBoatData->dataBytes = new unsigned char[pBoatData->nDataSize];
	}
	if (pBoatData->nDataSize!=nDataSize) {
		return false;//unexpected data size
	}
	nNumBytesToReceive = pBoatData->nDataSize+1;
	inBuf = new char[nNumBytesToReceive];
	nNumReceived = recv(m_connectedSock,inBuf,nNumBytesToReceive,0);
	if (nNumReceived==0) {
		//timeout trying to receive data
		BoatCommand::DeleteBoatData(pBoatData);
		delete []inBuf;
		return false;
	}
	else if (nNumReceived==SOCKET_ERROR) {
		//DisplayLastSockError();
		BoatCommand::DeleteBoatData(pBoatData);
		delete []inBuf;
		//error occurred, flush rx buffer
		this->FlushRXBuffer(m_connectedSock);
		return false;
	}
	else if (nNumReceived<nNumBytesToReceive) {
		CString sErrMsg;
		sErrMsg.Format("Only %d of %d bytes received from boat.",nNumReceived,nNumBytesToReceive);
		//AfxMessageBox(sErrMsg);
		BoatCommand::DeleteBoatData(pBoatData);
		delete []inBuf;
		//error occurred, flush rx buffer
		this->FlushRXBuffer(m_connectedSock);
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
			unsigned char *videoBytes = new unsigned char[nImageBytes];
			nNumReceived = ReceiveLargeDataChunk(m_connectedSock,(char *)videoBytes,nImageBytes);
			if (nNumReceived==nImageBytes) {//image bytes received successfully, save to temporary image file
				CString sTmpImgFilename="";
				CString sTmp = TMP_IMAGE_FILENAME;
				sTmpImgFilename.Format("%s\\%s",m_sRootFolder,sTmp);
				CFile imgFile;
				if (imgFile.Open(sTmpImgFilename,CFile::modeCreate|CFile::modeReadWrite|CFile::shareDenyNone)) {
					imgFile.Write(videoBytes,(UINT)nNumReceived);
					imgFile.Close();
				}
			}
			delete []videoBytes;
		}
	}
	else if (pBoatData->nPacketType == LIST_REMOTE_SCRIPTS) {
		bSkipProcessData = true;
		int nNumBytes = 0;
		memcpy(&nNumBytes, pBoatData->dataBytes, sizeof(int));
		//make sure number of bytes is not too large
		if (nNumBytes < 4096) {
			bool bRetval = DownloadBytes(m_connectedSock, nNumBytes, LIST_REMOTE_SCRIPTS);
			delete[]inBuf;
			return bRetval;
		}
	}
	else if (pBoatData->nPacketType == LIST_REMOTE_DATA) {
		bSkipProcessData = true;
		int nNumBytes = 0;
		memcpy(&nNumBytes, pBoatData->dataBytes, sizeof(int));
		//make sure number of bytes is not too large
		if (nNumBytes < 4096) {
			bool bRetval = DownloadBytes(m_connectedSock, nNumBytes, LIST_REMOTE_DATA);
			delete[]inBuf;
			return bRetval;
		}
	}
	else if (pBoatData->nPacketType == LIST_REMOTE_LOG) {
		bSkipProcessData = true;
		int nNumBytes = 0;
		memcpy(&nNumBytes, pBoatData->dataBytes, sizeof(int));
		//make sure number of bytes is not too large
		if (nNumBytes < 4096) {
			bool bRetval = DownloadBytes(m_connectedSock, nNumBytes, LIST_REMOTE_LOG);
			delete[]inBuf;
			return bRetval;
		}
	}
	else if (pBoatData->nPacketType == LIST_REMOTE_IMAGE) {
		bSkipProcessData = true;
		int nNumBytes = 0;
		memcpy(&nNumBytes, pBoatData->dataBytes, sizeof(int));
		bool bRetval = DownloadBytes(m_connectedSock, nNumBytes, LIST_REMOTE_IMAGE);
		delete[] inBuf;
		return bRetval;
	}
	else if (pBoatData->nPacketType == FILE_RECEIVE) {
		bSkipProcessData = true;
		int nNumBytes = 0;
		memcpy(&nNumBytes, pBoatData->dataBytes, sizeof(int));
		//make sure number of bytes is not too large
		if (nNumBytes < 100000000) {
			bool bRetval = DownloadBytes(m_connectedSock, nNumBytes, FILE_RECEIVE);
			delete[]inBuf;
			return bRetval;
		}
	}
	delete []inBuf;
	if (!bSkipProcessData&&!ProcessBoatData(pBoatData)) {
		return false;
	}
	BoatCommand::DeleteBoatData(pBoatData);
	return true;//command was sent successfully
}

bool NetworkCaptain::Stop() {//stop thrusters
	PROPELLER_STATE propState;
	if (!Captain::Stop(&propState)) {
		return false;
	}
	REMOTE_COMMAND rc;
	rc.nCommand = THRUST_ON;
	rc.nNumDataBytes = sizeof(PROPELLER_STATE);
	rc.pDataBytes = (unsigned char *)&propState;
	return SendNetworkCommand(&rc);

}


bool NetworkCaptain::FindSupportedSensors() {//query supported data (i.e. find out what types of data the boat is capable of collecting)
	REMOTE_COMMAND rc;
	rc.nCommand = SUPPORTED_SENSOR_DATA;
	rc.nNumDataBytes = 0;
	rc.pDataBytes = NULL;
	if (SendNetworkCommand(&rc)) {
		if (!ReceiveBoatData()) {
			return false;
		}
		//receive info for the type of each sensor
		if (m_nNumSensorsAvailable>0) {
			if (!ReceiveBoatData()) {
				return false;
			}
		}	
	}
	else {
		return false;
	}
	return true;
}

bool NetworkCaptain::RequestSensorData(CString &sSensorData) {//query the boat for any sensor data that it might have (eg: water temperature, pH, turbidity, etc.)
	sSensorData="";
	for (int i=0;i<this->m_nNumSensorsAvailable;i++) {
		if (this->m_sensorTypes[i]==WATER_TEMP_DATA) {
			CString sWaterTemp="";
			REMOTE_COMMAND rc;
			rc.nCommand = WATER_TEMP_DATA_PACKET;
			rc.nNumDataBytes = 0;
			rc.pDataBytes = NULL;
			if (SendNetworkCommand(&rc)) {
				if (!ReceiveBoatData()) {
					return false;
				}
			}
			else {
				return false;
			}
			sWaterTemp = FormatWaterTemp();//format the received water temperature data
			sSensorData+=(sWaterTemp+", ");
		}
		else if (this->m_sensorTypes[i]==PH_DATA) {
			CString sWaterPH="";
			REMOTE_COMMAND rc;
			rc.nCommand = WATER_PH_DATA_PACKET;
			rc.nNumDataBytes = 0;
			rc.pDataBytes = NULL;
			if (SendNetworkCommand(&rc)) {
				if (!ReceiveBoatData()) {
					return false;
				}
			}
			else {
				return false;
			}
			sWaterPH = FormatWaterPH();//format the received water temperature data
			sSensorData+=(sWaterPH+", ");
		}
		else if (this->m_sensorTypes[i]==WATER_TURBIDITY) {
			CString sWaterTurbidity="";
			REMOTE_COMMAND rc;
			rc.nCommand = WATER_TURBIDITY_DATA_PACKET;
			rc.nNumDataBytes = 0;
			rc.pDataBytes = NULL;
			if (SendNetworkCommand(&rc)) {
				if (!ReceiveBoatData()) {
					return false;
				}
			}
			else {
				return false;
			}
			sWaterTurbidity = FormatWaterTurbidity();//format the received water temperature data
			sSensorData+=(sWaterTurbidity+", ");
		}
	}
	CString sDiagData = FormatDiagnosticsData();//format the received diagnostics data
	sSensorData+=(sDiagData+", ");

	int nSensorDataLength = sSensorData.GetLength();
	if (nSensorDataLength>0) {//get rid of trailing comma
		sSensorData = sSensorData.Left(nSensorDataLength-2);
	}
	return true;
}

//RequestVideoImage: request an image capture with feature markings in it from the boat 
//requestedImg = pointer to CImage object that is used to store the returned image
//nFeatureThreshold = feature threshold value from 0 to 255 that is used to control how feature markings
//are determined in the image. nFeatureThreshold==0 disables or removes feature markers from the image.
bool NetworkCaptain::RequestVideoImage(CImage *requestedImg, int nFeatureThreshold) {
	if (!requestedImg) return false;
	m_bRequestingVideoImage = true;
	REMOTE_COMMAND rc;
	rc.nCommand = VIDEO_DATA_PACKET;
	rc.nNumDataBytes = sizeof(int);
	rc.pDataBytes = new unsigned char[rc.nNumDataBytes];
	memcpy(rc.pDataBytes,&nFeatureThreshold,rc.nNumDataBytes);
	if (SendNetworkCommand(&rc)) {
		if (!ReceiveBoatData()) {
			delete []rc.pDataBytes;
			m_bRequestingVideoImage = false;
			return false;
		}
	}
	else {
		delete []rc.pDataBytes;
		m_bRequestingVideoImage = false;
		return false;
	}
	delete []rc.pDataBytes;
	CopyReceivedImage(requestedImg);
	m_bRequestingVideoImage = false;
	return true;
}

//ReceiveLargeDataChunk: tries to receive a large chunk of data over network socket connection
//sock = the socket connection which the function will use to try to receive data
//rxBytes = buffer used to store the received data
//nNumToReceive = the number of bytes to receive over the socket connection
//returns the number of bytes successfully read (and stored in rxBytes) over the socket connection
int NetworkCaptain::ReceiveLargeDataChunk(SOCKET sock, char * rxBytes, int nNumToReceive) {
	const int MAX_ALLOWED_TIMEOUTS = 5;
	int nNumRemaining = nNumToReceive;
	int nNumReceived=0;
	int nTimeoutCount=0;
	do {
		int nRX = recv(sock,&rxBytes[nNumReceived],nNumRemaining,0);
		if (nRX>0) {
			nNumRemaining-=nRX;
			nNumReceived+=nRX;
		}
		else {//timeout or error occurred
			nTimeoutCount++;
		}
	} while (nNumRemaining>0&&nTimeoutCount<MAX_ALLOWED_TIMEOUTS);
	return nNumReceived;	
}

void NetworkCaptain::FlushRXBuffer(SOCKET sock) {//read receive socket until no more data can be read (typically called if some unexpected data arrives and communications need to get back on track)
	char rxBuf[1024];
	int nNumReceived = 0;
	do {
		nNumReceived  = recv(sock,rxBuf,1024,0);
	} while (nNumReceived>0);
}

bool NetworkCaptain::IsConnected() {//return true if currently connected to the boat server
	return this->m_bConnected;
}

/**
 * @brief send command to quit the remote program running on AMOS
 * 
 * @return true if the command to quit the remote program was received successfully
 */
bool NetworkCaptain::QuitRemoteProgram() {
	REMOTE_COMMAND rc;
	rc.nCommand = QUIT_PROGRAM;
	rc.nNumDataBytes = 0;
	rc.pDataBytes = NULL;
	return SendNetworkCommand(&rc);
}

/**
 * @brief query the boat as to the name of the remote script currently running (if any) and what step the script is currently on
 *
 * @param sRemoteScriptStatus the status of the script that is currently running on the boat, ex: "MyAMOSScript.txt, 3 of 20". It indicates the name of the script currently running (if any), the current step number, and the total number of steps
 * @return true if the function was able to acquire the remote script status info successfully from the boat
 * @return false if there was a problem acquiring the remote script status
 */
bool NetworkCaptain::RequestRemoteScriptStatus(CString& sRemoteScriptStatus) {//query the boat as to the name of the remote script currently running (if any) and what step the script is currently on
	const int MAX_TRIES = 3;
	sRemoteScriptStatus = "";
	REMOTE_COMMAND rc;
	rc.nCommand = SCRIPT_STATUS_PACKET;
	rc.nNumDataBytes = 0;
	rc.pDataBytes = NULL;
	int i = 0;
	while (i < MAX_TRIES) {
		i++;
		if (SendNetworkCommand(&rc)) {
			//test	
			TRACE("try to receive remote script status...\n");
			//end test
			if (!ReceiveBoatData()) {
				if (i < MAX_TRIES) {
					Sleep(1000);
					continue;
				}
				return false;
			}
		}
		else {
			if (i < MAX_TRIES) {
				Sleep(1000);
				continue;
			}
			return false;
		}
		break;
	}
	sRemoteScriptStatus = FormatRemoteScriptStatus();
	return true;
}


/**
 * @brief send command to change the step of the currently executing remote program
 *
 * @param nStepChange the number of steps to advance (positive) or go back (negative) in the current remote script
 * @return 0 if the command was sent successfully sent, or negative if an error occurred.
 */
int NetworkCaptain::RemoteScriptStepChange(int nStepChange) {
	REMOTE_COMMAND rc;
	rc.nCommand = SCRIPT_STEP_CHANGE;
	rc.nNumDataBytes = sizeof(int);
	rc.pDataBytes = new unsigned char[rc.nNumDataBytes];

	memcpy(rc.pDataBytes, &nStepChange, rc.nNumDataBytes);
	int nRetval = 0;
	if (!SendNetworkCommand(&rc)) {
		nRetval = -1;
	}
	delete[]rc.pDataBytes;
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
int NetworkCaptain::GetRemoteFiles(char* remoteFiles, int nBufSize, int nFileType) {//get the names of remote files on AMOS of a particular file type
	if (nFileType != REMOTE_SCRIPT_FILES&&nFileType!=REMOTE_DATA_FILES&&nFileType!=REMOTE_LOG_FILES&&nFileType!=REMOTE_IMAGE_FILES) {//the only file types supported for now
		return -1;
	}
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

	int nRetval = 0;
	if (!SendNetworkCommand(&rc)) {
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
	return nRetval;
}

/**
 * @brief instruct AMOS to start using a particular remote script filename.
 *
 * @param remoteFileName pointer to a char buffer that holds the name of the remote script file.
 * @param nSize the size of the buffer pointed to by remoteFileName.
 * @return 0 if the command was executed successfully, or negative if an error occurred.
 */
int NetworkCaptain::UseRemoteScript(char* remoteFileName, int nSize) {
	REMOTE_COMMAND rc;
	rc.nCommand = USE_REMOTE_SCRIPT;
	rc.nNumDataBytes = sizeof(int) + strlen(remoteFileName);
	rc.pDataBytes = new unsigned char[rc.nNumDataBytes];
	memcpy(rc.pDataBytes, &nSize, sizeof(int));
	int nRemoteFilenameLength = strlen(remoteFileName);
	if (nRemoteFilenameLength > 0) {
		memcpy(&rc.pDataBytes[sizeof(int)], remoteFileName, nRemoteFilenameLength);
	}

	int nRetval = 0;
	if (!SendNetworkCommand(&rc)) {
		nRetval = -3;
	}
	delete[]rc.pDataBytes;
	return nRetval;
}

bool NetworkCaptain::DownloadBytes(SOCKET sock, int nNumBytes, int nDataType) {//download a largeish number of bytes at once
	bool bRetval = false;
	if (nDataType == LIST_REMOTE_SCRIPTS) {
		char* fileNameBytes = new char[nNumBytes + 1];
		int nNumReceived = ReceiveLargeDataChunk(sock, fileNameBytes, nNumBytes);
		if (nNumReceived == nNumBytes) {//bytes received successfully, save to m_sRemoteScriptsAvailable
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
		int nNumReceived = ReceiveLargeDataChunk(sock, fileNameBytes, nNumBytes);
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
	else if (nDataType == LIST_REMOTE_LOG) {
		char* fileNameBytes = new char[nNumBytes + 1];
		int nNumReceived = ReceiveLargeDataChunk(sock, fileNameBytes, nNumBytes);
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
	else if (nDataType == LIST_REMOTE_IMAGE) {
		char* fileNameBytes = new char[nNumBytes + 1];
		int nNumReceived = ReceiveLargeDataChunk(sock, fileNameBytes, nNumBytes);
		if (nNumReceived == nNumBytes) {//bytes received successfully, save to m_sRemoteImageFilesAvailable
			fileNameBytes[nNumReceived] = 0;//null terminate
			m_sRemoteRemoteImageFilesAvailable = CString(fileNameBytes);
			bRetval = true;
		}
		else {
			m_sRemoteRemoteImageFilesAvailable = "";
		}
		delete[]fileNameBytes;
	}
	else if (nDataType == FILE_RECEIVE) {
		if (!m_fileDestPath) {
			return false;
		}
		unsigned char* fileBytes = new unsigned char[nNumBytes + 1];
		int nNumReceived = ReceiveLargeDataChunk(sock, (char *)fileBytes, nNumBytes);
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
		delete[] fileBytes;
	}
	return bRetval;
}

/**
 * @brief send data from a file over the network connection
 *
 * @param filename pointer to a char buffer that holds the name of the file to be uploaded
 * @nFileNameSize the size of the buffer pointed to by filename
 * @destPath the destination path on the remote device, where the contents of the file will be copied to (cannot be NULL)
 * @nDestPathSize the size of the buffer pointed to by destPath (in bytes)
 * @return 0 if the command was executed successfully, or negative if an error occurred.
 */
int NetworkCaptain::SendFile(char* filename, int nFileNameSize, char* destPath, int nDestPathSize) {
	if (!filename || nFileNameSize <= 0 || !destPath) {
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
	int nNumRead = (int)fileToSend.Read(&fileBytes[sizeof(int)], nFileLength);
	if (nNumRead < nFileLength) {
		delete[]fileBytes;
		return -6;
	}
	memcpy(&fileBytes[sizeof(int) + nFileLength], destPath, nDestPathSize);
	REMOTE_COMMAND rc;
	rc.nCommand = FILE_TRANSFER;
	rc.nNumDataBytes = sizeof(int);
	rc.pDataBytes = new unsigned char[rc.nNumDataBytes];
	unsigned char dataSizeBytes[4];
	dataSizeBytes[0] = (unsigned char)((nNumDataBytesToSend & 0xff000000) >> 24);
	dataSizeBytes[1] = (unsigned char)((nNumDataBytesToSend & 0x00ff0000) >> 16);
	dataSizeBytes[2] = (unsigned char)((nNumDataBytesToSend & 0x0000ff00) >> 8);
	dataSizeBytes[3] = (unsigned char)(nNumDataBytesToSend & 0x000000ff);
	memcpy(rc.pDataBytes, dataSizeBytes, 4);//send info pertaining to the size of the next block of data to come

	int nRetval = 0;
	if (!SendNetworkCommand(&rc)) {
		nRetval = -5;
	}
	else {
		//now send a larger block of data that corresponds to the actual file contents and destination path
		if (send(m_connectedSock,(char *)fileBytes, nNumDataBytesToSend, NULL)!=nNumDataBytesToSend) {//send large amount of data over network connection
			nRetval = -7;
		}
	}
	delete[]rc.pDataBytes;
	delete[]fileBytes;
	
	return nRetval;
}

/**
 * @brief receive a copy of a remote file on AMOS
 *
 * @param remoteFilename the path to the remote filename on AMOS that we want to download.
 * @param destPath the local path of the copied file that was downloaded (should get downloaded to the current folder)
  * @return 0 if the command was executed successfully, or negative if an error occurred.
 */
int NetworkCaptain::ReceiveFile(char* remoteFilename, char* destPath) {//receive a remote file on AMOS
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

	int nRetval = 0;
	if (!SendNetworkCommand(&rc)) {
		nRetval = -2;
	}
	else {
		//now send a block of data that corresponds to the path of the remote file
		DWORD dwNumWritten = 0;
		if (send(m_connectedSock, remoteFilename, nRemoteFilenameLength, NULL)!=nRemoteFilenameLength) {
			nRetval = -3;
		}
		//copy pointer to file destination path
		m_fileDestPath = destPath;
		//try to receive data from boat
		if (!ReceiveBoatData()) {
			nRetval = -4;
		}
	}
	delete[]rc.pDataBytes;
	return nRetval;
}

/**
 * @brief send instruction to AMOS to refresh a particular settings type based on the current contents of the prefs.txt file
 *
 * @param nSettingsType the particular type of settings that we want to refresh (see CommandList.h for list of possible settings types)
 * @return 0 if the command was executed successfully, or negative if an error occurred.
 */
int NetworkCaptain::RefreshSettings(int nSettingsType) {//send instruction to AMOS to refresh a particular settings type based on the current contents of the prefs.txt file
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
	int nRetval = 0;
	if (!SendNetworkCommand(&rc)) {
		nRetval = -1;
	}
	delete[]rc.pDataBytes;
	return nRetval;
}


/**
 * @brief send RTK correction data to AMOS (used for making differential GPS measurements, i.e. getting improved accuracy and precision)
 *
 * @param rtkBuf the buffer of RTK bytes received from the RTK base station
 * @param nRTKBufSize the size of the buffer pointed to by rtkBuf
 * @return 0 if the command was executed successfully, or negative if an error occurred. This function just sends the bytes and does not check for any sort of a response from AMOS
 */
int NetworkCaptain::SendRTKData(unsigned char* rtkBuf, int nRTKBufSize) {
	REMOTE_COMMAND rc;
	rc.nCommand = RTK_CORRECTION;
	rc.nNumDataBytes = nRTKBufSize;
	rc.pDataBytes = new unsigned char[rc.nNumDataBytes];
	memcpy(rc.pDataBytes, rtkBuf, nRTKBufSize);
	int nRetval = 0;
	if (!SendNetworkCommand(&rc)) {
		nRetval = -1;
	}
	delete[]rc.pDataBytes;
	return nRetval;
}