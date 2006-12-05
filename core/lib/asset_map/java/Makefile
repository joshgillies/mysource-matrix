#/**
#* +--------------------------------------------------------------------+
#* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd       |
#* | ACN 084 670 600                                                    |
#* +--------------------------------------------------------------------+
#* | IMPORTANT: Your use of this Software is subject to the terms of    |
#* | the Licence provided in the file licence.txt. If you cannot find   |
#* | this file please contact Squiz (www.squiz.net) so we may provide   |
#* | you a copy.                                                        |
#* +--------------------------------------------------------------------+
#*
#* $Id: Makefile,v 1.5 2006/12/05 05:28:40 bcaldwell Exp $
#*
#*/

# Makefile for Matrix java asset map
#
# @author Marc McIntyre <mmcintyre@squiz.net>


#xml
xmlsourcepath = src/net/squiz/matrix/xml
xmlsourcefiles = src/net/squiz/matrix/xml/*.java
xmlclassfiles = net/squiz/matrix/xml/*.class

# pluggable look and feel
#plafsourcefiles = src/net/squiz/matrix/plaf/*.java
#plafsourcepath = src/net/squiz/matrix/plaf
#plafclassfiles = net/squiz/matrix/plaf/*.class

# location of the xml files
xmlfiles = xml

# components
componentsourcepath = src/net/squiz/matrix
componentsourcefiles = $(xmlsourcefiles) #$(plafsourcefiles)
componentclassfiles = $(xmlclassfiles)# $(plafclassfiles) $(xmlfiles)

#simple
simplesourcefiles = src/net/squiz/matrix/assetmap/*.java
simpleclassfiles = net/squiz/matrix/assetmap/* $(componentclassfiles) META-INF/
simplesourcepath = src/net/squiz/matrix/assetmap

#complex
complexsourcepath = src/squiz/matrix/complexassetmap
complexsourcefiles = src/net/squiz/matrix/complexassetmap/*.java
complexclassfiles = net/squiz/matrix/complexassetmap/*.class $(simpleclassfiles)



# compiles the assetmap package
simple:
	javac -sourcepath $(simplesourcepath):$(componentsourcepath) -classpath java40.jar:. -d . $(simplesourcefiles) $(componentsourcefiles)
	#compile components after simple as it is dependent on it
	javac -sourcepath $(componentsourcepath) -classpath . -d . $(componentsourcefiles)

	 jar cvf assetmap.jar $(simpleclassfiles)

# compiles the complexassetmap package
complex: simple
	javac -sourcepath $(complexsourcepath) -classpath java40.jar:. -d . $(complexsourcefiles)
	jar cvfm complexassetmap.jar complexmanifest $(complexclassfiles)

# generates the documentation
doc:
	javadoc -d doc -sourcepath src net.squiz.matrix.*
