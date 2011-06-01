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
* $Id: import_asset_csv_to_matrix.php,v 1.6 2009/03/17 04:40:28 ewang Exp $
*
*/

/**
* CSV-to-Matrix Asset Import Script
* Source CSV to Matrix System
*
* @author	Mark Brydon <mbrydon@squiz.net>
* 28 Nov 2007
*
*
* Purpose:
*	Import a CSV file containing asset attribute and metadata values into Matrix as assets.
*	This script will convert the CSV input and create the assets directly on the Matrix system by referring
*	to metadata and attribute mapping files. The asset type can be specified on the command line
*
* Documentation:
*	This script can be used only with assets that can be created without interaction between other asset types.
*	For example, creating a Standard Page with separate Bodycopy DIVs is not supported.
*
*	This script has been developed and tested with:
*		- User assets
*		- Single Calendar Event assets
*
*	This script expands upon scripts/import/csv_to_xml_actions.php in that:
*		1. This script creates assets directly and does not output XML
*		2. This script outputs CSV to track the unique identifier supplied and the asset ID created
*		3. Item 2 can be leveraged to write small scripts to work on assets previously imported by this script
*		   (eg; assign permissions in another script to a page to user X imported in this script)
*
*	Assignment of (only) one metadata schema and relevant values is supported with this script.
*	No validation is performed on any attributes or metadata fields, so it is assumed that the
*	input provided to this script is correctly formatted
*
*	The Metadata Mapping CSV file must be in the following format:
*		metadata field asset ID, supplied field name
*		(eg; 1234, Title)
*
*	The Asset Attribute Mapping CSV file must be in the following format:
*		supplied field name, asset attribute name
*		(eg; USER_FIRST_NAME, first_name)
*
*/


/**
* Prints out some basic help info detailing how to use this script
*
* @return void
* @access public
*/
function printUsage()
{
	printStdErr("CSV to Matrix importer\n\n");
	printStdErr("Usage: import_asset_csv_to_matrix [system root] [type code] [csv import file] [parent id] [schema id] [meta map file] [attr map file]\n");
	printStdErr("system root            : The Matrix System root directory\n");
	printStdErr("type code              : The Matrix Asset Type Code of the assets to import\n");
	printStdErr("csv import file        : A CSV file containing the records to import as Matrix assets\n");
	printStdErr("parent id              : The asset ID of a Folder etc. under which the assets are to reside\n");
	printStdErr("schema id              : The asset ID of the Metadata Schema to apply to each asset\n");
	printStdErr("meta map file          : A CSV file containing attribute name-to-metadata field ID associations\n");
	printStdErr("attr map file          : A CSV file containing attribute name-to-asset attribute field associations\n");

}//end printUsage()


/**
* Prints the supplied string to "standard error" (STDERR) instead of the "standard output" (STDOUT) stream
*
* @param string	$string	The string to write to STDERR
*
* @return void
* @access public
*/
function printStdErr($string)
{
	fwrite(STDERR, "$string");

}//end printStdErr()


