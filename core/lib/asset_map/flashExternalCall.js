/**
* This function works in partnership with the ExternalCall flash class.
* What it allows is the execution of commands inside the flash that 
* you are unable to do normally.
*
*/

function flashExternalCall(swObj, cmd, params) {
	
	swObj.SetVariable('_root.external_call.cmd', cmd);

	for(i in params) {
		var name  = escape(i);
		var value = escape(params[i]);
		swObj.SetVariable('_root.external_call.add_param', name + '=' + value);
	}// end for	

	swObj.SetVariable('_root.external_call.exec', 'true');

}// end flashExternalCall()

