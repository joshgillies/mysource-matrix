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
* $Id: import_asset_csv_to_matrix.php,v 1.12 2011/08/08 04:42:31 akarelia Exp $
*
*/

/**
* CSV-to-Matrix Asset Import Script - uber edition
* Source CSV file to Matrix System
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
*	For example, creating a Standard Page and setting content on Bodycopy DIVs is not supported.
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
*		4. Assets can be edited and deleted as denoted by a record flag column
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


define('IMPORT_ADD_RECORD', 1);
define('IMPORT_EDIT_RECORD', 2);
define('IMPORT_DELETE_RECORD', 4);


/**
* Prints out some basic help info detailing how to use this script
*
* @return void
* @access public
*/
function printUsage()
{
	printStdErr("Source CSV file to Matrix importer\n\n");
	printStdErr("Usage: import_asset_csv_to_matrix [system root] [type code] [csv import file] [parent id] [schema id] [meta map file] [attr map file] (new assets live) (unique field name) (add edit delete field name)\n\n");
	printStdErr("REQUIRED ARGUMENTS\n");
	printStdErr("====================================================\n");
	printStdErr("system root       : The Matrix System root directory\n");
	printStdErr("type code         : The Matrix Asset Type Code of the assets to import\n");
	printStdErr("csv import file   : A CSV file containing the records to import as Matrix assets\n");
	printStdErr("parent id         : The asset ID of a Folder etc. under which the assets are to reside\n");
	printStdErr("schema id         : The asset ID of the Metadata Schema to apply to each asset\n");
	printStdErr("meta map file     : A CSV file containing attribute name-to-metadata field ID associations\n");
	printStdErr("attr map file     : A CSV file containing attribute name-to-asset attribute field associations\n");
	printStdErr("\n");
	printStdErr("OPTIONAL ARGUMENTS\n");
	printStdErr("====================================================\n");
	printStdErr("new assets live   : When set to '1' all assets added by the import will be set to 'Live'\n");
	printStdErr("unique field name : The field in the CSV file to be used for (A)dding, (E)diting, or (D)eleting\n");
	printStdErr("                    assets referenced by imported data\n");
	printStdErr("add edit delete\n");
	printStdErr("     field name   : The field in the CSV file used to determine the operation performed on imported data\n");
	printStdErr("ignore csv file   : A single column file which specifies the fields to be ignored when editing an existing asset (eg; when importing User\n");
	printStdErr("                    passwords from an Add operation but don't want to overwrite them during an Edit operation\n");

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
* @param string	$source_csv_filename		The CSV import file containing the asset attribute and metadata field specifications
* @param string	$asset_type_code			The Matrix asset type code or the assets which are to be created
* @param object	$parent_id					The asset ID of the parent asset under which the new assets are to reside
* @param int	$schema_id					The asset ID of the Metadata Schema to associate with the new assets
* @param array	$metadata_mapping			A structure containing Supplied Field Name to Metadata Field asset ID associations
* @param array	$attribute_mapping			A structure containing Supplied Field Name to Asset Attribute Name associations
* @param boolean$new_assets_live			When set to TRUE, assets created during the import will be set to 'Live'
* @param string $unique_record_field		The CSV field to be treated as a primary identifier for editing and deletion purposes
* @param string $record_modification_field	The CSV field to determine whether the associated record is to be (A)dded, (E)dited, or (D)eleted
* @param array	$ignore_fields				The fields for the ignoring
*
* @return array
* @access public
*/
function importAssets($source_csv_filename, $asset_type_code, $parent_id, $schema_id, Array $metadata_mapping, Array $attribute_mapping, $new_assets_live = FALSE, $unique_record_field = '', $record_modification_field = '', Array $ignore_fields = Array())
{
	$GLOBALS['SQ_SYSTEM']->setRunLevel(SQ_RUN_LEVEL_FORCED);

	$num_assets_imported = 0;
	$num_assets_modified = 0;
	$num_assets_deleted  = 0;

	$csv_fd = fopen($source_csv_filename, 'r');
	if (!$csv_fd) {
		printUsage();
		printStdErr("* The supplied CSV import file was not found\n\n");
		fclose($csv_fd);
		exit(-6);
	}

	$parent_asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($parent_id);
	if (!$parent_asset->id) {
		printUsage();
		printStdErr("* The specified parent asset was not found\n\n");
		exit(-7);
	}

	$header_line = TRUE;
	$headers = Array();

	$trash = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('trash_folder');
	$root_folder = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('root_folder');

	// Set to true if temporary trash folder is created, where all the assets to be deleted are moved to
	$temp_trash_folder = FALSE;

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
			// If a Record Modification Field was specified, we also require that a Unique Field was specified for the import
			// These two fields must be present in the CSV file for us to Edit and Delete existing assets. Otherwise, we'll just add
			$record_handling = IMPORT_ADD_RECORD;

			if (!empty($unique_record_field) && !empty($record_modification_field) && isset($asset_spec[$unique_record_field]) && isset($asset_spec[$record_modification_field])) {
				$record_modification_state = strtoupper($asset_spec[$record_modification_field]);
				switch ($record_modification_state) {
					case 'D':	$record_handling = IMPORT_DELETE_RECORD;
					break;

					case 'E':	$record_handling = IMPORT_EDIT_RECORD;
					break;
				}

				// Okey dokey, let's find the existing asset as we are either performing an (E)dit or (D)elete operation...
				// Also try to find an existing asset as we may be unfortunately performing an (A)dd that matches the unique
				// identifier, in which case we are probably intending to (E)dit the existing matching asset
				$existing_asset_id = 0;

				// Our search is limited to the exact asset type used for the import, and the parent root node (and children) specified
				// The unique field may be either one to be assigned to an attribute or to a metadata field
				$search_field = '';
				$search_value = '';

				if (isset($metadata_mapping[$unique_record_field])) {
					$search_type = 'metadata';
					$search_field = $metadata_mapping[$unique_record_field];
					$search_value = $asset_spec[$unique_record_field];
				}

				if (isset($attribute_mapping[$unique_record_field])) {
					$search_type = 'attribute';
					$search_field = $attribute_mapping[$unique_record_field];
					$search_value = $asset_spec[$unique_record_field];
				}

				$search = Array($search_type => Array(
													'field'	=> $search_field,
													'value' => $search_value,
												)
						  );

				$existing_assets = findAsset($parent_id, $asset_type_code, $search);
				if (count($existing_assets) > 1) {
					// Multiple matching assets - skip
					printStdErr("\n*\t* The record for '".$search_value."' matched multiple existing assets. Cannot determine how to proceed - continuing to the next record.\n");
					continue;
				}

				$existing_asset_id = reset($existing_assets);

				// If it is an (E)dit request and the asset was not found, then let's make it an (A)dd instead
				if (empty($existing_assets) && ($record_handling == IMPORT_EDIT_RECORD)) {
					printStdErr("\n*\t* The following 'Edit' request for '".$search_value."' has been changed to 'Add' as there is not an existing matching asset\n");
					$record_handling = IMPORT_ADD_RECORD;
				}

				// If it's there and we wanted to (A)dd, then make it an (E)dit instead
				if (($existing_asset_id > 0) && ($record_handling == IMPORT_ADD_RECORD)) {
					printStdErr("\n*\t* The following 'Add' request for '".$search_value."' has been changed to 'Edit' as this asset already exists.\n");
					$record_handling = IMPORT_EDIT_RECORD;
				}

				// If it is a (D)elete request and the asset was not found, then skip this record gracefully
				if (empty($existing_assets) && ($record_handling == IMPORT_DELETE_RECORD)) {
					printStdErr("\n*\t* Deletion request for asset with unique field value '".$search_value."' was aborted due to a missing matching asset. Continuing to the next record.\n");
					continue;
				}

				if ($record_handling == IMPORT_DELETE_RECORD) {

					// Deletify
					printStdErr('- Deleting asset');
					$asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($existing_asset_id);

					if ($asset) {

						// Create temporary trash folder, if not already created
						if (!$temp_trash_folder) {

							$GLOBALS['SQ_SYSTEM']->am->includeAsset('folder');
							$temp_trash_folder = new Folder();

							$temp_trash_folder->setAttrValue('name','temp_trash_folder');
							$link_array = Array (
								'asset'		=> $root_folder,
								'value'		=> '',
								'link_type'	=> SQ_LINK_TYPE_1,
							);
							$linkid = $temp_trash_folder->create($link_array);

							// If cannot create the temporary trash folder then we cannot delete any asset
							if (!$linkid) {
								printStdErr("\n*\t* Deletion request for asset with unique field value '".$search_value."' was aborted due to unable to create temporary trash folder. Continuing to the next record.\n");

								$GLOBALS['SQ_SYSTEM']->am->forgetAsset($asset);
								continue;
							}
						}

						// Move the asset to the temporary trash folder
						$asset_linkid_old = $GLOBALS['SQ_SYSTEM']->am->getLinkByAsset($parent_id, $asset->id);
						$linkid = $GLOBALS['SQ_SYSTEM']->am->moveLink($asset_linkid_old['linkid'], $temp_trash_folder->id, SQ_LINK_TYPE_1, -1);

						// If cannot move the asset to temporary trash folder then it cannot be deleted
						if (!$linkid) {
								printStdErr("\n*\t* Deletion request for asset with unique field value '".$search_value."' was aborted due to unable to move this asset to temporary trash folder. Continuing to the next record.\n");

								$GLOBALS['SQ_SYSTEM']->am->forgetAsset($asset);
								continue;
						}

						echo $search_value.','.$existing_asset_id.",D\n";
						$num_assets_deleted++;

					} // End if asset
				} else if ($record_handling == IMPORT_EDIT_RECORD) {
					// Editise

					// Ok we are editing - has the user specified fields to ignore at this point? If so, let's eliminificate
					foreach ($ignore_fields as $ignore_field_name => $val) {
						if (isset($asset_spec[$ignore_field_name])) {
							unset($asset_spec[$ignore_field_name]);
						}
					}

					printStdErr('- Modifying asset with unique field value');
					editAsset($existing_asset_id, $asset_spec, $attribute_mapping, $metadata_mapping, $schema_id);
					echo $search_value.','.$existing_asset_id.",E\n";
					$num_assets_modified++;
				}
			}

			if ($record_handling == IMPORT_ADD_RECORD) {
				$asset_info = createAsset($asset_spec, $asset_type_code, $parent_asset, $schema_id, $metadata_mapping, $attribute_mapping);
				$asset_id = 0;
				if (is_array($asset_info)) {
					$asset_id = reset($asset_info);
				}

				if ($asset_id) {
					// Ok see if we need to set it live
					if ($new_assets_live) {
						$new_asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($asset_id);
						$new_asset->processStatusChange(SQ_STATUS_LIVE);
						$GLOBALS['SQ_SYSTEM']->am->forgetAsset($new_asset);
					}

					echo key($asset_info).','.$asset_id.",A\n";
					$num_assets_imported++;
				}
			}


		}
	}// End while

	fclose($csv_fd);

	$GLOBALS['SQ_SYSTEM']->restoreRunLevel();

	// Now actually delete all the assets moved to "temporary purge folder" by purging this folder
	if ($temp_trash_folder && $GLOBALS['SQ_SYSTEM']->am->trashAsset($temp_trash_folder->id)) {

		$trash_folder = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('trash_folder');
		$trash_linkid = $GLOBALS['SQ_SYSTEM']->am->getLinkByAsset($trash_folder->id, $temp_trash_folder->id);

		if (isset($trash_linkid['linkid']) && $trash_linkid['linkid'] > 0) {

			$hh = $GLOBALS['SQ_SYSTEM']->getHipoHerder();

			$vars = Array(
				'purge_root_linkid' => $trash_linkid['linkid'],
			);

			$errors = $hh->freestyleHipo('hipo_job_purge_trash', $vars);

			if (!empty($errors)) {
				$error_msg = '';
				foreach($errors as $error) {
					$error_msg .= ' * '.$error['message'];
				}
				echo "Following errors occured while deleting asset(s):\n$error_msg\n";
			}
		}

	}

	$status_report = Array(
						'num_added'		=> $num_assets_imported,
						'num_modified'	=> $num_assets_modified,
						'num_deleted'	=> $num_assets_deleted,
					 );

	return $status_report;

}//end importAssets()


