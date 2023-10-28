#
# Hex Converter Tool
# 
# Usage:
# python3 hexconvertertool.py --input <input_file> --output <output_file> --mode [compress/decompress]
#

# When decompressing, the input file must be a text file with hex string. Output file will be a binary file.
# When compressing, the input file must be a binary file. Output file will be a text file with hex string that can be copied to the C Header file.

import argparse

def generate_file_from_hexstr(content: str):
    parts = content.split(',')
    bytearray_content = bytearray()
    for part in parts:
        bytearray_content.append(int(part, 16))
    return bytearray_content
    
def from_file_to_hex_string(file: bytearray):
    hex_string = ''
    for byte in file:
        hex_string += hex(byte) + ','
    print('count: ', len(file))
    return hex_string
    

if __name__ == '__main__':
    parser = argparse.ArgumentParser(description='Compress a file')
    parser.add_argument('-i', '--input', dest='input_file', help='Input file')
    parser.add_argument('-o', '--output', dest='output_file', help='Output file')
    parser.add_argument('-m', '--mode', dest='mode', help='Mode: compress/decompress')
    args = parser.parse_args()
    
    if not args.mode:
        raise Exception('Mode is required')
    if args.mode not in ['compress', 'decompress']:
        raise Exception('Mode must be compress or decompress')
    if not args.input_file:
        raise Exception('Input file is required')
    if not args.output_file:
        raise Exception('Output file is required')
    
    if args.mode == 'compress':
        with open(args.input_file, 'rb') as f:
            file = f.read()
        hex_string = from_file_to_hex_string(file)
        with open(args.output_file, 'w') as f:
            f.write(hex_string)
    elif args.mode == 'decompress':
        with open(args.input_file, 'r') as f:
            content = f.read()
        file = generate_file_from_hexstr(content)
        with open(args.output_file, 'wb') as f:
            f.write(file)
    else:
        raise Exception('Mode is not supported')
    
    print('Done')