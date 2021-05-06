#include "pch.h"
#include "Captain.h"
#include "Util.h"
#include <math.h>

extern "C" {
	__declspec (dllexport) int EncryptStr(char* destBuf, char *sourceBuf, int nSize) {
		//encrypt some text
		return BoatCommand::Encrypt(destBuf, sourceBuf, nSize);
	}

	__declspec (dllexport) int DecryptStr(char* destBuf, char* sourceBuf, int nSize) {
		//encrypt some text
		return BoatCommand::Decrypt(destBuf, sourceBuf, nSize);
	}
}


Captain::Captain(CString sRootFolder)
{
	m_sRootFolder = sRootFolder;
	m_fileDestPath = NULL;
	m_sRemoteScriptsAvailable = "";
	m_sRemoteDataAvailable = "";
	m_sRemoteRemoteImageFilesAvailable = "";
	m_sRemoteLogsAvailable = "";
	m_sGPSData = "";
	m_currentPropState.fRudderAngle=0;
	m_currentPropState.fPropSpeed=0;
	m_bLeakDetected=false;
	m_nSolarCharging=0;
	m_fCurrentDraw=0;
	m_dLatitude=0.0;
	m_dLongitude=0.0;
	m_fWaterTemp=0;
	m_fBatteryVoltage=0;
	m_fWirelessRXPower=0;
	m_fPH=0;
	m_fWaterTurbidity=0;
	m_fHumidityCPU = 0;
	m_fHumidityTempCPU = 0;
	m_fHumidityBatt = 0;
	m_fHumidityTempBatt = 0;
	m_bRequestingVideoImage = false;
	m_sRemoteScriptName = "";
	m_nRemoteScriptStepIndex = 0;//the index (0-based) of the step currently executing on the remote script of the boat
	m_nTotalNumberScriptSteps = 0;//the total # of steps in the remote script of the boat
	
	m_gpsTime=NULL;//the last known timestamp of a GPS reading from the boat
	m_nNumSensorsAvailable=0;//the total number of sensors that the boat has available
	m_sensorTypes=NULL;//the types of sensors that the boat has available, see SensorDataFile.h for a list of the supported sensor types
	memset(&m_compassData,0,sizeof(IMU_DATASAMPLE));
}

Captain::~Captain(void)
{
	if (m_gpsTime) {
		delete m_gpsTime;
		m_gpsTime=NULL;
	}
	if (m_sensorTypes) {
		delete []m_sensorTypes;
		m_sensorTypes=NULL;
	}
}

bool Captain::ForwardHo(PROPELLER_STATE *pPropState) {//move forward, increase forward speed (up to limit)
	IncrementPropSpeed();
	CopyCurrentPropState(pPropState);
	return true;
}

bool Captain::StarboardHo(PROPELLER_STATE *pPropState) {//move to right, increase right turning speed (up to limit)
	IncrementRudderAngle();
	CopyCurrentPropState(pPropState);
	return true;
}

bool Captain::PortHo(PROPELLER_STATE *pPropState) {//move to left, increase left turning speed (up to limit)
	DecrementRudderAngle();
	CopyCurrentPropState(pPropState);
	return true;
}

bool Captain::BackHo(PROPELLER_STATE *pPropState) {//move backward, increase backward speed (up to limit)
	DecrementPropSpeed();
	CopyCurrentPropState(pPropState);
	return true;
}

PROPELLER_STATE * Captain::GetCurrentPropState() {//returns the current state of the propellers as a pointer to a PROPELLER_STATE structure
	return &m_currentPropState;
}

void Captain::CopyCurrentPropState(PROPELLER_STATE *pPropState) {//copies the current propeller state into pPropState
	memcpy(pPropState,&m_currentPropState,sizeof(PROPELLER_STATE));
}

