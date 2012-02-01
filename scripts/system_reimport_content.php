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
* $Id: system_reimport_content.php,v 1.7 2012/02/01 01:10:18 ewang Exp $
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
* @version $Revision: 1.7 $
* @package MySource_Matrix
*/

if (php_sapi_name() != 'cli') {
	trigger_error("You can only run this script from the command line\n", E_USER_ERROR);
}

if (function_exists('iconv') === FALSE) {
	echo "This script uses the iconv module to convert character sets from one type to another.\n";
	echo "This module was not found on your server. Please install it and try again.\n";
	exit(1);
}

$SYSTEM_ROOT = (isset($_SERVER['argv'][1])) ? $_SERVER['argv'][1] : '';
if (empty($SYSTEM_ROOT) || !is_dir($SYSTEM_ROOT)) {
	trigger_error("You need to supply the path to the System Root as the first argument\n", E_USER_ERROR);
}
$db_conf = require $SYSTEM_ROOT.'/data/private/conf/db.inc';
array_shift($_SERVER['argv']);

$reportOnly = FALSE;
$verbose = FALSE;
if (isset($_SERVER['argv'][1]) && $_SERVER['argv'][1] == '--report') {
	$reportOnly = TRUE;
	array_shift($_SERVER['argv']);
}
if (isset($_SERVER['argv'][1]) && $_SERVER['argv'][1] == '--verbose') {
	$verbose = TRUE;
	array_shift($_SERVER['argv']);
}

/**
 * Prints out a usage message to let the user know what's happening.
 * This always shows up - because we are changing content without
 * any sort of validation apart from a confirmation message required.
 * It also shows the current encoding set in the db.inc file,
 * and shows the current encoding of the database.
 * It's up to the user to take the appropriate action.
 */
function usage()
{
	global $db_conf, $db_encoding;
	$encoding = '(empty)';
	if (isset($db_conf['db2']['encoding']) === TRUE) {
		$encoding = $db_conf['db2']['encoding'];
	}
	echo $_SERVER['argv'][0]." SYSTEM_ROOT [--report] [--verbose]\n";
	echo "This script will re-import content from files into the database.\n";
	echo "This is mainly useful for oracle systems where the wrong database\n";
	echo "encoding has been used and needs to be changed.\n";
	echo "Assets that are in workflow or in safe-edit will not be re-imported.\n";
	echo "\n";
	echo "This will overwrite existing content with no warning.\n";
	echo "\n";
	if ($db_conf['db2']['type'] == 'oci') {
		echo "Make sure the correct encoding has been set in db.inc before you start.\n";
		echo "It is currently set to:".$encoding."\n";
		echo "The database has this encoding: ".$db_encoding."\n";
		echo "\n";
	}
	echo "If you pass --report to the script, a report will be generated based on the\n";
	echo "number of pages this script will affect.\n\n";
	echo "If you pass --verbose to the script, it will show what happens per asset\n";
	echo "and also the asset name, attribute, context that is affected.\n";
}

/**
 * Prints a message out to the screen if verbose mode is enabled.
 * @param string $message The message to print out.
 *
 * @return void
 */
function printVerbose($message)
{
	global $verbose;
	if ($verbose) {
		print $message."\n";
	}
}

/*
 * Most commonly this script will be used to allow utf-8 chars, but in case it's not:
 * Further down where it loads the content, it can check if the content contains utf-8 chars
 * If it doesn't contain utf-8 chars, the database content is *not* updated.
 * If it does contain utf-8 chars, the content is updated.
 *
 * If encodeToUtf8 is FALSE, the update happens (the utf-8 character check is skipped).
 * If it's TRUE, the update is skipped IF there are no utf-8 chars in the content.
 *
 * If you're converting to something other than al32utf8, then the update will happen.
 * You can also overwrite this setting if you need to (small code change to comment out the encoding
 * check) so the updates happen no matter what.
 */
$encodeToUtf8 = FALSE;
$encoding = '(empty)';
if (isset($db_conf['db2']['encoding']) === TRUE) {
	$encoding = $db_conf['db']['encoding'];
}
if ($db_conf['db2']['type'] == 'oci' && strtolower($encoding) == 'al32utf8') {
	$encodeToUtf8 = TRUE;
}

require_once $SYSTEM_ROOT.'/core/include/init.inc';
require_once $SYSTEM_ROOT.'/core/lib/DAL/DAL.inc';
require_once $SYSTEM_ROOT.'/core/lib/MatrixDAL/MatrixDAL.inc';

$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
MatrixDAL::changeDb('db2');

$db_encoding = NULL;
if ($db_conf['db2']['type'] == 'oci') {
	$query = "SELECT VALUE FROM NLS_DATABASE_PARAMETERS WHERE PARAMETER = 'NLS_CHARACTERSET'";
	$results = MatrixDAL::executeSqlAll($query);
	$db_encoding = $results[0]['value'];
}

if ($reportOnly === FALSE) {
	usage();
	// Make sure the person running the script understands
	// the consequences. Content *WILL* be overwritten.
	echo "OK to proceed (type 'yes' to continue): ";
	$response = trim(fgets(STDIN));

	if ($response != 'yes') {
		echo "Aborting.\n";
		exit(1);
	}
}

