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
* $Id: regen_metadata_schemas.php,v 1.2 2008/04/22 05:47:16 mbrydon Exp $
*
*/

/**
* This script regenerates metadata for specified metadata schemas in the system.
* If schemas are specified, then all schemas in the system are regenerated.
*
* @author  Daniel Simmons <dsimmons@squiz.net>
* @version $Revision: 1.2 $
* @package MySource_Matrix
*/

ini_set('memory_limit', '-1');
error_reporting(E_ALL);
$metadata_schemas = Array();

if (php_sapi_name() != 'cli') {
	trigger_error("You can only run this script from the command line\n", E_USER_ERROR);
}

// Check for valid system root
$SYSTEM_ROOT = (isset($_SERVER['argv'][1])) ? $_SERVER['argv'][1] : '';
if (empty($SYSTEM_ROOT) || !is_dir($SYSTEM_ROOT)) {
	echo "ERROR: You need to supply the path to the System Root as the first argument\n";
	echo 'Usage: '.basename($_SERVER['argv'][0])." <system root> [schema ID]...\n";
	exit();
}

define('SQ_SYSTEM_ROOT', realpath($SYSTEM_ROOT));
require_once $SYSTEM_ROOT.'/core/include/init.inc';

$GLOBALS['SQ_SYSTEM']->setRunLevel(SQ_RUN_LEVEL_OPEN);

$hh =& $GLOBALS['SQ_SYSTEM']->getHipoHerder();

// If user has specified schema ID's manually, use those
if (isset($_SERVER['argv'][2])) {
	$metadata_schemas = $_SERVER['argv'];

	// Remove the first two arguments
	array_shift($metadata_schemas);
	array_shift($metadata_schemas);
} else {

	// Otherwise, get all the metadata schemas in the system
	$metadata_schemas = $GLOBALS['SQ_SYSTEM']->am->getTypeAssetids('metadata_schema', TRUE);
}

// Run a HIPO to regenerate each schema individually, so we can give feedback along the way
$numSchemasAttempted = 0;
$numSchemasRegenerated = 0;
foreach ($metadata_schemas as $schemaid) {

	$schema = $GLOBALS['SQ_SYSTEM']->am->getAsset($schemaid);
	if (!$schema) {
		trigger_error('Asset #'.$schemaid.' could not be found', E_USER_WARNING);
		continue;
	}

	// If this asset is not a metadata schema, print a warning and skip it
	if (!($schema instanceof Metadata_Schema)) {
		trigger_error('Asset #'.$schema->id.' is not a metadata schema', E_USER_WARNING);
		continue;
	}

	printName('Regenerating schema "'.$schema->name.'" (#'.$schema->id.')...');

	// Send an array with the single schema ID as the parameter for this HIPO
	$vars = Array('schemaids' => Array($schema->id));

	// Run HIPO
	$errors = $hh->freestyleHipo('hipo_job_regenerate_metadata', $vars);
	$numSchemasAttempted++;

	// If there were real errors returned, print them, otherwise print OK
	if (count($errors) > 0) {
		printUpdateStatus('!!!');

		// Print errors encountered
		echo "\nErrors / warnings encountered:\n";
		foreach ($errors as $error) {
			echo "\t- ".$error['message']."\n";
		}

		echo "\n";
	} else {
		printUpdateStatus('OK');
		$numSchemasRegenerated++;
	}

	$GLOBALS['SQ_SYSTEM']->am->forgetAsset($schema);

}//end foreach

$GLOBALS['SQ_SYSTEM']->restoreRunLevel();

echo "\nMetadata Schemas regenerated: ".$numSchemasRegenerated.' out of '.$numSchemasAttempted."\n";
echo "Done\n";



//--        HELPER FUNCTIONS        --//


/**
* Print the current action of the script
*
* @param string	$name	Current action
*
* @return void
* @access public
*/
function printName($name)
{
	printf ('%s%'.(60 - strlen($name)).'s', $name, '');

}//end printName()


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


?>
