
function superClass() {
    this.name = "Super";
}

superClass.prototype.testFn = function() {
	return "Super Class";
}

function midClass() {
	super();
    this.name += " -> Mid";
}
midClass.prototype = new superClass();
midClass.prototype.testFn = function() {
	return super.testFn() + " -> Mid Class";
}

function testClass() {
	super();
    this.name += " -> test";
}
testClass.prototype = new midClass();


///////////////////////////////////////////////////////
// Check the difference with this fn commented out
// and your expected output with this commented out
testClass.prototype.testFn = function() {
	return super.testFn() + " -> Test Class";
}

var blah = testClass();
trace(blah.testFn());
