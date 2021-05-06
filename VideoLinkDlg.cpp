// VideoLinkDlg.cpp : implementation file
//

#include "pch.h"
#include "..\filedata.h"
#include "VideoLinkDlg.h"

#define DEFAULT_FEATURE_THRESHOLD 50 //default threshold value for feature detection
#define DEFAULT_LOW_CANNY_THRESHOLD 100 //default low threshold value for Canny edge detection
#define DEFAULT_HIGH_CANNY_THRESHOLD 200 //default high threshold value for Canny edge detection

//detectinon types
#define CANNY_EDGE_DETECTION 0
#define FEATURE_DETECTION 1



// CVideoLinkDlg dialog

IMPLEMENT_DYNAMIC(CVideoLinkDlg, CDialog)

CVideoLinkDlg::CVideoLinkDlg(CString sRootFolder, NetworkCaptain *pNetCaptain, SerportCaptain *pSerportCaptain, CWnd* pParent /*=NULL*/)
	: CDialog(CVideoLinkDlg::IDD, pParent)
{
	m_sRootFolder = sRootFolder;
	m_pNetCaptain = pNetCaptain;
	m_pSerportCaptain = pSerportCaptain;
	m_nFeatureThreshold = DEFAULT_FEATURE_THRESHOLD;
	m_nLowCannyThreshold = DEFAULT_LOW_CANNY_THRESHOLD;
	m_nHighCannyThreshold = DEFAULT_HIGH_CANNY_THRESHOLD;
	m_nAlgorithmType = CANNY_EDGE_DETECTION;
	m_pImg = NULL;
	m_pParentWnd = pParent;
}

CVideoLinkDlg::~CVideoLinkDlg()
{
}

void CVideoLinkDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_VIDEOCAP_THRESHOLD_SLIDER, m_cThresholdSlider1);
	DDX_Control(pDX, IDC_VIDEOCAP_THRESHOLD_VAL, m_cThresholdValue1);
	DDX_Control(pDX, IDC_VIDEOCAP_CAPTUREWINDOW, m_cCaptureWindow);
	DDX_Control(pDX, IDC_VIDEOCAP_ALGORITHM, m_cImageAlgorithm);
	DDX_Control(pDX, IDC_VIDEOCAP_FEATUREDETECTION_LABEL, m_cThresholdLabel1);
	DDX_Control(pDX, IDC_VIDEOCAP_FEATUREDETECTION_LABEL2, m_cThresholdLabel2);
	DDX_Control(pDX, IDC_VIDEOCAP_THRESHOLD_SLIDER2, m_cThresholdSlider2);
	DDX_Control(pDX, IDC_VIDEOCAP_THRESHOLD_VAL2, m_cThresholdValue2);
}


BEGIN_MESSAGE_MAP(CVideoLinkDlg, CDialog)
	ON_WM_SHOWWINDOW()
	ON_NOTIFY(NM_RELEASEDCAPTURE, IDC_VIDEOCAP_THRESHOLD_SLIDER, &CVideoLinkDlg::OnNMReleasedcaptureVideocapThresholdSlider1)
	ON_CBN_SELCHANGE(IDC_VIDEOCAP_ALGORITHM, &CVideoLinkDlg::OnCbnSelchangeVideocapAlgorithm)
	ON_MESSAGE(WM_UPDATED_PICTURE, OnUpdatedPicture)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_VIDEOCAP_THRESHOLD_SLIDER2, &CVideoLinkDlg::OnNMCustomdrawVideocapThresholdSlider2)
	ON_WM_CLOSE()
END_MESSAGE_MAP()


// CVideoLinkDlg message handlers

void CVideoLinkDlg::OnShowWindow(BOOL bShow, UINT nStatus)
{
	CDialog::OnShowWindow(bShow, nStatus);
	SetupWindow();
}