/**
* Edits the specified asset with the specified attribute and metadata values
*
* @return void
* @access public
*/
function editAsset($asset_id, Array $asset_spec, Array $attribute_mapping, Array $metadata_mapping, $schema_id)
{
	$asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($asset_id);

	// Set attributes
	editAttributes($asset, $asset_spec, $attribute_mapping);
	printStdErr('.');

	// Assign metadata schema and values to the asset
	editMetadata($asset, $asset_spec, $metadata_mapping, $schema_id);
	printStdErr('.');

	// Free memory
	$GLOBALS['SQ_SYSTEM']->am->forgetAsset($asset);

	printStdErr(' => asset ID '.$asset_id."\n");

}//end editAsset()


/**
* Finds an existing asset matching the exact type with the metadata or attribute value supplied
*
* @return void
* @access public
*/
function findAsset($root_asset_id, $asset_type_code, Array $search)
{
	// Begin uberquery!
	$db = MatrixDAL::getDb();

	$search_type_attribute = isset($search['attribute']);
	$field_name = '';
	$field_value = '';

	if ($search_type_attribute) {
		$field_name = $search['attribute']['field'];
		$field_value = $search['attribute']['value'];
	} else {
		$field_name = $search['metadata']['field'];
		$field_value = $search['metadata']['value'];
	}

	$tree_id = '';

	// Grab a single tree ID so we can search our entire root asset
	$sql = 'SELECT t.treeid FROM sq_ast_lnk_tree t, sq_ast_lnk l WHERE l.linkid = t.linkid AND l.minorid = :root_asset_id LIMIT 1';
	try {
		$query = MatrixDAL::preparePdoQuery($sql);
		MatrixDAL::bindValueToPdo($query, 'root_asset_id', $root_asset_id);

		$tree_id = MatrixDAL::executePdoOne($query);
	} catch (Exception $e) {
		throw new Exception('Unable to search for an existing '.$asset_type_code.' asset: '.$e->getMessage());
	}

	if ($tree_id == '') return Array();

	// Query portion for restricting by attribute
	$attribute_sql_from = 'sq_ast_attr r, sq_ast_attr_val v ';

	// Query portion for restricting by metadata field value
	$metadata_sql_from = 'sq_ast_mdata_val m ';

	$sql = 'SELECT a.assetid, a.name '.
			'FROM sq_ast a, sq_ast_lnk l, sq_ast_lnk_tree t, '.(($search_type_attribute) ? $attribute_sql_from : $metadata_sql_from).
			'WHERE t.treeid LIKE :tree_id '.
			'AND l.linkid = t.linkid AND a.assetid = l.minorid ';

	if (!empty($asset_type_code)) {
		$sql .= 'AND a.type_code = :type_code ';
	}

	if ($search_type_attribute) {
		$sql .= ' AND v.assetid = a.assetid AND r.name = :field_name AND v.attrid = r.attrid AND v.custom_val = :field_val';
	} else {
		$sql .= ' AND m.assetid = a.assetid AND m.fieldid = :field_name AND m.value = :field_val';
	}

	try {
		$query = MatrixDAL::preparePdoQuery($sql);
		MatrixDAL::bindValueToPdo($query, 'tree_id', $tree_id.'%');
		MatrixDAL::bindValueToPdo($query, 'field_name', $field_name);
		MatrixDAL::bindValueToPdo($query, 'field_val', $field_value);
		if (!empty($asset_type_code)) {
			MatrixDAL::bindValueToPdo($query, 'type_code', $asset_type_code);
		}
		$matching_assets = MatrixDAL::executePdoAssoc($query, 0);
	} catch (Exception $e) {
		throw new Exception('Unable to search for an existing '.$asset_type_code.' asset: '.$e->getMessage());
	}

	return $matching_assets;

}//end findAsset()


