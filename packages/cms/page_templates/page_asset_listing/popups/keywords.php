<!--
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
* $Id: keywords.php,v 1.9.4.1 2013/03/08 05:23:07 ewang Exp $
*
*/
-->
<?php
	require_once dirname(__FILE__).'/../../../../../core/include/init.inc';
	if (!isset($_GET['type_code'])) return FALSE;
	$asset_type = preg_replace('/[^a-zA-Z0-9_]+/', '', $_GET['type_code']);
	$GLOBALS['SQ_SYSTEM']->am->includeAsset($asset_type);
	$asset = new $asset_type();
	$keywords = $asset->getAvailableKeywords();
?>

<html>
	<head>
		<title><?php echo htmlspecialchars(ucwords(str_replace('_', ' ', $asset_type))) ?> Format Keyword Replacements</title>
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
		<p><b><i>The following keyword replacements may be used for <?php echo htmlspecialchars(ucwords(str_replace('_', ' ', $asset_type))); ?> asset values.<br/>Note that the percentage signs (%) are required.</i></b></p>

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
