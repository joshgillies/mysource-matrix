export CLASSPATH=/opt/java-1.4/jre/lib/rt.jar:.:.
cd src
javac  ij/*.java ij/*/*.java ij/*/*/*.java
jar cf imagej.jar ij/*.* ij/*/*.* ij/*/*/*.* IJ_Props.txt
mv imagej.jar ..
for i in `find . | grep "\.class$"` 
do 
  rm $i
done
cd ..
jarsigner -keystore SquizKeyStore imagej.jar mykey
jarsigner -verify imagej.jar
