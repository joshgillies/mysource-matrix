function __dlg_onclose(code) {
	if (!document.all) {
		if (opener.dialogWins[code] && opener.dialogWins[code].returnFunc) { opener.dialogWins[code].returnFunc(null); }
	}
};

function __dlg_init(code) {
	if (!document.all) {
		// init dialogArguments, as IE gets it
		window.dialogArguments = opener.dialogWins[code].args;

		if (opener.dialogWins[code].isModal) {
			window.sizeToContent();
			window.sizeToContent();	// for reasons beyond understanding,
									// only if we call it twice we get the
									// correct size.
			window.addEventListener("unload", __dlg_onclose(code), true);
			// center on parent
			var px1 = opener.screenX;
			var px2 = opener.screenX + opener.outerWidth;
			var py1 = opener.screenY;
			var py2 = opener.screenY + opener.outerHeight;
			var x = (px2 - px1 - window.outerWidth) / 2;
			var y = (py2 - py1 - window.outerHeight) / 2;
			window.moveTo(x, y);
			var body = document.body;
			window.innerHeight = body.offsetHeight;
			window.innerWidth = body.offsetWidth;
		}
	}
};


// closes the dialog and passes the return info upper.
function __dlg_close(code, val) {
	if (document.all && !opener) { // modal in IE
		window.returnValue = val;
	} else {
		if (opener.dialogWins[code].returnFunc) { opener.dialogWins[code].returnFunc(val); }
	}
	window.close();
};