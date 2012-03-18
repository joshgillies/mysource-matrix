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
* $Id: csv_to_xml_actions.php,v 1.10.20.1 2012/03/18 22:41:31 ewang Exp $
*
*/

/**
* CSV and metadata mapping file to XML asset conversion script
* Command-line only
*
* @author  Mark Brydon <mbrydon@squiz.net>
* 30 Nov 2006
*
*
* Purpose:
* 		Conversion of a CSV file in conjunction with a metadata mapping file to XML
*		for importing into MySource Matrix using the import_from_xml.php script
*
* Requirements:
*		The XML conversion routines in the Matrix "fudge" package (file: [MATRIX_ROOT]/fudge/general/xml_converter.inc)
*
* Documentation:
*		Examples of CSV and XML input files and the XML output structure is provided below.
*		The XML output is written to "standard output", so redirection will need to be used to capture the output.
*
*		File structures
*		===============
*
* 		eg; CSV file input (first column must be "name", first line is header only):
*				name, address, phone, other
*				Page Name 1, Address 1, Phone 1, Favourite colour 1
*				Page Name 2, Address 2, Phone 2, Favourite colour 2
*				Page Name 3, Address 3, Phone 3, Favourite colour 3
*
*			Mapping file input (XML):
*				 <?xml version="1.0" encoding="iso-8859-1" ?>
*				 <!-- template only, usage given below -->
*				 <schema id="[metadata schema asset ID]" name_field="[alias of name field]" (group_by_field_id="[group by field asset ID]" | group_by_asset_field="[place assets under the asset ID specified in the named column]")>
*				   <field id="[field 1 asset ID]" alias="[corresponding column name from CSV]" (ignore="1") />
*				   <field id="[field n asset ID]" alias="[corresponding column name from CSV]" />
*				 </schema>
*
*				 <!-- possible usage, direct ungrouped asset import -->
*				 <schema id="1234" name_field="first_name">
*				   <field id="1236" alias="address" />
*				   <field id="1237" alias="phone" />
*				   <field id="1238" alias="colour" />
*				 </schema>
*
*				 <!-- OR automatic folder creation and grouping by a specified field... -->
*				 <schema id="1234" group_by_field="1236" name_field="first_name">
*				   <field id="1235" alias="first_name" />
*				   <field id="1236" alias="address" />
*				   <field id="1237" alias="phone" />
*				   <field id="1238" alias="colour" ignore="1" />
*				 </schema>
*
*				 <!-- OR to place assets under an associated parent asset with the asset ID specified in the named column... -->
*				 <schema id="1234" group_by_asset_field="parent_asset_id" name_field="first_name">
*				   <field id="1235" alias="first_name" />
*				   <field id="1236" alias="address" />
*				   <field id="1237" alias="phone" />
*				   <field id="1238" alias="colour" ignore="1" />
*				   <field id="1239" alias="parent_asset_id" ignore="1" />
*				 </schema>
*
*
*    	  	Output from this script (structure only - see xml_example.xml for a full example):
*				<?xml version="1.0" encoding="iso-8859-1" ?>
*				<actions>
*					<action>
*						<!-- asset actions -->
*					</action>
*					<!-- subsequent actions here -->
*				</actions>
*
*
*		Field tag attributes:
*
*			ignore:		Fields set with the "ignore" flag will not be included in the export. However, such fields can still be used for
*						grouping in the <schema> tag
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
	printStdErr("CSV and metadata mapping to XML converter\n");
	printStdErr('Usage: csv_to_xml_actions [system root] [csv file] [mapping file] [parent id] [asset type] (-unique) (-ignore_blank)');
	printStdErr('system root  : The Matrix System root directory');
	printStdErr('csv file     : A comma separated values file that represents the site structure');
	printStdErr('mapping file : An XML file containing column name to metadata mapping data');
	printStdErr('parent id    : The parent asset ID');
	printStdErr('asset type   : Asset type to create for each entry (eg; data_record)');
	printStdErr('-unique      : The "-unique" option will instruct the exporter to exclude duplicate records');
	printStdErr('-ignore_blank: The "-ignore_blank" option will not attempt to import fields which are blank*');
	printStdErr("               (* = empty fields or those composed entirely of spaces or dashes)\n");

}//end printUsage()


/**
* Prints the XML required to create an asset
*
* @param string	$action_id		The action ID
* @param string	$action_type	The action type (eg; create_asset)
* @param string	$type_code		The asset type (eg; page_standard)
* @param int	$link_type		The type of linking for the asset (eg; "1" = Type 1 linking)
* @param int	$parent_id		The asset ID of the parent of the new asset
*
* @return void
* @access public
*/
function printCreateAssetAction($action_id, $action_type, $type_code, $link_type, $parent_id)
{
	echo "<action>\n";
	echo '  <action_id>'.$action_id."</action_id>\n";
	echo '  <action_type>'.$action_type."</action_type>\n";
	echo '  <type_code>'.$type_code."</type_code>\n";
	echo '  <link_type>'.$link_type."</link_type>\n";
	echo '  <parentid>'.$parent_id."</parentid>\n";
	echo "</action>\n";

}//end printCreateAssetAction()