void Captain::IncrementPropSpeed() {//increase propeller speed
	float fRudderAngle = m_currentPropState.fRudderAngle;
	float fPropSpeed = m_currentPropState.fPropSpeed;
	fPropSpeed++;
	if (fPropSpeed>MAX_SPEED) fPropSpeed = MAX_SPEED;
	m_currentPropState.fPropSpeed = fPropSpeed;
	m_currentPropState.fRudderAngle = fRudderAngle;
}

void Captain::IncrementRudderAngle() {//increase rudder angle
	float fRudderAngle = m_currentPropState.fRudderAngle;
	float fPropSpeed = m_currentPropState.fPropSpeed;
	fRudderAngle++;
	if (fRudderAngle>MAX_RUDDER_ANGLE) {
		fRudderAngle = MAX_RUDDER_ANGLE;
	}
	m_currentPropState.fRudderAngle = fRudderAngle;
	m_currentPropState.fPropSpeed = fPropSpeed;
}

void Captain::DecrementRudderAngle() {//decrease rudder angle
	float fRudderAngle = m_currentPropState.fRudderAngle;
	float fPropSpeed = m_currentPropState.fPropSpeed;
	fRudderAngle--;
	if (fRudderAngle<MIN_RUDDER_ANGLE) {
		fRudderAngle = MIN_RUDDER_ANGLE;
	}
	m_currentPropState.fRudderAngle = fRudderAngle;
	m_currentPropState.fPropSpeed = fPropSpeed;
}

void Captain::DecrementPropSpeed() {//decrease propeller speed
	float fRudderAngle = m_currentPropState.fRudderAngle;
	float fPropSpeed = m_currentPropState.fPropSpeed;
	fPropSpeed--;
	if (fPropSpeed<0) {
		fPropSpeed = 0;
	}
	m_currentPropState.fRudderAngle = fRudderAngle;
	m_currentPropState.fPropSpeed = fPropSpeed;
}

/**
 * @brief display the most recent Windows Sockets error code
 * 
 */
void Captain::DisplayLastSockError() {
	int nLastWinsockError = WSAGetLastError();
	CString sWinsockErrMsg;
	sWinsockErrMsg.Format("Windows socket error: %d\n",nLastWinsockError);
	AfxMessageBox(sWinsockErrMsg);
}

/**
 * @brief display the specified error message (sErrMsg) plus display the most recent general Windows error code
 * 
 */
void Captain::DisplayLastError(CString sErrMsg) {
	DWORD dwLastError = GetLastError();
	CString sMsg;
	sMsg.Format("%s Windows error code: %u\n",sErrMsg, dwLastError);
	AfxMessageBox(sMsg);
}

CString Captain::FormatGPSData() {//format the current GPS data as a string, ex: 45.334523� N, 62.562533� W, 2018-06-18, 17:23:00
	char szDegSymbol[2];
	szDegSymbol[0] = (char)176;
	szDegSymbol[1] = 0;
	CString sRetval="";
	CString sSN = "N";
	if (this->m_dLatitude<0) sSN = "S";
	CString sEW = "E";
	if (this->m_dLongitude<0) sEW = "W";
	
	if (this->m_gpsTime) {
		sRetval.Format("%.6f%s %s, %.6f%s %s, %d-%02d-%02d, %02d:%02d:%02d",fabs(m_dLatitude),szDegSymbol,sSN,fabs(m_dLongitude),szDegSymbol,sEW,m_gpsTime->GetYear(),
			m_gpsTime->GetMonth(),m_gpsTime->GetDay(),m_gpsTime->GetHour(),m_gpsTime->GetMinute(),m_gpsTime->GetSecond());
	}
	return sRetval;
}