/**
* Creates a simple "TYPE 1" link between assets
*
* @param object	&$parent_asset	The parent asset
* @param object	&$child_asset	The child asset
*
* @return int
* @access public
*/
function createLink(Asset &$parent_asset, Asset &$child_asset)
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
function createAsset(Array $asset_spec, $asset_type_code, Asset &$parent_asset, $schema_id, Array $metadata_mapping, Array $attribute_mapping)
{
	$attribs = Array();

	printStdErr('- Creating asset');

	$asset = new $asset_type_code();
	printStdErr('.');

	// Set attributes
	editAttributes($asset, $asset_spec, $attribute_mapping);
	printStdErr('.');

	// Link the new asset under the parent folder
	$link_id = createLink($parent_asset, $asset);
	printStdErr('.');

	// Assign metadata schema and values to the asset
	editMetadata($asset, $asset_spec, $metadata_mapping, $schema_id);
	printStdErr('.');

	// Free memory
	$GLOBALS['SQ_SYSTEM']->am->forgetAsset($asset);

	printStdErr(' => asset ID '.$asset->id."\n");

	return Array(reset($asset_spec) => $asset->id);

}//end createAsset()


/**
* Edits attributes of an existing or about-to-be-created asset
*
*/
function editAttributes(Asset &$asset, Array $asset_spec, Array $attribute_mapping)
{
	$first_attr_name = '';
	$attrs_modified = FALSE;

	foreach ($attribute_mapping as $supplied_name => $attribute_name) {
		if ($first_attr_name == '') {
			$first_attr_name = $supplied_name;
			printStdErr(' '.$asset_spec[$first_attr_name]);
		}

		// Only set attributes when they are not set to that value already
		if (isset($asset_spec[$supplied_name]) && ($asset->attr($attribute_name) != $asset_spec[$supplied_name])) {
			$asset->setAttrValue($attribute_name, $asset_spec[$supplied_name]);
			$attrs_modified = TRUE;
		}
	}

	// Save attribute values and run updated (default behaviour)
	// As fixed in Bug 4141
	if ($attrs_modified) {
		$asset->saveAttributes();
	}

}//end editAttributes()


