<?php
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
* $Id: keyword_extraction.php,v 1.1 2013/09/24 01:10:35 ewang Exp $
*
*/
	define('SQ_SYSTEM_ROOT', dirname(dirname(dirname(dirname(dirname($_SERVER['SCRIPT_FILENAME']))))));
	require_once SQ_SYSTEM_ROOT.'/core/include/init.inc';
	require_once SQ_SYSTEM_ROOT.'/core/lib/html_form/html_form.inc';
	if (!isset($_GET['assetid'])) return FALSE;

	assert_valid_assetid($_GET['assetid']);
	$asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($_GET['assetid']);
	if (is_null($asset) || !$asset->writeAccess()) exit();
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
		require_once dirname(__FILE__).'/../../../../core/include/backend_outputter.inc';
		$o = new Backend_Outputter();

		$o->openSection(translate('keyword_extraction_for', translate('asset_format', $asset->attr('name'), $asset->id)));
		$o->openField('');
	?>
				<p><?php echo translate('kewords_for_asset', translate('asset_format', $asset->attr('name'), $asset->id)); ?></p>
				<p><?php echo translate('use_keywords_in_metadata_fields'); ?></p>

		<p>
		<fieldset>
			<legend><b><?php echo translate('extracted_keywords'); ?></b></legend>
			<?php
$sm = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('search_manager');
if (empty($sm)) {
	echo translate('keyword_list_not_available');
} else {

$keywords = $sm->extractKeywords($asset);
print implode(', ', $keywords);
}
?>
		</fieldset>
		</p>
<?php
$o->openField('', 'commit');
normal_button('cancel', translate('close_window'), 'window.close()');
$o->closeSection();
$o->paint();
?>
	</body>
</html>
