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
jarsigner -storetype pkcs12 -keystore SquizKeyStore imagej.jar 1 
jarsigner -verify imagej.jar
