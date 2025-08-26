param(
    [Parameter(Mandatory=$true)][string]$Triplet,
    [Parameter(Mandatory=$true)][string]$Renderer,
    [Parameter(Mandatory=$false)][switch]${With-ICU}
)

Set-Location (Split-Path $MyInvocation.MyCommand.Path -Parent)

# Check if this is an ARM64 build
$IsARM64 = $Triplet -match "arm64"

# Enhanced ARM64 environment setup
if ($IsARM64) {
    Write-Host "Setting up ARM64 build environment..." -ForegroundColor Green
    
    # Set environment variables to help vcpkg with ARM64 detection
    $env:VCPKG_FORCE_SYSTEM_BINARIES = '1'
    $env:VCPKG_DISABLE_COMPILER_TRACKING = '1'
    $env:VCPKG_KEEP_ENV_VARS = 'VCPKG_FORCE_SYSTEM_BINARIES;VCPKG_DISABLE_COMPILER_TRACKING'
    
    # Find Visual Studio installation with ARM64 tools
    $vsInstallPath = $null
    $vsWhere = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe"
    
    if (Test-Path $vsWhere) {
        $vsInstallPath = & $vsWhere -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.ARM64 -property installationPath
        if ($vsInstallPath) {
            Write-Host "Found Visual Studio with ARM64 tools at: $vsInstallPath" -ForegroundColor Green
            
            # Set up ARM64 cross-compilation environment
            $vcvarsPath = Join-Path $vsInstallPath "VC\Auxiliary\Build\vcvarsall.bat"
            if (Test-Path $vcvarsPath) {
                Write-Host "Setting up ARM64 cross-compilation environment..." -ForegroundColor Yellow
                
                # Create a temporary batch file to set up environment and run vcpkg
                $tempBatch = [System.IO.Path]::GetTempFileName() + ".bat"
                $vcpkgScript = @"
@echo off
call "$vcvarsPath" x64_arm64
set VCPKG_DEFAULT_TRIPLET=$Triplet
set VCPKG_FORCE_SYSTEM_BINARIES=1
set VCPKG_DISABLE_COMPILER_TRACKING=1
echo ARM64 environment variables set:
echo VCPKG_DEFAULT_TRIPLET=%VCPKG_DEFAULT_TRIPLET%
echo VCPKG_FORCE_SYSTEM_BINARIES=%VCPKG_FORCE_SYSTEM_BINARIES%
echo VCPKG_DISABLE_COMPILER_TRACKING=%VCPKG_DISABLE_COMPILER_TRACKING%
"@
                Set-Content -Path $tempBatch -Value $vcpkgScript -Encoding ASCII
                
                # Execute the environment setup
                $envOutput = & cmd.exe /c "$tempBatch && set"
                Remove-Item $tempBatch -Force
                
                # Parse and set environment variables from the batch output
                foreach ($line in $envOutput) {
                    if ($line -match '^([^=]+)=(.*)$') {
                        $name = $matches[1]
                        $value = $matches[2]
                        # Only set important environment variables, avoid overriding PowerShell vars
                        if ($name -in @('PATH', 'INCLUDE', 'LIB', 'LIBPATH', 'VCINSTALLDIR', 'WindowsSDKDir', 'VCPKG_DEFAULT_TRIPLET', 'VCPKG_FORCE_SYSTEM_BINARIES', 'VCPKG_DISABLE_COMPILER_TRACKING')) {
                            [Environment]::SetEnvironmentVariable($name, $value, 'Process')
                        }
                    }
                }
                Write-Host "ARM64 build environment configured successfully" -ForegroundColor Green
            }
        } else {
            Write-Warning "Visual Studio with ARM64 tools not found. Please install 'MSVC v143 - VS 2022 C++ ARM64 build tools'"
            Write-Host "You can install it via Visual Studio Installer > Individual Components > MSVC v143 - VS 2022 C++ ARM64 build tools" -ForegroundColor Yellow
        }
    }
}

