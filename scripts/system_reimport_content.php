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
* $Id: system_reimport_content.php,v 1.3 2011/11/29 22:26:44 csmith Exp $
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
* @version $Revision: 1.3 $
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

/*
 * Most commonly this script will be used to allow utf-8 chars, but in case it's not:
 * Further down where it loads the content, it can check if the content contains utf-8 chars
 * If it doesn't contain utf-8 chars, the database content is *not* updated.
 * If it does contain utf-8 chars, the content is updated.
 *
 * If checkUtf8 is FALSE, the update happens (the utf-8 character check is skipped).
 * If it's TRUE, the update is skipped IF there are no utf-8 chars in the content.
 *
 * If you're converting to something other than al32utf8, then the update will happen.
 * You can also overwrite this setting if you need to (small code change to comment out the encoding
 * check) so the updates happen no matter what.
 */
$checkUtf8 = FALSE;
$encoding = '(empty)';
if (isset($db_conf['db']['encoding']) === TRUE) {
	$encoding = $db_conf['db']['encoding'];
}
if ($db_conf['db']['type'] == 'oci' && strtolower($encoding) == 'al32utf8') {
	$checkUtf8 = TRUE;
}

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
 * We need the attribute id for the content so we can update it correctly.
 */
$query = "select attrid from sq_ast_attr where type_code='content_type_wysiwyg' and name='html'";
$results = MatrixDAL::executeSqlAll($query);
$attributeid = $results[0]['attrid'];

/**
 * The query includes ast_attr_val and ast_attr so we can get the contextid.
 * We need to include ast_attr so we can restrict the type code and name of the
 * attribute we're including, otherwise we end up with a row for each attribute id.
 *
 * See init.inc for other statuses,
 * but we only want live or under construction assets.
 * If they are in workflow or safe-edit (or something else)
 * they may have content changes already, and we don't want
 * to touch those.
 *
 * status 2  = under construction
 * status 16 = live
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

	// If we're checking for utf-8 chars, do it here (and also log the outcome).
	if ($checkUtf8 === TRUE) {
		// If we don't find any utf-8 chars, we don't need to update the content.
		$foundUtf8 = preg_match('%([\xc2-\xf4][\x80-\xbf]{1,3})%', $content);
		log_write('Found utf8 characters in file '.$filename.': '.($foundUtf8 == TRUE ? 'yes' : 'no'), 'reimport_content');
		if ($foundUtf8 == FALSE) {
			log_write('Updating of assetid '.$assetInfo['minorid'].' (context '.$contextid.') skipped', 'reimport_content');
			continue;
		}
	}

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

