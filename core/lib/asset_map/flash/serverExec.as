
// Create the Class
function serverExec(exec_path, content_type)
{
	this.exec_path    = exec_path;
	this.content_type = content_type;
	this.count = 0;
	this.xmls = new Object();

}

/**
* Execute the passed xml command by sending it to the server
*
* @param object		XML			xml command to send to server
* @param object		on_load_obj	the object to run the on_load_fn on
* @param function	on_load_fn	the fn to run once loading has occured
* @param string		root_node	the expected root node for the returned text
* @param string		desc		text desc for the progress bar
*
*
*/
serverExec.prototype.exec = function(xml_cmd, on_load_obj, on_load_fn, root_node, desc)
{
	// bit of cleanup
	for (var j in this.xmls) {
		if (this.xmls[i] != null && this.xmls[i].__php_exec.finished) this.xmls[i] = null;
	}

	var i = this.count++;
	this.xmls[i] = new XML();
	this.xmls[i].ignoreWhite = true;
	this.xmls[i].onLoad = serverExecXMLonLoad;

	this.xmls[i].__php_exec = new Object();
	this.xmls[i].__php_exec.i = i;
	this.xmls[i].__php_exec.on_load_obj = on_load_obj;
	this.xmls[i].__php_exec.on_load_fn  = on_load_fn;
	this.xmls[i].__php_exec.root_node   = root_node;

	_root.showProgressBar(desc);
	xml_cmd.contentType = this.content_type;
	xml_cmd.sendAndLoad(this.exec_path, this.xmls[i]);

}// end exec()

/**
* The onLoad fn for the this.xmls[i] object defined in exec() above
* Hence why we can use the 'this.' syntax to refer to the XML object
*
*/
function serverExecXMLonLoad(success) 
{

	var root = this.firstChild;

	// something buggered up with the connection
	if (!success || this.status != 0) {
		_root.hideProgressBar();
		_root.showDialog("Connection Failure to Server", "XML Status '" + this.status + "'\nPlease Try Again");

	// something barfed server side
	} else if (root.nodeName == "error") {
		_root.hideProgressBar();
		_root.showDialog("Server Error", root.firstChild.nodeValue);

	// we got an unexpected root node
	} else if (this.__php_exec.root_node != '' && root.nodeName != this.__php_exec.root_node) {
		_root.hideProgressBar();
		_root.showDialog("Connection Failure to Server", "Please Try Again");

	// everything went well, load 'em up
	} else {
		trace('All OK');
		this.__php_exec.on_load_obj[this.__php_exec.on_load_fn](this);
		_root.hideProgressBar();

	}// end if

	this.__php_exec.finished = true;

}// end serverExecXMLonLoad()

