<?php

$SYSTEM_ROOT = (isset($_SERVER['argv'][1])) ? $_SERVER['argv'][1] : '';
if (empty($SYSTEM_ROOT)) {
    echo "ERROR: You need to supply the path to the System Root as the first argument\n";
    exit();
}

if (!is_dir($SYSTEM_ROOT) || !is_readable($SYSTEM_ROOT.'/core/include/init.inc')) {
    echo "ERROR: Path provided doesn't point to a Matrix installation's System Root. Please provide correct path and try again.\n";
    exit();
}

require_once $SYSTEM_ROOT.'/core/include/init.inc';


$root_user = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('root_user');
$GLOBALS['SQ_SYSTEM']->setCurrentUser($root_user);

$GLOBALS['SQ_SYSTEM']->setRunLevel(SQ_RUN_LEVEL_FORCED);


// regenerate all design assets
$design_ids = $GLOBALS['SQ_SYSTEM']->am->getTypeAssetids('login_design', TRUE, TRUE);
$design_ids += $GLOBALS['SQ_SYSTEM']->am->getTypeAssetids('design', TRUE, TRUE);
$design_ids += $GLOBALS['SQ_SYSTEM']->am->getTypeAssetids('password_change_design', TRUE, TRUE);
foreach ($design_ids as $design_id => $type) {
	$vars = Array('assetid' => $design_id);
	$hh = $GLOBALS['SQ_SYSTEM']->getHipoHerder();
	@$hh->freestyleHipo('hipo_job_regenerate_design', $vars);
}

?>
