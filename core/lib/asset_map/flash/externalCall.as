/**
* Copyright (c) 2003 - Squiz Pty Ltd
*
* $Id: externalCall.as,v 1.12 2003/10/30 23:20:49 dwong Exp $
* $Name: not supported by cvs2svn $
*/


/**
* This class adds the ability to external function calls to be made from outside the
* Flash player (eg in JS), uses the flashExternalCall.js
*/
function ExternalCall()
{

	this.params    = {};

	this.cmd       = "";
	this.add_param = "";
	this.exec      = "";

	this.watch("add_param", externalCallWatch);
	this.watch("exec",      externalCallWatch);

	// Set ourselves up as a broadcaster
    ASBroadcaster.initialize(this);

}

/**
* Called whenever the add_param or exec variables are changed
*
* @param string	property	name of property that changed
* @param string	old_val		the old value of the propery
* @param string	new_val		the new value of the propery
*
*/
function externalCallWatch(property, old_val, new_val)
{
	switch(property) {
		case "add_param" :
			if (this.cmd != "") {
				var tmp = new_val.split("=", 2);
				var name  = unescape(tmp[0]);
				var value = unescape(tmp[1]);
				this.params[name] = value;
			}
		break;
		case "exec" :
			if (this.cmd != "" && new_val == "true") {
//				trace("onExternalCall : " + this.cmd);
//				for(var i in this.params) 
//					trace("params -> " + i + " : " + this.params[i]);
				this.broadcastMessage("onExternalCall", this.cmd, this.params);
				// reset the storage units
				this.params = {};
				this.cmd    = "";
			}

		break;
	}// end switch
	return new_val;

}//end externalCallWatch()


/**
* Called whenever the add_param or exec variables are changed
*
* @param string		cmd			the fn command to call when in the JS
* @param Object()	params		an assoc array holding the params to be passed to the JS fn
*
*/
ExternalCall.prototype.makeExternalCall = function(cmd, params)
{
//	trace('cmd://' + cmd);
	fscommand('flashToJsCall', 'cmd://' + cmd);

	for(i in params) {
		switch(typeof params[i]) {
			case "object"   :
			case "function" :
				// just ignore these
				break;
			default :
				var name  = escape(i);
				var value = escape(params[i]);
//				trace('add_param://' + name + '=' + value);
				fscommand('flashToJsCall', 'add_param://' + name + '=' + value);
		}// end switch
	}// end for

	fscommand('flashToJsCall', 'exec://true');
}//end makeExternalCall()

// Now create the object, we do this here so the JS file knows what the thing is called
_root.external_call = new ExternalCall();