bool Captain::ProcessBoatData(BOAT_DATA *pBoatData) {//process data from boat, return true if boat data could be successfully processed
	if (pBoatData->nPacketType==GPS_DATA_PACKET) {
		if (pBoatData->nDataSize!=sizeof(GPS_DATA)) {
			return false;
		}
		memcpy(&m_dLatitude,pBoatData->dataBytes,sizeof(double));
		memcpy(&m_dLongitude,&pBoatData->dataBytes[sizeof(double)],sizeof(double));
		time_t gpsTime;
		memcpy(&gpsTime,&pBoatData->dataBytes[2*sizeof(double)],sizeof(time_t));
		if (!m_gpsTime) {
			m_gpsTime = new CTime(gpsTime);
		}
		else *m_gpsTime = CTime(gpsTime);
		return true;
	}
	else if (pBoatData->nPacketType==COMPASS_DATA_PACKET) {
		if (pBoatData->nDataSize!=sizeof(IMU_DATASAMPLE)) {
			return false;
		}
		memcpy(&m_compassData,pBoatData->dataBytes,sizeof(IMU_DATASAMPLE));
		return true;
	}
	else if (pBoatData->nPacketType==SUPPORTED_SENSOR_DATA) {
		//reset sensor types and number of sensors
		m_nNumSensorsAvailable = 0;
		if (this->m_sensorTypes) {
			delete m_sensorTypes;
			m_sensorTypes = NULL;
		}
		if (pBoatData->nDataSize!=sizeof(int)) {
			return false;
		}
		memcpy(&this->m_nNumSensorsAvailable,pBoatData->dataBytes,sizeof(int));//calling function needs to read in additional packet to determine sensor types
		m_sensorTypes = new int[m_nNumSensorsAvailable];
		return true;
	}
	else if (pBoatData->nPacketType==SENSOR_TYPES_INFO) {
		//make sure that enough data was received to define each of the sensor types
		if (pBoatData->nDataSize!=(m_nNumSensorsAvailable*sizeof(int))) {
			return false;
		}
		for (int i=0;i<m_nNumSensorsAvailable;i++) {
			memcpy(&m_sensorTypes[i],&pBoatData->dataBytes[4*i],sizeof(int));
		}
		return true;
	}
	else if (pBoatData->nPacketType==WATER_TEMP_DATA_PACKET) {
		if (pBoatData->nDataSize!=sizeof(float)) {
			return false;
		}
		m_fWaterTemp = Util::BytesToFloat(pBoatData->dataBytes);
		return true;
	}
	else if (pBoatData->nPacketType==WATER_PH_DATA_PACKET) {
		if (pBoatData->nDataSize!=sizeof(float)) {
			return false;
		}
		m_fPH = Util::BytesToFloat(pBoatData->dataBytes);
		return true;
	}
	else if (pBoatData->nPacketType==WATER_TURBIDITY_DATA_PACKET) {
		if (pBoatData->nDataSize!=sizeof(float)) {
			return false;
		}
		m_fWaterTurbidity = Util::BytesToFloat(pBoatData->dataBytes);
		return true;
	}
	else if (pBoatData->nPacketType==BATTVOLTAGE_DATA_PACKET) {
		if (pBoatData->nDataSize!=sizeof(float)) {
			return false;
		}
		m_fBatteryVoltage = Util::BytesToFloat(pBoatData->dataBytes);
		return true;
	}
	else if (pBoatData->nPacketType==LEAK_DATA_PACKET) {
		if (pBoatData->nDataSize!=sizeof(int)) {
			return false;
		}
		m_bLeakDetected = (bool)Util::BytesToInt(pBoatData->dataBytes);
		return true;
	}
	else if (pBoatData->nPacketType==DIAGNOSTICS_DATA_PACKET) {
		if (pBoatData->nDataSize!=(7*sizeof(float)+sizeof(int))) {
			return false;
		}
		m_fBatteryVoltage = Util::BytesToFloat(pBoatData->dataBytes);
		m_fCurrentDraw = Util::BytesToFloat(&pBoatData->dataBytes[sizeof(float)]);
		m_fHumidityCPU = Util::BytesToFloat(&pBoatData->dataBytes[2*sizeof(float)]);
		m_fHumidityTempCPU = Util::BytesToFloat(&pBoatData->dataBytes[3*sizeof(float)]);
		m_fHumidityBatt = Util::BytesToFloat(&pBoatData->dataBytes[4*sizeof(float)]);
		m_fHumidityTempBatt = Util::BytesToFloat(&pBoatData->dataBytes[5*sizeof(float)]);
		m_fWirelessRXPower = Util::BytesToFloat(&pBoatData->dataBytes[6*sizeof(float)]);
		m_nSolarCharging = Util::BytesToInt(&pBoatData->dataBytes[7*sizeof(float)]);
		return true;
	}
	else if (pBoatData->nPacketType == SCRIPT_STATUS_PACKET) {
		if (pBoatData->nDataSize != (REMOTE_SCRIPT_NAMELENGTH + 2 * sizeof(int))) {
			return false;
		}
		char* szTempFilename = new char[REMOTE_SCRIPT_NAMELENGTH + 1];
		memcpy(szTempFilename, pBoatData->dataBytes, REMOTE_SCRIPT_NAMELENGTH);
		szTempFilename[REMOTE_SCRIPT_NAMELENGTH] = 0;//make sure name is NULL terminated
		m_sRemoteScriptName = CString(szTempFilename);
		delete[]szTempFilename;
		m_nRemoteScriptStepIndex = Util::BytesToInt(&pBoatData->dataBytes[REMOTE_SCRIPT_NAMELENGTH]);
		m_nTotalNumberScriptSteps = Util::BytesToInt(&pBoatData->dataBytes[REMOTE_SCRIPT_NAMELENGTH+sizeof(int)]);
		return true;
	}
	return false;
}

