/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd       |
* | ABN 77 084 670 600                                                 |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.com.au) so we may provide|
* | you a copy.                                                        |
* +--------------------------------------------------------------------+
*
* $Id: popup.js,v 1.13 2012/08/30 00:56:51 ewang Exp $
*
*/

function __dlg_onclose(code) {
	if (!document.all) {
		if (opener.dialogWins[code] && opener.dialogWins[code].returnFunc) { opener.dialogWins[code].returnFunc(null); }
	}
};

function __dlg_init(code) {
	// init dialogArguments, as IE gets it
	window.dialogArguments = top.opener.dialogWins[code].args;

	if (top.opener.dialogWins[code].isModal) {
		window.top.sizeToContent();
		window.top.sizeToContent();	// for reasons beyond understanding,
								// only if we call it twice we get the
								// correct size.
		window.top.addEventListener("unload", __dlg_onclose(code), true);
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
};


// closes the dialog and passes the return info upper.
function __dlg_close(code, val) {
	if (!opener && top.opener) {
		if (top.opener.dialogWins[code].returnFunc) { top.opener.dialogWins[code].returnFunc(val); }
	} else {
		if (opener.dialogWins[code].returnFunc) { opener.dialogWins[code].returnFunc(val); }
	}
	window.top.close();
};