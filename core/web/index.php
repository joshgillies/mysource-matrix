<?

require_once '../include/init.inc';
$GLOBALS['SQ_RESOLVE']->start();
//$GLOBALS['SQ_RESOLVE']->am = new Asset_Manager();
//
//$folder = &$GLOBALS['SQ_RESOLVE']->am->getAsset(4);
////$links = $root_folder->getLinks(SQ_LINK_UNITE | SQ_LINK_EXCLUSIVE, '', 'S');
////pre_echo($links);
//
//$folder->deleteLink(76);

/*
 /// RESET SORT ORDERS ///
$db = &$GLOBALS['SQ_RESOLVE']->getDb();

$sql = 'SELECT majorid, minorid
		FROM sq_asset_link
		ORDER BY majorid, sort_order';

$all = $db->getAll($sql);

$blah = Array();
foreach($all as $data) {
	if (!isset($blah[$data['majorid']])) $blah[$data['majorid']] = 0;
	else $blah[$data['majorid']]++;
	
	$sql = 'UPDATE sq_asset_link
			SET sort_order = '.$db->quote($blah[$data['majorid']]).'
			WHERE majorid = '.$db->quote($data['majorid']).'
			  AND minorid = '.$db->quote($data['minorid']);
	pre_echo($sql);

	$db->query($sql);

}
*/
 

/*
$trash_folder = &$GLOBALS['SQ_RESOLVE']->am->getAsset(3);
$links = $trash_folder->getLinks(SQ_LINK_EXCLUSIVE | SQ_LINK_UNITE, '', 'S');

pre_echo('------------------------------------------------------------------------');
pre_echo($links);
*/
/*
$GLOBALS['SQ_RESOLVE']->am->includeAsset('folder');
$GLOBALS['SQ_RESOLVE']->am->includeAsset('user');
$root_folder = &$GLOBALS['SQ_RESOLVE']->am->getAsset(1);

for ($i = 0; $i < 5; $i++) {

	// Now just create some useful folders 
	$folder = new Folder();
	$folder->create('Folder '.$i);
	$GLOBALS['SQ_RESOLVE']->am->registerAsset($folder);
	$root_folder->createLink($folder, SQ_LINK_UNITE);


	for ($j = 0; $j < 10; $j++) {

		// Now just create some useful folders 
		$user = new User();
		$user->create('user_'.$i.'_'.$j, 'blah', 'User '.$i, 'Number '.$j);
		$GLOBALS['SQ_RESOLVE']->am->registerAsset($user);
		$folder->createLink($user, SQ_LINK_UNITE);

	}// end for

}// end for

*/




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

/*
$root_folder = &$GLOBALS['SQ_RESOLVE']->am->getAsset(1);
pre_echo($root_folder->attr('name'));

$links = $root_folder->getLinks(SQ_LINK_EXCLUSIVE | SQ_LINK_UNITE, 'folder');
pre_echo($links);
pre_echo('------------------------------------');
$links = $root_folder->getLinks(SQ_LINK_EXCLUSIVE | SQ_LINK_UNITE);
pre_echo($links);
//pre_echo($root_folder->_tmp['links']);
*/


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