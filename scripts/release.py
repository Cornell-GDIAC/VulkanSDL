"""
Release creation script for VulkanSDL

When we release VulkanSDL to students, we are giving them a full source code 
release to SDL, SDL_image, and SDL_ttf, as well as the SDL App specific 
extensions. To enable continuous integration, these subprojects are arranged 
as submodules of this repository. But that is not the optimal way to organize 
this for a release.

This script will copy the latest code from the submodules into the release 
folder. The scripts and templates will all be modified to reflect this new 
organization. The templates and build files will all be updated to reflect this 
new orgnization. The resulting release will be a folder of all of the source 
and header files, together with a python script for configuring SDL projects.

Author: Walker M. White
Date:   7/10/24
"""
import os
import os.path
import argparse
import util
import shutil
import glob
import yaml
import traceback

# Default values
VERSION   = '2.1.0'
DIRECTORY = os.path.join('..','release')
MANIFEST  = 'MANIFEST.yml'


def setup():
    """
    Returns an initialized parser for this script.

    :return: The parsed args
    :rtype:  ``Namespace``
    """
    parser = argparse.ArgumentParser(description='Create a VulkanSDL release.')
    parser.add_argument('directory', type=str, nargs='?', default=DIRECTORY, help='The release directory')
    parser.add_argument('-v', '--version', type=str, help='The build version')
    parser.add_argument('-m', '--manifest', type=str, help='The build manifest')
    return parser.parse_args()


def load_from_path(path,name):
    """
    Loads a module with the given name in the specified path.

    This is the preferred way to import an external module without poisoning sys.path.
    The module is assigned the given name, and is executed. The function returns the
    module object after execution.

    The path specified as a list of strings, ensuring that it is platform agnostic.

    :param path: The path to the module
    :type path: ``list`` of ``str``

    :param name: The module name (without the py)
    :type name: ``str``

    :return: The module object
    :rtype: ``module``
    """
    import importlib.util
    import os.path
    full = name+'.py' if path is None else os.path.join(*path,name+'.py')
    assert os.path.isfile(full),'%s is not a valid file' % repr(full)

    spec = importlib.util.spec_from_file_location(name,full)
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


def check_safe_replace(patterns):
    """
    Returns a set of warnings if the replacement patterns are not safe.

    A replacement pattern is a dictionary mapping old strings to their replacements. To
    be well-defined this dictionary must ensure that (1) no key is a prefix of any other
    and (2) no key is a substring of any value. This function varifies that this is the
    case. If so, it returns an empty list. Otherwise, it returns a list of warnings,
    identifying the violations.

    :param patterns: The set of replacement patterns
    :type patterns: ``dict``
    """
    warnings = []

    # Check if any key is prefix of any other
    keys = list(patterns.keys())
    keys.sort()
    for pos in range(len(keys)-1):
        curr = keys[pos]
        next = keys[pos+1]
        size = len(curr)

        if curr == next[:size]:
            warnings.append('Pattern %s is a prefix of pattern %s.' % (repr(curr),repr(next)))

    # Check if any key is within any value
    for k1 in patterns:
        v = patterns[k1]
        for k2 in keys:
            if k1 != k2 and k2 in v:
                warnings.append('Replacement %s contains pattern %s.' % (repr(v),repr(k2)))

    return warnings


def reescape(text):
    """
    Restores escape characters removed from text during processing.

    PyYAML does not recognize escape characters, and turns \t into \\t and so on. This
    can cause a problem with our replacement patterns. This function restores those
    escape characters. Any time it sees a \\, it assumes that is an unescaped character.

    :param text: The text to modify
    :type text: ``str``
    """
    escapes = {'n': '\n','f':'\f','r':'\r','t':'\t'}

    result = ''
    prev = False
    for c in text:
        if not prev and c != '\\':
            result += c
        elif prev:
            if c in escapes:
                result += escapes[c]
            else:
                result += c
            prev = False
        else:
            prev = True

    return result


def deploy_manifest(path,manifest):
    """
    Copies all files specified by the manifest to the given path

    The manifest describes what files are to be copied, where they are to be
    copied to, and what their names are in the new location. The manifest should
    be a nested dictionary parsed from a YAML or JSON file.

    :param path: The path to copy to
    :type path:  ``str``

    :params manifest: The file manifest for the copy process
    :type manifest: ``dict``
    """
    if not 'copy' in manifest:
        return

    srcroot = manifest['srcroot']
    for item in manifest['copy']:
        if 'comment' in item:
            print('-- %s' % item['comment'])
        elif 'destination' in item:
            try:
                if 'file' in item:
                    src = os.path.join(srcroot,util.posix_to_path(item['file']))
                    dst = os.path.join(path,util.posix_to_path(item['destination']))
                    util.merge_copy(src,dst)
                elif 'directory' in item:
                    prefix = os.path.join(srcroot,util.posix_to_path(item['directory']))
                    if 'filter' in item and item['filter'] and 'selection' in item:
                        files = []
                        if type(item['selection']) == str:
                            selection = os.path.join(prefix,util.posix_to_path(item['selection']))
                            files.extend(glob.glob(selection))
                        else:
                            try:
                                for filt in item['selection']:
                                    selection = os.path.join(prefix,util.posix_to_path(filt))
                                    files.extend(glob.glob(selection))
                            except:
                                pass
                        for src in files:
                            dst = os.path.join(path,util.posix_to_path(item['destination']),src[len(prefix)+1:])
                            util.merge_copy(src,dst)
                    else:
                        src = prefix
                        dst = os.path.join(path,util.posix_to_path(item['destination']))
                        util.merge_copy(src,dst)
                elif 'group' in item:
                    group  = os.path.join(srcroot,util.posix_to_path(item['group']))
                    folder = os.path.join(path,util.posix_to_path(item['destination']))
                    for src in glob.glob(group):
                        elt = os.path.split(src)[1]
                        dst = os.path.join(folder,elt)
                        util.merge_copy(src,dst)
            except:
                print('WARNING: Copy failed for %s.' % repr(item))