void CVideoLinkDlg::SetupWindow() {//initializes window
	CString sPrefsFilename = "";
	sPrefsFilename.Format("%s\\prefs.cfg",m_sRootFolder);
	filedata prefsFile(sPrefsFilename.GetBuffer(sPrefsFilename.GetLength()));
	m_nFeatureThreshold = prefsFile.getInteger("[prefs]","feature_threshold");
	if (m_nFeatureThreshold==0) {
		m_nFeatureThreshold = DEFAULT_FEATURE_THRESHOLD;
	}
	this->m_nLowCannyThreshold = prefsFile.getInteger("[prefs]","low_canny_threshold");
	if (m_nLowCannyThreshold==0) {
		m_nLowCannyThreshold = DEFAULT_LOW_CANNY_THRESHOLD;
	}
	this->m_nHighCannyThreshold = prefsFile.getInteger("[prefs]","high_canny_threshold");
	if (m_nHighCannyThreshold==0) {
		m_nHighCannyThreshold = DEFAULT_HIGH_CANNY_THRESHOLD;
	}

	m_cThresholdSlider1.SetRange(0,255);
	m_cThresholdSlider2.SetRange(0,255);
	if (this->m_cImageAlgorithm.GetCount()==0) {
		m_cImageAlgorithm.AddString("Canny Edge Detection");
		m_cImageAlgorithm.AddString("FAST Feature Detection");
	}
	this->m_nAlgorithmType = prefsFile.getInteger("[prefs]","image_algorithm");
	m_cImageAlgorithm.SetCurSel(m_nAlgorithmType);

	if (m_nAlgorithmType==CANNY_EDGE_DETECTION) {
		ShowCannyEdgeDetectionControls();
	}
	else {
		ShowFeatureDetectionControls();
	}
}

//GetVideoInfo: gets an integer value that corresponds to the threshold value(s) used and the algorithm used for processing the image (either for features or edge detection)
//returns the threshold value that the user has chosen with the slider control in this dialog
int CVideoLinkDlg::GetVideoInfo() {
	int nVideoInfoVal=0;//value that stores low and high thresholds (for Canny edge detection) or just the feature threshold value, as well as the type of algorithm (edge or feature detection) to use
	//lowest byte of nVideoInfoVal is for low edge threshold (Canny edge detection) or just the threshold value for feature detection
	//2nd lowest byte of nVideoInfoVal is for the high edge threshold (Canny edge detection) or just 0 for feature detection
	//2nd highest byte is 0 for Canny edge detection or 1 for feature detection or 2 for low resolution image
	if (this->m_nAlgorithmType==CANNY_EDGE_DETECTION) {
		nVideoInfoVal = m_nLowCannyThreshold + (m_nHighCannyThreshold<<8);
	}
	else {//feature detection
		int nFeatureVal = 1;
		nVideoInfoVal = this->m_nFeatureThreshold + (nFeatureVal<<16);
	}
	return nVideoInfoVal;
}

void CVideoLinkDlg::OnNMReleasedcaptureVideocapThresholdSlider1(NMHDR *pNMHDR, LRESULT *pResult)
{
	int nThresholdVal = m_cThresholdSlider1.GetPos();
	CString sThreshold;
	sThreshold.Format("%d",nThresholdVal);
	m_cThresholdValue1.SetWindowText(sThreshold);
	if (this->m_nAlgorithmType==CANNY_EDGE_DETECTION) {
		this->m_nLowCannyThreshold = nThresholdVal;
	}
	//save new threshold value to prefs.cfg file
	SaveSettings();
	*pResult = 0;
}

void CVideoLinkDlg::UpdateImage(CImage *pImg) {
	if (!pImg) return;
	HBITMAP hBitmap = (HBITMAP)(*pImg);
	m_cCaptureWindow.SetBitmap(hBitmap);
}
void CVideoLinkDlg::OnCbnSelchangeVideocapAlgorithm()
{
	m_nAlgorithmType = this->m_cImageAlgorithm.GetCurSel();
	if (m_nAlgorithmType==CANNY_EDGE_DETECTION) {
		ShowCannyEdgeDetectionControls();
	}
	else {
		ShowFeatureDetectionControls();
	}
	SaveSettings();
}

