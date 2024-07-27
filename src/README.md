# SDL App Extensions
---
This directory is a collection of extensions that we have found useful over the
years. In particular, mobile devices need support for safe area queries and 
device orientation (as opposed to display orienation). We also need to be able 
to reliably identify devices for network communications.

Note that these extensions are very nontraditional in that they are not just 
simple C modules added to SDL. Many of these extensions need to access data
from the build system, such as resource strings. In the case of Android, the
Java files needed to be extended. However, instead of modifying the SDL Java
files, we subclassed them in a new activity called `APPActivity`. See the 
comments in that file in the Android project template.