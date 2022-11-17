from multiprocessing.pool import ThreadPool
from PIL import Image
import os
import shutil
import argparse
import subprocess
import time
import zlib

# C compiler
CC = "/usr/bin/gcc"
CC_WIN = "/usr/bin/x86_64-w64-mingw32-gcc"

# C compiler flags
CFLAGS = "-O2 -march=native -Iinclude -Itmp"

# C libraries
LFLAGS = "-lglfw -lGL -lGLU -lm -lz"
LFLAGS_WIN = "-static -lglfw3 -lopengl32 -lgdi32 -lm -lz -Wl,-subsystem,windows"

# C sources
CSRC = "sources"

# Shader sources
SSRC = "shaders"

# Texture sources
TSRC = "atlas.png"

# Executable filename
EXE = "csweep"

# Object file build path
objectPath = "tmp"

# Cpu's used for compiling
compileProcesses = 2

def build():
    start_time = time.time()
    # Generate shader.h
    if genShaderHeader(SSRC + '/'):
        print("generated shaders.h")
    else: 
        print("couldnt generate shaders.h")

    # Generate texture.h
    if genTextureHeader(TSRC):
        print("generated texture.h")
    else: 
        print("couldnt generate texture.h")

    # Compile object files on multiple threads
    compileObjectFiles(CSRC + '/', compileProcesses)

    # Compile object files to executable
    cObjects = os.listdir(objectPath)
    stringfy : str = ""
    for obj in cObjects:
        if obj.find('.o') != -1:
            stringfy += " " + objectPath + '/' + obj
    cArgs : str = CC + " " + CFLAGS + " -o " + EXE + stringfy + " " + LFLAGS
    print(cArgs)
    subprocess.run(cArgs.split(' '), stdout=subprocess.PIPE)
    print("compiled in", (time.time() - start_time))

def compileObjectFiles(dir: str, thr: int):
    if not os.path.exists(objectPath):
        os.mkdir(objectPath)
    
    sources = os.listdir(dir)

    pool = ThreadPool(thr)

    pool.map(compileObj, sources)

    pool.close()
    pool.join()

def compileObj(sources):
    for cFile in sources.split(' '):
        cArgs : str = CC + " " + CFLAGS + " -c -o " + objectPath + '/' + cFile.replace(".c", ".o") + " " + CSRC + '/' + cFile + " " + LFLAGS
        print(cArgs)
        subprocess.run(cArgs.split(' '), stdout=subprocess.PIPE)

def genShaderHeader(path: str):
    if not os.path.exists(objectPath):
        os.mkdir(objectPath)
    dir = os.listdir(path)

    headerFile = ""

    for folder in dir:
        shader = os.listdir(path + folder)

        vertex = ""
        fragment = ""

        for file in shader:
            if file.find(".vert") != -1:
                vertex = file
            else:
                fragment = file
        
        desc = open(path + folder + '/' + vertex, "rb")

        s = desc.read()
        s += bytes([0])
        length = len(s)
        desc.close()
        desc = open(path + folder + '/' + fragment, "rb")
        s += desc.read()
        desc.close()

        compressed = zlib.compress(s)

        c = ""
        for b in compressed:
            c += hex(b).upper().replace('X', 'x') +", "

        headerFile += "static const unsigned int " + folder.upper() + "_FS_OFFSET = " + hex(length).upper().replace('X', 'x') + ", " + folder.upper() + "_I_LENGTH = " + hex(len(s)).upper().replace('X', 'x') + ";\nstatic const unsigned char " + folder.upper() + "[] = { " + c + "};\n\n"

    desc = open(objectPath + "/shaders.h", "w")
    desc.write("#ifndef SHADER_H\n#define SHADER_H\n\n" + headerFile + "#endif")
    desc.close()
    return 1

def genTextureHeader(path: str):
    if not os.path.exists(objectPath):
        os.mkdir(objectPath)
    img = Image.open(path)
    atlas = open(objectPath + "/atlas.h", "w")

    buffer = list(img.getdata())
    x, y = img.size

    tmp = []
    for rgb in buffer:
        if len(rgb) == 4:
            for i in range(4):
                tmp.append(rgb[i])
        else:
            if rgb[0] == 255 and rgb[1] == 255 and rgb[2] == 255:
                tmp.append(0)
                tmp.append(0)
                tmp.append(0)
                tmp.append(0)
            else:
                tmp.append(rgb[0])
                tmp.append(rgb[1])
                tmp.append(rgb[2])
                tmp.append(255)

    compressed = zlib.compress(bytes(tmp))

    c = ""
    for b in compressed:
        c += hex(b).upper().replace('X', 'x') +", "

    data = "static const unsigned int ATLAS_WIDTH = " + hex(x).upper().replace('X', 'x') + ", ATLAS_HEIGTH = " + hex(y).upper().replace('X', 'x') + ";\nstatic const unsigned char ATLAS[] = { " + c + "}; \n\n"

    atlas.write("#ifndef ATLAS_H\n#define ATLAS_H\n\n" + data + "#endif");

    atlas.close()
    return 1

def clean():
    if os.path.exists(objectPath):
        shutil.rmtree(objectPath)
    if os.path.exists(EXE):
        os.remove(EXE)
    elif os.path.exists(EXE + '.exe'):
        os.remove(EXE + '.exe')

parser = argparse.ArgumentParser()
parser.add_argument("--threads", help="Cpu's used for compiling", type=int)
parser.add_argument("--clean", help="Removes generated directories and files", action="store_true")
parser.add_argument("--headers", help="Generates development headers", action="store_true")
parser.add_argument("--win", help="Compiles for windows", action="store_true")
args = parser.parse_args()

if args.win:
    CC = CC_WIN
    LFLAGS = LFLAGS_WIN
    print("target platform windows")

if args.headers:
    if genShaderHeader(SSRC + '/'):
        print("generated shaders.h")
    else: 
        print("couldnt generate shaders.h")
    if genTextureHeader(TSRC):
        print("generated texture.h")
    else: 
        print("couldnt generate texture.h")
elif args.clean:
    clean()
else:
    if args.threads != None and args.threads > 1:
        compileProcesses = args.threads
    build()