/**
* Prints the supplied string to "standard error" (STDERR) instead of the "standard output" (STDOUT) stream
*
* @param string	$source_csv_filename	The CSV import file containing the asset attribute and metadata field specifications
* @param string	$asset_type_code		The Matrix asset type code or the assets which are to be created
* @param object	$parent_id				The asset ID of the parent asset under which the new assets are to reside
* @param int	$schema_id				The asset ID of the Metadata Schema to associate with the new assets
* @param array	$metadata_mapping		A structure containing Supplied Field Name to Metadata Field asset ID associations
* @param array	$attribute_mapping		A structure containing Supplied Field Name to Asset Attribute Name associations
*
* @return void
* @access public
*/
function createAssets($source_csv_filename, $asset_type_code, $parent_id, $schema_id, $metadata_mapping, $attribute_mapping)
{
	$num_assets_imported = 0;

	$csv_fd = fopen($source_csv_filename, 'r');
	if (!$csv_fd) {
		printUsage();
		printStdErr("* The supplied CSV import file was not found\n\n");
		fclose($csv_fd);
		exit(-6);
	}

	$parent_asset =& $GLOBALS['SQ_SYSTEM']->am->getAsset($parent_id);
	if (!$parent_asset->id) {
		printUsage();
		printStdErr("* The specified parent asset was not found\n\n");
		exit(-7);
	}

	$header_line = TRUE;
	$headers = Array();

	while (($data = fgetcsv($csv_fd, 1024, ',')) !== FALSE) {
		$num_fields = count($data);

		$asset_spec = Array();

		foreach ($data as $key => $val) {
			if ($header_line) {
				$headers[$key] = trim($val);
			} else {
				$asset_spec[$headers[$key]] = $val;
			}
		}

		if ($header_line) {
			$header_line = FALSE;
		} else {
			$asset_info = createAsset($asset_spec, $asset_type_code, $parent_asset, $schema_id, $metadata_mapping, $attribute_mapping);
			$asset_id = 0;
			if (is_array($asset_info)) {
				$asset_id = reset($asset_info);
			}

			if ($asset_id) {
				echo key($asset_info).','.$asset_id."\n";
				$num_assets_imported++;
			}
		}
	}

	fclose($csv_fd);

	return $num_assets_imported;

}//end createAssets()


/**
* Creates a simple "TYPE 1" link between assets
*
* @param object	&$parent_asset	The parent asset
* @param object	&$child_asset	The child asset
*
* @return int
* @access public
*/
function createLink(&$parent_asset, &$child_asset)
{
	// Link the asset to the parent asset
	$link = Array(
				'asset'			=> &$parent_asset,
				'link_type'		=> 1,
				'value'			=> '',
				'sort_order'	=> NULL,
				'is_dependant'	=> FALSE,
				'is_exclusive'	=> FALSE,
			);

	$link_id = $child_asset->create($link);

	return $link_id;

}//end createLink()


/**
* Create an asset of a specified type with associated metadata and attribute settings
*
* @param array	$asset_spec			The specification of the asset fields directly from the CSV import file
* @param string	$asset_type_code	The Matrix asset type code or the asset which is to be created
* @param object	&$parent_asset		The parent asset under which the new asset is to reside
* @param int	$schema_id			The asset ID of the Metadata Schema to associate with the new asset
* @param array	$metadata_mapping	A structure containing Supplied Field Name to Metadata Field asset ID associations
* @param array	$attribute_mapping	A structure containing Supplied Field Name to Asset Attribute Name associations
*
* @return void
* @access public
*/
function createAsset($asset_spec, $asset_type_code, &$parent_asset, $schema_id, $metadata_mapping, $attribute_mapping)
{
	$attribs = Array();

	printStdErr('- Creating asset');

	$asset =& new $asset_type_code();
	printStdErr('.');

	// Set attributes
	$first_attr_name = '';
	foreach ($attribute_mapping as $supplied_name => $attribute_name) {
		if ($first_attr_name == '') {
			$first_attr_name = $supplied_name;
			printStdErr(' '.$asset_spec[$first_attr_name]);
		}

		if (isset($asset_spec[$supplied_name])) {
			$asset->setAttrValue($attribute_name, $asset_spec[$supplied_name]);
		}
	}
	printStdErr('.');

	// Link the new asset under the parent folder
	$link_id = createLink($parent_asset, $asset);
	printStdErr('.');

	// Assign metadata schema and values to the asset
	
	$mm = $GLOBALS['SQ_SYSTEM']->getMetadataManager();
	$mm->setSchema($asset->id, $schema_id, TRUE);

	foreach ($metadata_mapping as $supplied_field_name => $metadata_field_id) {
		if (isset($asset_spec[$supplied_field_name])) {
			$metadata = Array($metadata_field_id => Array ( 
															0 => Array(
																	'value' => $asset_spec[$supplied_field_name],
																	'name' => $supplied_field_name 
																)
													)
							);

			$mm->setMetadata($asset->id, $metadata);
		}
	}
	printStdErr('.');

	// Free memory
	$GLOBALS['SQ_SYSTEM']->am->forgetAsset($asset);

	printStdErr(' => asset ID '.$asset->id."\n");

	return Array($asset_spec[$first_attr_name] => $asset->id);

}//end createAsset()


