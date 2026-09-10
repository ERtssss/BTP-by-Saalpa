#!/usr/bin/env python3
import argparse, json, zipfile
from pathlib import Path

def main():
    ap=argparse.ArgumentParser()
    ap.add_argument('--library', required=True, type=Path)
    ap.add_argument('--icon', required=True, type=Path)
    ap.add_argument('--output', required=True, type=Path)
    a=ap.parse_args()
    if not a.library.is_file(): raise SystemExit(f'Missing library: {a.library}')
    if not a.icon.is_file(): raise SystemExit(f'Missing icon: {a.icon}')
    manifest={
        'type':'preload-native',
        'name':'Bedrock Tools Plus',
        'author':'Saalpa',
        'version':'1.0.0',
        'entry':'libBTP.so',
        'icon':'icon.png',
        'minecraft_versions':[],
        'overwrite_files':['icon.png'],
        'overwrite_folders':[]
    }
    a.output.parent.mkdir(parents=True, exist_ok=True)
    with zipfile.ZipFile(a.output,'w',zipfile.ZIP_DEFLATED,compresslevel=9) as z:
        z.writestr('manifest.json',json.dumps(manifest,indent=2,ensure_ascii=False)+'\n')
        z.write(a.library,'libBTP.so')
        z.write(a.icon,'icon.png')
    with zipfile.ZipFile(a.output) as z:
        names=set(z.namelist())
        expected={'manifest.json','libBTP.so','icon.png'}
        if names != expected: raise SystemExit(f'Bad package entries: {sorted(names)}')
    print(a.output.resolve())
if __name__=='__main__': main()
