<!--
/**
* Copyright (c) 2003 - Squiz Pty Ltd
*
* $Id: asset_map_help.php,v 1.1 2003/11/05 05:31:15 gsherwood Exp $
* $Name: not supported by cvs2svn $
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