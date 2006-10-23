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
* $Id: metadata_manager.inc,v 1.110.2.5 2006/10/23 00:28:13 rhoward Exp $
*
*/

require_once SQ_FUDGE_PATH.'/general/text.inc';

/**
* Metadata_Manager
*
* Purpose
*    Manages the editing of metadata in the system
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Revision: 1.110.2.5 $
* @package MySource_Matrix
*/
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
	* If no values have been entered, the default schema values are checked
	*
	* @param string	$assetid	the ID of the asset to check
	*
	* @return boolean
	* @access public
	*/
	function requiredFieldsComplete($assetid)
	{
		$asset =& $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
		if (is_null($asset)) return TRUE;

		$schemas = $this->getSchemas($asset->id, TRUE);
		if (empty($schemas)) return TRUE;

		$values = $this->getMetadata($asset->id);

		foreach ($schemas as $schemaid) {
			$schema =& $GLOBALS['SQ_SYSTEM']->am->getAsset($schemaid);
			if (is_null($schema)) continue;
			if (!is_a($schema, 'metadata_schema')) continue;
			$edit_fns = $schema->getEditFns();
			if (!$edit_fns->requiredFieldsComplete($schema, $values)) {
				return FALSE;
			}
		}

		return TRUE;

	}//end requiredFieldsComplete()


	/**
	* Check if all required fields have been completed for an asset
	*
	* @param string	$assetid	the ID of the asset to check
	*
	* @return boolean
	* @access public
	*/
	function allowsMetadata($assetid)
	{
		$asset =& $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
		if (is_null($asset)) return FALSE;
		$edit_fns = $asset->getEditFns();
		return isset($edit_fns->static_screens['metadata']);

	}//end allowsMetadata()


//--        SCHEMAS        --//


	/**
	* Get an array of schemaIDs for all schemas applied and denied on an asset
	*
	* if $granted is null returns Array(schemaid)
	* otherwise returns Array(schemaid => granted[1|0])
	*
	* @param string		$assetid	the ID of the asset to get schemas for
	* @param boolean	$granted	type of access : null = all, TRUE = applied, FALSE = denied
	*
	* @return array
	* @access public
	*/
	function getSchemas($assetid, $granted=NULL)
	{
		$db =& $GLOBALS['SQ_SYSTEM']->db;

		$sql = '  SELECT DISTINCT schemaid, granted
				  FROM '.SQ_TABLE_RUNNING_PREFIX.'ast_mdata ';
		$where = 'assetid = '.$db->quote($assetid);
		if (!is_null($granted)) {
			$where .= ' AND granted = '.$db->quote((($granted) ? '1' : '0'));
		}
		$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where);

		$result = $db->getAll($sql.$where);
		assert_valid_db_result($result);

		$schemas = Array();
		foreach ($result as $data) {
			if (is_null($granted)) {
				$schemas[$data['schemaid']] = $data['granted'];
			} else {
				$schemas[] = $data['schemaid'];
			}
		}

		return $schemas;

	}//end getSchemas()


	/**
	* Apply a schema to or Deny from, an asset
	*
	* @param string		$assetid	the ID of the asset to set the schema on
	* @param string		$schemaid	the ID of the schema to set
	* @param boolean	$granted	is this schema applied (TRUE) or denied (FALSE)?
	*
	* @return boolean
	* @access public
	*/
	function setSchema($assetid, $schemaid, $granted)
	{
		$schema =& $GLOBALS['SQ_SYSTEM']->am->getAsset($schemaid);
		if (is_null($schema)) {
			trigger_localised_error('SYS0047', E_USER_WARNING, $schemaid);
			return FALSE;
		}

		if (!is_a($schema, 'metadata_schema')) {
			trigger_localised_error('SYS0045', E_USER_WARNING, $schema->name, $schemaid);
			return FALSE;
		}

		$asset =& $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
		if (is_null($asset)) {
			trigger_localised_error('SYS0046', E_USER_WARNING, $assetid);
			return FALSE;
		}

		if (!$asset->adminAccess('metadata')) {
			trigger_localised_error('SYS0031', E_USER_WARNING, $asset->name);
			return FALSE;
		}

		// get the current schemas that are set
		$schemas = $this->getSchemas($assetid);

		// check if this schema is already set
		if (isset($schemas[$schemaid])) {
			if ((bool)$schemas[$schemaid] == $granted) {
				// schema is set with same access level
				return TRUE;
			} else {
				// schema is set but with the opposite access level
				$new_access     = ($granted) ? 'apply'  : 'deny';
				$current_access = ($granted) ? 'denied' : 'applied';

				trigger_localised_error('SYS0032', E_USER_WARNING, $new_access, $schema->name, $asset->name, $current_access);
				return FALSE;
			}
		}

		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
		$db =& $GLOBALS['SQ_SYSTEM']->db;
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		$sql = 'INSERT INTO
					sq_ast_mdata
					(
						assetid,
						schemaid,
						granted
					)
					VALUES
					(
						'.$db->quoteSmart($asset->id).',
						'.$db->quoteSmart($schemaid).',
						'.$db->quoteSmart((($granted) ? '1' : '0')).'
					)';

		$result = $db->query($sql);
		assert_valid_db_result($result);

		if ($granted) {
			$em =& $GLOBALS['SQ_SYSTEM']->getEventManager();
			$em->broadcastEvent($asset, 'MetadataUpdate', Array('schemaid' => $schemaid));
			$GLOBALS['SQ_SYSTEM']->broadcastTriggerEvent('trigger_event_metadata_updated', $asset);
			$this->generateContentFile($assetid, FALSE);
		}

		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
		return TRUE;

	}//end setSchema()


	/**
	* Remove a set schema from an asset
	*
	* @param string	$assetid	the ID of the asset to delete the schema from
	* @param string	$schemaid	the ID of the schema to delete
	*
	* @return boolean
	* @access public
	*/
	function deleteSchema($assetid, $schemaid)
	{
		$asset =& $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
		if (is_null($asset)) {
			trigger_localised_error('SYS0036', E_USER_WARNING, $assetid);
			return FALSE;
		}

		if (!$asset->adminAccess('metadata')) {
			trigger_localised_error('SYS0029', E_USER_WARNING, $asset->name);
			return FALSE;
		}

		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
		$db =& $GLOBALS['SQ_SYSTEM']->db;
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		$sql = 'DELETE FROM
					sq_ast_mdata
				WHERE
						schemaid	= '.$db->quoteSmart($schemaid).'
					AND	assetid		= '.$db->quoteSmart($asset->id);

		$result = $db->query($sql);
		assert_valid_db_result($result);

		$em =& $GLOBALS['SQ_SYSTEM']->getEventManager();
		$em->broadcastEvent($asset, 'MetadataDeleted', Array('schemaid' => $schemaid));
		$GLOBALS['SQ_SYSTEM']->broadcastTriggerEvent('trigger_event_metadata_updated', $asset);
		$this->generateContentFile($assetid, FALSE);

		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
		return TRUE;

	}//end deleteSchema()


	/**
	* Get an array of assetIDs that have the passed schemaID applied or denied on them
	*
	* @param string		$schemaid	the ID of the schema to get assets for
	* @param boolean	$granted	type of access : null = all, TRUE = applied, FALSE = denied
	*
	* @return array
	* @access public
	*/
	function getSchemaAssets($schemaid, $granted=NULL)
	{
		$db =& $GLOBALS['SQ_SYSTEM']->db;

		$sql = 'SELECT DISTINCT
					assetid, granted
				FROM
					'.SQ_TABLE_RUNNING_PREFIX.'ast_mdata ';

		$where = 'schemaid = '.$db->quote($schemaid);
		if (!is_null($granted)) {
			$where .= ' AND granted = '.$db->quote((($granted) ? '1' : '0'));
		}
		$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where);

		$result = $db->getAll($sql.$where);
		assert_valid_db_result($result);

		$assets = Array();
		foreach ($result as $data) {
			if (is_null($granted)) {
				$assets[$data['assetid']] = $data['granted'];
			} else {
				$assets[] = $data['assetid'];
			}
		}

		return $assets;

	}//end getSchemaAssets()


	/**
	* Get an array of metatata fields IDs for a metadata schema
	*
	* @param string|array $schemaids	single schemaid or an array of schema ids whose fields we are getting
	*
	* @return array
	* @access public
	* @see Asset_Manager::getChildren()
	*/
	function getMetadataFields($schemaids)
	{
		if (!is_array($schemaids)) {
			return $GLOBALS['SQ_SYSTEM']->am->getChildren($schemaids, 'metadata_field', FALSE);
		} else {
			$fieldids = Array();
			foreach ($schemaids as $schemaid) {
				$field_children = $GLOBALS['SQ_SYSTEM']->am->getChildren($schemaid, 'metadata_field', FALSE);
				foreach ($field_children as $fieldid => $field_type) {
					$fieldids[$fieldid] = $field_type;
				}
			}
			return $fieldids;
		}

	}//end getMetadataFields()


