"""
this python test file checks if the result of the decoder is correct
takes all the images with extension `*jpg` and converts them to ppm
and compares the converted image with the result of the decoder
"""
from PIL import Image
import os
import subprocess


def produceImages(inputFile: str, outputFilePpm: str, outputFileBmp: str):
    try:
        executable = "./tests/testUtils/produceImages"
        completedProcess = subprocess.run([executable, inputFile, outputFilePpm, outputFileBmp], capture_output = True, text = True)
        if completedProcess.returncode == 1:
            print(f"ERROR: could not decode image: {inputFile}")
    except FileNotFoundError:
        print(f"Error: The executable '{executable}' was not found.")
    except subprocess.CalledProcessError as e:
        print(f"Error occurred while running the executable: {e}")

# compares the ppm produced by the decoder to the ppm produced by python
def comparePpm(file_1: str, file_2: str) -> int:
    try:
        executable = "./tests/testUtils/compareppm"
        completedProcess = subprocess.run([executable, file_1, file_2], capture_output = True, text = True)
        return completedProcess.returncode

    except FileNotFoundError:
        print(f"Error: The executable '{executable}' was not found.")
    except subprocess.CalledProcessError as e:
        print(f"Error occurred while running the executable: {e}")

def compareBmptoPpm(ppmFile: str, bmpFile: str) -> int:
    try:
        executable = "./tests/testUtils/comparebmptoppm"
        completedProcess = subprocess.run([executable, ppmFile, bmpFile], capture_output = True, text = True)
        return completedProcess.returncode

    except FileNotFoundError:
        print(f"Error: The executable '{executable}' was not found.")
    except subprocess.CalledProcessError as e:
        print(f"Error occurred while running the executable: {e}")

def getJpegFiles(directoryPath: str) -> list[str]:
    return [i for i in os.listdir(directoryPath) if i.lower().endswith(".jpg")]

def jpegToPpm(inputPath: str, outputPath: str) -> None:
    with Image.open(inputPath) as img:
        img.save(outputPath, format='PPM')

def test() -> None:
    inputDirectoryPath = "baselineImages"
    outputDirectoryPath = "tests"
    for inputFile in getJpegFiles(inputDirectoryPath):
        outputFile = inputFile[:-3]
        produceImages(f"{inputDirectoryPath}/{inputFile}", f"{outputDirectoryPath}/{outputFile}ppm.c", f"{outputDirectoryPath}/{outputFile}bmp.c")
        jpegToPpm(f"{inputDirectoryPath}/{inputFile}", f"{outputDirectoryPath}/{outputFile}ppm.python")
        print(f"testing {outputFile}ppm:\t\t\t", end = '')
        if comparePpm(f"{outputDirectoryPath}/{outputFile}ppm.c", f"{outputDirectoryPath}/{outputFile}ppm.python") == 1:
            print("FAILED")
        else:
            print("OK")
        print(f"testing {outputFile}bmp:\t\t\t", end = '')
        if comparePpm(f"{outputDirectoryPath}/{outputFile}ppm.c", f"{outputDirectoryPath}/{outputFile}bmp.c") == 0:
            print("FAILED")
        else:
            print("OK")
        
        os.remove(f"{outputDirectoryPath}/{outputFile}ppm.c")
        os.remove(f"{outputDirectoryPath}/{outputFile}ppm.python")
        os.remove(f"{outputDirectoryPath}/{outputFile}bmp.c")



if __name__ == "__main__":
    test()
