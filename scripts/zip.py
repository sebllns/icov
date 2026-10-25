import zipfile
import pathlib

icov_file = 'test_Doi_Suthep'

with zipfile.ZipFile(icov_file + '.zip', mode="w") as archive:
    archive.write(icov_file + '.info')
    for file_path in pathlib.Path(icov_file + '.icov/').iterdir():
        archive.write(file_path)
