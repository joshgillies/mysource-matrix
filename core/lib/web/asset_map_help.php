<!--
/**
* +--------------------------------------------------------------------+
* | Squiz.net Open Source Licence                                      |
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
* $Id: asset_map_help.php,v 1.4 2004/01/16 11:45:56 brobertson Exp $
* $Name: not supported by cvs2svn $
*/

/**
* Help file for the Asset Map
*
* @author  Greg Sherwood <gsherwood@squiz.net>
* @version $Version$ - 1.0
* @package MySource_Matrix
*/

-->

<?php
require_once dirname(dirname(dirname(__FILE__))).'/include/init.inc';
?>

<html>
	<head>
		<title>System Help</title>
		<style>
			body {
				background-color:	#FFFFFF;
			}

			body, p, td, ul, li, input, select, textarea{
				color:				#000000;
				font-family:		Arial, Verdana Helvetica, sans-serif;
				font-size:			10px;
			}

			h1 {
				font-size:			14px;
				font-weight:		bold;
			}

			.status-colour-div {
				padding:			2px;
				padding-left:		5px;
				color:				#000000;
				border:				1px solid #000000;
				font-size:			10px;
				font-weight:		bold;
				letter-spacing:		2px;
				text-align:			left;
				width:				300px;
				margin:				5px;
			}
		</style>
	</head>

	<body>
		<h1>System Help</h1>

		<p>
		<span style="font-size: 120%"><i>For help on using this system please contact the system owner for <?php echo SQ_CONF_SYSTEM_NAME ?></i></span></br>
		<table border="0">
			<tr><td align="right"><b>System Owner:</b></td><td><?php echo SQ_CONF_SYSTEM_OWNER ?></td></tr>
			<tr><td align="right"><b>General Enquiries:</b></td><td><a href="mailto:<?php echo SQ_CONF_DEFAULT_EMAIL ?>"><?php echo SQ_CONF_DEFAULT_EMAIL ?></a></td></tr>
			<tr><td align="right"><b>Technical Enquiries:</b></td><td><a href="mailto:<?php echo SQ_CONF_TECH_EMAIL ?>"><?php echo SQ_CONF_TECH_EMAIL ?></a></td></tr>
		</table>
		</p>

		<p>
		<hr style="color: #000000; height: 1px;" />
		Running <a href="<?php echo SQ_SYSTEM_URL ?>" target="_blank"><?php echo SQ_SYSTEM_LONG_NAME; ?></a>
		</p>
	</body>
</html>