/************************** MAIN PROGRAM ****************************/

ini_set('memory_limit', '-1');

if ((php_sapi_name() != 'cli')) {
trigger_error("You can only run this script from the command line\n", E_USER_ERROR);
}

// Matrix system root directory
$argv = $_SERVER['argv'];
$GLOBALS['SYSTEM_ROOT'] = (isset($argv[1])) ? $argv[1] : '';
if (empty($GLOBALS['SYSTEM_ROOT'])) {
	printUsage();
	printStdErr("* The Matrix system root directory must be specified as the first parameter\n\n");
	exit(-99);
}

require_once $GLOBALS['SYSTEM_ROOT'].'/core/include/init.inc';

// Has a Matrix Type Code been supplied?
$asset_type_code = $argv[2];
if (empty($asset_type_code)) {
	printUsage();
	printStdErr("* A Matrix Type Code must be specified as the second parameter\n\n");
	exit(-1);
} else {
	// Check Matrix Type Code
	$GLOBALS['SQ_SYSTEM']->am->includeAsset($asset_type_code);
}

// Has an XML filename been supplied?
$source_csv_filename   = $argv[3];
if (empty($source_csv_filename)) {
	printUsage();
	printStdErr("* A source CSV filename must be specified as the third parameter\n\n");
	exit(-2);
}

// Has a parent ID been supplied?
$parent_id  = (int)$argv[4];
if ($parent_id == 0) {
	printUsage();
	printStdErr("* A Parent asset ID must be specified as the fourth parameter\n\n");
	exit(-3);
}

// Has a schema ID been supplied?
$schema_id  = (int)$argv[5];
if ($schema_id == 0) {
	printUsage();
	printStdErr("* A Metadata Schema asset ID must be specified as the fifth parameter\n\n");
	exit(-4);
}

// Has a CSV mapping file been supplied?
$mapping_filename  = $argv[6];
if (empty($mapping_filename)) {
	printUsage();
	printStdErr("* A Metadata Field CSV mapping file must be specified as the sixth parameter\n\n");
	exit(-5);
}

// Has a CSV attribute mapping file been supplied?
$attribute_mapping_filename  = $argv[7];
if (empty($mapping_filename)) {
	printUsage();
	printStdErr("* An attribute CSV mapping file must be specified as the seventh parameter\n\n");
	exit(-6);
}

// Do the supplied files exist?
$csv_fd = fopen($mapping_filename, 'r');
if (!$csv_fd) {
	printUsage();
	printStdErr("* The supplied metadata mapping file was not found\n\n");
	fclose($csv_fd);
	exit(-7);
}

$metadata_mapping = Array();

while (($data = fgetcsv($csv_fd, 1024, ',')) !== FALSE) {
	$num_fields = count($data);

	if ($num_fields == 2) {
		$metadata_mapping[trim($data[1])] = (int)$data[0];
	}
}
fclose($csv_fd);

$csv_fd = fopen($attribute_mapping_filename, 'r');
if (!$csv_fd) {
	printUsage();
	printStdErr("* The supplied attribute mapping file was not found\n\n");
	fclose($csv_fd);
	exit(-8);
}

$attribute_mapping = Array();

while (($data = fgetcsv($csv_fd, 1024, ',')) !== FALSE) {
	$num_fields = count($data);

	if ($num_fields == 2) {
		$attribute_mapping[trim($data[0])] = trim($data[1]);
	}
}
fclose($csv_fd);

$GLOBALS['SQ_SYSTEM']->setRunLevel(SQ_RUN_LEVEL_FORCED);

$num_assets_imported = createAssets($source_csv_filename, $asset_type_code, $parent_id, $schema_id, $metadata_mapping, $attribute_mapping);

$GLOBALS['SQ_SYSTEM']->restoreRunLevel();

printStdErr("\n- All done\n");
printStdErr('  Assets imported : '.$num_assets_imported."\n");

?>