def configure_manifest(path,manifest):
    """
    Configures any files in the manifest that must be modified after placement.

    The manifest identifies what files should be modified, the original values and
    their replacements.  The manifest should
    be a nested dictionary parsed from a YAML or JSON file.

    :param path: The path to copy to
    :type path:  ``str``

    :params manifest: The file manifest for the configuration process
    :type manifest: ``dict``
    """
    if not 'configure' in manifest:
        return

    for item in manifest['configure']:
        if 'comment' in item:
            print('-- %s' % item['comment'])
        elif 'file' in item and 'substitutions' in item:
            patterns = {}
            for pair in item['substitutions']:
                if 'old' in pair and 'new' in pair:
                    old = reescape(pair['old'])
                    new = reescape(pair['new'])
                    patterns[old] = new

            warnings = check_safe_replace(patterns)
            for warn in warnings:
                print('WARNING [%s]: %s' % (item['file'],warn))

            file = os.path.join(path,util.posix_to_path(item['file']))
            util.file_replace(file,patterns)
        elif 'link' in item and 'name' in item:
            src = os.path.abspath(os.path.join(path,util.posix_to_path(item['link'])))
            dst = os.path.abspath(os.path.join(path,util.posix_to_path(item['name'])))
            dir = os.path.dirname(dst)
            src = os.path.relpath(src, dir)
            isdir = 'directory' in item and item['directory']
            os.symlink(src, dst, isdir)


def prune_manifest(path,manifest):
    """
    Removes any files that were unnecessarily copied during placement

    This function is primarily for XCode projects. For these projects, it is easier to
    copy the whole project and then remove what is needed later, rather than copy one
    element at a time.

    :param path: The path to copy to
    :type path:  ``str``

    :params manifest: The file manifest for the removal process
    :type manifest: ``dict``
    """
    if not 'remove' in manifest:
        return

    for item in manifest['remove']:
        try:
            if 'comment' in item:
                print('-- %s' % item['comment'])
            elif 'file' in item:
                file = os.path.join(path,util.posix_to_path(item['file']))
                if os.path.exists(file):
                    os.remove(file)
            elif 'directory' in item:
                directory = os.path.join(path,util.posix_to_path(item['directory']))
                if os.path.exists(directory):
                    shutil.rmtree(directory)
            elif 'group' in item:
                wildcard = os.path.join(path,util.posix_to_path(item['group']))
                for item in glob.glob(wildcard):
                    if os.path.isdir(item):
                        shutil.rmtree(item)
                    elif os.path.exists(item):
                        os.remove(item)
                
        except:
            traceback.print_exc()
            print('WARNING: Removal failed for %s.' % repr(item))


def main():
    """
    Runs the release script
    """
    args = setup()

    # Determine how we ran this script
    root = os.path.split(os.path.relpath(__file__))[0]

    path = os.path.join(root,util.posix_to_path(args.directory))
    version  = VERSION if args.version is None else args.version
    manifest = MANIFEST if args.manifest is None else args.manifest
    manifest = os.path.join(root,util.posix_to_path(manifest))

    if not os.path.exists(path):
        try:
            os.mkdir(path)
        except:
            pass

    if not os.path.isdir(path):
        raise ValueError(repr(path)+' is not a valid directory')
    path = os.path.join(*util.path_split(path),'VulkanSDL')

    if not os.path.isfile(manifest):
        raise ValueError(repr(manifest)+' is not a valid manifest file')

    data = None
    try:
        with open(manifest, encoding = 'utf-8') as file:
            data = file.read()
            data = yaml.load(data,Loader=yaml.Loader)
        data['srcroot'] = os.path.join(root,util.posix_to_path(data['srcroot']))
    except:
        raise ValueError(repr(manifest)+' is not a valid manifest file')

    # Clear anything already there
    if os.path.isdir(path):
        shutil.rmtree(path)

    print("Creating SDL_app release")
    os.mkdir(path)
    try:
        deploy_manifest(path,data)
    except:
        traceback.print_exc()
        raise ValueError(repr(manifest)+' is not a valid manifest file')

    configure_manifest(path,data)
    prune_manifest(path,data)


if __name__ == '__main__':
    main()
