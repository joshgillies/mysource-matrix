#/**
#* +--------------------------------------------------------------------+
#* | Squiz.net Open Source Licence                                      |
#* +--------------------------------------------------------------------+
#* | Copyright (c), 2003 Squiz Pty Ltd (ABN 77 084 670 600).            |
#* +--------------------------------------------------------------------+
#* | This source file may be used subject to, and only in accordance    |
#* | with, the Squiz Open Source Licence Agreement found at             |
#* | http://www.squiz.net/licence.                                      |
#* | Make sure you have read and accept the terms of that licence,      |
#* | including its limitations of liability and disclaimers, before     |
#* | using this software in any way. Your use of this software is       |
#* | deemed to constitute agreement to be bound by that licence. If you |
#* | modify, adapt or enhance this software, you agree to assign your   |
#* | intellectual property rights in the modification, adaptation and   |
#* | enhancement to Squiz Pty Ltd for use and distribution under that   |
#* | licence.                                                           |
#* +--------------------------------------------------------------------+
#*
#* $Id: Makefile,v 1.2 2004/04/04 23:48:35 mmcintyre Exp $
#* $Name: not supported by cvs2svn $
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
simpleclassfiles = net/squiz/matrix/assetmap/* $(componentclassfiles) #META-INF/
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
	javac -sourcepath $(complexsourcepath) -classpath . -d . $(complexsourcefiles)
	jar cvfm complexassetmap.jar complexmanifest $(complexclassfiles)

# generates the documentation
doc: 
	javadoc -d doc -sourcepath src net.squiz.matrix.*
