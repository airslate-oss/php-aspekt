# This file is part of the Aspekt.
#
# (c) airSlate Inc. <support@airslate.com>
#
# For the full copyright and license information, please view
# the LICENSE file that was distributed with this source code.

# 1. Amend PATH environment variable
#    - c:\Projects\php-sdk\bin
#    - c:\Projects\php-devpack
#    - c:\Projects\php
#    - <path to your vcvarsall.bat>
# 2. Open cmd.exe as an administrator
# 3. Go to the project root (eg. "cd C:\Projects\php-aspekt")
# 4. Run pwsh.exe
# 5. Run all necessary PowerShell as follows.
# NOTE: Some commands may be omitted. You may need to specify command's parameters.
#     Import-Module -Force .\.ci\build.psm1
#     SetupPrerequisites
#     InstallPhpSdk -Version 2.1.9
#     InstallPhp -Version 7.2 -BuildType Win32 -VC 15 -Platform x64
#     InstallPhpDevPack -PhpVersion 7.2 -BuildType Win32 -VC 15 -Platform x64
# 6. Exit from pwsh.exe by typing "exit"
# 7. Run all necessary batch files as follows (if needed):
#    "%VS140COMNTOOLS%\VsDevCmd.bat" -arch=amd64
#    vcvarsall.bat x86_amd64
#    phpsdk_setvars
# 8. phpize
# 9. configure.bat --with-prefix=c:\Projects\php --with-php-build=c:\Projects\php-devpack --enable-aspekt=shared
# 10. nmake /f Makefile
# 11. nmake /f Makefile test

Set-Variable `
	-name PHP_SDK_URI `
	-value "https://github.com/Microsoft/php-sdk-binary-tools" `
	-Scope Global `
	-Option ReadOnly `
	-Force

Set-Variable `
	-name PHP_URI `
	-value "http://windows.php.net/downloads/releases" `
	-Scope Global `
	-Option ReadOnly `
	-Force

Set-Variable `
	-name ARCH `
	-value $null `
	-Scope Global `
	-Force

Set-Variable `
	-name VSCOMNTOOLS `
	-value $null `
	-Scope Global `
	-Force

$DebugPreference = "Continue"

