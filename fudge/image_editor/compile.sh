set -x
export CLASSPATH=/usr/lib/jvm/java-1.5.0-sun-1.5.0.22/jre/lib/rt.jar:.:.
cd src
javac  ij/*.java ij/*/*.java ij/*/*/*.java
jar cf imagej.jar ij/*.* ij/*/*.* ij/*/*/*.* IJ_Props.txt
mv imagej.jar ..
for i in `find . | grep "\.class$"` 
do 
  rm $i
done
cd ..
jarsigner -keystore SquizKeyStore imagej.jar squiz 
jarsigner -verify imagej.jar
