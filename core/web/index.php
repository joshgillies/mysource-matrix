<?

require_once '../include/init.inc';
$GLOBALS['SQ_RESOLVE']->start();

//$GLOBALS['SQ_RESOLVE']->am = new Asset_Manager();

/* INSTALL */
/*
include_once(SQ_INCLUDE_PATH.'/package_manager.inc');
$pm = new Package_Manager('__core__');
$result = $pm->updatePackageDetails();
$pm = new Package_Manager('cms');
$result = $pm->updatePackageDetails();

$GLOBALS['SQ_RESOLVE']->am->includeAsset('user');
$user = new User();
$assetid = $user->create('root', 'root', 'bcr', 'Root', 'User');
pre_echo('Asset Id : '.$assetid);
*/

#$root_user = &$GLOBALS['SQ_RESOLVE']->am->getAsset(2);
#pre_echo($root_user->attr('username'));
#$page2 = &$GLOBALS['SQ_RESOLVE']->am->getAsset(3);
#pre_echo($page);
#$links = $page1->createLink($page2, 'united');
#$links = $page1->getLinks('united', 'page');
#$links = $page1->getLinks('united');
#pre_echo($links);
#pre_echo($page1->_tmp['links']);




#$site = &$GLOBALS['SQ_RESOLVE']->getObject(1);

#pre_echo($site);
/*
include_once(SQ_INCLUDE_PATH.'/package_manager.inc');
$pm = new Package_Manager('__core__');
$result = $pm->updatePackageDetails();
pre_echo("Result : ".gettype($result));
pre_echo($result);
*/
#$am = &$GLOBALS['SQ_RESOLVE']->getAssetManager();
#$am->includeAsset('page');
#pre_echo($am->getParentList('page'));

//pre_echo($am);



?>