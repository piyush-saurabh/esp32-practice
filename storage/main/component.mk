#
# "main" pseudo-component makefile.
#
# (Uses default behaviour of compiling all source files in directory, adding 'include' to include path.)

# include text(html) file
COMPONENT_EMBED_TXTFILES := ../files/index.html

# include another text (.txt) file [Note the use of +]
COMPONENT_EMBED_TXTFILES += ../files/sample.txt

# include binary (image) file
COMPONENT_EMBED_FILES := ../files/pinout.png