//--        METADATA VALUES        --//


	/**
	* Get the metadata values set for an asset on a particular schema
	*
	* The array returned may have some fields missing. If there is no field value it
	* means that the field is using the default value (whatever that happens to be
	* at the time). This cuts down on storage and allows defaults to change over time
	* without updating metadata values.
	*
	* @param string	$assetid	the ID of the asset to get metadata for
	* @param string	$schemaid	optional ID of the schema to only the get metadata for
	*
	* @return array
	* @access public
	*/
	function getMetadata($assetid, $schemaid=0)
	{
		$asset =& $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
		if (is_null($asset)) {
			trigger_localised_error('SYS0043', E_USER_WARNING, $assetid);
			return FALSE;
		}

		// check that the schema we are getting metadata for is actually set
		$schemas = $this->getSchemas($assetid, TRUE);
		if ($schemaid) {
			if (!in_array($schemaid, $schemas)) return Array();
			// restrict to just the one schema
			$schemas = Array($schemaid);
		}

		if (empty($schemas)) return Array();

		$db =& $GLOBALS['SQ_SYSTEM']->db;

		// And to think I didn't like it when this generating query got pulled out of the Asset_Manager::getChildren() :)
		// Instead of doing any thinking...let them do it all for us and use it in our query
		$sql = 'SELECT
					mv.fieldid, a.name, mv.value
				FROM
					'.SQ_TABLE_RUNNING_PREFIX.'ast_mdata_val mv
				INNER JOIN
					'.SQ_TABLE_RUNNING_PREFIX.'ast a ON mv.fieldid = a.assetid';

		$where = '	mv.assetid = '.$db->quoteSmart($asset->id).'
				AND (';

		foreach ($schemas as $schemaid) {
			$schema =& $GLOBALS['SQ_SYSTEM']->am->getAsset($schemaid);
			if (is_null($schema)) {
				trigger_localised_error('SYS0044', E_USER_WARNING, $schemaid);
				return FALSE;
			}
			$where .= '
						mv.fieldid IN (
						'.implode(' ', $GLOBALS['SQ_SYSTEM']->am->generateGetChildrenQuery($schema, 'metadata_field', FALSE, NULL, NULL, NULL, FALSE)).'
						) OR ';

		}// end foreach

		$where = trim($where, ' OR ').'
					)';

		$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 'mv');
		$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 'a');

		$metadata = $db->getAssoc($sql.$where);
		assert_valid_db_result($metadata);

		return $metadata;

	}//end getMetadata()


	/**
	* Get the default metadata values for a particular schema
	*
	* @param string	$schemaid	the ID of the schema to get default metadata for
	*
	* @return array
	* @access public
	*/
	function getSchemaDefaultValues($schemaid)
	{
		static $cached_field_names = Array();

		$am =& $GLOBALS['SQ_SYSTEM']->am;

		$schema =& $am->getAsset($schemaid);
		if (is_null($schema)) {
			trigger_localised_error('SYS0040', E_USER_WARNING, $schemaid);
			return Array();
		}

		$fields = $this->getMetadataFields(Array($schemaid));

		$default_values = Array();
		$field_names = Array();
		$fields_to_check = Array();

		foreach ($fields as $fieldid => $type_code) {
			if (isset($cached_field_names[$fieldid])) {
				$field_names[$fieldid] = $cached_field_names[$fieldid];
			} else {
				$fields_to_check[] = $fieldid;
			}
		}
		$new_field_names = $GLOBALS['SQ_SYSTEM']->am->getAssetInfo($fields_to_check, 'metadata_field', FALSE, 'name');
		$field_names += $new_field_names;

		foreach ($fields as $fieldid => $type_code) {
			if (!isset($cached_field_names[$fieldid])) {
				$cached_field_names[$fieldid] = $field_names[$fieldid];
			}

			$default_values[$fieldid] = Array(
											'name'	=> $field_names[$fieldid],
											'value'	=> $this->getMetadataFieldDefaultValue($fieldid),
										);
		}

		return $default_values;

	}//end getSchemaDefaultValues()


	/**
	* Generate an array of current values for the passed metadata field names
	*
	* If the passed fields array is empty, all field values will be returned
	*
	* @param string	$assetid		the ID of the asset to get field values for
	* @param array	$field_names	an array of field names
	*
	* @return array
	* @access public
	*/
	function getMetadataFieldValues($assetid, $field_names=Array())
	{
		$db =& $GLOBALS['SQ_SYSTEM']->db;
		$am =& $GLOBALS['SQ_SYSTEM']->am;

		$db->show_queries = TRUE;

		// the array of field values we are going to return
		$field_values = Array();

		$asset =& $am->getAsset($assetid);
		if (is_null($asset)) {
			trigger_localised_error('SYS0038', E_USER_WARNING, $assetid);
			return $field_values;
		}

		$schemas = $this->getSchemas($asset->id, TRUE);
		if (empty($schemas)) return $field_values;

		// get the values without any replacements or special processing
		$values = $this->getMetadata($asset->id);
		foreach ($schemas as $schemaid) {
			$schema =& $am->getAsset($schemaid);
			if (is_null($schema)) {
				trigger_localised_error('SYS0039', E_USER_WARNING, $schemaid);
				return $field_values;
			}

			if (!is_a($schema, 'metadata_schema')) {
				trigger_localised_error('SYS0037', E_USER_WARNING, $schema->name, $schemaid);
				return $field_values;
			}

			// Get all the fields that are in this schema (optionally restricted to the passed named fields)
			$sql_parts = $am->generateGetChildrenQuery($schema, 'metadata_field', FALSE);
			// if they only want specific fields, then restrict the search to those
			if (!empty($field_names)) {
				$sql_parts['where'] .= ' AND a.name IN ('.$db->quote($field_names[0]);
				for ($i = 1; $i < count($field_names); $i++) {
					$sql_parts['where'] .= ', '.$db->quote($field_names[$i]);
				}
				$sql_parts['where'] .= ')';
			}

			$fields = $db->getAssoc(implode(' ', $sql_parts));
			assert_valid_db_result($fields);

			foreach ($fields as $fieldid => $type_code) {

				$field =& $am->getAsset($fieldid, $type_code);

				if (is_null($field)) {
					trigger_localised_error('SYS0041', E_USER_WARNING, $schemaid);
					continue;
				}

				$value_str = $field->getMetadataValue($asset, (isset($values[$field->id]) ? $values[$field->id]['value'] : NULL));
				require_once SQ_CORE_PACKAGE_PATH.'/metadata/metadata_field/metadata_field.inc';
				Metadata_Field::decodeValueString($value_str, $value='', $components=Array());
				$field_values[$field->name] = $value;

				$am->forgetAsset($field);
				unset($field);

			}// end for

			$am->forgetAsset($schema);
			unset($schema);

		}//end foreach schema

		$am->forgetAsset($asset);
		unset($asset);

		return $field_values;

	}//end getMetadataFieldValues()


	/**
	* Set metadata values for an asset
	*
	* @param string	$assetid	the ID of the asset to set the values for
	* @param array	$metadata	the values to set
	*
	* @return boolean
	* @access public
	*/
	function setMetadata($assetid, $metadata)
	{
		$db =& $GLOBALS['SQ_SYSTEM']->db;

		$asset =& $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
		if (is_null($asset)) {
			trigger_localised_error('SYS0043', E_USER_WARNING, $assetid);
			return FALSE;
		}

		if (!$asset->writeAccess('metadata')) {
			trigger_localised_error('SYS0030', E_USER_WARNING, $asset->name);
			return FALSE;
		}

		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
		$db =& $GLOBALS['SQ_SYSTEM']->db;
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		// find all the fields...for two reasons.
		// 1. make sure they are editable
		// 2. we need to do some cleanup incase there has been a change in the fields that are available for this asset
		$quoted_fieldids = Array();
		foreach (array_keys($metadata) as $fieldid) {

			$field =& $GLOBALS['SQ_SYSTEM']->am->getAsset($fieldid);
			if (is_null($field) || !$field->attr('editable')) {
				unset($metadata[$fieldid]);
				continue;
			}
			$quoted_fieldids[] = $db->quoteSmart($fieldid);

		}//end foreach

		// Remove any old values...it's just easier than having to screw about with old fields and new fields, different assetids for the same name etc
		$where = '	assetid = '.$db->quoteSmart($asset->id);
		if (!empty($quoted_fieldids)) {
			$where .= ' AND fieldid NOT IN ('.implode(',', $quoted_fieldids).')';
		}

		$sql = 'DELETE FROM
					sq_ast_mdata_val
				WHERE
					'.$where;

		$result = $db->query($sql);
		assert_valid_db_result($result);

		if (!empty($quoted_fieldids)) {
			// Find any fields that have existing entries...so we know what to update and what to insert
			$sql = 'SELECT
						fieldid
					FROM
						'.SQ_TABLE_RUNNING_PREFIX.'ast_mdata_val';

			$where = '	assetid = '.$db->quote($asset->id).'
					 AND fieldid IN ('.implode(',', $quoted_fieldids).')';

			$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where);
			$updates = $db->getCol($sql.$where);
			assert_valid_db_result($updates);

			$sql = 'INSERT INTO
						sq_ast_mdata_val
						(
							assetid,
							fieldid,
							value
						)
						VALUES
						(
							?,
							?,
							?
						)';

			$insert_prepared = $db->prepare($sql);
			assert_valid_db_result($insert_prepared);

			$sql = 'UPDATE
						sq_ast_mdata_val
					SET
						value	= ?
					WHERE
							assetid	= ?
						AND	fieldid	= ?';

			$update_prepared = $db->prepare($sql);
			assert_valid_db_result($update_prepared);

			for (reset($metadata); NULL !== ($fieldid = key($metadata)); next($metadata)) {

				// we need to insert
				if (!in_array($fieldid, $updates)) {

					$values = Array(
								$asset->id,
								$fieldid,
								$metadata[$fieldid]['value'],
							  );

					$result = $db->execute($insert_prepared, $values);
					assert_valid_db_result($result);

				// we are updating
				} else {
					$values = Array(
								$metadata[$fieldid]['value'],
								$asset->id,
								$fieldid,
							  );

					$result = $db->execute($update_prepared, $values);
					assert_valid_db_result($result);

				}//end if

			}//end for

			$db->freePrepared($insert_prepared);
			$db->freePrepared($update_prepared);

		}//end if

		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();

		$asset =& $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
		$asset->_updated();
		$em =& $GLOBALS['SQ_SYSTEM']->getEventManager();
		// broadcasted the assetids of the updated fields
		$em->broadcastEvent($asset, 'MetadataUpdate', Array('fieldids' => array_keys($metadata)));
		$GLOBALS['SQ_SYSTEM']->broadcastTriggerEvent('trigger_event_metadata_updated', $asset);

		return TRUE;

	}//end setMetadata()


