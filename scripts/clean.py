"""
Script to clean the tutorial projects

We regularly use the tutorial projects to test out releases. But doing so
corrupts the suffix entry (making the projects not usable on other Apple
Developer accounts). This simple script reverts the config files to their 
original state.

Author: Walker M. White
Date:   7/23/24
"""
import os, os.path
import argparse
import util

DIRECTORY = os.path.join('..','tutorials')
RESET_VALUE = 'suffix: true                    # Set to true to avoid global appid collisions'

def setup():
    """
    Returns an initialized parser for this script.

    :return: The parsed args
    :rtype:  ``Namespace``
    """
    parser = argparse.ArgumentParser(description='Clean the tutorial directory.')
    parser.add_argument('directory', type=str, nargs='?', default=DIRECTORY, help='The release directory')
    return parser.parse_args()


def fix_configs(path):
    """
    Recursively fixes the config.yml found somewhere in this path.
    
    This function replaces the suffix entry in the config file with the one 
    specified by RESET_VALUE.
    
    :param path: The directory to search
    :type path:  ``str``
    """
    for item in os.listdir(path):
        fullpath = os.path.join(path,item)
        if os.path.isdir(fullpath):
            fix_configs(fullpath)
        elif os.path.isfile(fullpath) and item == 'config.yml':
            contents = ''
            with open(fullpath) as file:
                contents = file.read().split('\n')
                for pos in range(len(contents)):
                    line = contents[pos]
                    if line.startswith('suffix:'):
                        contents[pos] = RESET_VALUE
                contents = '\n'.join(contents)
            
            if contents:
                with open(fullpath,'w') as file:
                    file.write(contents)


def remove_suffix(path,suffix):
    """
    Recursively removes all files with the given suffix from the path.
    
    :param path: The directory to search
    :type path:  ``str``

    :param suffix: The file suffix to filter
    :type suffix:  ``str``
    """
    for item in os.listdir(path):
        fullpath = os.path.join(path,item)
        if os.path.isdir(fullpath):
            remove_suffix(fullpath,suffix)
        elif os.path.isfile(fullpath):
            end = os.path.splitext(fullpath)[1]
            if suffix == end:
                os.remove(fullpath)


def main():
    """
    Runs the clean-up script
    """
    args = setup()

    # Determine how we ran this script
    root = os.path.split(os.path.relpath(__file__))[0]

    path = os.path.join(root,util.posix_to_path(args.directory))
    fix_configs(path)
    remove_suffix(path,'.spv')


if __name__ == '__main__':
    main()