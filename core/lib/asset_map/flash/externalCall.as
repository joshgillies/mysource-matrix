
/**
* This class adds the ability to external function calls to be made from outside the
* Flash player (eg in JS), uses the flashExternalCall.js
*/
function externalCall() {

	this.registered_cmds = {};
	this.params    = {};

	this.cmd       = "";
	this.add_param = "";
	this.exec      = "";

	this.watch("add_param", externalCallWatch);
	this.watch("exec",      externalCallWatch);


}

/**
* Registers a command that is recognised by this object
*
* @param string	cmd_name	command name to be recognised
* @param object	target_obj	the object to run the target_fn on
* @param string	target_fn	the name of the fn to run when a call occurs
*
*/
externalCall.prototype.registerCmd = function(cmd_name, target_object, target_fn)
{
	this.registered_cmds[cmd_name] = {obj: target_object, fn: target_fn};
}

/**
* Called whenever the add_param or exec variables are changed
*
* @param string	property	name of property that changed
* @param string	old_val		the old value of the propery
* @param string	new_val		the new value of the propery
*
*/
function externalCallWatch(property, old_val, new_val){
	switch(property) {
		case "add_param" :
			if (this.cmd != "") {
				var tmp = new_val.split("=");
				var name  = unescape(tmp[0]);
				var value = unescape(tmp[1]);
				this.params[name] = value;				
			}
		break;
		case "exec" :
			if (this.cmd != "" && new_val == "true") {
				if (this.registered_cmds[this.cmd] != undefined) {
					// make the call to the execute function
					var obj = this.registered_cmds[this.cmd].obj;
					var fn = this.registered_cmds[this.cmd].fn;
					obj[fn](this.params);

					// reset the storage units
					this.params = {};
					this.cmd    = "";
				}
			}

		break;
	}// end switch
	return new_val;

}// end externalCallWatch()

// Now create the object, we do this here so the JS file knows what the thing is called
_root.external_call = new externalCall();
