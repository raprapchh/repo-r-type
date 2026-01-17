Write-Host "=== Configuration du réseau Windows pour R-Type ==="
Set-NetConnectionProfile -NetworkCategory Private -ErrorAction SilentlyContinue
netsh advfirewall firewall add rule name="Allow ICMPv4" protocol=icmpv4:8,any dir=in action=allow
netsh advfirewall firewall add rule name="RType UDP 4242" dir=in action=allow protocol=UDP localport=4242
netsh advfirewall firewall add rule name="RType Client" dir=in action=allow program="$INSTDIR\r-type_client.exe" enable=yes
netsh advfirewall firewall add rule name="RType Server" dir=in action=allow program="$INSTDIR\r-type_server.exe" enable=yes

Write-Host "Configuration terminée !"
