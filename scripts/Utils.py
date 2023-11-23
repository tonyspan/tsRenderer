import os;
import sys;
import platform;
import requests;
import tqdm;
import zipfile;

def Download(url, path):

    if(os.path.isfile(path) is True):
        return;

    request = requests.get(url, stream = True)
    percentage = int(request.headers.get("content-lenght", 0))

    with open(path, "wb") as file, tqdm.tqdm(desc = path, total = percentage, unit = "iB", unit_scale = True, unit_divisor = 1024) as bar:
        for data in request.iter_content(chunk_size = 1024):
            size = file.write(data)
            bar.update(size)

def Unzip(path, location):
    with zipfile.ZipFile(path, 'r') as zip_ref:
        for file_info in zip_ref.infolist():
            zip_ref.extract(file_info, location)

def NormalizeAndGetAbsolutePath(path):
    path = os.path.abspath(path)
    path = os.path.normpath(path)

    return path