<!--
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd       |
* | ACN 084 670 600                                                    |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.net) so we may provide   |
* | you a copy.                                                        |
* +--------------------------------------------------------------------+
*
* $Id: about.html,v 1.10 2006/12/06 05:11:08 bcaldwell Exp $
*
*/
-->

<html style="width:380px; height:265px;">
	<head>
		<title>About HTMLArea</title>
		<script type="text/javascript" src="../../core/popup.js"></script>
		<script type="text/javascript">
			function closeAbout() {
				__dlg_close("about", null);
			}
		</script>
		<style>
		html,body,textarea { font-family: tahoma,verdana,arial; font-size: 11px; padding: 0px; margin: 0px; }
		tt { font-size: 120%; }
		body { padding: 5px; background: ButtonFace; color: ButtonText; }
		</style>
	</head>
	<body onload="__dlg_init('about')">
		<div style="font-family: arial black, arial; font-size: 28px;
		letter-spacing: -2px;">HTMLArea 3.0<br/><span style="font-family: arial black, arial; font-size: 12px;
		letter-spacing: 1px;">PHP Plugin Based Mod</span></div>
		<p>A free WYSIWYG editor replacement for <tt>&lt;textarea&gt;</tt> fields.</p>
		<p>For full source code and docs, visit:<br />
		<a href="http://www.interactivetools.com/products/htmlarea/" target="_blank">http://www.interactivetools.com/products/htmlarea/</a></p>
		<p>PHP Plugin Based Modifications by <a href="mailto: greg@squiz.net">Greg Sherwood</a> for Squiz.Net, visit:<br />
		<a href="http://www.squiz.net" target="_blank">http://www.squiz.net/</a></p>
		<button style="float: right" type="button" onclick="return closeAbout();">Close</button>
		<p>&copy; 2002, 2003 interactivetools.com, inc. All Rights Reserved.</p>
	</body>
</html>
