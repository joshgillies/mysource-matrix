function __dlg_onclose() {
	if (!document.all) {
		if (opener.dialogWin.returnFunc) { opener.dialogWin.returnFunc(null); }
	}
};

function __dlg_init() {
	if (!document.all) {
		// init dialogArguments, as IE gets it
		window.dialogArguments = opener.dialogWin.args;
		window.addEventListener("unload", __dlg_onclose, true);
	}
	var body = document.body;
};

// closes the dialog and passes the return info upper.
function __dlg_close(val) {
	if (opener.dialogWin.returnFunc) { opener.dialogWin.returnFunc(val); }
	window.close();
};
