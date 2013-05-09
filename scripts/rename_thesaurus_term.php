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
* $Id: rename_thesaurus_term.php,v 1.1.2.3 2013/05/09 05:11:50 akarelia Exp $
*
*/

/**
* This script will rename a thesaurus term, update the sq_ast_mdata_val 
* for the updated thes term and then regenerate the assets using it
*
* This script is basically an extract from the following
*    - thesaurus_term_edit_fns.inc
*    - hipo_job_rename_thesaurus_term.inc
*    - regen_metadata_schemas.php
*
* @author  Ash Karelia <akarelia@squiz.com.au>
* @version $Revision: 1.1.2.3 $
* @package MySource_Matrix
*/

if (ini_get('memory_limit') != '-1') ini_set('memory_limit', '-1');
error_reporting(E_ALL);

if (php_sapi_name() != 'cli') {
	trigger_error("You can only run this script from the command line\n", E_USER_ERROR);
}

// Check for valid system root
$SYSTEM_ROOT = (isset($_SERVER['argv'][1])) ? $_SERVER['argv'][1] : '';
if (empty($SYSTEM_ROOT)) {
	echo "ERROR: You need to supply the path to the System Root as the first argument\n";
	usage();
	exit();
}

if (!is_dir($SYSTEM_ROOT) || !is_readable($SYSTEM_ROOT.'/core/include/init.inc')) {
	echo "ERROR: Path provided doesn't point to a Matrix installation's System Root. Please provide correct path and try again.\n";
	usage();
	exit();
}

$THES_TERM_ID = (isset($_SERVER['argv'][2])) ? $_SERVER['argv'][2] : '';
if (empty($THES_TERM_ID)) {
	echo "ERROR: Please provide valid assetid for the Thesaurus Term.\n";
	usage();
	exit();
}

$THES_TERM_NAME_NOW = (isset($_SERVER['argv'][3])) ? $_SERVER['argv'][3] : '';
if (empty($THES_TERM_NAME_NOW)) {
	echo "ERROR: Please provide valid Name of the Thesaurus Term.\n";
	usage();
	exit();
}

$THES_TERM_NAME_THEN = (isset($_SERVER['argv'][4])) ? $_SERVER['argv'][4] : '';
if (empty($THES_TERM_NAME_THEN)) {
	echo "ERROR: Please provide valid Name to change the Thesaurus Term to.\n";
	usage();
	exit();
}

define('BATCH_SIZE', 50);																	// The number of assets being processed in one thread.
define('MAX_CONCURRENCY', 3);
define('SYNCH_FILE', $SYSTEM_ROOT.'/data/private/logs/thesaurus_term_rename_regen_metadata.assetid');

$pid_prepare    = pcntl_fork();
	switch ($pid_prepare) {
		case -1:
			break;
		case 0:

			require_once $SYSTEM_ROOT.'/core/include/init.inc';
			// This ridiculousness allows us to workaround Oracle, forking and CLOBs
			// if a query is executed that returns more than 1 LOB before a fork occurs,
			// the Oracle DB connection will be lost inside the fork.
			// have to place all queries inside a forked process, and store back results to a physical file

			$THES_TERM_ASSET = $GLOBALS['SQ_SYSTEM']->am->getAsset($THES_TERM_ID, 'thesaurus_term', FALSE);

			if (is_null($THES_TERM_ASSET)) exit(1);

			if ($THES_TERM_ASSET->name != $THES_TERM_NAME_NOW) exit(1);

			// all the things we need are provided?
			// lets get the ball rolling
			echo "Renaming Thesaurus Term.\t\t\t";
			$GLOBALS['SQ_SYSTEM']->setRunLevel(SQ_RUN_LEVEL_FORCED);
			$THES_TERM_ASSET->setAttrValue('name', $THES_TERM_NAME_THEN);
			if (!$THES_TERM_ASSET->saveAttributes()) {
				$THES_TERM_ASSET->setAttrValue('name', $THES_TERM_NAME_NOW);
				printUpdateStatus("!!!");
				echo "ERROR: Couldn't change the name for the Thesaurus Term.\n";
				$GLOBALS['SQ_SYSTEM']->restoreRunLevel();
				exit();
			}
			$GLOBALS['SQ_SYSTEM']->restoreRunLevel();
			printUpdateStatus("Done");

			// once we are done with renaming the terms in the DB
			// find out the metadata schema that uses this thesaurus
			echo "Finding Metadata Schemas to regenerate.\t\t";
			$schemas_to_regen = _findSchemas($THES_TERM_ID);
			printUpdateStatus("Done");

			echo "Updating Thesaurus Term on Assets.\t\t";
			renameTerm(array_values($schemas_to_regen), $THES_TERM_NAME_NOW, $THES_TERM_NAME_THEN);
			printUpdateStatus("Done");

			$mm = $GLOBALS['SQ_SYSTEM']->getMetadataManager();
			$total_assets = Array();
			foreach ($schemas_to_regen as $schemaid => $metadata_fieldid) {
				$schema = $GLOBALS['SQ_SYSTEM']->am->getAsset($schemaid);
				if (!$schema) {
					trigger_error('Schema asset #'.$schemaid.' could not be found', E_USER_WARNING);
					continue;
				}
				if (!($schema instanceof Metadata_Schema)) {
					trigger_error('Asset #'.$schema->id.' is not a metadata schema', E_USER_WARNING);
					continue;
				}
				$total_assets = array_merge($total_assets, $mm->getSchemaAssets($schemaid, TRUE));
			}

			// Save the list into a file so we can access the list from the parent process
			file_put_contents(SYNCH_FILE, implode(',', $total_assets));

			exit(0);
			// waiting for child exit signal
			$status = null;
			pcntl_waitpid(-1, $status);

		break;

		default:
			$status = null;
			pcntl_waitpid(-1, $status);

		break;

	}//end switch