CString Captain::FormatCompassData() {//format the current compass data as a string, ex: Heading = 175.2�, Roll = 1.4�, Pitch = 1.8�, Temp = 19.2 �C
	char szDegSymbol[2];
	szDegSymbol[0] = (char)176;
	szDegSymbol[1] = 0;
	CString sRetval="";
	sRetval.Format("Heading = %.1f%s, Roll = %.1f%s, Pitch = %.1f%s, Temp = %.1f %sC",m_compassData.heading,szDegSymbol,m_compassData.roll,szDegSymbol,m_compassData.pitch,szDegSymbol,(m_compassData.mag_temperature+m_compassData.acc_gyro_temperature)/2,szDegSymbol);
	return sRetval;
}

bool Captain::Stop(PROPELLER_STATE *pPropState) {//stop thrusters, set speed of both to zero
	pPropState->fRudderAngle=0.0;
	pPropState->fPropSpeed=0.0;
	this->m_currentPropState.fRudderAngle=0.0;
	this->m_currentPropState.fPropSpeed=0.0;
	return true;
}

CString Captain::FormatWaterTemp() {//format the current water temperature data as a string, ex: Water temp = 23.2�
	char szDegSymbol[2];
	szDegSymbol[0] = (char)176;
	szDegSymbol[1] = 0;
	CString sWaterTemp="";
	sWaterTemp.Format("Water Temp = %.1f %sC",this->m_fWaterTemp,szDegSymbol);
	return sWaterTemp;
}

CString Captain::FormatVoltage() {//format the available voltage
	CString sVoltage="";
	sVoltage.Format("Voltage = %.2f V",this->m_fBatteryVoltage);
	return sVoltage;
}

CString Captain::FormatWaterPH() {//format the current water pH data as a string, ex: pH = 7.8�
	CString sWaterPH="";
	sWaterPH.Format("pH = %.3f",this->m_fPH);
	return sWaterPH;
}

CString Captain::FormatWaterTurbidity() {//format the current water turbidity as a string, ex: Turbidity = 0.087
	CString sWaterTurbidity="";
	sWaterTurbidity.Format("Turbidity = %.3f",this->m_fWaterTurbidity);
	return sWaterTurbidity;
}

