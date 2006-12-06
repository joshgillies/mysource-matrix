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
* $Id: upgrade_metadata_storage.php,v 1.5 2006/12/06 05:39:52 bcaldwell Exp $
*
*/

/**
* Upgrade the wayt that the metadata storage is done.
*
* @author  Blair Robertson <brobertson@squiz.co.uk>
* @version $Revision: 1.5 $
* @package MySource_Matrix
*/
error_reporting(E_ALL);
if ((php_sapi_name() != 'cli')) trigger_error("You can only run this script from the command line\n", E_USER_ERROR);

$SYSTEM_ROOT = (isset($_SERVER['argv'][1])) ? $_SERVER['argv'][1] : '';
if (empty($SYSTEM_ROOT) || !is_dir($SYSTEM_ROOT)) {
	trigger_error("You need to supply the path to the System Root as the first argument\n", E_USER_ERROR);
}

require_once $SYSTEM_ROOT.'/core/include/init.inc';

// ask for the root password for the system
echo 'Enter the root password for "'.SQ_CONF_SYSTEM_NAME.'": ';
$root_password = rtrim(fgets(STDIN, 4094));

// check that the correct root password was entered
$root_user = &$GLOBALS['SQ_SYSTEM']->am->getSystemAsset('root_user');
if (!$root_user->comparePassword($root_password)) {
	trigger_error("The root password entered was incorrect\n", E_USER_ERROR);
}

// log in as root
if (!$GLOBALS['SQ_SYSTEM']->setCurrentUser($root_user)) {
	trigger_error("Failed logging in as root user\n", E_USER_ERROR);
}

$db = &$GLOBALS['SQ_SYSTEM']->db;
$am = &$GLOBALS['SQ_SYSTEM']->am;
$mm = &$GLOBALS['SQ_SYSTEM']->getMetadataManager();

$am->includeAsset('metadata_field');

// first check that the new table we need exists
$tables = $db->getListOf('tables');
assert_valid_db_result($tables);
if (!in_array('sq_ast_metadata_value', $tables)) {
	trigger_error('You need to run install/step_02.php to install the new table required for the new metadata storage', E_USER_ERROR);
}
unset($tables);

echo "\n\n\n";
echo "+---------------------------------------------+\n";
echo "| UPDATING Metadata_Field_Date default values |\n";
echo "+---------------------------------------------+\n";

$date_fields = $am->getTypeAssetids('metadata_field_date', false, true);
foreach ($date_fields as $assetid => $type_code) {
	$asset = &$am->getAsset($assetid, $type_code);
	if (is_null($asset)) continue;

	$default = $asset->attr('default');
	// if the default looks like a timestamp, let's update it
	if (is_numeric($default)) {
		printName('Setting "'.$asset->name.'" (#'.$asset->id.') value to iso8601');

		$default = ts_iso8601($default);

		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		if (!$am->acquireLock($asset->id, 'attributes', $asset->id, true, NULL)) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			printUpdateStatus('FAIL : '.__LINE__);
			continue;
		}

		if (!$asset->setAttrValue('default', $default)) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			printUpdateStatus('FAIL : '.__LINE__);
			continue;
		}

		if (!$asset->saveAttributes()) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			printUpdateStatus('FAIL : '.__LINE__);
			continue;
		}

		$am->releaseLock($asset->id, 'attributes');

		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
		printUpdateStatus('OK');

	}//end if

}//end foreach

echo "\n\n\n";
echo "+---------------------------------------------+\n";
echo "| Putting metadata into sq_ast_metadata_value |\n";
echo "+---------------------------------------------+\n";

$sql = 'SELECT DISTINCT m.assetid, a.type_code
		FROM sq_ast_metadata m INNER JOIN sq_ast a ON m.assetid = a.assetid
		WHERE m.granted = 1
		  AND m.metadata != '.$db->quote('');
$result = $db->query($sql);
assert_valid_db_result($result);

while (null !== ($row = $result->fetchRow())) {
	$asset = &$am->getAsset($row['assetid'], $row['type_code']);
	if (is_null($asset)) continue;
	printName('Setting "'.$asset->name.'" (#'.$asset->id.')');

	if (!$am->acquireLock($asset->id, 'metadata', $asset->id, true, NULL)) {
		printUpdateStatus('FAIL : '.__LINE__);
		continue;
	}

	$sql = 'SELECT schemaid, metadata
			FROM sq_ast_metadata m
			WHERE m.granted = 1
			  AND m.metadata != '.$db->quote('').'
			  AND m.assetid = '.$db->quote($asset->id);

	$old_metadata = $db->getAssoc($sql);
	assert_valid_db_result($old_metadata);

	$new_metadata = Array();

	for (reset($old_metadata); NULL !== ($schemaid = key($old_metadata)); next($old_metadata)) {

		$old_schema_data = unserialize(current($old_metadata));
		if ($old_schema_data === false) continue;

		$schema = &$am->getAsset($schemaid);
		if (is_null($schema)) continue;

		$sections = &$am->getChildren($schemaid, 'metadata_section', false);
		foreach ($sections as $sectionid => $section_type_code) {
			$section = &$am->getAsset($sectionid, $section_type_code);

			// we don't have anything on this section? next!
			if (empty($old_schema_data['sections'][$section->name])) continue;

			$fields  = &$am->getChildren($sectionid, 'metadata_field', false);
			foreach ($fields as $fieldid => $field_type_code) {
				$field = &$am->getAsset($fieldid, $field_type_code);

				// we don't have anything on this field? next!
				if (empty($old_schema_data['sections'][$section->name]['fields'][$field->name])) continue;
				$field_data = &$old_schema_data['sections'][$section->name]['fields'][$field->name];

				// if this field is using the default data there is nothing we need to do, we don't need to save it in the DB
				if (!empty($field_data['using_default'])) continue;
				if (!isset($field_data['value'])) continue;

				if (is_a($field, 'metadata_field_date')) {
					if (is_numeric($field_data['value'])) {
						$field_data['value'] = ts_iso8601($field_data['value']);
					}
				}

				$value_str = Metadata_Field::encodeValueString($field_data['value'], (isset($field_data['value_components']) ? $field_data['value_components'] : Array()));
				$new_metadata[$field->id] = Array(
												'name'  => $field->name,
												'value' => $value_str,
											);

			}// end foreach

		}// end foreach

	}// end for

	if (!empty($new_metadata)) pre_echo($new_metadata);

	if (!$mm->setMetadata($asset->id, $new_metadata)) {
		printUpdateStatus('FAIL : '.__LINE__);
	}//end if

	if (!$mm->generateContentFile($asset->id)) {
		printUpdateStatus('FAIL : '.__LINE__);
	}//end if

	printUpdateStatus('OK');
	$am->releaseLock($asset->id, 'metadata');

}// end while

$result->free();

exit();

  ////////////////////////
 //  HELPER FUNCTIONS  //
////////////////////////
function printName($name)
{
	printf ('%s%'.(60 - strlen($name)).'s', $name, '');

}//end printName()


function printUpdateStatus($status)
{
	echo "[ $status ]\n";

}//end printUpdateStatus()


?>