/**
* Prints the XML required to perform trigger-based magic on an asset (eg; set_permissions, set_metadata_schema)
*
* @param string	$action_id		The action ID
* @param string	$action_type	The action type (eg; create_asset)
* @param string	$asset			An action ID of a previous asset to be used as the context for this trigger action
* @param array	$settings		An associative array (key/value pairs) containing data specific to this trigger action
*
* @return void
* @access public
*/
function printCreateTriggerAction($action_id, $action_type, $asset, $settings=Array())
{
	echo "<action>\n";
	echo '  <action_id>'.$action_id."</action_id>\n";
	echo '  <action_type>'.$action_type."</action_type>\n";
	echo '  <asset>[['.$asset."]]</asset>\n";

	foreach ($settings as $setting => $value) {
		echo '  <'.$setting.'>'.$value.'</'.$setting.">\n";
	}

	echo "</action>\n";

}//end printCreateTriggerAction()


/**
* Reduces multiple contiguous spaces in the string to a single space
*
* @param string	$string	The string to reduce
*
* @return void
* @access public
*/
function compactSpaces($string)
{
	return compactCharacters($string, ' ');

}//end compactSpaces()


/**
* Reduces multiple contiguous characters in the string to a single character
*
* @param string	$string	The string to reduce
* @param string	$char	The character to be reduced in the string
*
* @return void
* @access public
*/
function compactCharacters($string, $char)
{
	$orig_string = '';
	while ($orig_string != $string) {
		$orig_string = $string;
		$string = str_replace($char.$char, $char, $string);
	}

	return $string;

}//end compactCharacters()


