"""
CMake script for Python

Technically we do not need to make a separate CMake directory for Python. We 
can just use the normal cmake build directory. Howevever, cmake for Android 
requires a lot of settings that can be hard to remember. This script simplifies
the process for testing\

Author: Walker M. White
Version: March 3, 2026
"""
import argparse
import subprocess
import os, os.path
import sys
from pathlib import Path
import re

def setup():
    """
    Returns an initialized parser for this script.
    
    :return: The parsed args
    :rtype:  ``Namespace``
    """
    parser = argparse.ArgumentParser(description='Build a VulkanSDL project.')
    parser.add_argument('source', type=str, help='The source directory')
    parser.add_argument('build', type=str, help='The build directory',nargs='?')
    parser.add_argument('-s', '--sdk', type=str, help='The ANDROID_HOME directory')
    parser.add_argument('-n', '--ndk',  type=str, help='The NDK version identifier')
    parser.add_argument('-p', '--platform',  type=int, help='The Android platform number')
    parser.add_argument('-a', '--abi', type=str,
                        choices=['arm64-v8a','armeabi-v7a','x86','x86_64'],
                        help='The Android ABI')
    return parser


def parse_version(s):
    """
    Returns the version number parsed into a three element tuple.
    
    The three-element tuple is (major, minor, patch). It returns None if s is
    not a simple dotted-integer version.
    
    Example: '29.0.13113456' -> (29, 0, 13113456)
    Example: '21.0.6113669'  -> (21, 0, 6113669)
    Example: '28'            -> (28, 0, 0)
    
    :param s: The string to parse
    :type s:  ``str``
    
    :return: the version number parsed into a three element tuple
    :rtype:  tuple or None
    """
    if type(s) != str:
        return None
    
    VERSION_RE = re.compile(r"^(\d+)(?:\.(\d+))?(?:\.(\d+))?$")
    m = VERSION_RE.match(s)
    if not m:
        return None
    
    parts = [int(p) if p is not None else 0 for p in m.groups()]
    return (parts[0], parts[1], parts[2])


def get_ndk(parser,args):
    """
    Returns the path to the Android NDK
    
    This method uses the SDK directory given to the parser, or $ANDROID_HOME if
    it does not exist. It matches the most recent NDK that is less than or 
    equal to the one requested.
    
    This method passes an error to the parser and returns None if it cannot
    find the NDK directory.
    
    :param parser: The argument parser
    :type parser:  ``ArgumentParser``
    
    :param args: The parsed args
    :type args:  ``Namespace``
    
    :return: The path to the Android NDK
    :rtype: ``str``
    """
    sdk = args.sdk
    if sdk is None:
        sdk = os.environ.get("ANDROID_HOME")
    
    if sdk is None:
        parser.error(f'SDK must be specifed if $ANDROID_HOME is not set.')
        return None
    elif not os.path.exists(sdk):
        parser.error(f'{repr(sdk)} does not exist.')
        return None
    elif not os.path.isdir(sdk):
        parser.error(f'{repr(sdk)} is not a directory.')
        return None
    
    
    # Now search for ndk directory
    path = os.path.join(sdk,'ndk')
    if not os.path.isdir(path):
        parser.error(f'Could not locate NDK directory {repr(path)}.')
        return None
    
    target = parse_version(args.ndk)
    ndks = []
    if target is None:
        for item in os.listdir(path):
            ndks.append(item)
    else:
        ndks = []
        for item in os.listdir(path):
            version = parse_version(item)
            if (version[0] <= target[0] and 
                    (target[1] == 0 or version[1] <= target[1]) and
                    (target[2] == 0 or version[2] <= target[2])
                ):
                ndks.append(item)
    ndks.sort()
    return os.path.join(path,ndks[-1])


def main():
    """
    Runs the cmake script
    """
    parser = setup()
    args = parser.parse_args()
    ndk  = get_ndk(parser,args)
    
    abi = args.abi
    if abi is None:
        abi = 'arm64-v8a'
    
    platform = args.platform
    if platform is None:
        platform = 28
    
    source = args.source
    build  = args.build
    if build is None:
        build = '.'
    
    command = ['cmake','-S',source,'-B',build,
                f'-DANDROID_ABI={abi}',
                f'-DANDROID_PLATFORM=android-{platform}',
                f'-DANDROID_NDK={repr(ndk)}',
                f'-DANDROID_STL=c++_static',
                f"-DCMAKE_TOOLCHAIN_FILE='{ndk}/build/cmake/android.toolchain.cmake'"]
    if sys.platform == "win32":
        command.append('-G')
        command.append('Ninja')
    
    print(f'Using NDK directory {repr(ndk)}')
    subprocess.run(command,check=True)


if __name__ == '__main__':
    main()