/**
* Edits metadata of an existing or about-to-be-created asset
*
*/
function editMetadata(Asset &$asset, Array $asset_spec, Array $metadata_mapping, $schema_id)
{
	$mm = $GLOBALS['SQ_SYSTEM']->getMetadataManager();
	$mm->setSchema($asset->id, $schema_id, TRUE);

	foreach ($metadata_mapping as $supplied_field_name => $metadata_field_id) {
		if (isset($asset_spec[$supplied_field_name])) {
			$metadata = Array($metadata_field_id => Array (
															0 => Array(
																	'value' => $asset_spec[$supplied_field_name],
																	'name' => $supplied_field_name,
																)
													)
							);

			$mm->setMetadata($asset->id, $metadata);
		}
	}

	// Be nice and regen the metadata for the lovely people out there
	$mm->regenerateMetadata($asset->id);

}//end editMetadata()


/************************** MAIN PROGRAM ****************************/

if (ini_get('memory_limit') != '-1') ini_set('memory_limit', '-1');

if ((php_sapi_name() != 'cli')) {
trigger_error("You can only run this script from the command line\n", E_USER_ERROR);
}

// Matrix system root directory
$argv = $_SERVER['argv'];
$GLOBALS['SYSTEM_ROOT'] = (isset($argv[1])) ? $argv[1] : '';
if (empty($GLOBALS['SYSTEM_ROOT'])) {
	printUsage();
	printStdErr("* The Matrix system root directory must be specified as the first argument\n\n");
	exit(-99);
}

