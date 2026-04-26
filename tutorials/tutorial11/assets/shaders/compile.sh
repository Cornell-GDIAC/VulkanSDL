#!/bin/sh

SRCPATH=$0
if [ ! -e "${SRCPATH}" ]; then
  case $SRCPATH in
    (*/*) exit 1;;
    (*) prg=$(command -v -- "${SRCPATH}") || exit;;
  esac
fi
SRCPATH=$(
  cd -P -- "$(dirname -- "${SRCPATH}")" && pwd -P
) || exit


Help()
{
   # Display Help
   echo "Script to compile GLSL shaders into SPRIV."
   echo
   echo "This script requires glslc to be in your path."
   echo
   echo "Syntax: compile.sh [-c|h]"
   echo "options:"
   echo "c     Clean, removing the SPIRV shaders."
   echo "h     Print this Help."
   echo
}

Clean() 
{
	rm -rf "${SRCPATH}"/*.spv
}

while getopts ":hc" option; do
   case $option in
      h) # display Help
         Help
         exit;;
      c) # clean shaders
         Clean
         exit;;
     \?) # Invalid option
         echo "Error: Invalid option"
         exit;;
   esac
done

glslc "${SRCPATH}/shader.vert" -o vert.spv
glslc "${SRCPATH}/shader.frag" -o frag.spv