//CopyReceivedImage: checks to see if a file of name TMP_IMAGE_FILENAME was saved to the root program folder, and if so, loads it into copyToLocation, then deletes the file
bool Captain::CopyReceivedImage(CImage *copyToLocation) {
	if (!copyToLocation) return false;
	CString sTmpImgFilename="";
	CString sTmp = TMP_IMAGE_FILENAME;
	sTmpImgFilename.Format("%s\\%s",m_sRootFolder,sTmp);
	CFile imgFile;
	BOOL bOpened = imgFile.Open(sTmpImgFilename,CFile::modeRead|CFile::shareDenyNone);
	if (!bOpened) {
		return false;//file does not exist or cannot be opened
	}
	imgFile.Close();
	HRESULT hResult = copyToLocation->Load(sTmpImgFilename);
	if (hResult!=S_OK) {
		return false;//unable to load image file into copyToLocation
	}
	//delete image file
	//DeleteFile(sTmpImgFilename);
	return true;
}

CString Captain::FormatLeakData() {//format the received leak sensor data
	CString sLeakData="Leak Detected: NO";
	if (this->m_bLeakDetected) {
		sLeakData="Leak Detected: YES";
	}
	return sLeakData;
}

CString Captain::FormatDiagnosticsData() {//format the received diagnostics data
	char szDegSymbol[2];
	szDegSymbol[0] = (char)176;
	szDegSymbol[1] = 0;
	CString sRXPower="";
	if (m_fWirelessRXPower!=0) {
		sRXPower.Format("RX Power: %.0f dBm",m_fWirelessRXPower);
	}
	else sRXPower = "RX Power: N.A.";
	CString sDiagData="";
	if (this->m_nSolarCharging>0) {
		sDiagData.Format("Voltage: %.3f V, RH_CPU = %.1f %%, Temp_CPU = %.1f %sC, RH_BATT = %.1f %%, Temp_BATT = %.1f %sC, %s, Solar: YES",
			this->m_fBatteryVoltage, this->m_fHumidityCPU, this->m_fHumidityTempCPU, szDegSymbol, this->m_fHumidityBatt, this->m_fHumidityTempBatt, szDegSymbol,sRXPower);
	}
	else sDiagData.Format("Voltage: %.3f V, RH_CPU = %.1f %%, Temp_CPU = %.1f %sC, RH_BATT = %.1f %%, Temp_BATT = %.1f %sC, %s Solar: NO",
		this->m_fBatteryVoltage, this->m_fHumidityCPU, this->m_fHumidityTempCPU, szDegSymbol,this->m_fHumidityBatt, this->m_fHumidityTempBatt, szDegSymbol,sRXPower);
	return sDiagData;
}

void Captain::CopyCommand(REMOTE_COMMAND *pDest, REMOTE_COMMAND *pSRC) {//copies contents of pSRC into pDest
	if (!pDest||!pSRC) return;
	if (pDest->pDataBytes) {
		delete pDest->pDataBytes;
		pDest->pDataBytes = NULL;
	}
	pDest->nCommand = pSRC->nCommand;
	int nNumDataBytes = pSRC->nNumDataBytes;
	pDest->nNumDataBytes = nNumDataBytes;
	if (nNumDataBytes>0) {
		pDest->pDataBytes = new unsigned char[nNumDataBytes];
		memcpy(pDest->pDataBytes,pSRC->pDataBytes,nNumDataBytes);
	}
}

bool Captain::IsRequestingVideoImage() {//return true if we are currently trying to receive a video image
	return this->m_bRequestingVideoImage;
}

CString Captain::FormatRemoteScriptStatus() {//format the status of the script currently running on the boat (if any) the current step # and the total # of steps
	CString sRemoteScriptStatus = "";
	if (m_sRemoteScriptName.GetLength() > 0) {
		sRemoteScriptStatus.Format("%s, %d of %d", m_sRemoteScriptName, m_nRemoteScriptStepIndex + 1, m_nTotalNumberScriptSteps);
	}
	return sRemoteScriptStatus;
}