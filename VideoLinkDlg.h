#pragma once
#include "afxcmn.h"
#include "afxwin.h"
#include "NetworkCaptain.h"
#include "SerportCaptain.h"
#include "resource.h"


#define WM_UPDATED_PICTURE WM_USER+1
#define WM_NOTIFY_VIDEO_CLOSED WM_USER+2

// CVideoLinkDlg dialog
#ifdef _AMOS_LINK
#define AMOSLINK_API _declspec (dllexport)
extern AFX_EXTENSION_MODULE AMOSLinkDLL;
#else
#define AMOSLINK_API _declspec (dllimport)
#endif

class AMOSLINK_API CVideoLinkDlg : public CDialog
{
	DECLARE_DYNAMIC(CVideoLinkDlg)

public:
	CVideoLinkDlg(CString sRootFolder, NetworkCaptain *pNetCaptain, SerportCaptain *pSerportCaptain, CWnd* pParent = NULL);   // standard constructor
	virtual ~CVideoLinkDlg();
	int GetVideoInfo();//gets an integer value that corresponds to the threshold value(s) used and the algorithm used for processing the image (either for features or edge detection)
	void UpdateImage(CImage *pImg);
	void SetImage(CImage *pImg);//set image data (for later updating in dialog)
// Dialog Data
	enum { IDD = IDD_VIDEOCAP_WINDOW };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	afx_msg void OnNMReleasedcaptureVideocapThresholdSlider1(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnCbnSelchangeVideocapAlgorithm();
	afx_msg void OnNMCustomdrawVideocapThresholdSlider2(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnClose();
	LRESULT OnUpdatedPicture(WPARAM wParam, LPARAM lParam);//called after a picture has been downloaded 

private:
	CWnd *m_pParentWnd;//parent window of this dialog (i.e. the main program window)
	int m_nFeatureThreshold;//threshold used for feature detection
	int m_nLowCannyThreshold;//low threshold used for Canny edge detection
	int m_nHighCannyThreshold;//high threshold used for Canny edge detection
	int m_nAlgorithmType;//whether to use feature detection or (Canny) edge detection
	NetworkCaptain *m_pNetCaptain;//pointer to network captain object, used for getting video images from boat over network connection
	SerportCaptain *m_pSerportCaptain;//pointer to serial port captain object, used for getting video images from boat over wireless serial connection
	CSliderCtrl m_cThresholdSlider1;//slider used to control the threshold number for feature detection (0 to 255) or Canny edge detection (low threshold value)
	CStatic m_cThresholdValue1;//numeric value that shows the value selected by the 1st slider control
	CStatic m_cCaptureWindow;//the area of the window where the video capture frames are displayed
	CString m_sRootFolder;//the folder where this program is run from
	CComboBox m_cImageAlgorithm;
	CStatic m_cThresholdLabel1;//the descriptive text label that appears above the 1st slider
	CStatic m_cThresholdLabel2;//the descriptive text label that appears above the 2nd slider
	CSliderCtrl m_cThresholdSlider2;//slider used to control the high threshold number for Canny edge detection (0 to 255)
	CStatic m_cThresholdValue2;//numeric value that shows the value selected by the 2nd slider control
	CImage *m_pImg;//pointer to image data 

	void SetupWindow();//initializes window, and starts thread for grabbing video capture frames from boat
	void SaveSettings();//save current settings to prefs.cfg file
	void ShowCannyEdgeDetectionControls();//show the controls for Canny edge detection
	void ShowFeatureDetectionControls();//show the controls for feature detection (and hide the controls related to Canny edge detection)
};