// restore the to-be-processed assetid from the physical file
$total_assets    = Array();
if (file_exists(SYNCH_FILE)) {
	$total_assets_str    = file_get_contents(SYNCH_FILE);
} else {
	trigger_error ("Unable to find Synch File, probably because the user executing this script does not have permission to write to this folder \nOR\nThe supplied assetid of the Thersaurus Term is not not valid", E_USER_WARNING);
	exit(0);
}//end else

$total_assets    = explode(',', $total_assets_str);
$total_assets    = array_unique($total_assets);
// Chunk them up so we can process each batch when forking
$chunk_assets    = array_chunk($total_assets, BATCH_SIZE);
$current_asset_list    = Array();

echo "Total Asset found to regenerate schema on : \t".count($total_assets)."\n\n";
$fork_num    = 0;		// Determine how many child process we have forked
while (!empty($chunk_assets)) {
	// do 50 assets per process
	// the main reason we need to chunk and fork is because  memory leakage prevention
	// call hipo job directly will cause slowly increasing memory usage, and eventually explode
	$current_asset_list    = array_pop($chunk_assets);
	$pid	= pcntl_fork();
	$fork_num++;
	switch ($pid) {
		case -1:
			trigger_error('Process failed to fork while regenerating metadata', E_USER_ERROR);
			exit(1);
			break;
		case 0:

			require_once $SYSTEM_ROOT.'/core/include/init.inc';
			$mm    = $GLOBALS['SQ_SYSTEM']->getMetadataManager();

			foreach ($current_asset_list as $assetid) {
				$asset    = $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
				$GLOBALS['SQ_SYSTEM']->setRunLevel(SQ_RUN_LEVEL_FORCED);
				$contextids = array_keys($GLOBALS['SQ_SYSTEM']->getAllContexts());
				foreach ($contextids as $contextid) {
					if (!$mm->regenerateMetadata($assetid, NULL)) {
						trigger_error('Asset failed to regenrate metedata #'.$assetid.'', E_USER_WARNING);
						continue;
					} else {
						echo 'Regenerated Metadata for assetid '.$assetid." (Context #$contextid)\r";
					}
				}

				$GLOBALS['SQ_SYSTEM']->am->forgetAsset($asset);
				unset($asset);

			}//end foreach

			unset($current_asset_list);
			exit(0);
			// waiting for child exit signal
			$status = null;
			pcntl_waitpid(-1, $status);

			break;
		default:
			// We only want to fork a maximum number of child process, so if we've already reached the max num, sit and wait
			if ($fork_num >= MAX_CONCURRENCY) {
				$status = null;
				pcntl_waitpid(-1, $status);
				$fork_num--;
			}//end if

			if (empty($chunk_assets)) {
				// We wait for all the fork child to finish
				while ($fork_num > 0) {
					$status = null;
					pcntl_waitpid(-1, $status);
					$fork_num--;
				}//end
			}//end if

			break;

	}//end switch & thread
}//end while


if (file_exists(SYNCH_FILE)) {
	unlink(SYNCH_FILE);
}//end if

echo "\n\nScript successfully finished!\n\n";

exit(0);




/**
* Print whether or not the last action was successful
*
* @param string	$status	'OK' for success, or '!!!' for fail
*
* @return void
* @access public
*/
function printUpdateStatus($status)
{
	echo "[ $status ]\n";

}//end printUpdateStatus()


/**
* Prints the usage for the script
*
* @return void
* @access public
*/
function usage()
{
	echo "php ".basename(__FILE__)." <SYSTEM_ROOT> <termid_to_rename> <existing_name> <rename_to>\n\n";

}//end usage()



