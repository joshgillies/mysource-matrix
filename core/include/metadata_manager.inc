<?php
/**
* +--------------------------------------------------------------------+
* | Squiz.net Open Source Licence                                      |
* +--------------------------------------------------------------------+
* | Copyright (c), 2003 Squiz Pty Ltd (ABN 77 084 670 600).            |
* +--------------------------------------------------------------------+
* | This source file may be used subject to, and only in accordance    |
* | with, the Squiz Open Source Licence Agreement found at             |
* | http://www.squiz.net/licence.                                      |
* | Make sure you have read and accept the terms of that licence,      |
* | including its limitations of liability and disclaimers, before     |
* | using this software in any way. Your use of this software is       |
* | deemed to constitute agreement to be bound by that licence. If you |
* | modify, adapt or enhance this software, you agree to assign your   |
* | intellectual property rights in the modification, adaptation and   |
* | enhancement to Squiz Pty Ltd for use and distribution under that   |
* | licence.                                                           |
* +--------------------------------------------------------------------+
*
* $Id: metadata_manager.inc,v 1.39 2004/05/26 06:47:56 lwright Exp $
* $Name: not supported by cvs2svn $
*/


/**
* Metadata_Manager
*
* Purpose
*    Manages the editing of metadata in the system
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Version$ - 1.0
* @package MySource_Matrix
*/
require_once SQ_FUDGE_PATH.'/general/text.inc';

class Metadata_Manager extends MySource_Object
{

	/**
	* Constructor
	*
	*/
	function Metadata_Manager()
	{
		$this->MySource_Object();

	}//end constructor


	/**
	* Check if all required fields have been completed for an asset
	*
	* @param int	$assetid	the ID of the asset to check
	*
	* @access public
	* @return boolean
	*/
	function requiredFieldsComplete($assetid)
	{
		$assetid = (int) $assetid;
		$asset = &$GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
		if (is_null($asset)) return true;

		$schemas = $this->getSchemas($asset->id, true);
		if (empty($schemas)) return true;

		foreach ($schemas as $schemaid) {
			$schema = &$GLOBALS['SQ_SYSTEM']->am->getAsset($schemaid);
			if (is_null($schema)) continue;
			if (!is_a($schema, 'metadata_schema')) continue;
			$edit_fns = $schema->getEditFns();
			$values = $this->getSchemaMetadata($asset->id, $schema->id);
			if (!$edit_fns->requiredFieldsComplete($schema, $values)) return false;
		}

		return true;

	}//end requiredFieldsComplete()


	/**
	* Check if all required fields have been completed for an asset
	*
	* @param int	$assetid	the ID of the asset to check
	*
	* @access public
	* @return boolean
	*/
	function allowsMetadata($assetid)
	{
		$asset = &$GLOBALS['SQ_SYSTEM']->am->getAsset((int) $assetid);
		if (is_null($asset)) return false;
		$edit_fns = $asset->getEditFns();
		return isset($edit_fns->static_screens['metadata']);

	}//end allowsMetadata()


//--        SCHEMAS        --//


	/**
	* Get an array of schemaIDs for all schemas applied and denied on an asset
	*
	* @param int		$assetid	the ID of the asset to get schemas for
	* @param boolean	$access		type of access : null = all, true = applied, false = denied
	*
	* @access public
	* @return array( schemaid => access[1|0] )
	*/
	function getSchemas($assetid, $access=null)
	{
		$db = &$GLOBALS['SQ_SYSTEM']->db;

		$sql = '  SELECT DISTINCT schemaid, access
				  FROM '.SQ_TABLE_RUNNING_PREFIX.'asset_metadata ';
		$where = 'assetid = '.$db->quote($assetid);
		if (!is_null($access)) $where .= ' AND access = '.$db->quote((($access) ? '1' : '0'));
		$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where);

		$result = $db->getAll($sql.$where);
		if (DB::isError($result)) {
			trigger_error($result->getMessage().'<br/>'.$result->getUserInfo(), E_USER_WARNING);
			return Array();
		}

