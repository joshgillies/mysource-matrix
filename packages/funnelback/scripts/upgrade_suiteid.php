<?php

$root_dir = dirname(dirname(dirname(dirname(__FILE__))));

require_once $root_dir.'/core/include/init.inc';

$GLOBALS['SQ_SYSTEM']->setRunLevel(SQ_RUN_LEVEL_FORCED);
$GLOBALS['SQ_SYSTEM']->setCurrentUser($GLOBALS['SQ_SYSTEM']->am->getSystemAsset('root_user'));
$suitem = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('suite_manager');
if ($suitem !== NULL) {
	$pages = $GLOBALS['SQ_SYSTEM']->am->getTypeAssetids('funnelback_search_page');
	foreach ($pages as $assetid) {
		$asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
		if ($asset !== NULL) {
			$systemid = $asset->attr('systemid');
			$product  = $suitem->getProductBySystemid($systemid);
			if (isset($product[0]['suiteid'])) {
				$asset->setAttrValue('systemid', $product[0]['suiteid']);
				$asset->saveAttributes();
			}
			$GLOBALS['SQ_SYSTEM']->am->forgetAsset($asset, TRUE);
		}
		unset($asset);
	}//end foreach
}//end if
$GLOBALS['SQ_SYSTEM']->restoreCurrentUser();
$GLOBALS['SQ_SYSTEM']->restoreRunLevel();

?>