function renameTerm($fieldids, $old_term, $new_term)
{
	// Do the rename per asset and play nice with ORACLE
	$chunk_size = 1000;
	$field_chunks = array_chunk($fieldids, $chunk_size);
	foreach ($field_chunks as $field_chunk) {
		// Quoting Shakespeare
		foreach ($field_chunk as $index => $field_assetid) {
			$field_chunk[$index] = MatrixDAL::quote($field_assetid);
		}//end foreach


		$sql = "SELECT value, assetid, fieldid, contextid FROM sq_ast_mdata_val WHERE value like '$old_term,%' OR value like '%,$old_term' OR value like '%,$old_term,%'";

		$results = MatrixDAL::executeSqlAssoc($sql);

		if (!empty($results)) {

			foreach($results as $index => $result) {
				$asset_id = $result['assetid'];
				$value    = $result['value'];
				$field_id = $result['fieldid'];
				$contextid= $result['contextid'];

				$pattern_1 = '/(.*)'.$old_term.'$/';
				$pattern_2 = '/^'.$old_term.'(.*)/';
				$pattern_3 = '/(.*)'.$old_term.'(.*)/';

				$replacement_1 = '$1'.$new_term;
				$replacement_2 = $new_term.'$1';
				$replacement_3 = '$1'.$new_term.'$2';

				if (preg_match($pattern_2, $value)) {
					$new_value = preg_replace($pattern_2, $replacement_2, $value);
				} else if (preg_match($pattern_1, $value)) {
					$new_value = preg_replace($pattern_1, $replacement_1, $value);
				} else {
					$new_value = preg_replace($pattern_3, $replacement_3, $value);
				}

				// Run the Query against the current assetid
				if (MatrixDAL::getDbType() === 'oci') {
					$sql  = 'UPDATE sq_ast_mdata_val SET value=:new_value WHERE TO_CHAR(value)=:old_value AND contextid=:contextid AND fieldid=:fieldid AND assetid=:assetid';
				} else {
					$sql  = 'UPDATE sq_ast_mdata_val SET value=:new_value WHERE value=:old_value AND contextid=:contextid AND fieldid=:fieldid AND assetid=:assetid';
				}//end if

				try{
					$query = MatrixDAL::preparePdoQuery($sql);
					MatrixDAL::bindValueToPdo($query, 'new_value', $new_value);
					MatrixDAL::bindValueToPdo($query, 'old_value', $value);
					MatrixDAL::bindValueToPdo($query, 'contextid', $contextid);
					MatrixDAL::bindValueToPdo($query, 'fieldid', $field_id);
					MatrixDAL::bindValueToPdo($query, 'assetid', $asset_id);
					MatrixDAL::execPdoQuery($query);
				} catch (Exception $e) {
					throw new Exception('DB Error: '.$e->getMessage());
				}
			}
		}

		// Run the Query against the current assetid
		if (MatrixDAL::getDbType() === 'oci') {
			$sql  = 'UPDATE sq_ast_mdata_val SET value=:new_term WHERE TO_CHAR(value)=:old_term AND fieldid IN ('.implode(',', $field_chunk).')';
		} else {
			$sql  = 'UPDATE sq_ast_mdata_val SET value=:new_term WHERE value=:old_term AND fieldid IN ('.implode(',', $field_chunk).')';
		}//end if

		// Update EVERYTHING
		try {
			$query = MatrixDAL::preparePdoQuery($sql);
			MatrixDAL::bindValueToPdo($query, 'new_term', $new_term);
			MatrixDAL::bindValueToPdo($query, 'old_term', $old_term);
			MatrixDAL::execPdoQuery($query);
		} catch (Exception $e) {
			throw new Exception('DB Error: '.$e->getMessage());
		}
	}

}//end renameTerm()


/**
* Find a schemas and fields per thesaurus
*
* @return boolean
* @access public
*/
function _findSchemas($thesaurus_fieldid)
{
	$metadata_schema = Array();

	// Match the schemas and fields to the terms to change
	$current_schemas = $GLOBALS['SQ_SYSTEM']->am->getTypeAssetids('metadata_schema', TRUE);
	foreach ($current_schemas as $schemaid) {
		$schema = $GLOBALS['SQ_SYSTEM']->am->getAsset($schemaid, 'metadata_schema', TRUE);
		if (!is_null($schema)) {
			$fields = $GLOBALS['SQ_SYSTEM']->am->getChildren($schemaid, 'metadata_field_thesaurus');
			foreach ($fields as $fieldid => $field_type) {
				$field = $GLOBALS['SQ_SYSTEM']->am->getAsset($fieldid, 'metadata_field_thesaurus', TRUE);
				if (!is_null($field)) {
					$field_thesaurus = $field->attr('thesaurus_assetid');
					if ($field_thesaurus == preg_replace('|(:[0-9]+)|', '', $thesaurus_fieldid)) {
						// We have a winner!
						$metadata_schema[$schemaid] = $fieldid;
					}//end if
				}//end if
			}//end foreach
		}//end if
	}//end foreach

	return $metadata_schema;

}//end _findSchemas()


?>