		$schemas = Array();
		foreach ($result as $data) {
			if (is_null($access)) $schemas[$data['schemaid']] = $data['access'];
			else $schemas[] = $data['schemaid'];
		}

		return $schemas;

	}//end getSchemas()


	/**
	* Apply a schema to or Deny from, an asset
	*
	* @param int		$assetid	the ID of the asset to set the schema on
	* @param int		$schemaid	the ID of the schema to set
	* @param boolean	$access		is this schema applied (TRUE) or denied (FALSE)?
	*
	* @access public
	* @return boolean
	*/
	function setSchema($assetid, $schemaid, $access)
	{
		$assetid  = (int)  $assetid;
		$schemaid = (int)  $schemaid;
		$access   = (bool) $access;

		$schema = &$GLOBALS['SQ_SYSTEM']->am->getAsset($schemaid);
		if (is_null($schema)) {
			trigger_error('Failed setting metadata schema, Schema Asset #'.$schemaid.' does not exist', E_USER_WARNING);
			return false;
		}

		if (!is_a($schema, 'metadata_schema')) {
			trigger_error('Failed setting metadata schema, "'.$schema->name.'" (Asset #'.$schemaid.') is not a metadata schema', E_USER_WARNING);
			return false;
		}

		$asset = &$GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
		if (is_null($asset)) {
			trigger_error('Failed setting metadata schema, Asset #'.$assetid.' does not exist', E_USER_WARNING);
			return false;
		}

		if (!$asset->adminAccess('metadata')) {
			trigger_error('Cannot set metadata schema for "'.$asset->name.'", permission denied', E_USER_WARNING);
			return false;
		}

		// get the current schemas that are set
		$schemas = $this->getSchemas($assetid);

		// check if this schema is already set
		if (isset($schemas[$schemaid])) {
			if ((bool)$schemas[$schemaid] == $access) {
				// schema is set with same access level
				return true;
			} else {
				// schema is set but with the opposite access level
				$new_access     = ($access) ? 'apply'  : 'deny';
				$current_access = ($access) ? 'denied' : 'applied';

				trigger_error('Could not '.$new_access.' metadata schema "'.$schema->name.'" on "'.$asset->name.'" because this asset already has this schema '.$current_access, E_USER_WARNING);
				return false;
			}
		}

		$db = &$GLOBALS['SQ_SYSTEM']->db;
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		// add the schema to the database
		$values = Array('assetid'  => $asset->id,
						'schemaid' => $schemaid,
						'metadata' => '',
						'access'   => (($access) ? '1' : '0')
						);
		if (!$GLOBALS['SQ_SYSTEM']->rollbackInsert('asset_metadata', $values)) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			return false;
		}

		// add defaults to the indexing service
		if ($access) {
			$this->indexMetaData($asset->id, Array($schemaid));
		}

		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		return true;

	}//end setSchema()


	/**
	* Remove a set schema from an asset
	*
	* @param int	$assetid	the ID of the asset to delete the schema from
	* @param int	$schemaid	the ID of the schema to delete
	*
	* @access public
	* @return boolean
	*/
	function deleteSchema($assetid, $schemaid)
	{
		$assetid  = (int) $assetid;
		$schemaid = (int) $schemaid;

		$asset = &$GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
		if (is_null($asset)) {
			trigger_error('Failed deleting metadata schema, Asset #'.$assetid.' does not exist', E_USER_WARNING);
			return false;
		}

		if (!$asset->adminAccess('metadata')) {
			trigger_error('Cannot delete metadata schema for "'.$asset->name.'", permission denied', E_USER_WARNING);
			return false;
		}

		$db = &$GLOBALS['SQ_SYSTEM']->db;
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		// delete and add rollback entries
		$where = 'schemaid    = '.$db->quote($schemaid).'
				  AND assetid = '.$db->quote($asset->id);
		if (!$GLOBALS['SQ_SYSTEM']->rollbackDelete('asset_metadata', $where)) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			return false;
		}

		$this->indexMetaData($asset->id, Array($schemaid));

		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		return true;

	}//end deleteSchema()


	/**
	* Get an array of assetIDs that have the passed schemaID applied or denied on them
	*
	* @param int		$schemaid	the ID of the schema to get assets for
	* @param boolean	$access		type of access : null = all, true = applied, false = denied
	*
	* @access public
	* @return array( schemaid => access[1|0] )
	*/
	function getSchemaAssets($schemaid, $access=null)
	{
		$db = &$GLOBALS['SQ_SYSTEM']->db;

		$sql = '  SELECT DISTINCT assetid, access
				  FROM '.SQ_TABLE_RUNNING_PREFIX.'asset_metadata ';
		$where = 'schemaid = '.$db->quote($schemaid);
		if (!is_null($access)) $where .= ' AND access = '.$db->quote((($access) ? '1' : '0'));
		$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where);

		$result = $db->getAll($sql.$where);
		if (DB::isError($result)) {
			trigger_error($result->getMessage().'<br/>'.$result->getUserInfo(), E_USER_WARNING);
			return Array();
		}

		$assets = Array();
		foreach ($result as $data) {
			if (is_null($access)) $assets[$data['assetid']] = $data['access'];
			else $assets[] = $data['assetid'];
		}

		return $assets;

	}//end getSchemaAssets()


	/**
	* Get an array of metatata fields IDs for a metadata schema
	*
	* @param int $schemaid	the schema whos fields we are getting
	*
	* @return Array(int)
	* @access public
	*/
	function getMetadataFields($schemaid)
	{
		$fields_array = Array();
		$am = &$GLOBALS['SQ_SYSTEM']->am;
		$sectionids = $am->getChildren($schemaid, 'metadata_section', false);

		foreach ($sectionids as $sectionid => $section_type) {
			$fieldids = $am->getChildren($sectionid, 'metadata_field', false);
			$fields_array = array_merge($fields_array, array_keys($fieldids));
		}

		return $fields_array;

	}//end getMetadataFields()



