"""
Python Script for CMake Builds

While CMake is naturally "out of source", there are still many reasons why we
want to autogenerate the CMake files. In particular, it makes it easier to link
in a new CUGL base (as upgrades are released) without having to change any files
in the project.

Note that, because of how CMake works, this script both autogenerates a new
CMake file and then makes a separate build directory. For cleanliness, the
new build directory is a subdirectory of the original build directory. This
structure helps with Flatpak generation on Linux.

Author: Walker M. White
Version: 7/10/24
"""
import os, os.path
import shutil
import glob
import re, string
from subprocess import check_output
from . import util

# The subbuild folder for cmake
MAKEDIR = 'cmake'

# Supported header extensions
HEADER_EXT = ['.h', '.hh', '.hpp', '.hxx', '.hm', '.inl', '.inc']
# Supportes source extensions
SOURCE_EXT = ['.cpp', '.c', '.cc', '.cxx', '.m', '.mm','.asm', '.asmx','.swift']


def expand_cmake_sources(path, filetree):
    """
    Returns the string of source files to insert into CMake file

    This string should replace __SOURCE_FILES__ in the makefile.

    :param path: The path to the root directory for the filters
    :type path:  ``str1``

    :param filetree: The file tree storing both files and filters
    :type filetree:  ``dict``

    :return: The string of source files to insert into Android.mk
    :rtype:  ``str``
    """
    result = ''
    for key in filetree:
        # Recurse on directories
        if type(filetree[key]) == dict:
            result += expand_cmake_sources(path+'/'+key,filetree[key])
        else:
            category = filetree[key]
            if category in ['all', 'cmake'] and os.path.splitext(key)[1] in SOURCE_EXT:
                result += '\n    %s/%s' % (path,key)
    return result


def expand_cmake_includes(path, filetree):
    """
    Returns a set of directories to add to CMake for inclusion

    :param path: The path to the root directory for the filters
    :type path:  ``str1``

    :param filetree: The file tree storing both files and filters
    :type filetree:  ``dict``

    :return: A set of directories to add to Android.mk for inclusion
    :rtype:  ``set``
    """
    result = set()
    for key in filetree:
        # Recurse on directories
        if type(filetree[key]) == dict:
            if path is None:
                result.update(expand_cmake_includes(key,filetree[key]))
            else:
                result.update(expand_cmake_includes(path+'/'+key,filetree[key]))
        else:
            category = filetree[key]
            if category in ['all', 'cmake'] and not (os.path.splitext(key)[1] in SOURCE_EXT):
                if path is None:
                    result.add('')
                else:
                    result.add(path)

    return result


def place_project(config):
    """
    Places the CMakeLists.txt in the project directory

    :param config: The project configuration settings
    :type config:  ``dict``

    :return: The project directory
    :rtype:  ``str``
    """
    entries = ['short','sdl2','build']
    util.check_config_keys(config,entries)

    # Create the build folder if necessary
    build = config['build']
    if not os.path.exists(build):
        os.mkdir(build)

    # Clear and create the build folder
    project = util.remake_dir(build,MAKEDIR)
    src  = os.path.join(config['sdl2'],'templates','cmake','ReadMe.md')
    dst = os.path.join(project, 'ReadMe.md')
    shutil.copyfile(src,dst)

    # Copy the CMakefile
    src = os.path.join(config['sdl2'],'templates','cmake','CMakeLists.txt')
    dst = os.path.join(project, 'CMakeLists.txt')
    shutil.copyfile(src,dst)

    # Copy the flatpak directory
    appid = config['appid']
    if 'suffix' in config and config['suffix']:
        appid = '.'.join(appid.split('.')[:-1])

    src = os.path.join(config['sdl2'],'templates','cmake','flatpak')
    dst = os.path.join(project, 'flatpak')
    shutil.copytree(src, dst, copy_function = shutil.copy)

    ymlsrc = os.path.join(dst,'__APP_ID__.yml')
    ymldst = os.path.join(dst,appid+'.yml')
    shutil.move(ymlsrc,ymldst)

    ymlsrc = os.path.join(dst,'__APP_ID__-validation.yml')
    ymldst = os.path.join(dst,appid+'-validation.yml')
    shutil.move(ymlsrc,ymldst)

    # Make the work folder
    workdir = os.path.join(project,'cmake')
    os.mkdir(workdir)
    return project


