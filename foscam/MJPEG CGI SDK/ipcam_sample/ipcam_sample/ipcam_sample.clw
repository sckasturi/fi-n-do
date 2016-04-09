; CLW file contains information for the MFC ClassWizard

[General Info]
Version=1
LastClass=CIpcam_sampleDlg
LastTemplate=CDialog
NewFileInclude1=#include "stdafx.h"
NewFileInclude2=#include "ipcam_sample.h"

ClassCount=4
Class1=CIpcam_sampleApp
Class2=CIpcam_sampleDlg

ResourceCount=3
Resource2=IDD_IPCAM_SAMPLE_DIALOG
Resource1=IDR_MAINFRAME
Resource3=IDD_IPCAM_SAMPLE_DIALOG (English (U.S.))

[CLS:CIpcam_sampleApp]
Type=0
HeaderFile=ipcam_sample.h
ImplementationFile=ipcam_sample.cpp
Filter=N

[CLS:CIpcam_sampleDlg]
Type=0
HeaderFile=ipcam_sampleDlg.h
ImplementationFile=ipcam_sampleDlg.cpp
Filter=D
BaseClass=CDialog
VirtualFilter=dWC



[DLG:IDD_IPCAM_SAMPLE_DIALOG]
Type=1
ControlCount=3
Control1=IDOK,button,1342242817
Control2=IDCANCEL,button,1342242816
Control3=IDC_STATIC,static,1342308352
Class=CIpcam_sampleDlg

[DLG:IDD_IPCAM_SAMPLE_DIALOG (English (U.S.))]
Type=1
Class=CIpcam_sampleDlg
ControlCount=23
Control1=ID_CONNECT,button,1342242817
Control2=IDCANCEL,button,1342242816
Control3=IDC_IP,SysIPAddress32,1342242816
Control4=IDC_PORT,edit,1350631552
Control5=IDC_STATIC,static,1342308352
Control6=IDC_STATIC,static,1342308352
Control7=IDC_USER,edit,1350631552
Control8=IDC_STATIC,static,1342308352
Control9=IDC_PWD,edit,1350631584
Control10=IDC_STATIC,static,1342308352
Control11=ID_PLAY_AUDIO,button,1342242817
Control12=ID_DISCONNECT,button,1342242817
Control13=ID_PLAY_VIDEO,button,1342242817
Control14=ID_STOP_VIDEO,button,1342242817
Control15=ID_STOP_AUDIO,button,1342242817
Control16=IDC_STATIC,static,1342308352
Control17=IDC_CONN_STATUS,edit,1350633600
Control18=IDC_STATIC,static,1342308352
Control19=IDC_VIDEO_STATUS,edit,1350633600
Control20=IDC_STATIC,static,1342308352
Control21=IDC_AUDIO_STATUS,edit,1350633600
Control22=IDC_BMP,static,1342177294
Control23=IDC_FOSCAM_NAME,static,1342308352