//--        METADATA VALUES        --//


	/**
	* Get the metadata values set for an asset on a particular schema
	*
	* The arry returned may have some fields missing. If there is no field value it
	* means that the field is using the default value (whatever that happens to be
	* at the time). This cuts down on storage and allows defaults to change over time
	* without updating metadata values.
	*
	* @param int	$assetid	the ID of the asset to get metadata for
	* @param int	$schemaid	the ID of the schema to get metadata for
	*
	* @access public
	* @return array
	*/
	function getSchemaMetadata($assetid, $schemaid)
	{
		$assetid  = (int) $assetid;
		$schemaid = (int) $schemaid;

		$asset = &$GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
		if (is_null($asset)) {
			trigger_error('Failed getting metadata, Asset #'.$assetid.' does not exist', E_USER_WARNING);
			return false;
		}

		$schema = &$GLOBALS['SQ_SYSTEM']->am->getAsset($schemaid);
		if (is_null($schema)) {
			trigger_error('Failed getting metadata, Schema #'.$schemaid.' does not exist', E_USER_WARNING);
			return false;
		}

		if (!is_a($schema, 'metadata_schema')) {
			trigger_error('Failed getting metadata, "'.$schema->name.'" (Asset #'.$schemaid.') is not a metadata schema', E_USER_WARNING);
			return false;
		}

		// check that the schema we are getting metadata for is actually set
		$schemas = $this->getSchemas($assetid, true);
		if (!in_array($schemaid, $schemas)) return Array();

		$db = &$GLOBALS['SQ_SYSTEM']->db;

		$sql = '  SELECT metadata
				  FROM '.SQ_TABLE_RUNNING_PREFIX.'asset_metadata ';
		$where = 'assetid = '.$db->quote($asset->id).'
				    AND schemaid = '.$db->quote($schemaid).'
				    AND access = 1';
		$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where);

		$metadata = $db->getOne($sql.$where);
		if (DB::isError($metadata)) {
			trigger_error($metadata->getMessage().'<br />'.$metadata->getUserInfo(), E_USER_WARNING);
			return Array();
		}

		$metadata = unserialize($metadata);
		if (!is_array($metadata)) $metadata = Array();
		return $metadata;

	}//end getSchemaMetadata()


	/**
	* Generate an array of current values for the passed metadata field names
	*
	* @param int			$assetid	the ID of the asset to get field values for
	* @param array(string)	$fields		an array of field names
	*
	* @access public
	* @return array(field => value)
	*/
	function getMetadataFieldValues($assetid, $fields)
	{
		if (empty($fields)) return Array();

		// the array of field values we are going to return
		$field_values = Array();
		foreach ($fields as $name) $field_values[$name] = '';

		$assetid = (int) $assetid;

		$asset = &$GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
		if (is_null($asset)) {
			trigger_error('Failed generating keyword replacements, Asset #'.$assetid.' does not exist', E_USER_WARNING);
			return $field_values;
		}

		$schemas = $this->getSchemas($asset->id, true);
		if (empty($schemas)) return $field_values;

		foreach ($schemas as $schemaid) {
			$schema = &$GLOBALS['SQ_SYSTEM']->am->getAsset($schemaid);
			if (is_null($schema)) {
				trigger_error('Failed generating keyword replacements, Schema #'.$schemaid.' does not exist', E_USER_WARNING);
				return $field_values;
			}

			if (!is_a($schema, 'metadata_schema')) {
				trigger_error('Failed generating keyword replacements, "'.$schema->name.'" (Asset #'.$schemaid.') is not a metadata schema', E_USER_WARNING);
				return $field_values;
			}

			$values = $this->getSchemaMetadata($asset->id, $schema->id);
			$edit_fns = $schema->getEditFns();
			$edit_fns->getMetadataFieldValues($asset, $schema, $values, $field_values);
		}

		return $field_values;

	}//end getMetadataFieldValues()


	/**
	* Set metadata values for an asset
	*
	* @param int	$assetid	the ID of the asset to set the values for
	* @param array	$metadata	the values to set
	*
	* @access public
	* @return boolean
	*/
	function setMetadata($assetid, $metadata)
	{
		$assetid = (int) $assetid;

		$asset = &$GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
		if (is_null($asset)) {
			trigger_error('Failed setting metadata, Asset #'.$assetid.' does not exist', E_USER_WARNING);
			return false;
		}

		if (!$asset->writeAccess('metadata')) {
			trigger_error('Cannot set metadata for "'.$asset->name.'", permission denied', E_USER_WARNING);
			return false;
		}

		$schemaids = $this->getSchemas($asset->id, true);
		if (empty($schemaids)) return false;

		
		foreach ($schemaids as $schemaid) {
			$schema = &$GLOBALS['SQ_SYSTEM']->am->getAsset($schemaid);
			if (!isset($metadata[$schema->name])) continue;
			if (false === ($this->setSchemaMetadata($assetid, $schemaid, $metadata[$schema->name]))) return false;
		}

		$this->indexMetadata($assetid, $schemaids);

		return true;

	}//end setMetadata()


	/**
	* sets metadata for an asset that is specific to a schema
	*
	* @param integer	$assetid	the assetid to apply the schema for
	* @param integer	$schemaid	the id of the schema to apply the metadata to
	* @param array		$metadata	the metadata values to set
	*
	* @return boolean
	* @access public
	*/
	function setSchemaMetadata($assetid, $schemaid, $metadata)
	{
		$db = &$GLOBALS['SQ_SYSTEM']->db;
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');
		
		$asset = &$GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
		$schema = &$GLOBALS['SQ_SYSTEM']->am->getAsset($schemaid);

		if (is_null($schema)) {
			trigger_error('Failed setting metadata, Schema #'.$schemaid.' does not exist', E_USER_WARNING);
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			return false;
		}

		if (!is_a($schema, 'metadata_schema')) {
			trigger_error('Failed setting metadata, "'.$schema->name.'" (Asset #'.$schemaid.') is not a metadata schema', E_USER_WARNING);
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			return false;
		}

		$values = Array('metadata' => $db->quote(serialize($metadata)));
		$where = 'assetid = '.$db->quote($asset->id).'
					AND schemaid = '.$db->quote($schema->id);
		if (!$GLOBALS['SQ_SYSTEM']->rollbackUpdate('asset_metadata', $values, $where)) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			return false;
		}

		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');

		return true;

	}//end setSchemaMetadata()



