; Installer for R-Type Clone
!include "MUI2.nsh"

Name "R-Type Clone"
OutFile "R-Type-Installer.exe"
InstallDir "$PROGRAMFILES\R-TypeClone"
InstallDirRegKey HKCU "Software\R-TypeClone" ""
RequestExecutionLevel admin

!define MUI_ABORTWARNING
!define MUI_ICON "${NSISDIR}\Contrib\Graphics\Icons\modern-install.ico"
!define MUI_UNICON "${NSISDIR}\Contrib\Graphics\Icons\modern-uninstall.ico"

!insertmacro MUI_PAGE_WELCOME
!insertmacro MUI_PAGE_DIRECTORY
!insertmacro MUI_PAGE_INSTFILES
!insertmacro MUI_PAGE_FINISH

!insertmacro MUI_UNPAGE_WELCOME
!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES
!insertmacro MUI_UNPAGE_FINISH

!insertmacro MUI_LANGUAGE "French"

Section "Application" SEC01
    SetOutPath "$INSTDIR"

    File "..\bin\windows\r-type_client.exe"
    File "..\bin\windows\r-type_server.exe"

    SetOutPath "$INSTDIR\client\assets"
    File /r "..\client\assets\*.*"

    SetOutPath "$INSTDIR\client\fonts"
    File /r "..\client\fonts\*.*"

    SetOutPath "$INSTDIR\client\sprites"
    File /r "..\client\sprites\*.*"

    SetOutPath "$INSTDIR\server\assets"
    File /r "..\server\assets\*.*"

    SetOutPath "$INSTDIR\config"
    File /r "..\config\*.*"

    WriteUninstaller "$INSTDIR\Uninstall.exe"

    WriteRegStr HKCU "Software\R-TypeClone" "" "$INSTDIR"
    WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\R-TypeClone" "DisplayName" "R-Type Clone"
    WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\R-TypeClone" "UninstallString" "$INSTDIR\Uninstall.exe"
SectionEnd

Section "Raccourcis" SEC02
    CreateDirectory "$SMPROGRAMS\R-Type Clone"
    CreateShortcut "$SMPROGRAMS\R-Type Clone\R-Type Client.lnk" "$INSTDIR\r-type_client.exe"
    CreateShortcut "$SMPROGRAMS\R-Type Clone\R-Type Server.lnk" "$INSTDIR\r-type_server.exe"
    CreateShortcut "$SMPROGRAMS\R-Type Clone\Uninstall.lnk" "$INSTDIR\Uninstall.exe"
    CreateShortcut "$DESKTOP\R-Type Client.lnk" "$INSTDIR\r-type_client.exe"
SectionEnd

Section "Uninstall"
    Delete "$INSTDIR\r-type_client.exe"
    Delete "$INSTDIR\r-type_server.exe"
    Delete "$INSTDIR\Uninstall.exe"

    RMDir /r "$INSTDIR\client"
    RMDir /r "$INSTDIR\server"
    RMDir /r "$INSTDIR\config"
    RMDir "$INSTDIR"

    Delete "$SMPROGRAMS\R-Type Clone\R-Type Client.lnk"
    Delete "$SMPROGRAMS\R-Type Clone\R-Type Server.lnk"
    Delete "$SMPROGRAMS\R-Type Clone\Uninstall.lnk"
    RMDir "$SMPROGRAMS\R-Type Clone"
    Delete "$DESKTOP\R-Type Client.lnk"

    DeleteRegKey HKCU "Software\R-TypeClone"
    DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\R-TypeClone"
SectionEnd

Function .onInstSuccess
    MessageBox MB_OK "Installation de R-Type Clone terminée avec succès !"
FunctionEnd
