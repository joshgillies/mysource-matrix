
/**
* This class controls the sending and retrieval of data from the server in XML
*
*/
function serverExec(exec_path, content_type)
{
	this.exec_path    = exec_path;
	this.content_type = content_type;
	this.count = 0;
	this.xmls = new Object();

}

/**
* Execute the passed xml command by sending it to the server
* Returns a unique (on a per movie load) index that can be used to identify 
* results from multiple of the same commands
*
* @param object	XML			xml command to send to server
* @param object	on_load_obj	the object to run the on_load_fn on
* @param string	on_load_fn	the fn name to run once loading has occured
* @param string	root_node	the expected root node for the returned text
* @param string	desc		text desc for the progress bar
*
* @return int
* @access public
*/
serverExec.prototype.exec = function(xml_cmd, on_load_obj, on_load_fn, root_node, desc)
{
	// bit of cleanup
	for (var j in this.xmls) {
		if (this.xmls[j] != null && this.xmls[j].__server_exec.finished) this.xmls[j] = null;
	}

	var i = this.count++;
	this.xmls[i] = new XML();
	this.xmls[i].ignoreWhite = true;
	this.xmls[i].onLoad = serverExecXMLonLoad;

	this.xmls[i].__server_exec = new Object();
	this.xmls[i].__server_exec.i = i;
	this.xmls[i].__server_exec.on_load_obj = on_load_obj;
	this.xmls[i].__server_exec.on_load_fn  = on_load_fn;
	this.xmls[i].__server_exec.root_node   = root_node;

	_root.showProgressBar(desc);
	xml_cmd.contentType = this.content_type;
	xml_cmd.sendAndLoad(this.exec_path, this.xmls[i]);

	return i;

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
		trace(this);
		_root.showDialog("Connection Failure to Server", "XML Status '" + this.status + "'\nPlease Try Again");

	// something barfed server side
	} else if (root.nodeName == "error") {
		_root.hideProgressBar();
		_root.showDialog("Server Error", root.firstChild.nodeValue);

	// we got an unexpected root node
	} else if (this.__server_exec.root_node != '' && root.nodeName != this.__server_exec.root_node) {
		_root.hideProgressBar();
		_root.showDialog("Connection Failure to Server", "Please Try Again");

	// everything went well, load 'em up
	} else {
		trace('All OK -> ' + this.toString());
		this.__server_exec.on_load_obj[this.__server_exec.on_load_fn](this, this.__server_exec.i);
		_root.hideProgressBar();

	}// end if

	this.__server_exec.finished = true;

}// end serverExecXMLonLoad()

