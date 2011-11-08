<?php
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
* $Id: system_reimport_content.php,v 1.1 2011/11/08 03:00:16 csmith Exp $
*
*/

/**
* Use this script to import bodycopy content from files under data/private.
* This is most useful for oracle systems with incorrect database encoding
* was originally set (eg the database is set to WE8ISO8859P1 but the content
* is really utf8). It will not come out correctly and there's not much you can
* do because the database hasn't saved it correctly.
*
* Make sure you set the new encoding in the db.inc file BEFORE you start this
* script.
*
* Usage: php scripts/system_reimport_content.php [SYSTEM_ROOT]
*
* @version $Revision: 1.1 $
* @package MySource_Matrix
*/

if (php_sapi_name() != 'cli') {
	trigger_error("You can only run this script from the command line\n", E_USER_ERROR);
}

$SYSTEM_ROOT = (isset($_SERVER['argv'][1])) ? $_SERVER['argv'][1] : '';
if (empty($SYSTEM_ROOT) || !is_dir($SYSTEM_ROOT)) {
	trigger_error("You need to supply the path to the System Root as the first argument\n", E_USER_ERROR);
}
$db_conf = require $SYSTEM_ROOT.'/data/private/conf/db.inc';

function usage()
{
	global $db_conf;
	$encoding = '(empty)';
	if (isset($db_conf['db']['encoding']) === TRUE) {
		$encoding = $db_conf['db']['encoding'];
	}
	echo "This script will re-import content from files into the database.\n";
	echo "This is mainly useful for oracle systems where the wrong database\n";
	echo "encoding has been used and needs to be changed.\n";
	echo "Assets that are in workflow or in safe-edit will not be re-imported.\n";
	echo "\n";
	echo "This will overwrite existing content with no warning.\n";
	echo "\n";
	if ($db_conf['db']['type'] == 'oci') {
		echo "Make sure the correct encoding has been set in db.inc before you start.\n";
		echo "It is currently set to:".$encoding."\n";
		echo "\n";
	}
}

usage();

// Make sure the person running the script understands
// the consequences. Content *WILL* be overwritten.
echo "OK to proceed (type 'yes' to continue): ";
$response = trim(fgets(STDIN));

if ($response != 'yes') {
	echo "Aborting.\n";
	exit(1);
}

require_once $SYSTEM_ROOT.'/core/include/init.inc';
require_once $SYSTEM_ROOT.'/core/lib/DAL/DAL.inc';
require_once $SYSTEM_ROOT.'/core/lib/MatrixDAL/MatrixDAL.inc';

$db_error = false;
try {
	$db_connection = MatrixDAL::dbConnect($db_conf['db']);
} catch (Exception $e) {
	echo "Unable to connect to the db: " . $e->getMessage() . "\n";
	$db_error = true;
}

if ($db_error) {
	exit;
}

MatrixDAL::changeDb('db');
$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

/**
 * See init.inc for other statuses,
 * but we only want live or under construction assets.
 * If they are in workflow or safe-edit (or something else)
 * they may have content changes already, and we don't want
 * to touch those.
 */
$asset_statuses = array(
						2, // under construction
						16, // live
					);

/**
 * We need the attribute id for the content so we can update it correctly.
 */
$query = "select attrid from sq_ast_attr where type_code='content_type_wysiwyg' and name='html'";
$results = MatrixDAL::executeSqlAll($query);
$attributeid = $results[0]['attrid'];

/**
 * The query includes ast_attr_val and ast_attr so we can get the contextid.
 * We need to include ast_attr so we can restrict the type code and name of the
 * attribute we're including, otherwise we end up with a row for each attribute id.
 */
$query = <<<EOT
select l.majorid, l.minorid, a.type_code, av.contextid from sq_ast_lnk l inner join sq_ast a on (a.assetid=l.majorid)
	inner join sq_ast_attr_val av on (a.assetid=av.assetid) inner join sq_ast_attr atr on (av.attrid=atr.attrid)
WHERE
  (a.status = 2 OR a.status = 16)
  AND
  a.type_code = 'bodycopy_div'
  AND
  atr.type_code=a.type_code
  AND
  atr.name='name'
EOT;

$results = MatrixDAL::executeSqlAll($query);
$count = count($results);
$done  = 0;
echo "Processing ".$count." assets .. \n";

foreach ($results as $assetInfo) {
	$done++;
	if ($done % 10 == 0) {
		echo $done." assets complete.\r";
	}

	$contextid = $assetInfo['contextid'];
	
	// The file is stored in the asset (bodycopy_div) - the majorid
	// but the data is stored in the db in the content below it - the minorid.
	$pathSuffix  = asset_data_path_suffix($assetInfo['type_code'], $assetInfo['majorid']);
	$privatePath = SQ_DATA_PATH.'/private/'.$pathSuffix;

	$filename = $privatePath.'/content_file.php';
	if ($contextid > 0) {
		$filename = $privatePath.'/content_file.'.$contextid.'.php';
	}

	// In case we get a context but the asset doesn't have different content,
	// this file won't exist.
	// If that's the case, we don't need to update anything.
	if (file_exists($filename) === FALSE) {
		continue;
	}
	$content = file_get_contents($filename);

	log_write('Updating assetid '.$assetInfo['minorid'].' (context '.$contextid.') with content from '.$filename, 'reimport_content');
	try {
		$sql   = "UPDATE sq_ast_attr_val SET custom_val=:custom_val WHERE assetid=:assetid AND attrid=:attrid AND contextid=:contextid";
		$query = MatrixDAL::preparePdoQuery($sql);
		MatrixDAL::bindValueToPdo($query, 'assetid', $assetInfo['minorid'], PDO::PARAM_STR);
		MatrixDAL::bindValueToPdo($query, 'attrid',  $attributeid, PDO::PARAM_INT);
		MatrixDAL::bindValueToPdo($query, 'contextid',  $contextid, PDO::PARAM_INT);
		MatrixDAL::bindValueToPdo($query, 'custom_val', $content, PDO::PARAM_LOB);
		MatrixDAL::execPdoQuery($query);
	} catch (DALException $e) {
		throw new Exception('Unable to update asset '.$assetInfo['minorid'].' with content from '.$filename.': '.$e->getMessage());
	}
}
echo $done." assets complete.\n";
echo "All done!\n";

$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');