void CVideoLinkDlg::OnNMCustomdrawVideocapThresholdSlider2(NMHDR *pNMHDR, LRESULT *pResult)
{
	int nThresholdVal = this->m_cThresholdSlider2.GetPos();
	CString sThreshold;
	sThreshold.Format("%d",nThresholdVal);
	m_cThresholdValue2.SetWindowText(sThreshold);
	this->m_nHighCannyThreshold = nThresholdVal;
	
	//save new threshold value to prefs.cfg file
	SaveSettings();
	*pResult = 0;
}

void CVideoLinkDlg::SaveSettings() {//save current settings to prefs.cfg file
CString sPrefsFilename = "";
	sPrefsFilename.Format("%s\\prefs.cfg",m_sRootFolder);
	filedata prefsFile(sPrefsFilename.GetBuffer(sPrefsFilename.GetLength()));
	prefsFile.writeData("[prefs]","feature_threshold",m_nFeatureThreshold);
	prefsFile.writeData("[prefs]","low_canny_threshold",m_nLowCannyThreshold);
	prefsFile.writeData("[prefs]","high_canny_threshold",m_nHighCannyThreshold);
	prefsFile.writeData("[prefs]","image_algorithm",m_nAlgorithmType);
}

void CVideoLinkDlg::ShowCannyEdgeDetectionControls() {//show the controls for Canny edge detection
	this->m_cThresholdLabel2.ModifyStyle(0,WS_VISIBLE);
	this->m_cThresholdSlider2.ModifyStyle(0,WS_VISIBLE);
	this->m_cThresholdValue2.ModifyStyle(0,WS_VISIBLE);
	m_cThresholdLabel1.SetWindowText("Canny Edge Low Threshold (0-255)");
	m_cThresholdSlider1.SetPos(this->m_nLowCannyThreshold);
	m_cThresholdLabel2.SetWindowText("Canny Edge High Threshold (0-255)");
	m_cThresholdSlider2.SetPos(this->m_nHighCannyThreshold);
	CString sVal1;
	sVal1.Format("%d",m_nLowCannyThreshold);
	m_cThresholdValue1.SetWindowText(sVal1);
	CString sVal2;
	sVal2.Format("%d",m_nHighCannyThreshold);
	m_cThresholdValue2.SetWindowText(sVal2);
	Invalidate(TRUE);
}

void CVideoLinkDlg::ShowFeatureDetectionControls() {//show the controls for feature detection (and hide the controls related to Canny edge detection)
	this->m_cThresholdLabel2.ModifyStyle(WS_VISIBLE,0);
	this->m_cThresholdSlider2.ModifyStyle(WS_VISIBLE,0);
	this->m_cThresholdValue2.ModifyStyle(WS_VISIBLE,0);
	this->m_cThresholdLabel1.SetWindowText("Feature Threshold Value (0-255)");
	this->m_cThresholdSlider1.SetPos(this->m_nFeatureThreshold);
	CString sVal;
	sVal.Format("%d",m_nFeatureThreshold);
	this->m_cThresholdValue1.SetWindowText(sVal);
}
void CVideoLinkDlg::OnClose()
{
	if (m_pParentWnd) {
		m_pParentWnd->PostMessage(WM_NOTIFY_VIDEO_CLOSED, 0, 0);
	}
	if (m_pSerportCaptain) {
		if (m_pSerportCaptain->IsDownloadRunning()) {
			m_pSerportCaptain->StopDownloadThread();
		}
	}
	CDialog::OnClose();
	DestroyWindow();
}

LRESULT CVideoLinkDlg::OnUpdatedPicture(WPARAM wParam, LPARAM lParam) {//called from message sent by download thread that was created by the serial port captain
	if (!m_pSerportCaptain) {
		return 1;
	}
	//m_pSerportCaptain->CopyReceivedImage(m_pSerportCaptain->m_requestedImg);
	if (this->GetSafeHwnd()) {
		//SetImage(m_pSerportCaptain->m_requestedImg);
		UpdateImage(m_pImg);
		m_pImg->Detach();
	}
	return 0;
}

void CVideoLinkDlg::SetImage(CImage *pImg) {//set image data (for later updating in dialog)
	m_pImg = pImg;
}