require_once $GLOBALS['SYSTEM_ROOT'].'/core/include/init.inc';

// Has a Matrix Type Code been supplied?
$asset_type_code = $argv[2];
if (empty($asset_type_code)) {
	printUsage();
	printStdErr("* A Matrix Type Code must be specified as the second argument\n\n");
	exit(-1);
} else {
	// Check Matrix Type Code
	$GLOBALS['SQ_SYSTEM']->am->includeAsset($asset_type_code);
}

// Has an XML filename been supplied?
$source_csv_filename   = $argv[3];
if (empty($source_csv_filename)) {
	printUsage();
	printStdErr("* A source CSV filename must be specified as the third argument\n\n");
	exit(-2);
}

// Has a parent ID been supplied?
$parent_id  = (int)$argv[4];
if ($parent_id == 0) {
	printUsage();
	printStdErr("* A Parent asset ID must be specified as the fourth argument\n\n");
	exit(-3);
}

// Has a schema ID been supplied?
$schema_id  = (int)$argv[5];
if ($schema_id == 0) {
	printUsage();
	printStdErr("* A Metadata Schema asset ID must be specified as the fifth argument\n\n");
	exit(-4);
}

// Has a CSV mapping file been supplied?
$mapping_filename  = $argv[6];
if (empty($mapping_filename)) {
	printUsage();
	printStdErr("* A Metadata Field CSV mapping file must be specified as the sixth argument\n\n");
	exit(-5);
}

