import sys;
import os;
import platform;
import shutil;

import Utils;

def SDLDownload(location, version):

    # TODO: Maintain 2 places with this info
    target = f"{location}" + "/SDL2-" + f"{version}"

    if os.path.exists(target):
        # print("Required version already exists")
        target = Utils.NormalizeAndGetAbsolutePath(target)
        print(target)
        return

    url = "";
    zipPath = "";

    if(platform.system() == "Windows"):
        url = f"https://github.com/libsdl-org/SDL/releases/download/release-{version}/SDL2-devel-{version}-VC.zip"
        zipPath = f"{location}.zip"

    if(platform.system() == "Linux"):
        # TODO
        url = ""
        zipPath = ""

    assert url != ""
    assert zipPath != ""

    Utils.Download(url, zipPath)

    Utils.Unzip(zipPath, location)

    # Remove .zip file
    os.remove(zipPath)

    target = Utils.NormalizeAndGetAbsolutePath(target)
    print(target)

# Run
SDLDownload(sys.argv[1], sys.argv[2])