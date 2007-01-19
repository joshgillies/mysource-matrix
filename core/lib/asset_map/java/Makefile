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
#* $Id: Makefile,v 1.7 2007/01/19 05:01:01 rong Exp $
#*
#*/

# Makefile for Matrix java asset map
#
# @author Marc McIntyre <mmcintyre@squiz.net>
# @author Rayn Ong <rong@squiz.net>


# asset map component
component = assetmap core debug inspector matrixtree plaf ui
componentpaths = $(patsubst %,net.squiz.matrix.%, $(component))
componentsourcefiles = net/squiz/cuetree/*.java $(patsubst %,net/squiz/matrix/%/*.java, $(component))
componentclassfiles = net/squiz/cuetree/*.class $(patsubst %,net/squiz/matrix/%/*.class, $(component))


# compiles the assetmap package
# note on java40.jar
#    Netscape LiveConnect libraries for connecting from Java to Javascript.
#    We do not distribute this file due to licensing concerns.
#    It is easily acquirable from most Netscape based browser installs
#    Place this file in core/lib/asset_map/java/src directory
# note on SquizKeyStore
#    Keystore file with certificate information (mykey)
#    http://java.sun.com/j2se/1.4.2/docs/guide/plugin/developer_guide/rsa_signing.html
#    http://java.sun.com/j2se/1.4.2/docs/tooldocs/windows/keytool.html
#    Cerberus Ticket: #KVW-52908-317: Squiz - Code Signing Certificate
simple:
	cd src && javac -classpath java40.jar $(componentsourcefiles)
	cd src && jar cvfm ../assetmap.jar ../simplemanifest $(componentclassfiles) META-INF
	jarsigner -keystore SquizKeyStore assetmap.jar mykey
	jarsigner -verify assetmap.jar


# compiles the complexassetmap package
complex: simple
	cd src && jar cvfm ../complexassetmap.jar ../complexmanifest $(componentclassfiles) META-INF
	jarsigner -keystore SquizKeyStore complexassetmap.jar mykey
	jarsigner -verify complexassetmap.jar


# get rid of the class files
clean:
	cd src && rm $(componentclassfiles)


# generates the documentation
doc:
	javadoc -d doc -sourcepath src $(componentpaths)
