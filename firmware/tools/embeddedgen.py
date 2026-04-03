from argparse import ArgumentParser
from pathlib import Path
import gzip, math, re, os

parser = ArgumentParser(description="SGU Embedded Gen")
parser.add_argument("-i", "--inputpath", required=True, help="Input path (files to embed)")
parser.add_argument("-o", "--outputcodepath", required=True, help="Output path for code")
parser.add_argument("-c", "--compress", default=None, help="Input compression configuration file")
parser.add_argument("--compress-all", action="store_true", help="Compress all compressible files (text, svg, html, js, css, etc.)")

args = parser.parse_args()

# Extensions that benefit from gzip compression
COMPRESSIBLE_EXTENSIONS = {'.html', '.htm', '.css', '.js', '.svg', '.txt', '.md', '.json', '.xml'}

def GetSizeString(size):
    unit = int(math.log2(size) / 10) if size else 0
    size /= math.pow(2, unit * 10)
    return f"{round(size)} {' KMGTPE'[unit]}B"

def BinToHexArray(data):
    hexdata = "\\x" + data.hex("_", 1).replace("_", "\\x")
    output = []
    pos = 0
    while pos < len(hexdata):
        output.append(f'"{hexdata[pos:pos+400]}"')
        pos += 400
    return "\n".join(output)

class PackedFile:
    def __init__(self, inputpath, path, compress=False):
        # Relative path should remove the input path and keep the relative part
        relpath = os.path.relpath(str(path).replace("\\", "/"), inputpath.replace("\\", "/")).replace("\\", "/")

        self.path = Path(path)  # Path object
        self.relpath = relpath  # Relative normalized path
        self.keyname = "EF_EFILE_%s" % re.sub("[^a-zA-Z0-9]", "_", self.relpath).upper()
        self.blob = self.path.open("rb").read()  # Binary data
        self.original_size = len(self.blob)   # decompressed file size (no null)
        self.compressed = False
        if compress:
            try:
                self.blob = gzip.compress(self.blob, 9)
                self.compressed = True
            except:
                print(f"WARNING: Failed to compress {self.relpath} !")
        self.send_size = len(self.blob)       # bytes to send over the wire
        self.blob += b"\x00"  # End marker
        self.blob += b"\x00" * (4 - (len(self.blob) % 4))  # alignment
        self.flags = 'EF_EFLAGS_GZip' if self.compressed else 'EF_EFLAGS_None'

try:
    diInput = Path(args.inputpath)
    if not diInput.exists():
        raise Exception("Input path doesn't exists")

    diOutputCodePath = Path(args.outputcodepath)
    if not diOutputCodePath.exists():
        raise Exception("Output path doesn't exists")

    compressConfig = Path(args.compress) if args.compress else None
    if compressConfig and not compressConfig.exists():
        raise Exception("No GZIP config file provided")

    # Scan for files
    compressFiles = [Path(file) for file in compressConfig.read_text().splitlines()] if compressConfig else []
    files = []

    for file in diInput.rglob("*.*"):
        # Determine whether to compress this file
        compress_by_config = len([p for p in [file, *file.parents] if p in compressFiles]) > 0
        compress_by_ext = args.compress_all and file.suffix.lower() in COMPRESSIBLE_EXTENSIONS
        myfile = PackedFile(args.inputpath, file, compress_by_config or compress_by_ext)
        files.append(myfile)
        print(f"Adding file: {myfile.relpath} {'(compressed)' if myfile.compressed else ''}")

    fileTXT = diOutputCodePath / "EmbeddedFiles.txt"
    print(f"Export list: {fileTXT}")
    fileTXT.write_text("\n".join(file.relpath for file in files))

    fileB = diOutputCodePath / "EmbeddedFiles.bin"
    print(f"Generating file: {fileB}")
    fileB.write_bytes(b"".join(file.blob for file in files))

    fileH = diOutputCodePath / "EmbeddedFiles.h"
    print(f"Generating file: {fileH}")
    with fileH.open("w") as fp:
        fp.write("#ifndef _EMBEDDEDFILES_H_\n")
        fp.write("#define _EMBEDDEDFILES_H_\n")
        fp.write("\n")
        fp.write("#ifdef __cplusplus\n")
        fp.write("extern \"C\" {\n")
        fp.write("#endif\n")
        fp.write("\n")
        fp.write("#include <stdint.h>\n")
        fp.write("\n")
        fp.write("typedef enum\n")
        fp.write("{\n")
        fp.write("    EF_EFLAGS_None = 0,\n")
        fp.write("    EF_EFLAGS_GZip = 1,\n")
        fp.write("} EF_EFLAGS;\n")
        fp.write("\n")
        fp.write("typedef struct\n")
        fp.write("{\n")
        fp.write("    const char* filename;\n")
        fp.write("    uint32_t length;             /*!< @brief Original (decompressed) file size */\n")
        fp.write("    uint32_t compressed_length;  /*!< @brief Bytes to send over the wire (equals length when not compressed) */\n")
        fp.write("    EF_EFLAGS flags;\n")
        fp.write("    const uint8_t* start_addr;\n")
        fp.write("} EF_SFile;\n")
        fp.write("\n")
        fp.write("typedef enum\n")
        fp.write("{\n")
        for file in files:
            fp.write(f"    {file.keyname} = {files.index(file)},    /*!< @brief File: {file.relpath} (size: {GetSizeString(file.original_size)}) */\n")
        fp.write(f"    EF_EFILE_COUNT = {len(files)}\n")
        fp.write("} EF_EFILE;\n")
        fp.write("\n")
        fp.write("/*! @brief Check if compressed flag is active */\n")
        fp.write("#define EF_ISFILECOMPRESSED(x) ((x & EF_EFLAGS_GZip) == EF_EFLAGS_GZip)\n")
        fp.write("\n")
        fp.write("extern const EF_SFile EF_g_files[EF_EFILE_COUNT];\n")
        fp.write("extern const uint8_t EF_g_blobs[];\n")
        fp.write("\n")
        fp.write("#ifdef __cplusplus\n")
        fp.write("}\n")
        fp.write("#endif\n")
        fp.write("\n")
        fp.write("#endif\n")

    fileC = diOutputCodePath / "EmbeddedFiles.c"
    print(f"Generating file: {fileC}")
    with fileC.open("w") as fp:
        bigBlobs = b"".join(file.blob for file in files)
        fp.write('#include "EmbeddedFiles.h"\n')
        fp.write("\n")
        fp.write(f"/*! @brief Total size: {GetSizeString(sum(file.original_size for file in files))}, Packed size: {GetSizeString(len(bigBlobs))} */\n")
        fp.write("const EF_SFile EF_g_files[EF_EFILE_COUNT] = \n")
        fp.write("{\n")
        offset = 0
        for file in files:
            compressed_length = file.send_size  # equals original size when not compressed
            fp.write(f"    [{file.keyname}] = {{ \"{file.relpath}\", {file.original_size}, {compressed_length}, {file.flags}, &EF_g_blobs[{offset}]}},")
            fp.write(f"/* wire: {GetSizeString(file.send_size)}, original: {GetSizeString(file.original_size)} */")
            fp.write("\n")
            offset += len(file.blob)
        fp.write("};\n")
        fp.write("\n")
        fp.write(f"const uint8_t EF_g_blobs[] = \n{BinToHexArray(bigBlobs)};\n")

except Exception as e:
    # raise (e)
    print(f"Error: {e}")