//--        (RE)GENERATE METADATA        --//


	/**
	* Generate a PHP file that can be included to print all metadata fields for an asset
	*
	* The generated file contains PHP code to do any keyword replacing based on
	* current values instead of the ones at the time of caching.
	*
	* @param int	$assetid		the ID of the asset to generate the file for
	* @param int	$regenerate		regenerate the DB information as well
	*
	* @access public
	* @return boolean
	*/
	function generateContentFile($assetid, $regenerate=false)
	{
		$assetid = (int) $assetid;

		$asset = &$GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
		if (is_null($asset)) {
			trigger_error('Failed caching metadata, Asset #'.$assetid.' does not exist', E_USER_WARNING);
			return false;
		}

		$schemas = $this->getSchemas($asset->id, true);
		if (empty($schemas)) return false;

		// an array of all metadata tag value
		$tag_values = Array();

		// an array of keyword replacements that we are going to have to write some PHP code to produce
		$keywords = Array();

		// an array to hold the complete DB metadata if we are regenerating
		$db_metadata = Array();

		ob_start();
		foreach ($schemas as $schemaid) {
			$schema = &$GLOBALS['SQ_SYSTEM']->am->getAsset($schemaid);
			if (is_null($schema)) {
				trigger_error('Failed caching metadata, Schema #'.$schemaid.' does not exist', E_USER_WARNING);
				return false;
			}

			if (!is_a($schema, 'metadata_schema')) {
				trigger_error('Failed caching metadata, "'.$schema->name.'" (Asset #'.$schemaid.') is not a metadata schema', E_USER_WARNING);
				return false;
			}

			$values = Array();
			$values = $this->getSchemaMetadata($asset->id, $schema->id);
			$edit_fns = $schema->getEditFns();
			$edit_fns->generateMetadata($schema, $values, $tag_values, $keywords);
			if ($regenerate) $db_metadata[$schema->name] = $values;

			$GLOBALS['SQ_SYSTEM']->am->forgetAsset($schema);
		}

		$metadata = ob_get_contents();
		ob_end_clean();

		ob_start();
			echo '<'.'?php'."\n";

			if (!empty($keywords)) {
				require_once SQ_FUDGE_PATH.'/general/text.inc';
				echo '$metadata_asset = &$GLOBALS[\'SQ_SYSTEM\']->am->getAsset('.$asset->id.');'."\n\n";
				$replacements = $this->generateKeywordReplacements($asset, $keywords);
				$tag_values = replace_keywords($tag_values, $replacements);
				echo "\n\n";
			}

			$nested_tag_values = Array();
			$tag_nested_index = Array();
			foreach ($tag_values as $n => $v) {
				$matches = Array();
				$nested_index = 0;
				preg_match_all('|\$metadata_values\[\'([^\]]+)\'\]|', $v, $matches);
				if (!empty($matches[1])) {
					foreach ($matches[1] as $field_name) {
						// we have a nested field name
						for ($i = 0; $i < count($nested_tag_values); $i++) {
							if (isset($nested_tag_values[$i][$field_name])) {
								$v = preg_replace('|(\$metadata_values)(\[\')('.$field_name.')(\'\])|', '$metadata_values_'.$i.'[\'\\3\']', $v);
								if ($i >= $nested_index) $nested_index = $i + 1;
								break;
							}
						}
					}
				}

				$nested_tag_values[$nested_index][$n] = $v;
				$tag_nested_index[$n] = $nested_index;
			}

			foreach ($nested_tag_values as $i => $values) {
				echo '$metadata_values_'.$i.' = Array(';
				foreach ($values as $n => $v) {
					$v = addslashes($v);
					$v = preg_replace('|(\$metadata_replacements\[)\\\\\'([^\]]+)\\\\\'(\])|', '\'.\\1\'\\2\'\\3.\'', $v);
					$v = preg_replace('|(\$metadata_values[^\[]+\[)\\\\\'([^\]]+)\\\\\'(\])|', '\'.\\1\'\\2\'\\3.\'', $v);
					echo "'$n' => '$v',\n";
				}
				echo ");\n";
			}

			echo '?'.'>';
			$php = ob_get_contents();
		ob_end_clean();

		$metadata_matches = Array();
		preg_match_all('|\$metadata_values\[\'([^\]]+)\'\]|', $metadata, $metadata_matches);
		foreach ($metadata_matches[1] as $field_name) {
			$metadata = preg_replace('|\$metadata_values\[\''.$field_name.'\'\]|', '$metadata_values_'.$tag_nested_index[$field_name].'[\''.$field_name.'\']', $metadata);
		}

		$metadata = $php."\n\n".$metadata;

		require_once SQ_FUDGE_PATH.'/general/file_system.inc';
		create_directory($asset->data_path);
		if (!string_to_file($metadata, $asset->data_path.'/metadata.php')) return false;

		if (!$regenerate) return true;
		else return $this->setMetadata($assetid, $db_metadata);

	}//end generateContentFile()


	/**
	* Generate PHP code to replace a set of keywords
	*
	* This function outputs the PHP required to generate all the keywords that
	* it is passed. All PHP generated places values in an array called $metadata_replacements
	* (which happens in the outputted PHP code). An array of replacements is returned that
	* can be used to replace all the keywords with a PHP string to output the correct value
	* from the $metadata_replacements array.
	*
	* This function can also return the values of the replacement values instead of the PHP code.
	*
	* @param object Asset	&$asset		the asset to use for replacment values
	* @param array			$keywords	an array of keywords in
	* @param boolean		$generating	generate the PHP code or not (which generates the value)
	*
	* @access public
	* @return array(keyword => replacement)
	*/
	function generateKeywordReplacements(&$metadata_asset, $keywords, $generating=true)
	{
		$metadata_replacements = Array();
		$keywords = array_unique($keywords);

		if (!empty($keywords) && $generating) echo '$metadata_replacements = Array();'."\n";

		foreach ($keywords as $keyword) {

			if ($generating) {
				$metadata_replacements[$keyword] = '$metadata_replacements[\''.$keyword.'\']';
			} else {
				ob_start();
			}

			switch ($keyword) {

				case 'asset_name'       :
				case 'asset_short_name' :
				case 'asset_version'    :
					echo '$metadata_replacements[\''.$keyword.'\'] = $metadata_asset->'.substr($keyword,6).';'."\n";
				break;

				case 'asset_url' :
					echo '$metadata_replacements[\''.$keyword.'\'] = $metadata_asset->getURL();'."\n";
				break;

				case 'asset_href' :
					echo '$metadata_replacements[\''.$keyword.'\'] = $metadata_asset->getHref();'."\n";
				break;

				case 'asset_created' :
				case 'asset_updated' :
					echo '$metadata_replacements[\''.$keyword.'\'] = date(\'Y-m-d H:i:s\', $metadata_asset->'.substr($keyword,6).');'."\n";
				break;

				case 'asset_created_short' :
				case 'asset_updated_short' :
					echo '$metadata_replacements[\''.$keyword.'\'] = date(\'Y-m-d\', $metadata_asset->'.substr($keyword,6,7).');'."\n";
				break;

				case 'asset_created_readable' :
				case 'asset_updated_readable' :
					echo '$metadata_replacements[\''.$keyword.'\'] = date(\'d M Y g:ia\', $metadata_asset->'.substr($keyword,6,7).');'."\n";
				break;

				case 'asset_created_by_name' :
				case 'asset_updated_by_name' :
					echo 'if (!isset($metadata_'.substr($keyword,6,7).'_by_user)) $metadata_'.substr($keyword,6,7).'_by_user = &$GLOBALS[\'SQ_SYSTEM\']->am->getAsset($metadata_asset->'.substr($keyword,6,7).'_userid);'."\n";
					echo '$metadata_replacements[\''.$keyword.'\'] = $metadata_'.substr($keyword,6,7).'_by_user->name;'."\n";
				break;

				case 'asset_created_by_first_name' :
				case 'asset_created_by_last_name'  :
				case 'asset_created_by_email'      :
				case 'asset_updated_by_first_name' :
				case 'asset_updated_by_last_name'  :
				case 'asset_updated_by_email'      :
					echo 'if (!isset($metadata_'.substr($keyword,6,7).'_by_user)) $metadata_'.substr($keyword,6,7).'_by_user = &$GLOBALS[\'SQ_SYSTEM\']->am->getAsset($metadata_asset->'.substr($keyword,6,7).'_userid);'."\n";
					echo '$metadata_replacements[\''.$keyword.'\'] = $metadata_'.substr($keyword,6,7).'_by_user->attr(\''.substr($keyword,17).'\');'."\n";
				break;

				case 'asset_read_permission'  :
				case 'asset_write_permission' :
				case 'asset_admin_permission' :
					$perm_type = str_replace('_permission', '', $keyword);
					$perm_type = substr($perm_type,6);
					echo '$metadata_'.$perm_type.'_perms = $GLOBALS[\'SQ_SYSTEM\']->am->getPermission($metadata_asset->id, SQ_PERMISSION_'.strtoupper($perm_type).', true, false, true);'."\n";
					echo '$metadata_'.$perm_type.'_perm_names = \'\';'."\n";
					echo 'foreach ($metadata_'.$perm_type.'_perms as $userid) {'."\n";
					echo '	$user = &$GLOBALS[\'SQ_SYSTEM\']->am->getAsset($userid);'."\n";
					echo '	if (is_null($user)) continue;'."\n";
					echo '	$metadata_'.$perm_type.'_perm_names .= $user->name.\', \';'."\n";
					echo '}'."\n";
					echo '$metadata_'.$perm_type.'_perm_names = trim($metadata_'.$perm_type.'_perm_names, \', \');'."\n";
					echo '$metadata_replacements[\''.$keyword.'\'] = $metadata_'.$perm_type.'_perm_names;'."\n";
				break;

				case 'system_name'   :
				case 'system_owner'  :
				case 'default_email' :
				case 'tech_email'    :
					echo '$metadata_replacements[\''.$keyword.'\'] = SQ_CONF_'.strtoupper($keyword).';'."\n";
				break;

				default :

					// searching for keywords that replace the content of another field
					if (substr($keyword, 0, 15) == 'metadata_field_') {

						$field_name = substr($keyword, 15);
						$metadata_replacements[$keyword] = '$metadata_values[\''.$field_name.'\']';

					} else {

						// searching for special date format keywords
						$key = substr($keyword, 0, 13);
						$extras = substr($keyword, 14);
						if (strpos($extras, '_') === false) {
							$format = $extras;
							$offset = '';
						} else {
							list ($format, $offset) = explode('_', $extras);
						}
						$format = str_replace('-', ' ', $format);

						switch ($key) {
							case 'asset_created':
							case 'asset_updated':
								if (empty($offset)) $date = '$metadata_asset->'.substr($key, 6);
								else $date = 'strtotime(\''.$offset.'\', $metadata_asset->'.substr($key, 6).')';
								
								echo '$metadata_replacements[\''.$key.'\'] = date(\''.$format.'\', '.$date.');'."\n";
							break;
							default:
								echo '$metadata_replacements[\''.$key.'\'] = \'\';'."\n";
							break;
						}

					}//end if

				break;

			}//end switch


			if (!$generating) {
				eval(ob_get_contents());
				ob_end_clean();
			}
		}

		return $metadata_replacements;

	}//end generateKeywordReplacements()


	/**
	* Regenerate both the cached tag metadata and the serialised DB values
	*
	* Basically just an alias to generateContentFile to
	*
	* @param int	$assetid	the ID of the asset to regenerate metadata for
	*
	* @access public
	* @return boolean
	*/
	function regenerateMetadata($assetid)
	{
		return $this->generateContentFile($assetid, true);

	}//end regenerateMetadata()


}//end class

?>