export CLASSPATH=/opt/blackdown-jdk-1.4.1/jre/lib/rt.jar:.:.
cd src
javac  ij/*.java ij/*/*.java ij/*/*/*.java
jar cf imagej.jar ij/*.* ij/*/*.* ij/*/*/*.* IJ_Props.txt
jarsigner -storepass keystore imagej.jar tomkey2
mv imagej.jar ..
cd ..