// Has a CSV attribute mapping file been supplied?
$attribute_mapping_filename  = $argv[7];
if (empty($mapping_filename)) {
	printUsage();
	printStdErr("* An attribute CSV mapping file must be specified as the seventh argument\n\n");
	exit(-6);
}

// Matrix status code for new assets
$new_assets_live = (isset($argv[8]) && ($argv[8] == 1));

// Has a Unique Record field been specified? (This is optional)
$unique_record_field = (isset($argv[9])) ? $argv[9] : '';

// Has a Record add / edit / delete column been specified? (This is optional)
$record_modification_field = (isset($argv[10])) ? $argv[10] : '';

// A file which determines the fields to ignore upon (E)dit (optional)
$ignore_csv_filename = (isset($argv[11])) ? $argv[11] : '';

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

$ignore_fields = Array();

if ($ignore_csv_filename != '') {
	$csv_fd = fopen($ignore_csv_filename, 'r');
	if (!$csv_fd) {
		printUsage();
		printStdErr("* The supplied ignore fields file was not found\n\n");
		fclose($csv_fd);
		exit(-42);
	}

	while (($data = fgetcsv($csv_fd, 1024, ',')) !== FALSE) {
		$num_fields = count($data);

		if ($num_fields == 1) {
			$ignore_fields[trim($data[0])] = 1;
		}
	}
	fclose($csv_fd);
}

$root_user = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('root_user');
$GLOBALS['SQ_SYSTEM']->setCurrentUser($root_user);

$status_report = importAssets($source_csv_filename, $asset_type_code, $parent_id, $schema_id, $metadata_mapping, $attribute_mapping, $new_assets_live, $unique_record_field, $record_modification_field, $ignore_fields);

$GLOBALS['SQ_SYSTEM']->restoreCurrentUser();

printStdErr("\n- All done\n");
printStdErr("\tAssets added    : ".$status_report['num_added']."\n");
printStdErr("\tAssets modified : ".$status_report['num_modified']."\n");
printStdErr("\tAssets deleted  : ".$status_report['num_deleted']."\n");

?>
