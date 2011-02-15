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
* $Id: keywords.php,v 1.7.14.1 2011/02/15 06:15:13 cupreti Exp $
*
*/
-->
<?php
	require_once dirname(__FILE__).'/../../../../../core/include/init.inc';
	if (!isset($_GET['type_code'])) return FALSE;

	$GLOBALS['SQ_SYSTEM']->am->includeAsset($_GET['type_code']);
	$asset = new $_GET['type_code']();
	$keywords = $asset->getAvailableKeywords();
?>

<html>
	<head>
		<title><?php echo htmlspecialchars(ucwords(str_replace('_', ' ', $_GET['type_code']))) ?> Format Keyword Replacements</title>
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
		<p><b><i>The following keyword replacements may be used for <?php echo htmlspecialchars(ucwords(str_replace('_', ' ', $_GET['type_code']))); ?> asset values.<br/>Note that the percentage signs (%) are required.</i></b></p>

		<p>
		<fieldset>
			<legend><b>Asset Information</b></legend>
			<table border="0" width="100%">
			<?php
			foreach ($keywords as $keyword => $description) {
				?>
				<tr><td valign="top" width="200"><b>%<?php echo $keyword?>%</b></td><td valign="top"><?php echo isset($description) ? $description : '' ?></td></tr>
				<?php
			}
			?>
			</table>
		</fieldset>
		</p>
	</body>
</html>
