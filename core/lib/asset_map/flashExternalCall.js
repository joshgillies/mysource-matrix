

function flashExternalCall(swObj, cmd, params) {
	
	swObj.setVariable('_root.external_call.cmd', cmd);

	for(i in params) {
		var name  = escape(i);
		var value = escape(params[i]);
		swObj.setVariable('_root.external_call.add_param', name + '=' + value);
	}// end for	

	swObj.setVariable('_root.external_call.exec', 'true');

}// end swCall

