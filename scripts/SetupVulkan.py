import requests
import os
import re
import subprocess

def compare_versions(version1, version2):
    v1_parts = version1.split('.')
    v2_parts = version2.split('.')

    for i in range(max(len(v1_parts), len(v2_parts))):
        v1 = int(v1_parts[i]) if i < len(v1_parts) else 0
        v2 = int(v2_parts[i]) if i < len(v2_parts) else 0

        if v1 < v2:
            return -1
        elif v1 > v2:
            return 1

    return 0

def get_latest_vulkansdk():
    base_path = "C:\\VulkanSDK"

    latest_version = None
    latest_version_path = None

    if os.path.exists(base_path):
        for entry in os.scandir(base_path):
            if entry.is_dir():
                version = entry.name
                if latest_version is None or compare_versions(version, latest_version) > 0:
                    latest_version = version
                    latest_version_path = entry.path
                    # platform dependent paths
                    latest_version_path = latest_version_path.replace('\\', '/')
    
    return latest_version, latest_version_path

def extract_version(string):
    pattern = r"(\d+\.\d+\.\d+\.\d+)"
    match = re.search(pattern, string)
    if match:
        return match.group(1)
    else:
        return None

# Download Vulkan SDK installer
# TODO: Make it so it downloads actually the latest
# Should be system or project wide?
url = "https://sdk.lunarg.com/sdk/download/1.3.250.1/windows/VulkanSDK-1.3.250.1-Installer.exe"
installer_path = "vulkan_sdk_installer.exe"

latest_installed_version = get_latest_vulkansdk()[0]

if latest_installed_version == None or compare_versions(url.split("/")[-3], latest_installed_version) > 0:
    response = requests.get(url)
    with open(installer_path, "wb") as installer_file:
        installer_file.write(response.content)

        # Run the Vulkan SDK installer silently
        subprocess.run([installer_path, "/S"], check=True)

        # Remove the installer
        subprocess.run(["del", installer_path], check=True)

print(get_latest_vulkansdk()[1])