import os;
import sys;
import platform;

import Utils;

def GlslangDownload(location):

    if(os.path.exists(location)):
        return

    urls = "";
    zipPath = f"{location}.zip";

    if(platform.system() == "Windows"):
        urls = ["https://github.com/KhronosGroup/glslang/releases/download/main-tot/glslang-master-windows-Release.zip",
                "https://github.com/KhronosGroup/glslang/releases/download/main-tot/glslang-master-windows-Debug.zip"]
    elif(platform.system() == "Linux"):
        urls = ["https://github.com/KhronosGroup/glslang/releases/download/main-tot/glslang-main-linux-Release.zip",
                "https://github.com/KhronosGroup/glslang/releases/download/main-tot/glslang-main-linux-Debug.zip"]

    assert urls != []

    for url in urls:
        Utils.Download(url, zipPath)
        Utils.Unzip(zipPath, location, True)

# Run
GlslangDownload(sys.argv[1])