$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

/**
 * We need the attribute id for the content so we can update it correctly.
 */
$query = "select
  attrid
from
  sq_ast_attr
where
  type_code='content_type_wysiwyg'
  and name='html'
  ";
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
select
  l.majorid,
  l.minorid,
  a.type_code,
  a.name,
  av.contextid,
  av.attrid
from
  sq_ast_lnk l inner join sq_ast a on (a.assetid=l.majorid)
  inner join sq_ast_attr_val av on (a.assetid=av.assetid)
  inner join sq_ast_attr atr on (av.attrid=atr.attrid)
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

// Currently we're reporting on the following charsets:
// utf-8
// iso-8859-1
// ascii
// Windows-1252
$charsetReport = array(
		'ascii'        => array(),
		'iso-8859-1'   => array(),
		'utf-8'        => array(),
		'windows-1252' => array(),
);

$start = time();
foreach ($results as $assetInfo) {
	$done++;
	if ($done % 50 == 0) {
	    $elapsed = time() - $start;
	    if ($elapsed > 0) {
			$frac = $done / $count;
			$remain = $elapsed / $frac - $elapsed;
			$pct = $frac * 100;
			printf("%.2f%%: %d assets processed in %d seconds, %.2f remaining.\r", $pct, $done, $elapsed, $remain);
	    }
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
	$oldContent = file_get_contents($filename);

	$contentIsUpdated = FALSE; $fileIsUpdated = FALSE;
	// If we're checking for utf-8 chars, do it here (and also log the outcome).
	if ($encodeToUtf8 === TRUE) {
		// If we don't find any utf-8 chars, we don't need to update the content.
		$assetName = 'Asset '.$assetInfo['minorid']
		            .' (child of asset '.$assetInfo['majorid']
		            .'): '.$assetInfo['name']
		            ;
		if (preg_match('%([\xc2-\xf4][\x80-\xbf]{1,3})%', $oldContent)) {
			# No need to convert content into UTF-8, but we do need to save it.
			$contentIsUpdated = TRUE;
			$content = $oldContent;
			$charsetReport['utf-8'][] = $assetName;
		} else if (preg_match('%([\x80-\x9f])%', $oldContent)) {
			# Convert content from Windows-1252 to UTF-8
			$content = iconv('Windows-1252','UTF-8',$oldContent);
			if ($content !== $oldContent) {
				$contentIsUpdated = TRUE;
				$fileIsUpdated = TRUE;
			}
			$charsetReport['windows-1252'][] = $assetName;
		} else if (preg_match('%([\xa0-\xff])%', $oldContent)) {
			# Convert content from ISO-8859-1 to UTF-8
			$content = iconv('ISO-8859-1','UTF-8',$oldContent);
			if ($content !== $oldContent) {
				$contentIsUpdated = TRUE;
				$fileIsUpdated = TRUE;
			}
			$charsetReport['iso-8859-1'][] = $assetName;
		} else {
			# else it's 7-bit ASCII which we can ignore, but we need for our report.
			$charsetReport['ascii'][] = $assetName;
		}
	} # Handle other encoding scenarios here.

	if ($reportOnly === FALSE) {
		if ($contentIsUpdated) { # We need up update the content in the database
			$message = 'Updating assetid '.$assetInfo['minorid'].' (context '.$contextid.') with content from '.$filename
				 .' (assetid='.$assetInfo['minorid'].',main attrid='.$attributeid.',this attrid='.$assetInfo['attrid'].")";
			log_write($message, 'reimport_content');
			printVerbose($message);

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
		} else {
			$message = 'Updating of assetid '.$assetInfo['minorid'].' (context '.$contextid.') skipped';
			log_write($message, 'reimport_content');
			printVerbose($message);
		}
		if ($fileIsUpdated) {
			$message = 'File contents have changed in re-encoding: saving file'.$filename;
			log_write($message, 'reimport_content');
			printVerbose($message);

			$fh = @fopen($filename, 'w');
			if ($fh === FALSE) {
				$message = 'Unable to open file '.$filename.' for writing - check permissions and try again.';
				log_write($message, 'reimport_content');
				printVerbose($message);
			} else {
				fwrite($fh, $content);
				fclose($fh);
			}
		}
	}
}

$elapsed = time() - $start;
printf("100.0%%: %d assets processed in %d seconds, 0 remaining.\n", $done, $elapsed);

foreach ($charsetReport as $charset => $assetList) {
	if (count($assetList) > 0) {
		echo "Found ".number_format(count($assetList))." pages with character set ".$charset.".";
		if ($reportOnly === FALSE) {
			if ($charset == 'utf-8') {
				echo " These were re-saved in UTF-8.";
			} else if ($charset != 'ascii') {
				echo " These were converted to UTF-8.";
			} else {
				echo " These did not need conversion.";
			}
		}
		echo "\n";
		if ($verbose) {
			# Print out the list of actual assets converted.
			foreach ($assetList as $assetName) {
				echo ' -> ', $assetName, "\n";
			}
		}
	}
}

echo "All done!\n";

$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');

