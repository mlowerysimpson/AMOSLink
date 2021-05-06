#pragma once
#include <atlimage.h>
#include <afxwin.h>
#include "..\..\AMOS\AMOS\Source\CommandList.h"
#include "..\..\AMOS\AMOS\Source\IMU.h"
#include "..\..\AMOS\AMOS\Source\SensorDataFile.h"
#include "..\..\AMOS\AMOS\Source\Rudder.h"

#define COMMAND_OK 0
#define CONNECT_OK 0
#define ERROR_CANNOT_CONNECT -1
#define ERROR_NOTCONNECTED -2
#define ERROR_COMMAND_FAILED -3
#define DEFAULT_SPEED 3 //default speed for propeller-based commands
#define MAX_SPEED 10 //maximum speed for propeller-based commanes

//button command types
#define THRUST_FORWARDS 0
#define THRUST_LEFT 1
#define THRUST_RIGHT 2
#define THRUST_BACK 3
#define REQUEST_IMAGE 4
#define THRUST_STOP 5

//definitions of file types
#define REMOTE_SCRIPT_FILES 0
#define REMOTE_DATA_FILES 1
#define REMOTE_LOG_FILES 2
#define REMOTE_IMAGE_FILES 3

#define TMP_IMAGE_FILENAME "tmp_img_output.jpg" //name of temporary image file for storing video captures from remote boat

#ifdef _AMOS_LINK
#define AMOSLINK_API _declspec (dllexport)
extern AFX_EXTENSION_MODULE AMOSLinkDLL;
#else
#define AMOSLINK_API _declspec (dllimport)
#endif

class AMOSLINK_API Captain
{
public:
	Captain(CString sRootFolder);
	~Captain(void);
	PROPELLER_STATE * GetCurrentPropState();//returns the current state of the propellers as a pointer to a PROPELLER_STATE structure
	CString m_sRootFolder;//the folder where this program was launched from 
	CString m_sGPSData;
	bool CopyReceivedImage(CImage *copyToLocation);//checks to see if a file of name TMP_IMAGE_FILENAME was saved to the root program folder, and if so, loads it into copyToLocation
	bool IsRequestingVideoImage();//return true if we are currently trying to receive a video image
	static void CopyCommand(REMOTE_COMMAND *pDest, REMOTE_COMMAND *pSRC);//copies contents of pSRC into pDest
	
protected:
	//functions
	bool Stop(PROPELLER_STATE *pPropState);//stop thrusters, set speed of both to zero
	bool ForwardHo(PROPELLER_STATE *pPropState);//move forward, increase forward speed (up to limit)
	bool StarboardHo(PROPELLER_STATE *propState);//move to right, increase right turning speed (up to limit)
	bool PortHo(PROPELLER_STATE *propState);//move to left, increase left turning speed (up to limit)
	bool BackHo(PROPELLER_STATE *propState);//move backward, increase backward speed (up to limit)
	void DisplayLastSockError();//display the most recent Windows Sockets error code
	void DisplayLastError(CString sErrMsg);//display the specified error message (sErrMsg) plus display the most recent general Windows error code
	CString FormatRemoteScriptStatus();//format the status of the script currently running on the boat (if any) the current step # and the total # of steps
	CString FormatGPSData();//format the current GPS data as a string, ex: 45.334523� N, 62.562533� W, 2018-06-18, 17:23:00
	CString FormatCompassData();//format the current compass data as a string, ex: Heading = 175.2�, Roll = 1.4�, Pitch = 1.8�, Temp = 19.2 �C
	CString FomatSensorData();//format the current sensor data as a string, ex: Water Temp = 23.2�, pH = 7.8,  
	CString FormatWaterTemp();//format the current water temperature data as a string, ex: Water temp = 23.2�
	CString FormatWaterPH();//format the current water pH data as a string, ex: pH = 7.8�
	CString FormatWaterTurbidity();//format the current water turbidity as a string, ex: Turbidity = 0.087
	CString FormatVoltage();//format the available voltage
	CString FormatLeakData();//format the received leak sensor data
	CString FormatDiagnosticsData();//format the received diagnostics data
	bool ProcessBoatData(BOAT_DATA *pBoatData);//process data from boat, return true if boat data could be successfully processed

	bool m_bRequestingVideoImage;//set to true when we are in the middle of receiving a video image

	//data
	char* m_fileDestPath;//the path where a file will be downloaded to
	int m_nSolarCharging;//set to 1 if the battery is being charged by solar power, otherwise 0
	bool m_bLeakDetected;//true if a leak was detected somewhere in the boat
	float m_fCurrentDraw;//the measured amount of current in Amps through the +12 V line
	float m_fBatteryVoltage;//the measured voltage of the battery on the boat (in V)
	float m_fWirelessRXPower;//the power level measured by the remote wireless receiver
	float m_fWaterTemp;//the last known water temperature measured by the boat
	float m_fPH;//the last known water pH value measured by the boat
	float m_fWaterTurbidity;//the last known water turbidity measured by the boat
	float m_fHumidityCPU;//the humidity inside the CPU enclosure of AMOS
	float m_fHumidityTempCPU;//the temperature as measured by the humidity sensor in the CPU enclosure of AMOS
	float m_fHumidityBatt;//the humidity inside the battery enclosure of AMOS
	float m_fHumidityTempBatt;//the temperature as measured by the humidity sensor in the battery enclosure of AMOS
	double m_dLatitude;//the last known latitude of the boat
	double m_dLongitude;//the last known longtitude of the boat
	CString m_sRemoteScriptName;//the filename of the script currently running on the boat
	CString m_sRemoteScriptsAvailable;//list of all the remote scripts currently available on AMOS (each name separated by '\n')
	CString m_sRemoteDataAvailable;//list of all the remote data files currently available on AMOS (each name separated by '\n')
	CString m_sRemoteLogsAvailable;//list of all the remote log files currently available on AMOS (each name separated by '\n')
	CString m_sRemoteRemoteImageFilesAvailable;//list of all the remote image files currently available on AMOS (each name separated by '\n')
	int m_nRemoteScriptStepIndex;//the index (0-based) of the step currently executing on the remote script of the boat
	int m_nTotalNumberScriptSteps;//the total # of steps in the remote script of the boat
	CTime *m_gpsTime;//the last known timestamp of a GPS reading from the boat
	IMU_DATASAMPLE m_compassData;//the last known compass data from the boat
	int m_nNumSensorsAvailable;//the total number of sensors that the boat has available
	int *m_sensorTypes;//the types of sensors that the boat has available, see SensorDataFile.h for a list of the supported sensor types


private:
	PROPELLER_STATE m_currentPropState;//the current state of the propellers, initialize left & right to zero speed

	//functions for modifying the state of the propellers
	void IncrementPropSpeed();//increase propeller speed
	void IncrementRudderAngle();//increase rudder angle
	void DecrementRudderAngle();//decrease rudder angle
	void DecrementPropSpeed();//decrease propeller speed
	void CopyCurrentPropState(PROPELLER_STATE *pPropState);//copies the current propeller state into pPropState
};
