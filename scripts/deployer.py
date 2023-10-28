import hexconvertertool
import os

def makefile(definename: str, bytearrayname: str, hexstr: str, size: int):
    content = """#ifndef _""" + definename + """_H_
#define _""" + definename + """_H_
const uint8_t """ + bytearrayname + """_gz[] = {""" + hexstr + """};
#define """ + bytearrayname + """_gz_len """ + str(size) + """

#endif
"""
    return content

if __name__ == '__main__':
    # index
    input_html = os.path.join('scripts', 'page', 'index.html')
    gzmiddle = os.path.join('scripts', 'page', 'index.gz')
    output_html = os.path.join('main', 'include', 'index.h')
    os.system('gzip -c -9 {input_html} > {gzmiddle}'.format(input_html=input_html, gzmiddle=gzmiddle))
    with open(gzmiddle, 'rb') as f:
        gzcontent_bytes = f.read()
    hexstr, count = hexconvertertool.from_file_to_hex_string(gzcontent_bytes)
    makefile_content = makefile("INDEX", "index_html", hexstr, count)
    with open(output_html, 'w') as f:
        f.write(makefile_content)
        
    # worker
    # input_html = os.path.join('scripts', 'page', 'worker.js')
    # gzmiddle = os.path.join('scripts', 'page', 'worker.gz')
    # output_html = os.path.join('main', 'include', 'worker.h')
    # os.system('gzip -c -9 {input_html} > {gzmiddle}'.format(input_html=input_html, gzmiddle=gzmiddle))
    # with open(gzmiddle, 'rb') as f:
    #     gzcontent_bytes = f.read()
    # hexstr, count = hexconvertertool.from_file_to_hex_string(gzcontent_bytes)
    # makefile_content = makefile("WORKER", "worker_js", hexstr, count)
    # with open(output_html, 'w') as f:
    #     f.write(makefile_content)