$vcpkg_temp_dir = '***'
foreach($letter in [byte][char]'Z'..[byte][char]'A')
{
    $vcpkg_temp_dir = '{0}:' -f [char]$letter
    if(-not (Test-Path $vcpkg_temp_dir))
    {
        & subst $vcpkg_temp_dir ([System.IO.Path]::Combine($PWD.Path, 'vendor', 'vcpkg'))
        $env:VCPKG_ROOT = ('{0}\' -f $vcpkg_temp_dir)
        break
    }
}

switch($Renderer)
{
    'EGL'    { $renderer_packages = @('egl', 'opengl-registry'); break }
    'OpenGL' { $renderer_packages = @('opengl-registry');        break }
    'Vulkan' { $renderer_packages = @();                         break }
    'All'    { $renderer_packages = @('egl', 'opengl-registry'); break }
}

# Bootstrap vcpkg if needed
if(-not (Test-Path ('{0}\vcpkg.exe' -f $vcpkg_temp_dir)))
{
    Write-Host "Bootstrapping vcpkg..." -ForegroundColor Yellow
    $bootstrapResult = & ('{0}\bootstrap-vcpkg.bat' -f $vcpkg_temp_dir)
    if ($LASTEXITCODE -ne 0) {
        Write-Error "Failed to bootstrap vcpkg"
        exit 1
    }
}

# Prepare vcpkg arguments
$vcpkg_args = @(
    '--disable-metrics',
    ('--overlay-triplets={0}' -f [System.IO.Path]::Combine($PWD.Path, 'vendor', 'vcpkg-custom-triplets')),
    ('--triplet={0}' -f $Triplet),
    '--clean-after-build'
)

# ARM64-specific vcpkg configurations
if ($IsARM64) {
    Write-Host "Configuring vcpkg for ARM64 build..." -ForegroundColor Yellow
    
    # Create ARM64-specific triplet if it doesn't exist
    $customTripletsDir = [System.IO.Path]::Combine($PWD.Path, 'vendor', 'vcpkg-custom-triplets')
    $arm64TripletPath = Join-Path $customTripletsDir "arm64-windows-custom.cmake"
    
    if (-not (Test-Path $customTripletsDir)) {
        New-Item -ItemType Directory -Path $customTripletsDir -Force
    }
    
    if (-not (Test-Path $arm64TripletPath)) {
        $arm64TripletContent = @"
set(VCPKG_TARGET_ARCHITECTURE arm64)
set(VCPKG_CRT_LINKAGE dynamic)
set(VCPKG_LIBRARY_LINKAGE static)
set(VCPKG_CMAKE_SYSTEM_NAME Windows)

# ARM64 specific configurations
set(VCPKG_ENV_PASSTHROUGH_UNTRACKED VCPKG_FORCE_SYSTEM_BINARIES VCPKG_DISABLE_COMPILER_TRACKING)
set(VCPKG_BUILD_TYPE release)

# Use host tools for cross-compilation
set(VCPKG_CROSSCOMPILING ON)
set(VCPKG_TARGET_IS_WINDOWS ON)
"@
        Set-Content -Path $arm64TripletPath -Value $arm64TripletContent -Encoding UTF8
        Write-Host "Created ARM64 custom triplet at: $arm64TripletPath" -ForegroundColor Green
    }
    
    # Use the custom triplet for ARM64
    if ($Triplet -eq "arm64-windows") {
        $vcpkg_args = $vcpkg_args | ForEach-Object { if ($_ -match "--triplet=") { "--triplet=arm64-windows-custom" } else { $_ } }
    }
    
    # Add ARM64-specific flags
    $vcpkg_args += @(
        '--allow-unsupported',
        '--x-install-root=' + [System.IO.Path]::Combine($PWD.Path, 'vendor', 'vcpkg', 'installed')
    )
    
    Write-Host "Using ARM64 vcpkg arguments: $($vcpkg_args -join ' ')" -ForegroundColor Cyan
}

$packages = @('install', 'curl', 'dlfcn-win32', 'glfw3', 'libuv', 'libjpeg-turbo', 'libpng', 'libwebp') + $renderer_packages + $(if(${With-ICU}) {@('icu')} else {@()})

Write-Host "Installing packages: $($packages[1..($packages.Length-1)] -join ', ')" -ForegroundColor Green
Write-Host "Triplet: $Triplet" -ForegroundColor Green

try {
    # Run vcpkg with all arguments
    $vcpkgExe = '{0}\vcpkg.exe' -f $vcpkg_temp_dir
    $allArgs = $vcpkg_args + $packages
    
    Write-Host "Executing: $vcpkgExe $($allArgs -join ' ')" -ForegroundColor Cyan
    
    & $vcpkgExe $allArgs
    
    if ($LASTEXITCODE -ne 0) {
        if ($IsARM64) {
            Write-Host "vcpkg installation failed for ARM64. Attempting workarounds..." -ForegroundColor Yellow
            
            # Try installing packages one by one for better error diagnosis
            foreach ($package in $packages[1..($packages.Length-1)]) {
                Write-Host "Attempting to install: $package" -ForegroundColor Yellow
                $singlePackageArgs = $vcpkg_args + @('install', $package)
                
                & $vcpkgExe $singlePackageArgs
                
                if ($LASTEXITCODE -eq 0) {
                    Write-Host "Successfully installed: $package" -ForegroundColor Green
                } else {
                    Write-Warning "Failed to install: $package"
                    
                    # For critical packages, try alternative approaches
                    if ($package -in @('curl', 'libuv', 'libpng', 'libjpeg-turbo')) {
                        Write-Host "Trying to install $package with different options..." -ForegroundColor Yellow
                        $fallbackArgs = @('--disable-metrics', '--triplet=arm64-windows', '--allow-unsupported', 'install', $package)
                        & $vcpkgExe $fallbackArgs
                        
                        if ($LASTEXITCODE -eq 0) {
                            Write-Host "Successfully installed $package with fallback method" -ForegroundColor Green
                        }
                    }
                }
            }
        } else {
            Write-Error "vcpkg installation failed with exit code: $LASTEXITCODE"
            exit $LASTEXITCODE
        }
    } else {
        Write-Host "All packages installed successfully!" -ForegroundColor Green
        
        # Verify ARM64 installation
        if ($IsARM64) {
            $arm64InstallDir = [System.IO.Path]::Combine($PWD.Path, 'vendor', 'vcpkg', 'installed', 'arm64-windows-custom')
            if (-not (Test-Path $arm64InstallDir)) {
                $arm64InstallDir = [System.IO.Path]::Combine($PWD.Path, 'vendor', 'vcpkg', 'installed', 'arm64-windows')
            }
            
            if (Test-Path $arm64InstallDir) {
                Write-Host "ARM64 packages successfully installed at: $arm64InstallDir" -ForegroundColor Green
                
                # List installed packages for verification
                $installedPackages = Get-ChildItem -Path (Join-Path $arm64InstallDir 'share') -Directory | Select-Object -ExpandProperty Name
                Write-Host "Installed ARM64 packages: $($installedPackages -join ', ')" -ForegroundColor Green
            }
        }
    }
    
} catch {
    Write-Error "vcpkg execution failed: $_"
    if ($IsARM64) {
        Write-Host "This might be due to ARM64 toolchain issues. Please ensure:" -ForegroundColor Yellow
        Write-Host "1. Visual Studio 2022 with ARM64 build tools is installed" -ForegroundColor Yellow
        Write-Host "2. Windows SDK 10.0.19041.0 or later is installed" -ForegroundColor Yellow
        Write-Host "3. CMake 3.20 or later is installed" -ForegroundColor Yellow
    }
    throw
} finally {
    subst $vcpkg_temp_dir /D
}

Write-Host "Package installation completed!" -ForegroundColor Green
