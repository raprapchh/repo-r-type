Write-Host "=== Configuration du réseau Windows pour R-Type ==="

Set-NetConnectionProfile -NetworkCategory Private -ErrorAction SilentlyContinue

netsh advfirewall firewall delete rule name="R-Type Server UDP" > $null 2>&1
netsh advfirewall firewall delete rule name="RType UDP 4242" > $null 2>&1
netsh advfirewall firewall delete rule name="RType Client" > $null 2>&1
netsh advfirewall firewall delete rule name="Allow ICMPv4" > $null 2>&1

netsh advfirewall firewall add rule name="R-Type (UDP 4242)" dir=in action=allow protocol=UDP localport=4242 profile=any

Write-Host "Configuration du pare-feu terminée !"
Write-Host "Le port 4242 (UDP) a été ouvert pour tous les profils réseau."