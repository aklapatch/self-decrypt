$PSScriptRoot = Split-Path -Parent -Path $MyInvocation.MyCommand.Definition
echo $PSScriptRoot
cd $PSScriptRoot
Start-Process "$PSScriptRoot/self-decrypt.exe"
sleep 1
$wshell = New-Object -ComObject wscript.shell;
$wshell.AppActivate("self-decrypt")
Sleep 1
echo "sending tab"
$wshell.SendKeys('{TAB}')
$wshell.SendKeys('{ENTER}')
echo "sending file"
$wshell.SendKeys("b")

$fshell = New-Object -ComObject wscript.shell;
$fshell.AppActivate("File Select")
echo "sending file"
$fshell.SendKeys(".gitignore")
$fshell.SendKeys('{TAB}')
$fshell.SendKeys('{ENTER}')

$wshell.SendKeys('{TAB}')
$wshell.SendKeys('{TAB}')
$wshell.SendKeys('{TAB}')
$wshell.SendKeys('{TAB}')
$wshell.SendKeys('{ENTER}')

rm "test.exe"
$fshell = New-Object -ComObject wscript.shell;
$fshell.AppActivate("File Select")
sleep 1
echo "sending file"
$fshell.SendKeys("test.exe")
$fshell.SendKeys('{TAB}')
$fshell.SendKeys('{ENTER}')

$wshell.SendKeys('{TAB}')
$wshell.SendKeys('{TAB}')
$wshell.SendKeys('{TAB}')
$wshell.SendKeys('{TAB}')
# enter a password
$wshell.SendKeys('{TAB}')
$wshell.SendKeys('j')

$wshell.SendKeys('{TAB}')
$wshell.SendKeys('{ENTER}')

# should get success message, hit okay on that
$wshell.SendKeys("{ENTER}")
$fshell = New-Object -ComObject wscript.shell;
$fshell.AppActivate("Success")
$fshell.SendKeys('{ENTER}')