/**
* Parses the supplied metadata mapping file (XML) and returns the metadata schema ID and field-to-column mappings
*
* @param string	$mapping_filename	The name of the XML metadata mapping file
*
* @return void
* @access public
*/
function getMetadataMapping($mapping_filename)
{
	include_once $GLOBALS['SYSTEM_ROOT'].'/fudge/general/xml_converter.inc';

	$metadata_mapping = Array(
							'metadata_schema_id'		=> 0,
							'metadata_fields'			=> Array(),
							'metadata_ignore_fields'	=> Array(),
							'metadata_required_fields'	=> Array(),
							'metadata_field_types'		=> Array(),
						);

	// Read metadata mapping info
	$xml = new XML_Converter();
	$xml_array = $xml->getArrayFromFile($mapping_filename);

	// No, not schemii
	$num_schemata = count($xml_array);

	// There should only be one schema defined in the mapping file
	if ($num_schemata != 1) {
		printStdErr("* A single metadata schema must be defined in the mapping file <schema id=\"[id]\" group_by_field=\"[field_id]\">...</schema>\n");
		exit(-11);
	}

	$schema = $xml_array['schema'];
	if ($num_schemata != 1) {
		printStdErr("* The mapping file must contain one root element named 'schema'\n");
		exit(-12);
	}

	$metadata_mapping['metadata_schema_id'] = (int)(trim($schema[0]['@id']));
	if ($metadata_mapping['metadata_schema_id'] == 0) {
		printStdErr("* A metadata schema asset id must be specified in the 'id' attribute of the <schema> tag\n");
		exit(-13);
	}

	$metadata_mapping['metadata_schema_group_field'] = 0;
	if (isset($schema[0]['@group_by_field_id'])) {
		$metadata_mapping['metadata_schema_group_field'] = (int)(trim($schema[0]['@group_by_field_id']));
	}

	$metadata_mapping['metadata_schema_asset_field'] = '';
	if (isset($schema[0]['@group_by_asset_field'])) {
		$metadata_mapping['metadata_schema_asset_field'] = trim($schema[0]['@group_by_asset_field']);
	}

	$metadata_mapping['metadata_schema_name_field'] = 0;
	if (isset($schema[0]['@name_field'])) {
		$metadata_mapping['metadata_schema_name_field'] = trim($schema[0]['@name_field']);
	}

	// There should be one level here, with all lines in the form <field id="[id]" alias="[alias]" ... />
	$num_levels = count($schema);
	if ($num_levels != 1) {
		printStdErr("* A single level of field definitions should be defined in the <schema> section in the form <field id=\"[id]\" alias=\"[alias]\" />...\n");
		exit(-14);
	}

	// Reduce the schema to one level only
	$schema = $schema[0];

	// Now hopefully the next (and only) lines are single tags called "field"
	$fields = $schema['field'];

	$num_fields = count($fields);

	// There is not much point continuing without the fields
	if ($num_fields == 0) {
		printStdErr("* Metadata fields must be defined in the <schema> section in the form <field id=\"[id]\" alias=\"[alias]\" />...\n");
		exit(-15);
	}

	$group_by_field_specified = ($metadata_mapping['metadata_schema_group_field'] > 0);
	$group_by_field_found = FALSE;

	$group_by_asset_specified = ($metadata_mapping['metadata_schema_asset_field'] != '');
	if ($group_by_field_specified && $group_by_asset_specified) {
		printStdErr("* Group by field and group by parent asset must be used separately\n");
		exit(-19);
	}

	$name_field_specified = ($metadata_mapping['metadata_schema_name_field'] != '');
	$name_field_found = FALSE;

	// Assign the mappings for each field
	foreach ($fields as $field) {
		$field_id = (int)trim($field['@id']);
		$field_alias = trim($field['@alias']);

		// All fields apart from the 'name' field which are present in the CSV file *must* be defined in the XML mapping, however fields
		// can be "ignored" and therefore will not be used in the import. Ignored fields can still be used for sorting which is nice
		$ignore_field = ((isset($field['@ignore'])) ? ((int)trim($field['@ignore']) == 1) : 0);

		if ($field_id == 0) {
			printStdErr("* A field id must be specified in the 'id' attribute of the <field /> tag\n");
			exit(-16);
		}

		// Check to see if this field is the specified group by field
		if ($group_by_field_specified) {
			if ($field_id == $metadata_mapping['metadata_schema_group_field']) {
				$group_by_field_found = TRUE;
			}
		}

		$metadata_mapping['metadata_fields'][$field_alias] = $field_id;

		if ($ignore_field) {
			$metadata_mapping['metadata_ignore_fields'][$field_alias] = 1;
		}

	}//end foreach

	// Perform a check to verify that the specified group by field is defined in the mapping file
	if ($group_by_field_specified && !$group_by_field_found) {
		printStdErr("* The specified group_by_field ID was not defined as a <field> in the mapping file\n");
		exit(-17);
	}

	// Perform a check to verify that the specified group by field is defined in the mapping file
	if (!$name_field_specified) {
		printStdErr("* A name_field must be specified in the <schema> tag\n");
		exit(-18);
	}

	return $metadata_mapping;

}//end getMetadataMapping()


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
	fwrite(STDERR, "$string\n");

}//end printStdErr()


