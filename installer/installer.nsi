; Installer for R-Type Clone
!include "MUI2.nsh"
!include "LogicLib.nsh"

Name "R-Type Clone"
OutFile "R-Type-Installer.exe"
InstallDir "$PROGRAMFILES\R-TypeClone"
InstallDirRegKey HKCU "Software\R-TypeClone" ""
RequestExecutionLevel admin

; Download URL must be provided at build time with makensis /DDOWNLOAD_URL="https://.../dist.zip"
!ifndef DOWNLOAD_URL
    !define DOWNLOAD_URL "https://github.com/OWNER/REPO/releases/download/latest/dist.zip"
!endif

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

        ; --- VC++ Redistributable Check ---
        DetailPrint "Vérification de Visual C++ Redistributable 2015-2022..."
        ReadRegDWORD $0 HKLM "SOFTWARE\Microsoft\VisualStudio\14.0\VC\Runtimes\x64" "Installed"
        ${If} $0 != 1
            DetailPrint "Visual C++ Redistributable non trouvé. Téléchargement..."
            inetc::get /caption "Téléchargement de VC++ Redist" /popup "Veuillez patienter..." "https://aka.ms/vs/17/release/vc_redist.x64.exe" "$TEMP\vc_redist.x64.exe" /end
            Pop $0
            ${If} $0 == "OK"
                DetailPrint "Installation de VC++ Redist..."
                ExecWait '"$TEMP\vc_redist.x64.exe" /quiet /norestart' $0
                DetailPrint "Installation terminée (code $0)"
                Delete "$TEMP\vc_redist.x64.exe"
            ${Else}
                DetailPrint "Échec du téléchargement de VC++ Redist: $0. L'application pourrait ne pas fonctionner."
            ${EndIf}
        ${Else}
            DetailPrint "Visual C++ Redistributable est déjà installé."
        ${EndIf}

        ; Web installer: download release archive and extract
        CreateDirectory "$INSTDIR"

        ; Download dist.zip from the release URL into the installation folder
        DetailPrint "Téléchargement des fichiers depuis: ${DOWNLOAD_URL}"
        ; Retry loop: up to 3 attempts with 3s delay
        StrCpy $R9 0
    DownloadRetryLoop:
        inetc::get /caption "Téléchargement des ressources" /popup "Veuillez patienter..." "${DOWNLOAD_URL}" "$INSTDIR\dist.zip" /end
        Pop $0
        ${If} $0 == "OK"
            Goto DownloadSucceeded
        ${EndIf}
        
        IntOp $R9 $R9 + 1
        DetailPrint "Téléchargement échoué: $0 (tentative $R9/3)"
        ${If} $R9 >= 3
            MessageBox MB_OK|MB_ICONEXCLAMATION "Échec du téléchargement des ressources après 3 tentatives: $0"
            Abort
        ${EndIf}
        
        Sleep 3000
        Goto DownloadRetryLoop
        
    DownloadSucceeded:
        DetailPrint "Extraction des fichiers..."
        nsExec::ExecToLog 'powershell -NoLogo -NonInteractive -Command "Expand-Archive -LiteralPath \"$INSTDIR\dist.zip\" -DestinationPath \"$INSTDIR\" -Force"'
        Pop $0
        ${If} $0 != 0
            MessageBox MB_OK|MB_ICONEXCLAMATION "Échec de l'extraction ZIP (code $0)"
            Abort
        ${EndIf}

        ; Cleanup archive
        Delete "$INSTDIR\dist.zip"

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
    ; Remove all files in the installation directory (including DLLs and assets)
    RMDir /r "$INSTDIR"

    ; Remove shortcuts
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