function SetupPrerequisites {
	Ensure7ZipIsInstalled
	EnsureRequiredDirectoriesPresent `
		-Directories C:\Downloads,C:\Projects
}

function InstallPhpSdk {
	param (
		[Parameter(Mandatory=$true)]  [System.String] $Version,
		[Parameter(Mandatory=$false)] [System.String] $InstallPath = "C:\Projects\php-sdk"
	)

	Write-Host "Install PHP SDK binary tools: ${Version}"

	$FileName  = "php-sdk-${Version}"
	$RemoteUrl = "${PHP_SDK_URI}/archive/${FileName}.zip"
	$Archive   = "C:\Downloads\${FileName}.zip"

	if (-not (Test-Path $InstallPath)) {
		if (-not [System.IO.File]::Exists($Archive)) {
			DownloadFile -RemoteUrl $RemoteUrl -Destination $Archive
		}

		$UnzipPath = "${Env:Temp}\php-sdk-binary-tools-${FileName}"
		If (-not (Test-Path "${UnzipPath}")) {
			Write-Host "Unpack to ${UnzipPath}"
			Expand-Item7zip -Archive $Archive -Destination $Env:Temp
		}

		Move-Item -Path $UnzipPath -Destination $InstallPath
	}
}

function InstallPhp {
	param (
		[Parameter(Mandatory=$true)]  [System.String] $Version,
		[Parameter(Mandatory=$true)]  [System.String] $BuildType,
		[Parameter(Mandatory=$true)]  [System.String] $VC,
		[Parameter(Mandatory=$true)]  [System.String] $Platform,
		[Parameter(Mandatory=$false)] [System.String] $InstallPath = "C:\Projects\php"
	)

	$Version = SetupPhpVersionString -Pattern $Version
	Write-Host "Install PHP: ${Version}"

	$RemoteUrl = "${PHP_URI}/php-${Version}-${BuildType}-vc${VC}-${Platform}.zip"
	$Archive   = "C:\Downloads\php-${Version}-${BuildType}-VC${VC}-${Platform}.zip"

	if (-not (Test-Path $InstallPath)) {
		if (-not [System.IO.File]::Exists($Archive)) {
			DownloadFile $RemoteUrl $Archive
		}

		Expand-Item7zip -Archive $Archive -Destination $InstallPath
	}

	if (-not (Test-Path "${InstallPath}\php.ini")) {
		Copy-Item "${InstallPath}\php.ini-development" "${InstallPath}\php.ini"
	}
}

function InstallPhpDevPack {
	param (
		[Parameter(Mandatory=$true)]  [System.String] $PhpVersion,
		[Parameter(Mandatory=$true)]  [System.String] $BuildType,
		[Parameter(Mandatory=$true)]  [System.String] $VC,
		[Parameter(Mandatory=$true)]  [System.String] $Platform,
		[Parameter(Mandatory=$false)] [System.String] $InstallPath = "C:\Projects\php-devpack"
	)

	Write-Host "Install PHP Dev pack: ${PhpVersion}"

	$Version = SetupPhpVersionString -Pattern $PhpVersion

	$RemoteUrl = "${PHP_URI}/php-devel-pack-${Version}-${BuildType}-vc${VC}-${Platform}.zip"
	$Archive   = "C:\Downloads\php-devel-pack-${Version}-${BuildType}-VC${VC}-${Platform}.zip"

	if (-not (Test-Path $InstallPath)) {
		if (-not [System.IO.File]::Exists($Archive)) {
			DownloadFile $RemoteUrl $Archive
		}

		$UnzipPath = "${Env:Temp}\php-${Version}-devel-VC${VC}-${Platform}"
		If (-not (Test-Path "$UnzipPath")) {
			Expand-Item7zip -Archive $Archive $Env:Temp
		}

		Move-Item -Path $UnzipPath -Destination $InstallPath
	}
}

function SetupPhpVersionString {
	param (
		[Parameter(Mandatory=$true)] [String] $Pattern
	)

	$RemoteUrl   = "${PHP_URI}/sha1sum.txt"
	$Destination = "${Env:Temp}\php-sha1sum.txt"

	If (-not [System.IO.File]::Exists($Destination)) {
		DownloadFile $RemoteUrl $Destination
	}

	$VersionString = Get-Content $Destination | Where-Object {
		$_ -match "php-($Pattern\.\d+)-src"
	} | ForEach-Object { $matches[1] }

	if ($VersionString -NotMatch '\d+\.\d+\.\d+') {
		throw "Unable to obtain PHP version string using pattern 'php-($Pattern\.\d+)-src'"
	}

	Write-Output $VersionString.Split(' ')[-1]
}

function EnsureRequiredDirectoriesPresent {
	param (
		[Parameter(Mandatory=$true)] [String[]] $Directories
	)

	foreach ($Dir in $Directories) {
		if (-not (Test-Path $Dir)) {
			New-Item -ItemType Directory -Force -Path ${Dir} | Out-Null
		}
	}
}

function Ensure7ZipIsInstalled  {
	if (-not (Get-Command "7z" -ErrorAction SilentlyContinue)) {
		$7zipInstallationDirectory = "${Env:ProgramFiles}\7-Zip"

		if (-not (Test-Path "${7zipInstallationDirectory}")) {
			throw "The 7-zip file archiver is needed to use this module"
		}

		$Env:Path += ";$7zipInstallationDirectory"
	}
}

function DownloadFile {
	param (
		[Parameter(Mandatory=$true)] [System.String] $RemoteUrl,
		[Parameter(Mandatory=$true)] [System.String] $Destination
	)

	$RetryMax   = 5
	$RetryCount = 0
	$Completed  = $false

	$WebClient = New-Object System.Net.WebClient
	$WebClient.Headers.Add('User-Agent', 'AppVeyor PowerShell Script')

	Write-Host "Downloading: ${RemoteUrl} => ${Destination} ..."

	while (-not $Completed) {
		try {
			$WebClient.DownloadFile($RemoteUrl, $Destination)
			$Completed = $true
		} catch  {
			if ($RetryCount -ge $RetryMax) {
				$ErrorMessage = $_.Exception.Message
				Write-Error -Message "${ErrorMessage}"
				$Completed = $true
			} else {
				$RetryCount++
			}
		}
	}
}

function Expand-Item7zip {
	param(
		[Parameter(Mandatory=$true)] [System.String] $Archive,
		[Parameter(Mandatory=$true)] [System.String] $Destination
	)

	if (-not (Test-Path -Path $Archive -PathType Leaf)) {
		throw "Specified archive file does not exist: ${Archive}"
	}

	if (-not (Test-Path -Path $Destination -PathType Container)) {
		New-Item $Destination -ItemType Directory | Out-Null
	}

	$Result   = (& 7z x "$Archive" "-o$Destination" -aoa -bd -y -r)
	$ExitCode = $LASTEXITCODE

	If ($ExitCode -ne 0) {
		throw "An error occurred while unzipping '${Archive}' to '${Destination}'"
	}
}