/**
* Ensures that the XML mapping contains a correct and complete definition of the target metadata schema
*
* @param array	&$metadata_mapping	The metadata mapping structure which is used to import the CSV data
*
* @return void
* @access public
*/
function validateMetadataMapping(&$metadata_mapping)
{
	// Start Matrix
	printStdErr('- Initialising Matrix...');

	// Check the schema is available in the system
	$schema_asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($metadata_mapping['metadata_schema_id']);
	if (!$schema_asset) {
		printStdErr('* The supplied schema (ID: '.$metadata_mapping['metadata_schema_id'].") could not be found in the system\n");
		exit(-21);
	}
	if ($schema_asset->type() != 'metadata_schema') {
		printStdErr('* The supplied schema (ID: '.$metadata_mapping['metadata_schema_id'].") is not a valid Metadata Schema in the system\n");
		exit(-22);
	}

	// Now we have the schema, find its fields and make sure that they were specified
	$mm = $GLOBALS['SQ_SYSTEM']->getMetadataManager();
	$metadata_fields = $mm->getMetadataFields(Array($metadata_mapping['metadata_schema_id']));

	// Fields specified in the system...
	$metadata_system_field_ids = array_keys($metadata_fields);
	$metadata_mapping_field_ids = array_values($metadata_mapping['metadata_fields']);

	$metadata_differences = array_diff($metadata_system_field_ids, $metadata_mapping_field_ids);

	if (count($metadata_differences) > 0) {
		printStdErr('* One or more system metadata fields were not specified in the mapping file');
		printStdErr('  The following metadata field IDs are required:');
		foreach ($metadata_differences as $metadata_field_id) {
			printStdErr('  '.$metadata_field_id);
		}
		exit(-23);
	}

	// All expected metadata fields were defined in the mapping file. Add the data types to our mapping.
	// This will be used while importing to verify that the fields are correct
	$metadata_mapping['metadata_field_types'] = $metadata_fields;

	$am = $GLOBALS['SQ_SYSTEM']->am;

	foreach ($metadata_fields as $metadata_field_id => $metadata_field_type) {
		$metadata_field_type = $metadata_field_type[0]['type_code'];
		$field = $am->getAsset($metadata_field_id, $metadata_field_type);
		$metadata_mapping['metadata_field_objects'][$metadata_field_id] = $field;

		if ($field->attr('required')) {
			if (isset($metadata_mapping['metadata_ignored_fields'][$metadata_field_id])) {
				printStdErr('* The metadata field (ID '.$metadata_field_id.') cannot be ignored upon import as it is mandatory');
				exit(-24);
			}

			$metadata_mapping['metadata_required_fields'][$metadata_field_id] = 1;
		}
	}

}//end validateMetadataMapping()


/**
* Ensures that the XML mapping contains a correct and complete definition of the target metadata schema
*
* @param object	&$metadata_field_object	A Matrix metadata field object to check for consistency
* @param string	$value					The value destined for the metadata field
*
* @return void
* @access public
*/
function validateMetadataField(&$metadata_field_object, $value)
{
	$valid_value = FALSE;

	$metadata_field_type = get_class($metadata_field_object);
	if ($metadata_field_type == 'Metadata_Field_Text') {
		$valid_value = TRUE;
	} else if ($metadata_field_type == 'Metadata_Field_Select') {
		// Verify that the intended value is available as a selection for this field
		$selection = $metadata_field_object->getSelectionAttribute();
		$valid_value = $selection->validateValue($value);
	} else if ($metadata_field_type == 'Metadata_Field_Thesaurus') {
		// Ensure that the supplied value is an asset ID of an existing Thesaurus asset
		$asset_id = (int)($value);
		if (($asset_id == $value) && ($asset_id > 0)) {
			$thesaurus = $GLOBALS['SQ_SYSTEM']->am->getAsset($asset_id);
			$asset_id = $thesaurus->id;
		}
		$valid_value = ($asset_id > 0);
	} else if ($metadata_field_type == 'Metadata_Field_Date') {
		include_once $GLOBALS['SYSTEM_ROOT'].'/fudge/general/datetime.inc';
		$valid_value = is_iso8601($value);
	}

	return $valid_value;

}//end validateMetadataField()


/************************** MAIN PROGRAM ****************************/

if ((php_sapi_name() != 'cli')) {
	trigger_error("You can only run this script from the command line\n", E_USER_ERROR);
}

// Matrix system root directory
$argv = $_SERVER['argv'];
$GLOBALS['SYSTEM_ROOT'] = (isset($argv[1])) ? $argv[1] : '';
if (empty($GLOBALS['SYSTEM_ROOT'])) {
	printUsage();
	printStdErr("* The Matrix system root directory must be specified as the first parameter\n");
	exit(-99);
}

require_once $GLOBALS['SYSTEM_ROOT'].'/core/include/init.inc';

// Has a CSV filename been supplied?
$csv_filename	= $argv[2];
if (empty($csv_filename)) {
	printUsage();
	printStdErr("* A CSV filename must be specified as the second parameter\n");
	exit(-1);
}

