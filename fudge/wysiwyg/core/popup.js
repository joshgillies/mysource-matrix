function __dlg_onclose() {
	if (!document.all) {
		opener.dialogWin.returnFunc(null);
	}
};

function __dlg_init() {
	if (!document.all) {
		// init dialogArguments, as IE gets it
		window.dialogArguments = opener.dialogWin.args;
		window.sizeToContent();
		window.sizeToContent();	// for reasons beyond understanding,
					// only if we call it twice we get the
					// correct size.
		window.addEventListener("unload", __dlg_onclose, true);
		var body = document.body;
		window.innerHeight = body.offsetHeight;
		window.innerWidth = body.offsetWidth;
	} else {
		var body = document.body;
		window.dialogHeight = body.offsetHeight + 50 + "px";
		window.dialogWidth = body.offsetWidth + "px";
	}
};

// closes the dialog and passes the return info upper.
function __dlg_close(val) {
	opener.dialogWin.returnFunc(val);
	window.close();
};
