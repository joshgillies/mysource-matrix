<?php
/**
* Copyright (c) 2003 - Squiz Pty Ltd
*
* $Id: metadata_manager.inc,v 1.7 2003/10/02 01:35:32 gsherwood Exp $
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

		if (!$asset->adminAccess(Array(), true, false)) {
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

		if (!$asset->adminAccess(Array(), true, false)) {
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

		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		return true;

	}//end deleteSchema()


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

		return unserialize($metadata);

	}//end getSchemaMetadata()


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

		if (!$asset->adminAccess(Array(), true, false)) {
			trigger_error('Cannot set metadata for "'.$asset->name.'", permission denied', E_USER_WARNING);
			return false;
		}

		$schemas = $this->getSchemas($asset->id, true);
		if (empty($schemas)) return false;

		$db = &$GLOBALS['SQ_SYSTEM']->db;
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		foreach ($schemas as $schemaid) {
			$schema = &$GLOBALS['SQ_SYSTEM']->am->getAsset($schemaid);
			if (is_null($schema)) {
				trigger_error('Failed setting metadata, Schema #'.$schemaid.' does not exist', E_USER_WARNING);
				return false;
			}

			if (!is_a($schema, 'metadata_schema')) {
				trigger_error('Failed setting metadata, "'.$schema->name.'" (Asset #'.$schemaid.') is not a metadata schema', E_USER_WARNING);
				return false;
			}

			if (!isset($metadata[$schema->name])) continue;

			$values = Array('metadata' => $db->quote(serialize($metadata[$schema->name])));
			$where = 'assetid = '.$db->quote($asset->id).'
					    AND schemaid = '.$db->quote($schema->id);
			if (!$GLOBALS['SQ_SYSTEM']->rollbackUpdate('asset_metadata', $values, $where)) {
				$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
				return false;
			}
		}

		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		return true;

	}//end setMetadata()


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


	/**
	* Generate a PHP file that can be included to print all metadata fields for an asset
	*
	* The generated file contains PHP code to do any keyword replacing based on
	* current values instead of the ones at the time of caching.
	*
	* @param int	$assetid	the ID of the asset to generate the fle for
	*
	* @access public
	* @return boolean
	*/
	function generateContentFile($assetid)
	{
		$assetid = (int) $assetid;

		$asset = &$GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
		if (is_null($asset)) {
			trigger_error('Failed caching metadata, Asset #'.$assetid.' does not exist', E_USER_WARNING);
			return false;
		}

		$schemas = $this->getSchemas($asset->id, true);
		if (empty($schemas)) return false;

		// an array of keyword replacements that we are going to have to write some PHP code to produce
		$keywords = Array();

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

			$values = $this->getSchemaMetadata($asset->id, $schema->id);
			$edit_fns = $schema->getEditFns();
			$edit_fns->generateMetadata($schema, $values, $keywords);
		}

		$metadata = ob_get_contents();
		ob_end_clean();

		if (!empty($keywords)) {
			ob_start();
			echo '<'.'?php'."\n";
			echo '$metadata_asset = &$GLOBALS[\'SQ_SYSTEM\']->am->getAsset('.$asset->id.');'."\n\n";
			$replacements = $this->generateKeywordReplacements($asset, $keywords);
			echo '?'.'>';
			$php = ob_get_contents();
			ob_end_clean();
			$metadata = replace_keywords($metadata, $replacements);
			$metadata = $php."\n\n".$metadata;
		}

		require_once SQ_FUDGE_PATH.'/general/file_system.inc';
		create_directory($asset->data_path);
		return string_to_file($metadata, $asset->data_path.'/metadata.php');

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
	* @param object Asset	&$asset		the asset to use for replacment values
	* @param array			$keywords	an array of keywords in
	*
	* @access public
	* @return array(keyword => replacement)
	*/
	function generateKeywordReplacements(&$asset, $keywords)
	{
		$replacements = Array();
		$keywords = array_unique($keywords);

		if (!empty($keywords)) echo '$metadata_replacements = Array();';

		foreach ($keywords as $keyword) {
			$replacements[$keyword] = '<'.'?php echo $metadata_replacements[\''.$keyword.'\']; ?'.'>';
			switch ($keyword) {

				case 'asset_name'       :
				case 'asset_short_name' :
				case 'asset_version'    :
					echo '$metadata_replacements[\''.$keyword.'\'] = $metadata_asset->'.substr($keyword,6).';'."\n";
				break;

				case 'asset_url' :
					echo '$metadata_replacements[\''.$keyword.'\'] = $metadata_asset->getURL();'."\n";
				break;

				case 'asset_created' :
				case 'asset_updated' :
					echo '$metadata_replacements[\''.$keyword.'\'] = date(\'Y-m-d H:i:s\', $metadata_asset->'.substr($keyword,6).');'."\n";
				break;

				case 'asset_created_short' :
				case 'asset_updated_short' :
					echo '$metadata_replacements[\''.$keyword.'\'] = date(\'Y-m-d\', $metadata_asset->'.substr($keyword,6,7).');'."\n";
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
					echo '$metadata_replacements[\''.$keyword.'\'] = \'\';'."\n";
				break;

			}//end switch

			echo "\n";
		}

		return $replacements;

	}//end generateKeywordReplacements()

}//end class

?>