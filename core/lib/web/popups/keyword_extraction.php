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

	require_once dirname(__FILE__).'/../../../../core/include/backend_outputter.inc';
	$o = new Backend_Outputter();

	$o->openSection(sprintf(translate('keyword_extraction_for'), sprintf(translate('asset_format'), $asset->attr('name'), $asset->id)));
	$o->openField('', 'wide_col');
?>

		<p><?php echo sprintf(translate('kewords_for_asset'), sprintf(translate('asset_format'), $asset->attr('name'), $asset->id)); ?></p>
		<p><?php echo translate('use_keywords_in_metadata_fields'); ?></p>

		<p>
		<fieldset>
			<legend><b><?php echo translate('Extracted keywords'); ?></b></legend>
			<?php
$sm = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('search_manager');
if (empty($sm)) {
	echo translate('Keyword list not available');
} else {

$keywords = $sm->extractKeywords($asset);
print implode(', ', $keywords);
}
?>
		</fieldset>
		</p>
<?php
$o->openField('', 'commit');
normal_button('cancel', translate('Close Window'), 'window.close()', 'style="margin-bottom: 20px;"');
$o->closeSection();
$o->paint();
?>
	</body>
</html>
