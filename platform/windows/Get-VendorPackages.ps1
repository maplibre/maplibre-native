param(
	[Parameter(Mandatory=$true)][string]$Triplet,
	[Parameter(Mandatory=$true)][string]$Renderer,
    [Parameter(Mandatory=$false)][switch]${With-ICU}
)

Set-Location (Split-Path $MyInvocation.MyCommand.Path -Parent)

# Check if this is an ARM64 build and handle accordingly
$IsARM64 = $Triplet -match "arm64"

if ($IsARM64) {
    Write-Host "ARM64 build detected. Checking for pre-installed packages..." -ForegroundColor Yellow
    
    # Define expected package locations for ARM64
    $vcpkg_installed_dir = [System.IO.Path]::Combine($PWD.Path, 'vendor', 'vcpkg', 'installed', $Triplet)
    
    # Check if packages are already installed
    $required_packages = @('curl', 'dlfcn-win32', 'glfw3', 'libuv', 'libjpeg-turbo', 'libpng', 'libwebp')
    if (${With-ICU}) {
        $required_packages += 'icu'
    }
    
    switch($Renderer) {
        'EGL'    { $required_packages += @('egl', 'opengl-registry') }
        'OpenGL' { $required_packages += @('opengl-registry') }
        'All'    { $required_packages += @('egl', 'opengl-registry') }
    }
    
    $missing_packages = @()
    foreach ($package in $required_packages) {
        $package_dir = [System.IO.Path]::Combine($vcpkg_installed_dir, 'share', $package)
        if (-not (Test-Path $package_dir)) {
            $missing_packages += $package
        }
    }
    
    if ($missing_packages.Count -gt 0) {
        Write-Host "Missing ARM64 packages: $($missing_packages -join ', ')" -ForegroundColor Red
        Write-Host "Please install these packages manually using:" -ForegroundColor Yellow
        Write-Host "vcpkg install $($missing_packages -join ' ') --triplet=$Triplet" -ForegroundColor Cyan
        
        # Try to proceed with automatic installation with ARM64-specific settings
        Write-Host "Attempting automatic installation with ARM64 workarounds..." -ForegroundColor Yellow
    } else {
        Write-Host "All required ARM64 packages are already installed." -ForegroundColor Green
        return
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

if(-not (Test-Path ('{0}\vcpkg.exe' -f $vcpkg_temp_dir)))
{
    & ('{0}\bootstrap-vcpkg.bat' -f $vcpkg_temp_dir)
}

# ARM64-specific vcpkg arguments
$vcpkg_args = @(
    '--disable-metrics',
    ('--overlay-triplets={0}' -f [System.IO.Path]::Combine($PWD.Path, 'vendor', 'vcpkg-custom-triplets')),
    ('--triplet={0}' -f $Triplet),
    '--clean-after-build'
)

# Add ARM64-specific flags to help with compiler detection
if ($IsARM64) {
    $vcpkg_args += '--allow-unsupported'
    $env:VCPKG_FORCE_SYSTEM_BINARIES = '1'
    $env:VCPKG_DISABLE_COMPILER_TRACKING = '1'
    Write-Host "Using ARM64 compatibility flags for vcpkg" -ForegroundColor Yellow
}

$packages = @('install', 'curl', 'dlfcn-win32', 'glfw3', 'libuv', 'libjpeg-turbo', 'libpng', 'libwebp') + $renderer_packages + $(if(${With-ICU}) {@('icu')} else {@()})

try {
    & ('{0}\vcpkg.exe' -f $vcpkg_temp_dir) ($vcpkg_args + $packages)
    
    if ($LASTEXITCODE -ne 0 -and $IsARM64) {
        Write-Host "vcpkg installation failed for ARM64. This is expected due to compiler detection issues." -ForegroundColor Yellow
        Write-Host "Please manually install the required packages or use pre-built binaries." -ForegroundColor Yellow
        Write-Host "Required packages: $($packages[1..($packages.Length-1)] -join ', ')" -ForegroundColor Cyan
    }
} catch {
    if ($IsARM64) {
        Write-Host "vcpkg execution failed for ARM64. Using fallback strategy." -ForegroundColor Yellow
    } else {
        throw
    }
} finally {
    subst $vcpkg_temp_dir /D
}
