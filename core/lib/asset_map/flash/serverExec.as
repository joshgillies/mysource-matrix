/**
* +--------------------------------------------------------------------+
* | MySource 3 - MySource Matrix                                       |
* +--------------------------------------------------------------------+
* | Copyright (c), 2003 Squiz Pty Ltd (ABN 77 084 670 600).            |
* +--------------------------------------------------------------------+
* | This source file may be used subject to, and only in accordance    |
* | with, the Squiz Open Source Licence Agreement found at             |
* | http://www.squiz.net/licence.                                      |
* | Make sure you have read and accept the terms of that licence,      |
* | including its limitations of liability and disclaimers, before     |
* | using this software in any way. Your use of this software is       |
* | deemed to constitute agreement to be bound by that licence. If you |
* | modify, adapt or enhance this software, you agree to assign your   |
* | intellectual property rights in the modification, adaptation and   |
* | enhancement to Squiz Pty Ltd for use and distribution under that   |
* | licence.                                                           |
* +--------------------------------------------------------------------+
*
* $Id: serverExec.as,v 1.13 2003/11/18 15:37:36 brobertson Exp $
* $Name: not supported by cvs2svn $
*/


/**
* This class controls the sending and retrieval of data from the server in XML
*
*/
function ServerExec(exec_path, content_type)
{
	this.exec_path    = exec_path;
	this.content_type = content_type;
	this.count = 0;
	this.xmls = new Object();

}

/**
* Initialise an execution, 
* Returns a unique (on a per movie load) index that can be used to identify 
* results from multiple calls of the same commands
*
* @param object	XML			xml command to send to server
* @param object	on_load_obj	the object to run the on_load_fn on
* @param string	on_load_fn	the fn name to run once loading has occured
* @param string	root_node	the expected root node for the returned text
*
* @return int
* @access public
*/
ServerExec.prototype.init_exec = function(xml_cmd, on_load_obj, on_load_fn, root_node)
{
	// bit of cleanup
	for (var j in this.xmls) {
		if (this.xmls[j] != null && this.xmls[j].__server_exec.finished) {
			delete this.xmls[j];
		}
	}

	var i = this.count++;
	this.xmls[i] = new Object();
	this.xmls[i].output = xml_cmd;
	this.xmls[i].input = new XML();
	this.xmls[i].input.onLoad = serverExecXMLonLoad;

	this.xmls[i].input.__server_exec = new Object();
	this.xmls[i].input.__server_exec.i = i;
	this.xmls[i].input.__server_exec.on_load_obj = on_load_obj;
	this.xmls[i].input.__server_exec.on_load_fn  = on_load_fn;
	this.xmls[i].input.__server_exec.root_node   = root_node;

	return i;

}// end init_exec()

/**
* Execute the xml cmd represented by the passed exec identifier
*
* @param int	exec_identifier	
* @param string	desc			text desc for the progress bar
*
* @access public
*/
ServerExec.prototype.exec = function(exec_identifier, desc)
{
	this.xmls[exec_identifier].input.__server_exec.prog_bar_id = _root.progress_bar.show(desc);
	this.xmls[exec_identifier].output.contentType = this.content_type;
	this.xmls[exec_identifier].output.sendAndLoad(this.exec_path, this.xmls[exec_identifier].input);

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
		_root.progress_bar.hide(this.__server_exec.prog_bar_id);
		_root.dialog_box.show("Connection Failure to Server", "XML Status '" + this.status + "'\nPlease Try Again");

	// something barfed server side
	} else if (root.nodeName == "error") {
		_root.progress_bar.hide(this.__server_exec.prog_bar_id);
		_root.dialog_box.show("Server Error", root.firstChild.nodeValue);

	// we got an unexpected root node
	} else if (this.__server_exec.root_node != '' && root.nodeName != this.__server_exec.root_node) {
		_root.progress_bar.hide(this.__server_exec.prog_bar_id);
		_root.dialog_box.show("Connection Failure to Server", "Unexpected Root XML Node '" + root.nodeName + "', expecting '" + this.__server_exec.root_node + "'\nPlease Try Again");

	// everything went well, load 'em up
	} else {
//		trace('All OK -> ' + this.toString());
		this.__server_exec.on_load_obj[this.__server_exec.on_load_fn](this, this.__server_exec.i);
		_root.progress_bar.hide(this.__server_exec.prog_bar_id);

	}// end if

	this.__server_exec.finished = true;

}// end serverExecXMLonLoad()