def config_cmake(config,project):
    """
    Configures the contents of CMakeLists.txt

    The CMakeLists.txt list template must be modified to use the values in the
    config file. This function reads the CMakeLists.txt into a string, modifies
    the string, and then writes out the result.

    :param config: The project configuration settings
    :type config:  ``dict``

    :param project: The project directory
    :type project:  ``str``
    """
    entries = ['root','name','short','version','sdl2','sources', 'assets', 'build_to_sdl2','build_to_root']
    util.check_config_keys(config,entries)

    cmake = os.path.join(project, 'CMakeLists.txt')

    # Set the application name and version
    context ={}
    context['__TARGET__'] = config['short']
    context['__APPNAME__'] = config['name']
    context['__VERSION__'] = config['version']

    # Set the SDL2 directory
    prefix = ['..']
    sdl2dir = os.path.join(*prefix,config['build_to_sdl2'])
    context['__SDL2DIR__'] = util.path_to_posix(sdl2dir)

    # Set the Asset directory
    assetdir = os.path.join(*prefix,config['build_to_root'],config['assets'])
    context['__ASSETDIR__'] = util.path_to_posix(assetdir)
    context['__APP_ID__'] = config['appid']

    # Set the sources
    filetree = config['source_tree']
    localdir = os.path.join(*prefix,config['build_to_root'])
    localdir = '${PROJECT_SOURCE_DIR}/'+util.path_to_posix(localdir)
    if len(config['source_tree']) == 1:
        key = list(config['source_tree'].keys())[0]
        localdir += '/'+util.path_to_posix(key)
        filetree = filetree[key]
    context['__SOURCELIST__'] = expand_cmake_sources(localdir,filetree)

    # Set the include directories
    inclist = []
    entries = config['include_dict']
    inclist.extend(entries['all'] if ('all' in entries and entries['all']) else [])
    inclist.extend(entries['cmake'] if ('cmake' in entries and entries['cmake']) else [])

    for item in expand_cmake_includes(None,filetree):
        inclist.append(item)

    incstr = ''
    for item in inclist:
        path = os.path.join(*prefix,config['build_to_root'],item)
        path = '${PROJECT_SOURCE_DIR}/'+util.path_to_posix(path)
        incstr += 'list(APPEND EXTRA_INCLUDES "'+path+'")\n'
    context['__EXTRA_INCLUDES__'] = incstr

    util.file_replace(cmake,context)


def config_flatpak(config,project):
    """
    Configures the contents of the flatpak directory.

    The Flatpak scripts must all be modified to use the unique id for this
    application.  This will read each of the three files and then Modify
    them.

    :param config: The project configuration settings
    :type config:  ``dict``

    :param project: The project directory
    :type project:  ``str``
    """
    entries = ['root','name','short','appid']
    util.check_config_keys(config,entries)

    appid = config['appid']
    if 'suffix' in config and config['suffix']:
        appid = '.'.join(appid.split('.')[:-1])

    short = config['short'].lower()

    context = {'__APP_ID__':appid, '__SHORT__':short}
    ymlfile = os.path.join(project, 'flatpak', appid+'.yml')
    util.file_replace(ymlfile,context)

    # Find the validation layers on this platform
    validationlib = find_validation()
    if validationlib:
        libname = os.path.split(validationlib)[1]
        context['__MOVE_VALIDATION_LAYER__\n'] = '    - mv %s /app/lib/%s\n' % (libname,libname)
        context['__COPY_VALIDATION_LAYER__\n'] = '    - type: file\n      path: %s\n' % validationlib
    else:
        print('   WARNING: Could not find validation layer for Flatpak')
        context['__MOVE_VALIDATION_LAYER__\n'] = ''
        context['__COPY_VALIDATION_LAYER__\n'] = ''

    ymlfile = os.path.join(project, 'flatpak', appid+'-validation.yml')
    util.file_replace(ymlfile,context)

    name = config['name']
    pattern = re.compile('[^\w_]+')
    shortcut = pattern.sub('',name)
    context['__GAME__'] = name
    context['__SHORTCUT__'] = shortcut

    shellfile = os.path.join(project, 'flatpak', 'build.sh')
    util.file_replace(shellfile,context)

    flatfile = os.path.join(project, 'flatpak', 'flatpak-run.sh')
    util.file_replace(flatfile,context)


def find_validation():
    """
    Returns the path to validation layer library on the current platform.

    This function is generally designed for Linux, as it is looking for an .so
    file. That is because it is for packaging validation layers with flatpak

    :return: The path to validation layer library on the current platform
    :rtype:  ``str``
    """
    libs = []
    if 'VULKAN_SDK' in os.environ:
        path = os.path.join(os.environ['VULKAN_SDK'],'lib')
        if os.path.exists(path):
            files = os.path.join(path,'*')
            libs = glob.glob(files)

    if len(libs) == 0:
        try:
            out = check_output(['ldconfig', '-p']).decode('ascii')
            libs = out.split('\n')
            libs = list(map(lambda x: x[x.find('=> ')+3:],libs))
        except:
            pass

    libs = list(filter(lambda x: 'libVkLayer_khronos_validation' in x, libs))

    if len(libs) > 0:
        return libs[0]
    return None


def make(config):
    """
    Creates the CMake build

    This only creates the CMake build; it does not actually build the project. To build
    the project, go to the CMake build directory and type:

        cmake --build .

    :param config: The project configuration settings
    :type config:  ``dict``
    """
    print()
    print('Configuring CMake files')
    print('-- Creating the build directory')
    project = place_project(config)
    print('-- Configuring top level CMakeLists.txt')
    config_cmake(config,project)
    print('-- Configuring Flatpak settings')
    config_flatpak(config,project)
