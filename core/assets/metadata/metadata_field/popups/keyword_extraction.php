<?php
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
* $Id: keyword_extraction.php,v 1.1 2004/07/05 03:43:34 lwright Exp $
* $Name: not supported by cvs2svn $
*/

	require_once dirname(__FILE__).'/../../../../../core/include/init.inc';
	require_once dirname(__FILE__).'/../../../../../core/lib/html_form/html_form.inc';
	if (!isset($_GET['assetid'])) return false;
	
	assert_valid_assetid($_GET['assetid']);
	$asset = &$GLOBALS['SQ_SYSTEM']->am->getAsset($_GET['assetid']);
?>

<html>
	<head>
		<title>'<?php echo $asset->attr('name') ?>' Keyword Extraction</title>
		<style>
			body {
				background-color:	#FFFFFF;
			}

			body, p, td, ul, li, input, select, textarea{
				color:				#000000;
				font-family:		Arial, Verdana Helvetica, sans-serif;
				font-size:			11px;
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
	<?php
		require_once dirname(__FILE__).'/../../../../../core/include/backend_outputter.inc';
		//$backend = new Backend();
		$o =& new Backend_Outputter();

		$o->openSection('Keyword Extraction for \''.$asset->attr('name').'\' (#'.$asset->id.')');
		$o->openField('&nbsp;');
	?>
				<p>These keywords have been extracted from the Search Manager's index for '<?php echo $asset->attr('name') ?>' (#<?php echo $asset->id ?>)  and are listed in descending order of importance according to the weightings set. They do not include index entries for metadata fields.</p>
				<p>You may copy and paste the contents of the box below into a metadata field if you so wish.</p>

		<p>
		<fieldset>
			<legend><b>Extracted Keywords</b></legend>
			<?php
$sm =& $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('search_manager');
if (empty($sm)) {
?>The search manager is not installed, therefore no keywords are available<?php
} else {

$keywords = $sm->extractKeywords($asset);
print implode(', ', $keywords);
}
?>
		</fieldset>
		</p>
<?php
$o->openField('', 'commit');
normal_button('cancel', 'Close Window', 'window.close()');
$o->closeSection();
$o->paint();
?>
	</body>
</html>