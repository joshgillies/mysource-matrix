<!--
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
* $Id: keywords.php,v 1.2 2003/11/18 15:43:38 brobertson Exp $
* $Name: not supported by cvs2svn $
*/
-->
<?php
	require_once dirname(__FILE__).'/../../../../../core/include/init.inc';
	if (!isset($_GET['type_code'])) return false;
	
	$GLOBALS['SQ_SYSTEM']->am->includeAsset($_GET['type_code']);
	$asset = new $_GET['type_code']();
	$keywords = $asset->getAssetKeywords(true);
?>

<html>
	<head>
		<title><?php echo ucwords(str_replace('_', ' ', $_GET['type_code'])) ?> Format Keyword Replacements</title>
		<style>
			body {
				background-color:	#FFFFFF;
			}

			body, p, td, ul, li, input, select, textarea{
				color:				#000000;
				font-family:		Arial, Verdana Helvetica, sans-serif;
				font-size:			10px;
			}

			fieldset {
				padding:			0px 10px 5px 5px;
				border:				1px solid #E0E0E0;
			}

			legend {
				color:				#2086EA;
			}
		</style>
	</head>

	<body>
		<p><b><i>The following keyword replacements may be used for <?php echo ucwords(str_replace('_', ' ', $_GET['type_code'])); ?> asset values.<br/>Note that the percentage signs (%) are required.</i></b></p>

		<p>
		<fieldset>
			<legend><b>Asset Information</b></legend>
			<table border="0" width="100%">
			<?php foreach ($keywords as $keyword => $info) { ?>
				<tr><td valign="top" width="200"><b>%<?php echo $keyword?>%</b></td><td valign="top"><?php echo isset($info['description']) ? $info['description'] : '' ?></td></tr>
			<?php } ?>
			</table>
		</fieldset>
		</p>
	</body>
</html>