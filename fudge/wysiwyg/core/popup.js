function __dlg_onclose(code) {
	if (!document.all) {
		if (opener.dialogWins[code] && opener.dialogWins[code].returnFunc) { opener.dialogWins[code].returnFunc(null); }
	}
};

function __dlg_init(code) {
	if (!document.all) {
		// init dialogArguments, as IE gets it
		window.dialogArguments = opener.dialogWins[code].args;
		window.addEventListener("unload", __dlg_onclose(code), true);
	}
	var body = document.body;
};

// closes the dialog and passes the return info upper.
function __dlg_close(code, val) {
	if (opener.dialogWins[code].returnFunc) { opener.dialogWins[code].returnFunc(val); }
	window.close();
};