//--        (RE)GENERATE METADATA        --//


	/**
	* Generate a PHP file that can be included to print all metadata fields for an asset
	*
	* The generated file contains PHP code to do any keyword replacing based on
	* current values instead of the ones at the time of caching.
	*
	* @param string		$assetid	the ID of the asset to generate the file for
	* @param boolean	$regenerate	regenerate the DB information as well
	*
	* @return boolean
	* @access public
	*/
	function generateContentFile($assetid, $regenerate=FALSE)
	{
		$asset =& $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
		if (is_null($asset)) {
			trigger_localised_error('SYS0034', E_USER_WARNING, $assetid);
			return FALSE;
		}

		$schemas = $this->getSchemas($asset->id, TRUE);
		if (empty($schemas)) return FALSE;

		// an array of all metadata tag values
		$tag_values = Array();

		// an array of keyword replacements that we are going to have to write some PHP code to produce
		$keywords = Array();

		// get the values without any replacements or special processing
		// Also used if we are regenerating
		$metadata = $this->getMetadata($asset->id);

		ob_start();
		foreach ($schemas as $schemaid) {
			$schema =& $GLOBALS['SQ_SYSTEM']->am->getAsset($schemaid);
			if (is_null($schema)) {
				trigger_localised_error('SYS0035', E_USER_WARNING, $schemaid);
				return FALSE;
			}

			if (!is_a($schema, 'metadata_schema')) {
				trigger_localised_error('SYS0033', E_USER_WARNING, $schema->name, $schemaid);
				return FALSE;
			}

			$edit_fns = $schema->getEditFns();
			$edit_fns->generateMetadata($schema, $metadata, $tag_values, $keywords);

			$GLOBALS['SQ_SYSTEM']->am->forgetAsset($schema);
		}

		$metadata_str = ob_get_contents();
		ob_end_clean();

		ob_start();
			echo '<'.'?php'."\n";
			if (!empty($keywords)) {
				require_once SQ_FUDGE_PATH.'/general/text.inc';
				echo '$am =& $GLOBALS[\'SQ_SYSTEM\']->am;'."\n";
				echo '$metadata_asset = &$am->getAsset('.$asset->id.');'."\n\n";
				echo '$metadata_asset =& $GLOBALS[\'SQ_SYSTEM\']->am->getAsset('.$asset->id.');'."\n\n";
				echo '/* 1 */', "\n";
				$replacements = $this->generateKeywordReplacements($asset, $keywords);
				echo '/* 2 */', "\n";
				$tag_values = replace_keywords($tag_values, $replacements);
				echo '/* 3 */', "\n";
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
								$v = preg_replace('|(\$metadata_values)(\[\')('.preg_quote($field_name, '|').')(\'\])|', '$metadata_values_'.$i.'[\'\\3\']', $v);
								if ($i >= $nested_index) $nested_index = $i + 1;
								break;
							}
						}
					}
				}

				$nested_tag_values[$nested_index][$n] = $v;
				$tag_nested_index[$this->escapeMetadata($n, TRUE)] = $nested_index;
			}

			foreach ($nested_tag_values as $i => $values) {
				echo '$metadata_values_'.$i.' = Array(';
				foreach ($values as $n => $v) {
					$n = $this->escapeMetadata($n, TRUE);
					$v = stripslashes($v);
					$v = $this->escapeMetadata($v, TRUE);
					$v = preg_replace('|(\$metadata_replacements\[)\\\\\'([^\]]+)\\\\\'(\])|', '\'.\\1\'\\2\'\\3.\'', $v);
					$v = preg_replace('|(\$metadata_values[^\[]+\[)\\\\\'([^\]]+)\\\\\'(\])|', '\'.\\1\'\\2\'\\3.\'', $v);
					echo "'$n' => '$v',\n";
				}
				echo ");\n";
			}

			echo '?'.'>';
			$php_str = ob_get_contents();
		ob_end_clean();

		$metadata_matches = Array();
		preg_match_all('|\$metadata_values\[\'([^\]]+)\'\]|', $metadata_str, $metadata_matches);
		foreach ($metadata_matches[1] as $escaped_field_name) {
			$metadata_str = preg_replace('|\$metadata_values\[\''.preg_quote($escaped_field_name, '|').'\'\]|', '$metadata_values_'.$tag_nested_index[$escaped_field_name].'[\''.$escaped_field_name.'\']', $metadata_str);
		}

		$metadata_str = $php_str."\n\n".$metadata_str;

		require_once SQ_FUDGE_PATH.'/general/file_system.inc';
		if (!create_directory($asset->data_path)) return FALSE;
		if (!string_to_file($metadata_str, $asset->data_path.'/metadata.php')) {
			return FALSE;
		}

		if (!$regenerate) {
			return TRUE;
		} else {
			return $this->setMetadata($assetid, $metadata);
		}

	}//end generateContentFile()


	/**
	* Generate an array of keyword replacements, or the PHP code to generate it
	*
	* This function outputs the PHP required to generate all the keywords that
	* it is passed. All PHP generated places values in an array called $metadata_replacements
	* (which happens in the outputted PHP code). An array of replacements is returned that
	* can be used to replace all the keywords with a PHP string to output the correct value
	* from the $metadata_replacements array. Return format is (keyword => replacement)
	* This function can also return the values of the replacement values instead of the PHP code.
	*
	* @param object		&$metadata_asset	the asset to use for replacment values
	* @param array		$keywords			an array of keywords in
	* @param boolean	$generating			generate the PHP code or not (which generates the value)
	*
	* @return array
	* @access public
	*/
	function generateKeywordReplacements(&$metadata_asset, $keywords, $generating=TRUE)
	{
		$metadata_replacements = Array();
		$keywords = array_unique($keywords);

		if (!empty($keywords) && $generating) {
			echo '$metadata_replacements = Array();'."\n";
		}

		$thumbnail_generated = FALSE;

		foreach ($keywords as $keyword) {

			if ($generating) {
				$metadata_replacements[$keyword] = '$metadata_replacements['.var_export($keyword, 1).']';
			} else {
				ob_start();
				echo '$am =& $GLOBALS[\'SQ_SYSTEM\']->am;'."\n";
			}

			// Date/Time keywords are processed first.
			$replaced = FALSE;
			$prefix = Array(
						'asset_created',
						'asset_updated',
						'asset_status_changed',
						'asset_published',
					  );
			foreach ($prefix as $name) {
				if (strpos($keyword, $name) === 0) {
					// NOTE: These format strings contain an escape '\' character
					// unlike the ones in getKeywordReplecement() asset.inc since they
					// will be concatenated within eval string.
					$date_formats = Array(
										'short'				=> 'Y-m-d',
										'readable'			=> 'd M Y g:ia',
										'readabledate'		=> 'd M Y',
										'readabletime'		=> 'g:ia',
										'iso8601'			=> 'Y-m-d\\TH:i:s',
										'rfc2822'			=> 'r',
										'rfc2822-dateonly'	=> 'D, d M Y',
										'ical'				=> 'Ymd\\THis',
									);
					if ($keyword == $name) {
						$default_format = 'Y-m-d H:i:s';
						if ($keyword == 'asset_published') {
							echo '$metadata_replacements['.var_export($keyword, 1).'] = (is_null($metadata_asset->published)) ? \'Never\' : date(\''.$default_format.'\', $metadata_asset->published);'."\n";
						} else {
							echo '$metadata_replacements['.var_export($keyword, 1).'] = date(\''.$default_format.'\', $metadata_asset->'.substr($keyword,6).');'."\n";
						}
						$replaced = TRUE;
					} else {
						$format_name = substr($keyword, strlen($name.'_'));
						if (array_key_exists($format_name, $date_formats)) {
							if ($keyword == 'asset_published') {
								echo '$metadata_replacements['.var_export($keyword, 1).'] = (is_null($metadata_asset->published)) ? \'Never\' : date(\''.$date_formats[$format_name].'\', $metadata_asset->published);'."\n";
							} else {
								if ($format_name == 'iso8601') {
									echo '$metadata_replacements['.var_export($keyword, 1).'] = date(\''.$date_formats[$format_name].'\', $metadata_asset->'.substr($keyword, strlen('asset_'), strlen($keyword) - strlen('_'.$format_name) - strlen('asset_')).').substr(date(\'O\'),0,3).\':\'.substr(date(\'O\'),-2);'."\n";
								} else {
									echo '$metadata_replacements['.var_export($keyword, 1).'] = date(\''.$date_formats[$format_name].'\', $metadata_asset->'.substr($keyword, strlen('asset_'), strlen($keyword) - strlen('_'.$format_name) - strlen('asset_')).');'."\n";
								}
							}
							$replaced = TRUE;
						}
					}
				}//end if
			}//end foreach which loops through $prefix array

			if (!$replaced) {
				switch ($keyword) {

					case 'asset_name'       :
					case 'asset_short_name' :
					case 'asset_version'    :
						echo '$metadata_replacements['.var_export($keyword, 1).'] = $metadata_asset->'.substr($keyword,6).';'."\n";
					break;

					case 'asset_version_major'    :
						echo 'list($metadata_replacements['.var_export($keyword, 1).'],,) = explode(\'.\',$metadata_asset->version);'."\n";
					break;

					case 'asset_version_minor'    :
						echo 'list(,$metadata_replacements['.var_export($keyword, 1).'],) = explode(\'.\',$metadata_asset->version);'."\n";
					break;

					case 'asset_version_micro'    :
						echo 'list(,,$metadata_replacements['.var_export($keyword, 1).']) = explode(\'.\',$metadata_asset->version);'."\n";
					break;

					case 'asset_url' :
						echo '$metadata_replacements['.var_export($keyword, 1).'] = $metadata_asset->getURL();'."\n";
					break;

					case 'asset_href' :
						echo '$metadata_replacements['.var_export($keyword, 1).'] = $metadata_asset->getHref();'."\n";
					break;

					case 'asset_thumbnail' :
					case 'asset_thumbnail_url' :
					case 'asset_thumbnail_caption' :
						if (!$thumbnail_generated) {
							echo '$thumbnail = null;';
							echo 'if ($metadata_asset->id) {';
							echo '    $link = $GLOBALS[\'SQ_SYSTEM\']->am->getLink($metadata_asset->id, SQ_LINK_NOTICE, \'image\', FALSE, \'thumbnail\');'."\n";
							echo '    if (!empty($link)) {'."\n";
							echo '        $thumbnail =& $GLOBALS[\'SQ_SYSTEM\']->am->getAsset($link[\'minorid\'], $link[\'minor_type_code\']);'."\n";
							echo '    }'."\n";
							echo '}'."\n";
						}
						switch ($keyword) {
							case 'asset_thumbnail' :
								echo '$metadata_replacements['.var_export($keyword, 1).'] = (is_null($thumbnail)) ? \'\' : $thumbnail->printImageTag(\'\', \'\', TRUE);'."\n";
							break;
							case 'asset_thumbnail_url' :
								echo '$metadata_replacements['.var_export($keyword, 1).'] = (is_null($thumbnail)) ? \'\' : $thumbnail->getURL();'."\n";
							break;
							case 'asset_thumbnail_caption' :
								echo '$metadata_replacements['.var_export($keyword, 1).'] = (is_null($thumbnail)) ? \'\' : $thumbnail->attr(\'caption\');'."\n";
							break;
						}
					break;

					case 'asset_created_by_name' :
					case 'asset_updated_by_name' :
					case 'asset_status_changed_by_name':
						$field_name = substr($keyword, strlen('asset_'), strlen($keyword) - strlen('_by_name') - strlen('asset_'));
						echo 'if (!isset($metadata_'.$field_name.'_by_user) && !empty($metadata_asset->'.$field_name.'_userid)) {'."\n";
						echo '	$metadata_'.$field_name.'_by_user =& $GLOBALS[\'SQ_SYSTEM\']->am->getAsset($metadata_asset->'.$field_name.'_userid, \'\', TRUE);'."\n";
						echo '}'."\n";
						echo 'if (!empty($metadata_'.$field_name.'_by_user)) {'."\n";
						echo '	$metadata_replacements['.var_export($keyword, 1).'] = $metadata_'.$field_name.'_by_user->name;'."\n";
						echo '} else {'."\n";
						echo '	$metadata_replacements['.var_export($keyword, 1).'] = \''.translate('unknown').'\';'."\n";
						echo '}'."\n";
					break;

					case 'asset_created_by_first_name' :
					case 'asset_created_by_last_name'  :
					case 'asset_created_by_email'      :
					case 'asset_updated_by_first_name' :
					case 'asset_updated_by_last_name'  :
					case 'asset_updated_by_email'      :
						echo 'if (!isset($metadata_'.substr($keyword,6,7).'_by_user)) {
						$metadata_'.substr($keyword,6,7).'_by_user =& $GLOBALS[\'SQ_SYSTEM\']->am->getAsset($metadata_asset->'.substr($keyword,6,7).'_userid,\'\', TRUE);'."\n}\n";
						echo 'if (!empty($metadata_'.substr($keyword,6,7).'_by_user)) { ';
						echo '$metadata_replacements['.var_export($keyword, 1).'] = $metadata_'.substr($keyword,6,7).'_by_user->attr(\''.substr($keyword,17).'\');'."\n";
						echo '} else {'."\n";
						echo '$metadata_replacements['.var_export($keyword, 1).'] = \''.translate('unknown').'\';'."\n";
						echo '}'."\n";

					break;

					// can't handle 'last published' stuff in the same way as created/updated because
					// there is a chance that it has never been published
					case 'asset_published_by_name' :
						echo 'if (empty($metadata_asset->published_userid)) {'."\n";
						echo '$metadata_replacements['.var_export($keyword, 1).'] = \'Never Published\';'."\n";
						echo '} else { if (!isset($metadata_published_by_user)) {
						$metadata_published_by_user =& $GLOBALS[\'SQ_SYSTEM\']->am->getAsset($metadata_asset->published_userid, \'\', TRUE);'."\n}\n";
						echo 'if (!empty($metadata_published_by_user)) { ';
						echo '$metadata_replacements['.var_export($keyword, 1).'] = $metadata_published_by_user->name;'."\n";
						echo '} else {'."\n";
						echo '$metadata_replacements['.var_export($keyword, 1).'] = \''.translate('unknown').'\';'."\n";
						echo '}}'."\n";
					break;

					case 'asset_published_by_first_name' :
					case 'asset_published_by_last_name'  :
					case 'asset_published_by_email'      :
						echo 'if (empty($metadata_asset->published_userid)) {'."\n";
						echo '$metadata_replacements['.var_export($keyword, 1).'] = \'\';'."\n";
						echo '} else { if (!isset($metadata_published_by_user)) {
						$metadata_published_by_user =& $GLOBALS[\'SQ_SYSTEM\']->am->getAsset($metadata_asset->published_userid, \'\', TRUE);'."\n}\n";
						echo 'if (!empty($metadata_published_by_user)) { ';
						echo '$metadata_replacements['.var_export($keyword, 1).'] = $metadata_published_by_user->attr(\''.substr($keyword,19).'\');'."\n";
						echo '} else {'."\n";
						echo '$metadata_replacements['.var_export($keyword, 1).'] = \''.translate('unknown').'\';'."\n";
						echo '}}'."\n";

					break;

					case 'asset_read_permission'  :
					case 'asset_write_permission' :
					case 'asset_admin_permission' :
					case 'asset_read_permission_email'  :
					case 'asset_write_permission_email' :
					case 'asset_admin_permission_email' :
					case 'asset_read_permission_email_linked'  :
					case 'asset_write_permission_email_linked' :
					case 'asset_admin_permission_email_linked' :
						$perm_type = preg_match('/asset_([^_]*)_.*$/', $keyword, $matches);
						$perm_type = $matches[1];
						echo '$metadata_'.$perm_type.'_perms = $am->getPermission($metadata_asset->id, SQ_PERMISSION_'.strtoupper($perm_type).', TRUE, FALSE, TRUE);'."\n";
						echo '$metadata_'.$perm_type.'_perm_info = Array();'."\n";
						echo 'foreach ($metadata_'.$perm_type.'_perms as $userid) {'."\n";
						echo '	$user = &$am->getAsset($userid);'."\n";
						echo '	if (is_null($user) || !$am->isTypeDecendant($user->type(), \'user\')) {'."\n";
						echo '		continue;'."\n";
						echo '	}'."\n";
						if (preg_match('/.*_email$/', $keyword)) {
							echo '$metadata_'.$perm_type.'_perm_info[] = $user->attr(\'email\');'."\n";
						} else if (preg_match('/.*_email_linked/', $keyword)) {
							echo '$email = $user->attr(\'email\');'."\n";
							echo '$metadata_'.$perm_type.'_perm_info[] = \'<a href="mailto:\'.$email.\'">\'.$email.\'</a>\';'."\n";
						} else {
							echo '$metadata_'.$perm_type.'_perm_info[] = $user->name;'."\n";
						}
						echo '}'."\n";
						echo '$metadata_replacements['.var_export($keyword, 1).'] = implode(\', \', $metadata_'.$perm_type.'_perm_info);'."\n";
					break;
					case 'system_name'   :
					case 'system_owner'  :
					case 'default_email' :
					case 'tech_email'    :
						echo '$metadata_replacements['.var_export($keyword, 1).'] = SQ_CONF_'.strtoupper($keyword).';'."\n";
					break;

					default :
						if (substr($keyword, 0, 11) == 'asset_role_') {
							echo '$metadata_roles_info = Array();'."\n";
							if (preg_match('/asset_role_([^_]*)(.*)$/', $keyword, $matches)) {
								$role_assetid = $matches[1];
								echo '$metadata_roles = $am->getRole($metadata_asset->id, '.$role_assetid.', NULL, FALSE, TRUE, TRUE);'."\n";
								echo 'if (!empty($metadata_roles)) {'."\n";
								echo '	foreach ($metadata_roles['.$role_assetid.'] as $role_userid) {'."\n";
								echo '		$user = $GLOBALS[\'SQ_SYSTEM\']->am->getAsset($role_userid);'."\n";
								echo '		if (is_null($user) || !$am->isTypeDecendant($user->type(), \'user\')) {'."\n";
								echo '			continue;'."\n";
								echo '		}'."\n";
								echo '		$email = $user->attr(\'email\');'."\n";
								if (!empty($matches[2]) && $matches[2] == '_email') {
								echo '		$metadata_roles_info[] = $email;'."\n";
								} else if (!empty($matches[2]) && $matches[2] == '_email_linked') {
								echo '		$metadata_roles_info[] = \'<a href="mailto:\'.$email.\'">\'.$email.\'</a>\';'."\n";
								} else {
								echo '		$metadata_roles_info[] = $user->name;'."\n";
								}
								echo '		$am->forgetAsset($user);'."\n";
								echo '	}'."\n";
								echo '}'."\n";
							}
							echo '		$metadata_replacements['.var_export($keyword, 1).'] = implode(\', \', $metadata_roles_info);'."\n";

							// searching for keywords that replace the content of another field
						} else if (substr($keyword, 0, 15) == 'metadata_field_') {
							$field_name = substr($keyword, 15);
							$metadata_replacements[$keyword] = '$metadata_values[\''.$field_name.'\']';

						} else if (substr($keyword, 0, 18) == 'asset_thumbnail_v_') {
							// Print the asset variety of a thumbnail
							$variety_name = substr($keyword, 18);
							if (!$thumbnail_generated) {
								echo '$thumbnail = null;'."\n";
								echo 'if ($metadata_asset->id) {'."\n";
								echo '    $link = $GLOBALS[\'SQ_SYSTEM\']->am->getLink($metadata_asset->id, SQ_LINK_NOTICE, \'image\', FALSE, \'thumbnail\');'."\n";
								echo '    if (!empty($link)) {'."\n";
								echo '        $thumbnail =& $GLOBALS[\'SQ_SYSTEM\']->am->getAsset($link[\'minorid\'], $link[\'minor_type_code\']);'."\n";
								echo '    }'."\n";
								echo '}'."\n";
							}

							echo '$metadata_replacements['.var_export($keyword, 1).'] = (is_null($thumbnail)) ? \'\' : $thumbnail->getKeywordReplacement(\'image_v_'.$variety_name.'\');';

						} else {
							// searching for special date format keywords
							$original = $keyword;
							$keyword = substr($keyword, 6);
							$key     = substr($keyword, 0, strpos($keyword, '_'));
							$extras  = substr($keyword, strpos($keyword, '_') +1);

							if (strpos($extras, '_') === FALSE) {
								$format = $extras;
								$offset = '';
							} else {
								list ($format, $offset) = explode('_', $extras);
							}
							$format = str_replace('~', ' ', $format);

							switch ($key) {
								case 'created':
								case 'updated':
								case 'published':
									if (empty($offset)) {
										$date = '$metadata_asset->'.$key;
									} else {
										$date = 'strtotime(\''.$offset.'\', $metadata_asset->'.$key.')';
									}
									echo '$metadata_replacements[\''.$original.'\'] = date(\''.$format.'\', '.$date.');'."\n";
								break;
							}
						}//end if
					break;
				}//end switch
			}//end if

			if (!$generating) {
				eval(ob_get_contents());
				ob_end_clean();
			}
		}//end foreach

		return $metadata_replacements;

	}//end generateKeywordReplacements()


	/**
	* Regenerate both the cached tag metadata and the serialised DB values
	*
	* @param string	$assetid	the ID of the asset to regenerate metadata for
	*
	* @return boolean
	* @access public
	*/
	function regenerateMetadata($assetid)
	{
		return $this->generateContentFile($assetid, TRUE);

	}//end regenerateMetadata()


	/**
	* Removes the metadata entries for the specified asset
	*
	* @param string	$assetid	the assetid of the asset to purge
	*
	* @return boolean
	* @access public
	*/
	function purgeMetadata($assetid)
	{
		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
		$db =& $GLOBALS['SQ_SYSTEM']->db;
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		$sql = 'DELETE FROM
					sq_ast_mdata
				WHERE
					assetid	= '.$db->quoteSmart($assetid);

		$result = $db->query($sql);
		assert_valid_db_result($result);

		$sql = 'DELETE FROM
					sq_ast_mdata_val
				WHERE
					assetid	= '.$db->quoteSmart($assetid);

		$result = $db->query($sql);
		assert_valid_db_result($result);

		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();

		return TRUE;

	}//end purgeMetadata()


	/**
	* Get the value of an asset's metadata field based on that field's asset ID
	*
	* @param string		$assetid		The ID of the asset that we want the
	*									metadata value of
	* @param string		$field_assetid	The field that we wish to check
	* @param boolean	$mute_errors	Set TRUE to mute errors that occur when
	*									the asset (specified in $assetid) does
	*									not have a schema applied to it
	*
	* @return array
	* @access public
	*/
	function getMetadataValueByAssetid($assetid, $field_assetid, $mute_errors=FALSE)
	{
		// is this actually a metadata field or what?!
		if (!$this->isMetadataFieldAssetid($field_assetid)) {
			if (!$mute_errors) {
				trigger_localised_error('SYS0308', E_USER_WARNING, $field_assetid);
			}
			return NULL;
		}

		// get the parent schema of this metadata field
		$schemaid = $GLOBALS['SQ_SYSTEM']->am->getParents($field_assetid, 'metadata_schema');

		// get the schemas that apply to this asset
		$schemas = $this->getSchemas($assetid, TRUE);

		if (array_intersect($schemas, array_keys($schemaid)) == Array()) {
			if (!$mute_errors) {
				// SYS0307: Asset #<assetid> does not have the schema for field #<field assetid> applied
				trigger_localised_error('SYS0307', E_USER_WARNING, $assetid, $field_assetid);
			}
			return NULL;
		}

		$db =& $GLOBALS['SQ_SYSTEM']->db;

		$sql = 'SELECT
					value
				FROM
					'.SQ_TABLE_RUNNING_PREFIX.'ast_mdata_val mdv';
		$where = 'mdv.assetid	= '.$db->quoteSmart($assetid).'
					AND	mdv.fieldid	= '.$db->quoteSmart($field_assetid);
		$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 'mdv');

		$value = $db->getOne($sql.$where);
		assert_valid_db_result($value);

		if (is_null($value)) {
			// not set - need the default value
			return $this->getMetadataFieldDefaultValue($field_assetid);
		} else {
			return $value;
		}

	}//end getMetadataValueByAssetid()


	/**
	* Returns whether the asset ID being passed into this function is some kind
	* of metadata field asset
	*
	* @param string	$field_assetid	the ID of the metadata field asset
	*
	* @return boolean
	* @access public
	*/
	function isMetadataFieldAssetid($field_assetid)
	{
		static $cached_tests = Array();

		if (!isset($cached_tests[$field_assetid])) {
			$db =& $GLOBALS['SQ_SYSTEM']->db;

			$sql = 'SELECT
						COUNT(*)
					FROM
						sq_ast_typ_inhd ati
							JOIN '.SQ_TABLE_RUNNING_PREFIX.'ast a ON ati.type_code = a.type_code';
			$where = 'a.assetid = '.$db->quoteSmart($field_assetid).'
						AND	ati.inhd_type_code	= '.$db->quoteSmart('metadata_field');
			$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 'a');

			$count = $db->getOne($sql.$where);
			assert_valid_db_result($count);
			$cached_tests[$field_assetid] = ($count > 0);
		}

		return $cached_tests[$field_assetid];

	}//end isMetadataFieldAssetid()


	/**
	* Returns whether the asset ID being passed into this function is some kind
	* of metadata field asset
	*
	* @param string	$field_assetid	the ID of the metadata field asset
	*
	* @return boolean
	* @access public
	*/
	function getMetadataFieldDefaultValue($field_assetid)
	{
		static $cached_defaults = Array();
		if (!in_array($field_assetid, array_keys($cached_defaults))) {
			if (!$this->isMetadataFieldAssetid($field_assetid)) {
				trigger_localised_error('SYS0308', E_USER_WARNING, $field_assetid);
			}

			$db =& $GLOBALS['SQ_SYSTEM']->db;

			$sql = 'SELECT
						default_val
					FROM
						'.SQ_TABLE_RUNNING_PREFIX.'ast_mdata_dflt_val mdv';
			$where = 'mdv.assetid = '.$db->quoteSmart($field_assetid);
			$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 'mdv');

			// if not found, getOne() will return null
			$default_val = $db->getOne($sql.$where);
			assert_valid_db_result($default_val);
			$cached_defaults[$field_assetid] = $default_val;
		}

		return $cached_defaults[$field_assetid];

	}//end getMetadataFieldDefaultValue()


	/**
	* Sets the default value of a certain metadata field asset (as identified
	* by assetid)
	*
	* @param string	$field_assetid	the ID of the metadata field asset
	* @param string	$value			the value to be stored
	*
	* @return boolean
	* @access public
	*/
	function setMetadataFieldDefaultValue($field_assetid, $value)
	{
		// is this actually a metadata field or what?!
		if (!$this->isMetadataFieldAssetid($field_assetid)) {
			trigger_localised_error('SYS0308', E_USER_WARNING, $field_assetid);
		}

		$db =& $GLOBALS['SQ_SYSTEM']->db;

		$sql = 'SELECT
					default_val
				FROM
					'.SQ_TABLE_RUNNING_PREFIX.'ast_mdata_dflt_val mdv';
		$where = 'mdv.assetid = '.$db->quoteSmart($field_assetid);
		$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 'mdv');

		$row = $db->getRow($sql.$where);
		assert_valid_db_result($row);

		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
		$db =& $GLOBALS['SQ_SYSTEM']->db;
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		// does the default value exist yet, then?
		if (is_null($row)) {
			$sql = 'INSERT INTO
						sq_ast_mdata_dflt_val
						(
							assetid,
							default_val
						)
					VALUES
						(
							'.$db->quoteSmart($field_assetid).',
							'.$db->quoteSmart($value).'
						)';
		} else {
			$sql = 'UPDATE
						sq_ast_mdata_dflt_val
					SET
						default_val = '.$db->quoteSmart($value).'
					WHERE
						assetid = '.$db->quoteSmart($field_assetid);
		}

		$value = $db->query($sql);
		assert_valid_db_result($value);

		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();

		return TRUE;

	}//end setMetadataFieldDefaultValue()


	/**
	* Escapes the passed value to be used inside quotes in a block of PHP (eg.
	* when the field names and values are cached out to each asset's metadata.php)
	*
	* @var string	$value			the value to escape
	* @var boolean	$escape_single	whether or not the apostrophes are escaped with backslashes; defaults to FALSE
	*
	* @return string
	* @access public
	*/
	function escapeMetadata($value, $escape_single=FALSE)
	{
		if ($escape_single) {
			$value = str_replace("'", "\'", $value);
		}

		// escape only html-specific chars for the moment
		$value = htmlspecialchars($value);

		// escape everything else (chars > 126)
		$result = '';
		for($i = 0; $i < strlen($value); ++$i) {
			$ord = ord($value[$i]);
			if($ord > 126) {
				$result .= "&#".$ord.";";
			} else {
				$result .= $value[$i];
			}
		}

		return $result;

	}//end escapeMetadataField()


}//end class

?>
