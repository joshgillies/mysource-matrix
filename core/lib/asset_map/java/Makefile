sourcefiles = src/squiz/matrix/*.java
classfiles = squiz/matrix/*.class

compile: $(sourcefiles)
	javac -sourcepath src/squiz/matrix -classpath . -d . src/squiz/matrix/*.java
	jar cvfm matrix.jar manifest squiz

doc: compile src/squiz/matrix/package.html
	javadoc -d doc -sourcepath src squiz.matrix