// Has a mapping filename been supplied?
$mapping_filename	= $argv[3];
if (empty($mapping_filename)) {
	printUsage();
	printStdErr("* A mapping filename must be specified as the third parameter\n");
	exit(-2);
}

// Has a parent ID been supplied?
$global_parent_id	= $argv[4];
if (empty($global_parent_id)) {
	printUsage();
	printStdErr("* A parent ID must be specified as the fourth parameter\n");
	exit(-3);
}

// Has an asset type been supplied?
$global_asset_type	= $argv[5];
if (empty($global_asset_type)) {
	printUsage();
	printStdErr("* An asset type must be specified as the fifth parameter\n");
	exit(-4);
}

$export_unique_records_only = FALSE;
$ignore_blank_fields = FALSE;

if (isset($argv[6])) {
	$export_unique_records_only = (strtolower($argv[6]) == '-unique');
	$ignore_blank_fields = (strtolower($argv[6]) == '-ignore_blank');
}

if (isset($argv[7])) {
	$export_unique_records_only = ($export_unique_records_only || (strtolower($argv[7]) == '-unique'));
	$ignore_blank_fields = ($ignore_blank_fields || (strtolower($argv[7]) == '-ignore_blank'));
}

// Do the supplied files exist?
$csv_fd = fopen($csv_filename, 'r');
if (!$csv_fd) {
	printUsage();
	printStdErr("* The supplied CSV file was not found\n");
	exit(-5);
}

$mapping_fd = fopen($mapping_filename, 'r');
if (!$mapping_fd) {
	printUsage();
	printStdErr("* The supplied mapping file was not found\n");
	exit(-6);
}
fclose($mapping_fd);

// Yippee, we have the appropriate files. Let's process them now
$headers = Array();
$metadata_mapping = getMetadataMapping($mapping_filename);

$group_by_field = $metadata_mapping['metadata_schema_group_field'];
$name_field = $metadata_mapping['metadata_schema_name_field'];

$is_header_line = TRUE;

// Store the action IDs so we don't clash with namespace
$action_ids = Array();

validateMetadataMapping($metadata_mapping);

printStdErr("- Exporting XML...\n");

echo "<?xml version=\"1.0\" encoding=\"iso-8859-1\" ?>\n";
echo "<actions>\n";

$group_names = Array();

// Find the name of the group by field if this was specified
$group_by_field_name = '';
$group_by_field_column = -1;
$group_by_asset_field_column = -1;
$name_field_column = -1;

if ($group_by_field > 0) {
	foreach ($metadata_mapping['metadata_fields'] as $field_name => $field_id) {
		if ($field_id == $group_by_field) {
			$group_by_field_name = $field_name;
			break;
		}
	}
}

$num_folders_created = 0;
$num_assets_created = 0;
$num_fields_ignored = 0;
$num_fields_blank = 0;
$num_records_ignored = 0;

$imported_records = Array();

