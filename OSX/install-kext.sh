#!/bin/bash
BASEDIR=$(dirname $0)
echo 'Installing Driver...'
sudo cp -R "$BASEDIR/heimdall.kext" /System/Library/Extensions
sudo chmod -R 755 /System/Library/Extensions/heimdall.kext
sudo chown -R root:wheel /System/Library/Extensions/heimdall.kext
sudo kextload /System/Library/Extensions/heimdall.kext
echo 'Installation complete. If Heimdall cannot recognise your device a reboot may be required.'
