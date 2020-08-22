@echo off

REM --time-passes
REM --stats
REM --enable-no-trapping-fp-math

llc.exe -filetype=obj ir_output.ll

lld-link -out:a.exe -defaultlib:libcmt ^
    "-libpath:C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Tools\MSVC\14.26.28801\lib\x64" ^
    "-libpath:C:\\Program Files (x86)\\Windows Kits\\10\\Lib\\10.0.17763.0\\ucrt\\x64" ^
    "-libpath:C:\\Program Files (x86)\\Windows Kits\\10\\Lib\\10.0.17763.0\\um\\x64" ^
    -nologo ir_output.obj