while (($data = fgetcsv($csv_fd, 1024, ',')) !== FALSE) {
	$num_fields = count($data);

	if ($num_fields >= 1) {
		if ($is_header_line) {
			$headers = $data;

			// Find the column ID of the group by field if this was specified
			if ($group_by_field_name != '') {
				foreach ($headers as $key => $field_name) {
					if ($field_name == $group_by_field_name) {
						$group_by_field_column = $key;
						break;
					}
				}
			}

			// Find the column ID of the group by asset field if this was specified
			if (isset($metadata_mapping['metadata_schema_asset_field'])) {
				if ($metadata_mapping['metadata_schema_asset_field'] != '') {
					foreach ($headers as $key => $field_name) {
						if ($field_name == $metadata_mapping['metadata_schema_asset_field']) {
							$group_by_asset_field_column = $key;
							break;
						}
					}
				}
			}

			// Find the 'name' field - the column to be used to name the asset
			foreach ($headers as $key => $field_name) {
				if ($field_name == $name_field) {
					$name_field_column = $key;

					// By definition, the 'name' field will be ignored (ie; not imported as metadata), unless it is specified as a <field>.
					// If this field is required in metadata, the schema should be configured to use %asset_name%
					// It is assumed that the 'name' field is populated with data for each record, although this does not need to be unique
					if (!isset($metadata_mapping['metadata_fields'][$name_field])) {
						$metadata_mapping['metadata_ignore_fields'][$name_field] = 1;
					}

					break;
				}
			}

			// The name field must be defined in the mapping file
			if ($name_field_column < 0) {
				printStdErr('* The specified name field "'.$name_field."\" was not found in the CSV file\n");
				exit(-7);
			}

			$is_header_line = FALSE;
		} else {

			// Ensure that we have all required fields before attempting to create this asset
			if (count($metadata_mapping['metadata_required_fields'])) {
				$data_available = TRUE;

				foreach ($metadata_mapping['metadata_required_fields'] as $required_field_id => $val) {
					// Check that the data is populated for each required column
					for ($n=0; $n<count($headers); $n++) {
						$column_name = $headers[$n];
						if (!isset($metadata_mapping['metadata_ignore_fields'][$column_name])) {
							$metadata_field_id = $metadata_mapping['metadata_fields'][$column_name];

							if ($metadata_field_id == $required_field_id) {
								$value = trim(compactSpaces($data[$n]));

								if ($value == '') {
									$data_available = FALSE;
									break;
								}
							}
						}

					}
					if (!$data_available) break;

				}

				// Skip ahead to the next record in the CSV file if the required fields are not populated
				if (!$data_available) {
					$num_records_ignored++;
					continue;
				}
			}

			// Remove extra spaces from the record name. Name the alias using the 'name' field
			$name = trim($data[$name_field_column]);
			$name = compactSpaces($name);

			// Tidy up the action ID
			$action_id = strtolower($name);
			$action_id = ereg_replace(' ', '_', $action_id);
			$action_id = ereg_replace(',', '', $action_id);

			// Aargh - a namespace clash! Better assign a new action ID for triggering purposes. This does not affect the asset name
			$n = 0;
			if (isset($action_ids[$action_id])) {
				$n = 1;
				while (isset($action_ids[$action_id.'_'.$n])) {
					$n++;
				}
			}

			// All better now, let's bags this action ID
			if ($n > 0) $action_id = $action_id.'_'.$n;
			$action_ids[$action_id] = 1;

			// Create a folder to house the assets if a group by field was specified and one does not already exist from this import
			$group_by_folder_name = '';
			$folder_name_orig = '';
			$folder_name = '';
			if ($group_by_field_column >= 0) {
				$value = trim($data[$group_by_field_column]);
				$value = compactSpaces($value);

				$folder_name_orig = $value;
				$folder_name = ereg_replace(' ', '_', $value);
				$folder_name = ereg_replace(',', ' ', $folder_name);

				$group_by_folder_name = 'create_folder_'.$folder_name;

				// Create a folder if one has not been created during this import with the specified name
				if (!isset($group_names[$value])) {
					$group_names[$value] = 1;

					// Create folder asset
					printStdErr('- Creating folder '.$value);
					printCreateAssetAction('create_folder_'.$folder_name, 'create_asset', 'folder', '1', $global_parent_id);

					$num_folders_created++;

					// Then name the folder...
					$settings = Array(
									'attribute'	=> 'name',
									'value'		=> $value,
								);
					printCreateTriggerAction('set_folder_'.$folder_name.'_name', 'set_attribute_value', 'output://create_folder_'.$folder_name.'.assetid', $settings);
				}

				// ...and give it a web path
				$settings = Array(
								'path'	=> $folder_name,
							);
				printCreateTriggerAction('set_folder_'.$folder_name.'_path', 'add_web_path', 'output://create_folder_'.$folder_name.'.assetid', $settings);
			}//end if

			// Now create an asset of the specified type (under the relevant group by folder, if specified)
			$asset_parent_id = $global_parent_id;
			if ($folder_name != '') {
				$asset_parent_id = '[[output://create_folder_'.$folder_name.'.assetid]]';
			}

			// Hold on to our metadata for exporting later. This is done so we can exclude duplicate records if they are encountered
			$record_metadata = Array();
			for ($n=0; $n<$num_fields; $n++) {
				$column_name = trim($headers[$n]);

				// Only process fields in which we are interested (ie; where "ignore" is not set)
				if (!isset($metadata_mapping['metadata_ignore_fields'][$column_name])) {
					// Any value set to dashes only OR blank denotes an empty field, so don't bother with this one. Otherwise compact contiguous spaces
					$value = trim(compactSpaces($data[$n]));

					if ($ignore_blank_fields && ((compactCharacters($value, '-') == '-') || ($value == ''))) {
						$num_fields_blank++;
						continue;
					}

					$metadata_field_id = $metadata_mapping['metadata_fields'][$column_name];

					if (!isset($metadata_field_id)) {
						printStdErr('* Metadata schema mapping is missing for the "'.$column_name."\" field. Cannot continue\n");
						exit(-8);
					}

					$record_metadata[$metadata_field_id] = $value;
				}
			}

			// If we are after unique records only (ie; the '-unique' parameter was specified), then ensure that this record is not a duplicate of one already processed. If it is, then it will be ignored
			if ($export_unique_records_only) {
				$record_serialised = serialize($record_metadata);

				if (isset($imported_records[$record_serialised])) {
					// This record has already been imported. Continue with the next record
					printStdErr('--- Ignoring duplicate asset '.$name.(($folder_name_orig != '') ? (' in folder "'.$folder_name_orig).'"' : ''));
					$num_records_ignored++;
					continue;
				} else {
					$imported_records[$record_serialised] = 1;
				}
			}

			// Find the parent so we can dynamically associate the child assets - this should be an Asset ID
			if ($group_by_asset_field_column >= 0) {
				$asset_parent_id = (int)$data[$group_by_asset_field_column];
				printStdErr('-- Targeting asset '.$asset_parent_id);
			}

			printStdErr('-- Creating asset '.$name.(($folder_name_orig != '') ? (' in folder "'.$folder_name_orig).'"' : ''));
			printCreateAssetAction($action_id, 'create_asset', $global_asset_type, '1', $asset_parent_id);

			$num_assets_created++;

			// Then name the asset...
			$settings = Array(
							'attribute'	=> 'name',
							'value'		=> $name,
						);
			printCreateTriggerAction('set_'.$action_id.'_name', 'set_attribute_value', 'output://'.$action_id.'.assetid', $settings);

			// ...and give it a web path
			$settings = Array(
							'path'	=> $action_id,
						);
			printCreateTriggerAction('set_'.$action_id.'_path', 'add_web_path', 'output://'.$action_id.'.assetid', $settings);

			// Set the appropriate metadata schema for this asset
			$settings = Array(
							'schemaid'	=> $metadata_mapping['metadata_schema_id'],
							'granted'	=> 1,
						);
			printCreateTriggerAction('set_'.$action_id.'_metadata_schema', 'set_metadata_schema', 'output://'.$action_id.'.assetid', $settings);

			// Finally, assign the relevant metadata
			foreach ($record_metadata as $metadata_field_id => $value) {
				$metadata_value_valid = validateMetadataField($metadata_mapping['metadata_field_objects'][$metadata_field_id], $value);

				if ($metadata_value_valid) {
					$settings = Array(
									'fieldid'	=> $metadata_field_id,
									'value'		=> $value,
								);
					printCreateTriggerAction('set_'.$action_id.'_metadata_value_'.$metadata_field_id, 'set_metadata_value', 'output://'.$action_id.'.assetid', $settings);
				} else {
					printStdErr("\n* The value (".$value.') set for field '.$metadata_field_id.' is invalid for a '.get_class($metadata_mapping['metadata_field_objects'][$metadata_field_id])." field\n");
					printStdErr('User Options ------------');
					printStdErr('(I)gnore this field and continue exporting subsequent records');
					printStdErr('(C)ancel export');

					$valid_choice = FALSE;
					while (!$valid_choice) {
						$user_choice = rtrim(strtolower(fgets(STDIN, 3)));
						$valid_choice = (($user_choice == 'i') || ($user_choice == 'c'));
					}

					if ($user_choice == 'c') {
						printStdErr("\n- Export cancelled by user");
						exit(-9);
					}

					$num_fields_ignored++;

				}

			}

		}//end else
	}//end if
}//end while

echo "</actions>\n";

fclose($csv_fd);

// We're done. Display some totals to show what has been accomplished
printStdErr("\n- All done, stats below:");
printStdErr('Folders created: '.$num_folders_created);
printStdErr('Assets created : '.$num_assets_created);
printStdErr('Records ignored: '.$num_records_ignored);
printStdErr('Fields ignored (by user) : '.$num_fields_ignored);
printStdErr('Fields ignored (blank)   : '.$num_fields_blank."\n");

// That's all folks :-D

?>
