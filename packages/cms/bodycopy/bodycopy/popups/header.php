<?php
/**
* +--------------------------------------------------------------------+
* | MySource 3 - MySource Matrix                                       |
* +--------------------------------------------------------------------+
* | Copyright (c), 2003 Squiz Pty Ltd (ABN 77 084 670 600).            |
* +--------------------------------------------------------------------+
* | This source file may be used subject to, and only in accordance    |
* | with, the Squiz Open Source Licence Agreement found at             |
* | http://www.squiz.net/licence.                                      |
* | Make sure you have read and accept the terms of that licence,      |
* | including its limitations of liability and disclaimers, before     |
* | using this software in any way. Your use of this software is       |
* | deemed to constitute agreement to be bound by that licence. If you |
* | modify, adapt or enhance this software, you agree to assign your   |
* | intellectual property rights in the modification, adaptation and   |
* | enhancement to Squiz Pty Ltd for use and distribution under that   |
* | licence.                                                           |
* +--------------------------------------------------------------------+
*
* $Id: header.php,v 1.8 2003/11/18 15:43:34 brobertson Exp $
* $Name: not supported by cvs2svn $
*/

/**
* Header Pop-Up
*
* Purpose
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Version$ - 1.0
* @package MySource_Matrix_Packages
* @subpackage cms
*/
?>
<html>
<head>
	<style type="text/css">
		html, body {
			background: #FCFCFC;
			color: #000000;
			font: 11px Tahoma,Verdana,sans-serif;
			margin: 0px;
			padding: 0px;
			padding: 0px;
		}

		table {
			font: 11px Tahoma,Verdana,sans-serif;
		}

		/* main popup title */
		.title {
			background: #402F48;
			color: #FFFFFF;
			font-weight: bold;
			font-size: 120%;
			padding: 3px 10px;
			margin-bottom: 10px;
			border-bottom: 1px solid black;
			letter-spacing: 4px;
		}

		/* fieldset styles */
		fieldset { 
			padding: 0px 10px 5px 5px;
			border-color: #725B7D;
		}

		.fl { width: 9em; float: left; padding: 2px 5px; text-align: right; }
		.fr { width: 7em; float: left; padding: 2px 5px; text-align: right; }

		/* form and form fields */
		form { padding: 0px; margin: 0px; }

		select, input, button {
			font: 11px Tahoma,Verdana,sans-serif;
		}

		button {
			width: 70px;
		}

		/* colour picker button styles */
		.buttonColor, .buttonColor-hilite {
			cursor: default;
			border: 1px solid;
			border-color: #9E86AA #725B7D #725B7D #9E86AA;
		}

		.buttonColor-hilite {
			border-color: #402F48;
		}

		.buttonColor-chooser, .buttonColor-nocolor, .buttonColor-nocolor-hilite {
			height: 0.6em;
			border: 1px solid;
			padding: 0px 1em;
			border-color: ButtonShadow ButtonHighlight ButtonHighlight ButtonShadow;
		}

		.buttonColor-nocolor, .buttonColor-nocolor-hilite { padding: 0px; }
		.buttonColor-nocolor-hilite { background: #402F48; color: #FFFFFF; }
	</style>
	<?php

	if ($_GET['browser'] != "ns") {
		?><script language="JavaScript" src="<?php echo sq_web_path('lib').'/js/detect.js';?>"></script><?php
	}
	?>

	<script language="JavaScript" type="text/javascript">
		if (is_ie4up || is_dom) {
			var owner = parent;
		} else {
			var owner = window;
		}// end if

		function popup_close() {
			popup_init = null;
			owner.bodycopy_hide_popup();
		}
	</script>
</head>

<?php
	if ($_GET['page_width'])  $table_width  = 'width="'.$_GET['page_width'].'"';
	if ($_GET['page_height']) $table_height = 'height="'.$_GET['page_height'].'"';
?>

<body topmargin="0" leftmargin="0" marginheight="0" marginwidth="0" onload="javascript: if(typeof popup_init == 'function') popup_init();" <?php echo $_GET['body_extra']?>>

<table <?php echo $table_width?> <?php echo $table_height?> cellspacing="1" cellpadding="0" border="0" bgcolor="#402F48">
	<tr>
		<td valign="top" align="center" bgcolor="#FCFCFC">