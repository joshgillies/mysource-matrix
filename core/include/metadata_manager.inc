<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd	   |
* | ACN 084 670 600													   |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.com.au) so we may provide|
* | you a copy.														   |
* +--------------------------------------------------------------------+
*
* $Id: metadata_manager.inc,v 1.246 2013/09/20 08:34:34 cupreti Exp $
*
*/

require_once SQ_FUDGE_PATH.'/general/text.inc';

/**
* Metadata_Manager
*
* Purpose
*	 Manages the editing of metadata in the system
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Revision: 1.246 $
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
		$asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
		if (is_null($asset)) return TRUE;

		// if this asset does not allow the user to set metadata values then
		// its okay to assume that we dont need check if schema is complete
		if (!$this->allowsMetadata($assetid)) return TRUE;

		$schemas = $this->getSchemas($asset->id, TRUE);
		if (empty($schemas)) return TRUE;

		$values = $this->getMetadata($asset->id);

		foreach ($schemas as $schemaid) {
			$schema = $GLOBALS['SQ_SYSTEM']->am->getAsset($schemaid);
			if (is_null($schema)) continue;
			if (!($schema instanceof Metadata_Schema)) continue;
			$edit_fns = $schema->getEditFns();
			if (!$edit_fns->requiredFieldsComplete($schema, $values, $asset->type())) {
				return FALSE;
			}
		}

		return TRUE;

	}//end requiredFieldsComplete()


	/**
	* Check if metadata can be set on an asset
	*
	* @param string	$assetid	the ID of the asset to check
	*
	* @return boolean
	* @access public
	*/
	function allowsMetadata($assetid)
	{
		$asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
		if (is_null($asset)) return FALSE;
		$edit_fns = $asset->getEditFns();
		return isset($edit_fns->static_screens['metadata']);

	}//end allowsMetadata()


//--		SCHEMAS		   --//


	/**
	* Get an array of schemaIDs for all schemas applied and denied on an asset
	*
	* if $granted is null returns Array(schemaid)
	* otherwise returns Array(schemaid => granted[1|0])
	*
	* @param string		$assetid	the ID of the asset to get schemas for
	* @param boolean	$granted	type of access : null = all, TRUE = applied, FALSE = denied
	* @param boolean	$cascades	cascade to children : null = all, TRUE = cascaded, FALSE = not cascaded
	* @param boolean	$sort_name	 sort by alphabetic order of schema name, default FALSE
	*
	* @return array
	* @access public
	*/
	function getSchemas($assetid, $granted=NULL, $cascades=NULL, $sort_name=FALSE)
	{
		// If its shadow assetid, get the schema of the bridge
		$assetid_parts = explode(':', $assetid);
		if (isset($assetid_parts[1])) {
			$assetid = $assetid_parts[0];
		}

		if($sort_name) {
		    $sql = '  SELECT DISTINCT m.schemaid, m.granted, m.cascades, a.name
				  FROM '.SQ_TABLE_RUNNING_PREFIX.'ast_mdata m,  '.SQ_TABLE_RUNNING_PREFIX.'ast a';
		    $where = 'm.schemaid = a.assetid AND m.assetid = :assetid';
		    $order_by = ' ORDER BY a.name ASC';
		}
		else {
		    $sql = '  SELECT DISTINCT schemaid, granted, cascades
				      FROM '.SQ_TABLE_RUNNING_PREFIX.'ast_mdata m';
		    $where = 'assetid = :assetid';
		    $order_by = '';
		}
		if (!is_null($granted)) {
			$where .= ' AND granted = :granted';
		}
		if (!is_null($cascades)) {
			$where .= ' AND cascades = :cascades';
		}
		$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 'm');

		try {
			$query = MatrixDAL::preparePdoQuery($sql.$where.$order_by);
			MatrixDAL::bindValueToPdo($query, 'assetid', $assetid);
			if (!is_null($granted)) {
				MatrixDAL::bindValueToPdo($query, 'granted', $granted ? '1' : '0', PDO::PARAM_STR);
			}
			if (!is_null($cascades)) {
				MatrixDAL::bindValueToPdo($query, 'cascades', $cascades ? '1' : '0', PDO::PARAM_STR);
			}
			$result = DAL::executePdoAll($query);
		} catch (Exception $e) {
			throw new Exception('Unable to get applied schemas for asset ID #'.$assetid.' due to database error: '.$e->getMessage());
		}

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
	* Get list of schemas for a certain asset, optionally filtered by cascading setting
	*
	* This is primarily used for showing settings on the Metadata Schemas
	* screen.
	*/
	function getAssetSchemaInfo($assetid, $schemaid=NULL, $cascades=NULL, $include_cascades=TRUE, $sort_name=FALSE)
	{
		if($sort_name) {
		    $sql = '  SELECT DISTINCT m.schemaid, m.granted, m.cascades, a.name
				  FROM '.SQ_TABLE_RUNNING_PREFIX.'ast_mdata m,  '.SQ_TABLE_RUNNING_PREFIX.'ast a';
		    $where = 'm.schemaid = a.assetid AND m.assetid = :assetid';
		    $order_by = ' ORDER BY a.name ASC';
		}
		else {
		    $sql = '  SELECT DISTINCT schemaid, granted, cascades
				  FROM '.SQ_TABLE_RUNNING_PREFIX.'ast_mdata m';
		    $where = 'assetid = :assetid';
		    $order_by = '';
		}
		
		if (!is_null($schemaid)) {
			$where .= ' AND schemaid = :schemaid';
		}
		if (!is_null($cascades)) {
			$where .= ' AND cascades = :cascades';
		}
		$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 'm');

		try {
			$query = MatrixDAL::preparePdoQuery($sql.$where.$order_by);
			MatrixDAL::bindValueToPdo($query, 'assetid', $assetid, PDO::PARAM_STR);
			if (!is_null($schemaid)) {
				MatrixDAL::bindValueToPdo($query, 'schemaid', $schemaid, PDO::PARAM_STR);
			}
			if (!is_null($cascades)) {
				MatrixDAL::bindValueToPdo($query, 'cascades', $cascades ? '1' : '0', PDO::PARAM_STR);
			}
			$result = MatrixDAL::executePdoAssoc($query);
		} catch (Exception $e) {
			throw new Exception('Unable to get asset metadata schemas of asset ID #'.$assetid.' due to database error: '.$e->getMessage());
		}

		$schemas = Array();
		foreach ($result as $data) {
			if ($include_cascades) {
				$schemas[$data['schemaid']] = Array(
												'granted'	=> $data['granted'],
												'cascades'	=> $data['cascades'],
											  );
			} else {
				$schemas[$data['schemaid']] = $data['granted'];
			}
		}

		// If a schemaid is specified, flatten out array
		if (!is_null($schemaid)) {
			if (array_key_exists($schemaid, $schemas)) {
				$schemas = $schemas[$schemaid];
			} else {
				$schemas = Array();
			}
		}

		return $schemas;

	}//end getAssetSchemaInfo()


	/**
	* Apply a schema to or Deny from, an asset
	*
	* @param string		$assetid	the ID of the asset to set the schema on
	* @param string		$schemaid	the ID of the schema to set
	* @param boolean	$granted	is this schema applied (TRUE) or denied (FALSE)?
	* @param boolean	$cascades	does this schema cascade to newly created assets?
	* @param boolean	$force_set	if true, existing metadata schemas will be updated regardless of whether or not the new metadata schemas are found to conflict with them
	*
	* @return boolean
	* @access public
	*/
	function setSchema($assetid, $schemaid, $granted, $cascades=TRUE, $force_set=FALSE)
	{
		$db_action = 'insert';
		$schema = $GLOBALS['SQ_SYSTEM']->am->getAsset($schemaid);
		if (is_null($schema)) {
			trigger_localised_error('SYS0047', E_USER_WARNING, $schemaid);
			return FALSE;
		}

		if (!($schema instanceof Metadata_Schema)) {
			trigger_localised_error('SYS0045', E_USER_WARNING, $schema->name, $schemaid);
			return FALSE;
		}

		$asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
		if (is_null($asset)) {
			trigger_localised_error('SYS0046', E_USER_WARNING, $assetid);
			return FALSE;
		}

		if (!$asset->adminAccess('metadata')) {
			trigger_localised_error('SYS0031', E_USER_WARNING, $asset->name);
			return FALSE;
		}

		// open a queue for all messages we are going to be logging
		$ms = $GLOBALS['SQ_SYSTEM']->getMessagingService();
		$ms->openLog();

		// get the current schemas that are set
		$schema_info = $this->getAssetSchemaInfo($assetid, $schemaid);

		// check if this schema is already set
		if (!empty($schema_info)) {

			if ((bool)$schema_info['granted'] == $granted) {
				// schema is set with same access level
				if ((bool)$schema_info['cascades'] == $cascades) {
					// same cascade level too, so no update needed
					return TRUE;
				} else {
					// same access level, but different cascade, so update it
					$db_action = 'update';
				}
			} else {
				if ($force_set) {
					$db_action = 'update';
				} else {
					// schema is set but with the opposite access level
					$new_access		= ($granted) ? 'apply'	: 'deny';
					$current_access = ($granted) ? 'denied' : 'applied';

					trigger_localised_error('SYS0032', E_USER_WARNING, $new_access, $schema->name, $asset->name, $current_access);
					return FALSE;
				}
			}
		}

		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		// log a message for the asset - this wont get logged until we close the queue
		$msg_reps = Array(
						'asset_name'		=> $asset->name,
						'metadata_schema'	=> $schema->name,
					);
		$msg_type = 'asset.metadata.schema.set.fulllog';
		$message = $ms->newMessage(Array(), $msg_type, $msg_reps);
		$message->parameters['assetid'] = $asset->id;
		$ms->logMessage($message);

		try {
			switch ($db_action) {
				case 'insert':
					$message = 'set';
					$bind_vars	= Array (
									'assetid'	=> $asset->id,
									'schemaid'	=> (string) $schemaid,
									'granted'	=> (string) ($granted ? '1' : '0'),
									'cascades'	=> (string) ($cascades ? '1' : '0'),
								  );
					$result = MatrixDAL::executeQuery('core', 'setMetadataSchemaOnAsset', $bind_vars);
					break;
				case 'update':
					$message = 'update';
					$bind_vars	= Array (
									'assetid'	=> $asset->id,
									'schemaid'	=> (string) $schemaid,
									'granted'	=> (string) ($granted ? '1' : '0'),
									'cascades'	=> (string) ($cascades ? '1' : '0'),
								  );
					$result = MatrixDAL::executeQuery('core', 'updateMetadataSchemaOnAsset', $bind_vars);
					break;
			}//end switch

		} catch (Exception $e) {
				throw new Exception('Unable to '.$message.' metadata schema id #'.$schemaid.' on assetid #'.$asset->id.' due to the following database error:'.$e->getMessage());
		}//end try catch

		if ($granted) {
			//TODO:
			if (!$asset->shouldFastTrack('metadata_manager_generate_content_file', $schemaid)) {
				$all_contexts = $GLOBALS['SQ_SYSTEM']->getAllContexts();
				foreach ($all_contexts as $all_contextid => $context_data) {
					$this->generateContentFile($assetid, FALSE, $all_contextid);
				}

				$em = $GLOBALS['SQ_SYSTEM']->getEventManager();
				
				// Needs broadcast this event for all the "FastTracked" schemas
				$fast_tracked_schemas = $asset->getFaskTrackedTaskAssetids('metadata_manager_generate_content_file');
				if (!empty($fast_tracked_schemas)) {
					foreach($fast_tracked_schemas as $ft_schemaid) {
						$em->broadcastEvent($asset, 'MetadataUpdate', Array('schemaid' => $ft_schemaid));
					}
				}
				$em->broadcastEvent($asset, 'MetadataUpdate', Array('schemaid' => $schemaid));
				$GLOBALS['SQ_SYSTEM']->broadcastTriggerEvent('trigger_event_metadata_updated', $asset);
			}
		}

		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
		$ms->closeLog();

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
		$asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
		if (is_null($asset)) {
			trigger_localised_error('SYS0036', E_USER_WARNING, $assetid);
			return FALSE;
		}

		if (!$asset->adminAccess('metadata')) {
			trigger_localised_error('SYS0029', E_USER_WARNING, $asset->name);
			return FALSE;
		}

		// open a queue for all messages we are going to be logging
		$ms = $GLOBALS['SQ_SYSTEM']->getMessagingService();
		$ms->openLog();

		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		// log a message for the asset - this wont get logged until we close the queue
		$schema = $GLOBALS['SQ_SYSTEM']->am->getAsset($schemaid);
		$msg_reps = Array(
						'asset_name'		=> $asset->name,
						'metadata_schema'	=> $schema->name,
					);
		$msg_type = 'asset.metadata.schema.delete.fulllog';
		$message = $ms->newMessage(Array(), $msg_type, $msg_reps);
		$message->parameters['assetid'] = $asset->id;
		$ms->logMessage($message);

		try {
			$bind_vars	= Array (
							'schemaid'	=> $schemaid,
							'assetid'	=> $asset->id,
						  );
			$result	= MatrixDAL::executeQuery('core', 'deleteSchemaOnAsset', $bind_vars);

		} catch (Exception $e) {
			throw new Exception('Unable to delete schemaid #'.$schemaid.' on assetid #'.$asset->id.' due to the following database error:'.$e->getMessage());
		}//end try catch

		$this->_cleanupMetadataValues($assetid);
		//TODO:
		$all_contexts = $GLOBALS['SQ_SYSTEM']->getAllContexts();
		foreach ($all_contexts as $all_contextid => $context_data) {
			if ($all_contextid === 0) {
				// always generate a plain metadata.php
				$this->generateContentFile($assetid, FALSE, $all_contextid);
			} else {
				$context_metadata = $this->getMetadata($assetid, 0, $all_contextid);
				if (empty($context_metadata) === FALSE) {
					$this->generateContentFile($assetid, FALSE, $all_contextid);
				}
			}
		}

		$em = $GLOBALS['SQ_SYSTEM']->getEventManager();
		$em->broadcastEvent($asset, 'MetadataDeleted', Array('schemaid' => $schemaid));
		$GLOBALS['SQ_SYSTEM']->broadcastTriggerEvent('trigger_event_metadata_updated', $asset);

		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
		$ms->closeLog();
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
		$sql = 'SELECT DISTINCT
					assetid, granted
				FROM
					'.SQ_TABLE_RUNNING_PREFIX.'ast_mdata ';

		$where = 'schemaid = :schemaid';
		if (!is_null($granted)) {
			$where .= ' AND granted = :granted';
		}
		$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where);

		try {
			$query = MatrixDAL::preparePdoQuery($sql.$where);
			MatrixDAL::bindValueToPdo($query, 'schemaid', $schemaid);
			if (!is_null($granted)) {
				MatrixDAL::bindValueToPdo($query, 'granted', $granted ? '1' : '0', PDO::PARAM_STR);
			}
			$result = MatrixDAL::executePdoAll($query);
		} catch (Exception $e) {
			throw new Exception('Unable to get assetid with schemaid #'.$schemaid.' applied due to the following database error:'.$e->getMessage());
		}//end try catch

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


//--		METADATA VALUES		   --//


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
	* @param int	$contextid	The context which we should get values from
	*
	* @return array
	* @access public
	*/
	function getMetadata($assetid, $schemaid=0, $contextid=NULL)
	{
		if ($contextid === NULL) {
			$contextid = $GLOBALS['SQ_SYSTEM']->getContextId();
		}

		$asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
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

		// And to think I didn't like it when this generating query got pulled out of the Asset_Manager::getChildren() :)
		// Instead of doing any thinking...let them do it all for us and use it in our query
		$sql = 'SELECT
					mv.fieldid, a.name, mv.value
				FROM
					'.SQ_TABLE_RUNNING_PREFIX.'ast_mdata_val mv
				INNER JOIN
					'.SQ_TABLE_RUNNING_PREFIX.'ast a ON mv.fieldid = a.assetid';

		$where = '	mv.assetid = :assetid
				AND mv.contextid = :contextid
				AND (';

		$bind_vars = Array();
		foreach ($schemas as $schemaid) {
			$schema = $GLOBALS['SQ_SYSTEM']->am->getAsset($schemaid);
			if (is_null($schema)) {
				trigger_localised_error('SYS0044', E_USER_WARNING, $schemaid);
				return FALSE;
			}

			$bind_prefix = 'gc_'.$schemaid.'_';
			$ret_val = $GLOBALS['SQ_SYSTEM']->am->generateGetChildrenQuery($schema, 'metadata_field', FALSE, NULL, NULL, NULL, TRUE, FALSE, NULL, NULL, TRUE, NULL, $bind_prefix);
			$sql_array = $ret_val['sql_array'];
			$bind_vars = array_merge($bind_vars, $ret_val['bind_vars']);
			$where .= '
						mv.fieldid IN ('.implode(' ', $sql_array).') OR ';

		}// end foreach

		$where = trim($where, ' OR ').'
					)';

		$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 'mv');
		$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 'a');

		try {
			$query = MatrixDAL::preparePdoQuery($sql.$where);
			MatrixDAL::bindValueToPdo($query, 'assetid', $asset->id);
			MatrixDAL::bindValueToPdo($query, 'contextid', $contextid);
			foreach ($bind_vars as $bind_var => $bind_value) {
				MatrixDAL::bindValueToPdo($query, $bind_var, $bind_value);
			}
			$metadata = MatrixDAL::executePdoGroupedAssoc($query);
		} catch (Exception $e) {
			// SYS0338 = Unable to get current metadata values due to DB error
			throw new Exception(translate_error('SYS0338', $asset->id, $contextid, $e->getMessage()));
		}//end try catch

		return $metadata;

	}//end getMetadata()


	/**
	* Remove entries from the mdata_val table that don't correspond to an applied schema
	* or a currently available context
	*
	* @param int	$assetid	The ID of the asset whose values we want to clean up
	*
	* @return void
	* @access private
	*/
	function _cleanupMetadataValues($assetid)
	{
		$contextids    = $GLOBALS['SQ_SYSTEM']->getAllContexts();
		$assetid_cond  = 'assetid = '.MatrixDAL::quote((string)$assetid);
		$fieldid_conds = Array();
		$bind_vars	   = Array();

		foreach ($this->getSchemas($assetid, TRUE) as $schemaid) {
			$schema = $GLOBALS['SQ_SYSTEM']->am->getAsset($schemaid, 'metadata_schema');

			$bind_prefix = 'gc_'.$schemaid.'_';
			$ret_val = $GLOBALS['SQ_SYSTEM']->am->generateGetChildrenQuery($schema, 'metadata_field', FALSE, NULL, NULL, NULL, TRUE, FALSE, NULL, NULL, TRUE, NULL, $bind_prefix);
			$sql_array = $ret_val['sql_array'];
			$bind_vars = array_merge($bind_vars, $ret_val['bind_vars']);

			$fieldid_conds[] = '(fieldid NOT IN ('.implode(' ', $sql_array).'))';
		}

		$sql = 'DELETE FROM sq_ast_mdata_val
				';
		$where = 'WHERE '.$assetid_cond;

		if (!empty($fieldid_conds)) {
			$where .= ' AND (('.implode("\n\tAND ", $fieldid_conds).') OR (contextid NOT IN ('.implode(', ', array_keys($contextids)).')))';
		}

		$sql .= $where;

		try {
			$query = MatrixDAL::preparePdoQuery($sql);
			foreach ($bind_vars as $bind_var => $bind_value) {
				MatrixDAL::bindValueToPdo($query, $bind_var, $bind_value);
			}
			MatrixDAL::execPdoQuery($query);
		} catch (Exception $e) {
			throw new Exception(translate_error('SYS0342', $assetid, $e->getMessage()));
		}


	}//end _cleanupMetadataValues()


	/**
	* Get the default metadata values for a particular schema
	*
	* @param string	$schemaid	the ID of the schema to get default metadata for
	* @param int	$contextid	Context to get the values for (omit for current)
	*
	* @return array
	* @access public
	*/
	function getSchemaDefaultValues($schemaid, $contextid=NULL)
	{
		if ($contextid === NULL) {
			$contextid = $GLOBALS['SQ_SYSTEM']->getContextId();
		}

		// Cache the field names here. We don't need to cache the value so much
		// because getMetadataFieldDefaultValue() does that for us
		static $cached_field_names = Array();

		$am =& $GLOBALS['SQ_SYSTEM']->am;

		$schema = $am->getAsset($schemaid);
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
											'value'	=> $this->getMetadataFieldDefaultValue($fieldid, $contextid),
										);
		}

		return $default_values;

	}//end getSchemaDefaultValues()

	
	/**
	 * Load metadata values, principally from $metadata_filename
	 *
	 * @param string $assetid
	 * @param string $metadata_filename
	 *
	 * @return Array
	 * @access public
	 */
	function loadMetadataFile($assetid, $metadata_filename)
	{
		$metadata  = Array();
		$asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
		$contextid = $GLOBALS['SQ_SYSTEM']->getContextId();

		// include the file but must not display anything
		ob_start();
			include $metadata_filename;
		ob_end_clean();
			
		$metadata['values'] = $metadata_values;
		$metadata['value_components'] = $metadata_value_components;
		$metadata['warnings'] = $metadata_warnings;
			
		return $metadata;
		
	}//end loadMetadata


	/**
	* Generate an array of current values for the passed metadata field names
	*
	* If the passed fields array is empty, all field values will be returned
	*
	* @param string	$assetid		the ID of the asset to get field values for
	* @param array	$field_names	an array of field names
	* @param int	$contextid		The context to get these values from
	*
	* @return array
	* @access public
	*/
	function getMetadataFieldValues($assetid, $field_names=Array(), $contextid=NULL)
	{
		if ($contextid === NULL) {
			$contextid = $GLOBALS['SQ_SYSTEM']->getContextId();
		}
		$contextid = (Int) $contextid;

		$default_metadata_file = 'metadata.php';
		if ($contextid === 0) {
			$metadata_basename = $default_metadata_file;
		} else {
			$metadata_basename = 'metadata.'.$contextid.'.php';
		}

		// Change the context here so the asset is retrieved in the right context
		// (particularly if a custom context is requested)
		$GLOBALS['SQ_SYSTEM']->changeContext($contextid);

		$am =& $GLOBALS['SQ_SYSTEM']->am;
		$asset = $am->getAsset($assetid);
		$field_values = Array();

		if (is_null($asset)) {
			trigger_localised_error('SYS0038', E_USER_WARNING, $assetid);
			$GLOBALS['SQ_SYSTEM']->restoreContext();
			return $field_values;
		}

		$schemas = $this->getSchemas($asset->id, TRUE);
		if (empty($schemas)) {
			$GLOBALS['SQ_SYSTEM']->restoreContext();
			return $field_values;
		}

		// we suppose that the metadata values are stored in the file called metadata.php
		// if the file does not exists it means that not metadata was generated because
		// no metatadata schema was applied to the asset
		// this will enhance the speed of this function and will replace any keyword
		$_metadata_filename = $asset->data_path.'/'.$metadata_basename;

		$metadata_file_exists = TRUE;
		if (!is_file($_metadata_filename)) {
			$metadata_file_exists = FALSE;
			if ($contextid !== 0) {
				// Use default context metadata file, if the metadata for current context does not exist
				$_metadata_filename = $asset->data_path.'/'.$default_metadata_file;
				$metadata_file_exists = is_file($_metadata_filename);
			}
		}

		if (!$metadata_file_exists) {
			$GLOBALS['SQ_SYSTEM']->restoreContext();
			return $field_values;
		}

		$metadata = $this->loadMetadataFile($assetid, $_metadata_filename);

		if (isset($metadata['values'])) {
			$metadata_values = $metadata['values'];
			
			// if the field_names is not empty return the value asked
			// else return all the values
			if (!empty($field_names)) {
				foreach ($field_names as $key => $value) {
					if (isset($metadata_values[$value])) {
						$field_values[$value] = $metadata_values[$value];
					} else if (isset($metadata_values[str_replace('_', ' ', $value)])) {
						$field_values[$value] = $metadata_values[str_replace('_', ' ',$value)];
					} else {
						$field_values[$value] = '';
					}
				}
			} else {
				$field_values = $metadata_values;
			}
		}

		$am->forgetAsset($asset);
		unset($asset);

		$GLOBALS['SQ_SYSTEM']->restoreContext();

		return $field_values;

	}//end getMetadataFieldValues()


	/**
	* Set metadata values for an asset
	*
	* @param string	$assetid	the ID of the asset to set the values for
	* @param array	$metadata	the values to set
	* @param int	$contextid	The context ID that this metadata setting applies for
	* @param boolean	$broadcast_all_fields	If true, the event "metadata updated" will be
												broadcasted for all metadata fields
	*
	* @return boolean
	* @access public
	*/
	function setMetadata($assetid, $metadata, $contextid=NULL, $update_asset=TRUE, $broadcast_all_fields=FALSE, $ignore_editable=FALSE)
	{
		if ($contextid === NULL) {
			$contextid = $GLOBALS['SQ_SYSTEM']->getContextId();
		}

		$asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
		if (is_null($asset)) {
			trigger_localised_error('SYS0043', E_USER_WARNING, $assetid);
			return FALSE;
		}

		if (!$asset->writeAccess('metadata')) {
			trigger_localised_error('SYS0030', E_USER_WARNING, $asset->name);
			return FALSE;
		}

		// open a queue for all messages we are going to be logging
		$ms = $GLOBALS['SQ_SYSTEM']->getMessagingService();
		$ms->openLog();

		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		$contextable_fields = Array(
								0 => Array(),
								1 => Array(),
							  );

		// Make sure all the fields exist and are editable
		foreach (array_keys($metadata) as $fieldid) {
			$flag_match_found = FALSE;
			$field = $GLOBALS['SQ_SYSTEM']->am->getAsset($fieldid);
			if (is_null($field) || !($ignore_editable || $field->attr('editable'))) {
				unset($metadata[$fieldid]);
				continue;
			} else {
				// Check for metadata section restrictions
				$current_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($field->id, SQ_LINK_TYPE_2, 'metadata_section', FALSE, 'minor', NULL, TRUE);
				foreach ($current_links as $link) {
					$section = $GLOBALS['SQ_SYSTEM']->am->getAsset($link['majorid'], $link['major_type_code']);
					if (is_null($section)) continue;
					$restrictions = $section->attr('restrict');
					if (!empty($restrictions)) {
						foreach ($restrictions as $type_code_restricted => $inherit_it) {
							if ($inherit_it && !$flag_match_found) {
								$asset_type_with_parents = $GLOBALS['SQ_SYSTEM']->am->getAssetTypeInfo(Array($asset->id));
								foreach($asset_type_with_parents[$asset->id] as $index => $asset_type) {
									if (array_key_exists($asset_type[0], $restrictions) && !$flag_match_found) {
										$flag_match_found = TRUE;
									}
								}
							} else if (array_key_exists($asset->type(), $restrictions)){
								$flag_match_found = TRUE;
							}

						}
						if (!$flag_match_found) {
							unset($metadata[$fieldid]);
						}
					}
				}//end foreach

				// If the field is still being tested, check whether it's
				// contextable or not, so it's appropriately handled
				if (array_key_exists($fieldid, $metadata)) {
					// A field is non-contextatble if its attribute says so
					// or it is a Metadata Select Field
					$contextable_value = (!$field->attr('is_contextable') || ($field instanceof Metadata_Field_Select)) ? 0 : 1;
					$contextable_fields[$contextable_value][] = (string)$fieldid;
				}
				continue;
			}

		}//end foreach
		
		$all_contexts = $GLOBALS['SQ_SYSTEM']->getAllContexts();

		// log a message for the asset - this wont get logged until we close the queue
		$metadata_updated = Array();
		foreach ($metadata as $fieldid => $value) {
			// Set default
			$mdata_name = 'Unknown Metadata Field';

			// Go and get the field name
			if ($fieldid > 0) {
				$mdata_field = $GLOBALS['SQ_SYSTEM']->am->getAsset($fieldid);
				$mdata_name = $mdata_field->name;
			}

			// Get the old value
			$mdata_values = $this->getMetadataFieldValues($asset->id);
			$old_value = ''; // Default to nothing set
			foreach ($mdata_values as $old_key => $old_val) {
				if ($old_key == $mdata_name) {
					$old_value = $old_val;
				}
			}
			
			// If the field is set to 'use default', it wouldn't be set to any value,
			if (!isset($value[0]['value'])) {
			    $value[0]['value'] = $this->getMetadataFieldDefaultValue($fieldid, $contextid);
			}

			// By pass if the new and old are the same (we don't need the message saying we made no changes!)
			if (!($old_value == $value[0]['value'])) {
				// This is for the trigger metadata updated
				$metadata_updated[$fieldid] = $value;

				// Message replacements
				$msg_reps = Array(
								'asset_name'		=> $asset->name,
								'metadata_field'	=> $mdata_name,
								'metadata_old'		=> $old_value,
								'metadata_new'		=> $value[0]['value'],
							);
				$msg_type = 'asset.metadata.set.fulllog';
				$message = $ms->newMessage(Array(), $msg_type, $msg_reps);
				$message->parameters['assetid'] = $asset->id;
				$ms->logMessage($message);
			}
		}//end foreach

		// Delete from mdata_val table where necessary
		$delete_fieldids = Array();
		$remaining = Array();
		foreach ($metadata as $fieldid => $details) {
			if ($details[0]['value'] === NULL) {
				// we want to revert this field to default, done by deleting it from the mdata_val table
				$delete_fieldids[] = (string) $fieldid;
			} else {
				$remaining[] = $fieldid;
			}
		}

		// We want to remove all non-contextable fields and then insert them again instead of updating
		// because we want to add new records for contexts that do not have metadata fields set yet
		// e.g. set metadata the first time for default context and alternate contexts still use metadata default values
		$delete_fieldids = array_merge($delete_fieldids, $contextable_fields[0]);
		// Get unique field IDs because non-contextable fields can be set to default value ('Use default' option in Metadata screen)
		$delete_fieldids = array_unique($delete_fieldids);

		if (!empty($delete_fieldids)) {
			try {
				// Find the contextable metadata fields in the lot - and delete
				// only this context for them
				if (count(array_intersect($delete_fieldids, $contextable_fields[1])) > 0) {
					$bind_vars	= Array (
									'assetid'	=> $assetid,
									'fieldid'	=> array_intersect($delete_fieldids, $contextable_fields[1]),
									'contextid'	=> $contextid,
								  );
					$result = MatrixDAL::executeQuery('core', 'deleteMetadataValueForAsset', $bind_vars);
				}

				// Non-contextable metadata fields - delete the whole lot!aa
				if (count(array_intersect($delete_fieldids, $contextable_fields[0])) > 0) {
					$bind_vars	= Array (
									'assetid'	=> $assetid,
									'fieldid'	=> array_intersect($delete_fieldids, $contextable_fields[0]),
									'contextid'	=> NULL,
								  );
					$result = MatrixDAL::executeQuery('core', 'deleteMetadataValueForAsset', $bind_vars);
				}
			} catch (Exception $e) {
				// SYS0341 = Unable to revert fields to default due to DB error on delete
				throw new Exception(translate_error('SYS0341', implode(', ', $delete_fieldids), $asset->id, $contextid, $e->getMessage()));
			}//end try catch
		}

		if (!empty($remaining)) {
			// Find any fields that have existing entries...so we know what to update and what to insert
            $sql = 'SELECT
                        fieldid, contextid, use_default
                    FROM
                        '.SQ_TABLE_RUNNING_PREFIX.'ast_mdata_val';
            $where = '  assetid = :assetid';
            $where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where);

            try {
                $query = MatrixDAL::preparePdoQuery($sql.$where);
                MatrixDAL::bindValueToPdo($query, 'assetid', $asset->id);
                $updates = MatrixDAL::executePdoAll($query);
            } catch (Exception $e) {
                // SYS0338 = Unable to get current values due to DB error
                throw new Exception(translate_error('SYS0338', $asset->id, $contextid, $e->getMessage()));
            }

            // Group the existing contexted entries by contextid
			// $existing_fieldids['contextid']['fieldid'] = 'use_default (0/1)';
			$existing_fieldids = Array();
			foreach($updates as $row) {
				if (!isset($existing_fieldids[$row['contextid']])) {
				$existing_fieldids[$row['contextid']] = Array();
					$existing_fieldids[$row['contextid']][$row['fieldid']] = Array();
				}
				$existing_fieldids[$row['contextid']][$row['fieldid']] = $row['use_default'];
            }//end foreach

			// Insert or update the remaining metadata fields as required
			foreach ($remaining as $fieldid) {
			// If the field is not contextable, set the context ID to null,
				// so it will be copied to all applicable contexts.
				if (array_search($fieldid, $contextable_fields[0]) !== FALSE) {
					$do_contextid = NULL;
				} else {
					$do_contextid = $contextid;
				}

				$bind_vars	= Array (
								'assetid'	=> $asset->id,
								'fieldid'	=> $fieldid,
								'value'		=> $metadata[$fieldid][0]['value'],
							  );

				if (!isset($existing_fieldids[$contextid][$fieldid])) {
					// for non contextable field, delete all entries first to avoid duplicate insertion error
					if(!isset($do_contextid)){
						try {
							$bind_vars['contextid'] = NULL;
							$result = MatrixDAL::executeQuery('core', 'deleteMetadataValueForAsset', $bind_vars);
						} catch (Exception $e) {
							throw new Exception(translate_error('SYS0339', $fieldid, $metadata[$fieldid][0]['value'], $asset->id, $contextid, $e->getMessage()));
						}

						// insert values of non-contextable for all contexts
						try {
							$bind_vars['contextid'] = NULL;
							$bind_vars['use_default'] = '1';
							$result = MatrixDAL::executeQuery('core', 'setMetadataValueForAsset', $bind_vars);
						} catch (Exception $e) {
							// SYS0339 = Cannot set new value of metadata due to DB error
							throw new Exception(translate_error('SYS0339', $fieldid, $metadata[$fieldid][0]['value'], $asset->id, $contextid, $e->getMessage()));
						}//end try catch
					} else {
						// insert values to this context
						try {
							$bind_vars['contextid'] = $contextid;
							$bind_vars['use_default'] = $contextid === 0 ? '1' : '0';
							$result = MatrixDAL::executeQuery('core', 'setMetadataValueForAsset', $bind_vars);
						} catch (Exception $e) {
							// SYS0339 = Cannot set new value of metadata due to DB error
							throw new Exception(translate_error('SYS0339', $fieldid, $metadata[$fieldid][0]['value'], $asset->id, $contextid, $e->getMessage()));
						}

						// Also copy the default context values to other contexts which are using default context values
						if ($contextid === 0) {
							foreach($all_contexts as $ctxid => $val) {
								if ($ctxid === 0) {
									// Skip for default context, we just inserted right above
									continue;
								}
								$copy_entry = FALSE;
								if (isset($existing_fieldids[$ctxid][$fieldid]) && $existing_fieldids[$ctxid][$fieldid]) {
									// There is an existing value for this field in this context
									// Remove it first to avoid duplicate insertion error
									try {
										$bind_vars['contextid'] = $ctxid;
										$result = MatrixDAL::executeQuery('core', 'deleteMetadataValueForAsset', $bind_vars);
									} catch (Exception $e) {
										throw new Exception(translate_error('SYS0339', $fieldid, $metadata[$fieldid][0]['value'], $asset->id, $contextid, $e->getMessage()));
									}
									$copy_entry = TRUE;
								} else if (!isset($existing_fieldids[$ctxid][$fieldid])) {
									// Entry does not exist for this context(ctxid) yet
									$copy_entry = TRUE;
								}

								if ($copy_entry) {
									try {
										$bind_vars['contextid'] = $ctxid;
										$bind_vars['use_default'] = '1';
										$result = MatrixDAL::executeQuery('core', 'setMetadataValueForAsset', $bind_vars);
									} catch (Exception $e) {
										// SYS0339 = Cannot set new value of metadata due to DB error
										throw new Exception(translate_error('SYS0339', $fieldid, $metadata[$fieldid][0]['value'], $asset->id, $contextid, $e->getMessage()));
									}
								}//end if
							}//end foreach
						}//end else
					}//end else if not contextable

				// we are updating
				} else {
					if (is_null($do_contextid) || $contextid === 0) {
						$query_name = 'updateMetadataValueForAsset';
					} else {
						$query_name = 'updateAlternateContextMetadataValueForAsset';
						$bind_vars['contextid'] = $contextid;
					}
					try {
						$result = MatrixDAL::executeQuery('core', $query_name, $bind_vars);
					} catch (Exception $e) {
						// SYS0340 = Cannot update value of metadata due to DB error
						throw new Exception(translate_error('SYS0340', $fieldid, $metadata[$fieldid][0]['value'], $asset->id, $contextid, $e->getMessage()));
					}//end try catch

				}//end if

			}//end for

		}//end if

		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
		$ms->closeLog();

		$asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
		if ($update_asset) $asset->_updated();
		$em = $GLOBALS['SQ_SYSTEM']->getEventManager();
		// broadcasted the assetids of the updated fields
		$updated_fields = Array('fieldids' => array_keys($metadata_updated));
		$em->broadcastEvent($asset, 'MetadataUpdate', $broadcast_all_fields ? Array('all') : Array('fieldids' => array_keys($metadata)));
		$GLOBALS['SQ_SYSTEM']->broadcastTriggerEvent('trigger_event_metadata_updated', $asset, $updated_fields);
                                                            
		return TRUE;


	}//end setMetadata()


//--		(RE)GENERATE METADATA		 --//


	/**
	* Generate a PHP file that can be included to print all metadata fields for an asset
	*
	* The generated file contains PHP code to do any keyword replacing based on
	* current values instead of the ones at the time of caching.
	*
	* @param string		$assetid		the ID of the asset to generate the file for
	* @param boolean	$regenerate		regenerate the DB information as well
	* @param int		$contextid		Context to generate the content file for
	* @param boolean	$update_asset	Update the asset?
	*
	* @return boolean
	* @access public
	*/
	function generateContentFile($assetid, $regenerate=FALSE, $contextid=NULL, $update_asset=TRUE)
	{
		if ($contextid === NULL) {
			$contextid = $GLOBALS['SQ_SYSTEM']->getContextId();
		}
		$contextid = (Int) $contextid;
		
		if ($contextid === 0) {
			$metadata_filename = 'metadata.php';
		} else {
			$metadata_filename = 'metadata.'.$contextid.'.php';
		}

		// Change to the context specified, before the asset is retrieved
		if ($contextid === 'default') {
			$regenerate = FALSE;
			$GLOBALS['SQ_SYSTEM']->changeContext(0);
		} else {
			$GLOBALS['SQ_SYSTEM']->changeContext($contextid);
		}

		$asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
		if (is_null($asset)) {
			trigger_localised_error('SYS0034', E_USER_WARNING, $assetid);
			$GLOBALS['SQ_SYSTEM']->restoreContext();
			return FALSE;
		}

		$schemas = $this->getSchemas($asset->id, TRUE, NULL, TRUE);
		if (empty($schemas)) {
			$GLOBALS['SQ_SYSTEM']->restoreContext();
			return FALSE;
		}

		// an array of all metadata tag values
		$tag_values = Array();

		// an array of keyword replacements that we are going to have to write some PHP code to produce
		$keywords = Array();

		// get the values without any replacements or special processing
		// Also used if we are regenerating
		if ($contextid === 'default') {
			$metadata = Array();
		} else {
			$metadata = $this->getMetadata($asset->id);
		}

		ob_start();
		foreach ($schemas as $schemaid) {
			$schema = $GLOBALS['SQ_SYSTEM']->am->getAsset($schemaid);
			if (is_null($schema)) {
				trigger_localised_error('SYS0035', E_USER_WARNING, $schemaid);
				$GLOBALS['SQ_SYSTEM']->restoreContext();
				return FALSE;
			}

			if (!($schema instanceof Metadata_Schema)) {
				trigger_localised_error('SYS0033', E_USER_WARNING, $schema->name, $schemaid);
				$GLOBALS['SQ_SYSTEM']->restoreContext();
				return FALSE;
			}

			$edit_fns = $schema->getEditFns();
			$edit_fns->generateMetadata($schema, $metadata, $tag_values, $keywords, $asset->type());

			$GLOBALS['SQ_SYSTEM']->am->forgetAsset($schema);
		}

		$metadata_str = ob_get_contents();
		ob_end_clean();

		ob_start();
			echo '<'.'?php'."\n";
			if (!empty($keywords)) {
				require_once SQ_FUDGE_PATH.'/general/text.inc';
				echo '$am =& $GLOBALS[\'SQ_SYSTEM\']->am;'."\n";
				echo '$metadata_asset = $am->getAsset('.$asset->id.', \'\', TRUE);'."\n\n";
				echo '/* 1 */', "\n";
				echo '$metadata_replacements = Array();'."\n";
				foreach ($keywords as $keyword) {
					if (!preg_match('/^metadata_field_/', $keyword)) {
						// escape single quotes only. back slash is already escaped and doubel quote must be kept to be consistent with code below
						echo '$metadata_replacements[\''.addcslashes($keyword, '\'').'\'] = $metadata_asset->getKeywordReplacement(\''.addcslashes($keyword, '\'').'\');'."\n";
					}
				}
				echo '/* 2 */', "\n";
				replace_keywords($tag_values, $replacements);
				echo '/* 3 */', "\n";
				echo "\n\n";
			}


			// the following handles the replacement of the %metadata_field_xxxxx% keywords by
			// looping over the copy of tag_values (field_list) - if the value doesn't have
			// any metadata_field keywords, shift it over to the "replaced" list

			// if the value does contain keywords, however, for each keyword check whether or not
			// replaced_list contains the desired field; do this until all possible keyword
			// replacements have been made (if everything is replaced, leaving no keywords, this
			// value will be picked up in the next pass and added to replaced_list)

			// if a pass is completed without any modifications to field_list or replaced_list being
			// made, we bail out and leave any circular references unreplaced
			$field_list    = $tag_values;
			$replaced_list = Array();

			$field_replacements = Array();

			// keep looping until we run out of items to shift over to $replaced_list
			do {
				$fields_modified = FALSE;
				foreach ($field_list as $n => $v) {

					// get all the %metadata_field_xxxxx% keywords in $v
					$value_keywords = retrieve_keywords_replacements($v);

					// cull any keywords that aren't in the format %metadata_field_xxxxx%
					$metadata_keywords = Array();
					foreach ($value_keywords as $keyword) {
						if (preg_match('/^metadata_field_/', $keyword)) {
							$metadata_keywords[] = $keyword;
						}
					}
					$metadata_keywords = array_unique($metadata_keywords);

					// if no metadata_field, put it directly on the $replaced stack,
					if (empty($metadata_keywords)) {
						// muck around with the stack; don't touch the original $metadata_values
						$replaced_list[$n] = $v;
						$field_replacements['metadata_field_'.$n] = $v;

						unset($field_list[$n]);

						$fields_modified = TRUE;

					} else {
						// Apply keyword modifiers
						foreach($metadata_keywords as $full_keyword) {
							if (!isset($field_replacements[$full_keyword])) {
								$modifiers = NULL;
								$part_keyword = parse_keyword($full_keyword, $modifiers);
								if ($part_keyword != $full_keyword && isset($field_replacements[$part_keyword])) {
									$keyword_value = $field_replacements[$part_keyword];
									apply_keyword_modifiers($keyword_value, $modifiers, Array('assetid' => $assetid));
									$field_replacements[$full_keyword] = $keyword_value;
								}
							}
						}//end foreach

						// replace all the field references with their replaced_list counterparts
						// if everything is successfully replaced, leave until next time
						// to move to replaced_list
						$pre_replacement = $field_list[$n];
						replace_keywords($field_list[$n], $field_replacements);

						// if keywords were replaced...
						if ($pre_replacement != $field_list[$n]) {
							// mirror value changes back to the original
							// $metadata_values list, and the keyword replacement list
							$tag_values[$n] = $field_list[$n];
							$fields_modified = TRUE;
						}
					}//end else (empty($field_refs))

				}//end foreach ($metadata_values)
			} while ($fields_modified);
	
			// print the values, to be referenced by the meta tags later
			echo '$metadata_values = Array('."\n";
			foreach ($tag_values as $n => $v) {
				$n = addslashes($this->escapeMetadata($n, TRUE));
				Metadata_Field::decodeValueString($v, $v2, $vc);
				// escape single quotes and backslashes
				$v2 = addcslashes($v2, "'\\");

				// unescape metadata_replacements
				// leave metadata_values for later - unescape them at runtime
				$v2 = preg_replace('/%([^%]+)%/', '\'.(isset($metadata_replacements[\'\1\']) ? $metadata_replacements[\'\1\'] : \'%\1%\' ).\'', $v2);
				echo "'$n' => '$v2',\n";
			}
			echo ");\n\n";

			// print the value components, to be referenced by the meta tags later
			echo '$metadata_value_components = Array('."\n";
			foreach ($tag_values as $n => $v) {
				$vc = Array();
				$n = addslashes($this->escapeMetadata($n, TRUE));
				Metadata_Field::decodeValueString($v, $v2, $vc);

				echo "'$n' => Array(";

				// Value components
				if (!empty($vc)) {
					foreach ($vc as $vc_key => $vc_value) {
						$vc_key   = addslashes($this->escapeMetadata($vc_key, TRUE));
						$vc_value = addslashes($this->escapeMetadata($vc_value, TRUE));
						echo "'$vc_key' => '".$vc_value."', ";
					}
				}

				// Close the array
				echo "),\n";
			}
			echo ");\n\n";

			// print the metadata manager reference for value encoding
			echo '$mm = $GLOBALS[\'SQ_SYSTEM\']->getMetadataManager();'."\n";

			// print the warnings
			echo '$metadata_warnings = Array();'."\n";

			// if the field_list is empty, it means we still have keywords left to replace
			if (!empty($field_list)) {
				echo '$metadata_warnings[\'keyword_circular_references\'] = Array('."\n";
				foreach ($field_list as $n => $v) {
					$n = $this->escapeMetadata($n, TRUE);
					echo "'$n',\n";
				}
				echo ");\n";
			}

			echo '?'.'>';
			$php_str = ob_get_contents();
		ob_end_clean();


		$metadata_str = $php_str."\n\n".$metadata_str;

		require_once SQ_FUDGE_PATH.'/general/file_system.inc';
		if (!create_directory($asset->data_path)) {
			$GLOBALS['SQ_SYSTEM']->restoreContext();
			return FALSE;
		}

		if (!string_to_file($metadata_str, $asset->data_path.'/'.$metadata_filename)) {
			$GLOBALS['SQ_SYSTEM']->restoreContext();
			return FALSE;
		}

		// Content regenerated, so clear the _tmp vars for metadata
		// Otherwise old values will stay in effect until the asset is reloaded
		if (isset($asset->_tmp['mm_replacements'])) {
			unset($asset->_tmp['mm_replacements']);
		}//end if

		// Content regenerated fire in the hole!
		$GLOBALS['SQ_SYSTEM']->broadcastTriggerEvent('trigger_event_after_metadata_updated', $asset);

		$result = FALSE;
		if (!$regenerate) {
			$result = TRUE;
		} else {
			$result = $this->setMetadata($assetid, $metadata, $contextid, $update_asset, TRUE);
		}

		if ($result) {
			$result = $this->_generateMetadataFieldFile($asset);
		}

		$GLOBALS['SQ_SYSTEM']->restoreContext();

		return $result;

	}//end generateContentFile()


	/**
	* Generate a PHP file that can be included to reassign Metadata Field values for an asset.
	* This is used before Metadata Field values are placed under Safe Edit and for
	* restoring values when Safe Edit is cancelled.
	*
	* This function is only called by the generateContentFile() function above,
	* and thus it is assumed that it will only be called in the context set in
	* that function.
	*
	* @param Asset	&$metadata_asset	the asset to generate the file for
	*
	* @return boolean
	* @access private
	*/
	private function _generateMetadataFieldFile(Asset &$metadata_asset)
	{
		$contextid = $GLOBALS['SQ_SYSTEM']->getContextId();

		if ($contextid === 0) {
			$filename = 'metadata_field_values.php';
		} else {
			$filename = 'metadata_field_values.'.$contextid.'.php';
		}

		$schemas = $this->getSchemas($metadata_asset->id, TRUE);
		if (empty($schemas)) return FALSE;

		$metadata_field_values = $this->getMetadata($metadata_asset->id);

		foreach ($schemas as $schema_id) {
			$default_values = $this->getSchemaDefaultValues($schema_id);

			// Allocate default value if not "manually" supplied
			foreach ($default_values as $field_id => $field) {
				if (!isset($metadata_field_values[$field_id])) {
					$metadata_field_values[$field_id][0] = NULL;
				}
			}
		}

		ob_start();
			echo serialize($metadata_field_values);
			$metadata_str = ob_get_contents();
		ob_end_clean();

		require_once SQ_FUDGE_PATH.'/general/file_system.inc';
		if (!create_directory($metadata_asset->data_path)) return FALSE;
		if (!string_to_file($metadata_str, $metadata_asset->data_path.'/'.$filename)) {
			return FALSE;
		}

		return TRUE;

	}//end _generateMetadataFieldFile()


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
					if ($keyword == $name) {
						$default_format = 'Y-m-d H:i:s';
						if ($keyword == 'asset_published') {
							echo '$metadata_replacements['.var_export($keyword, 1).'] = (is_null($metadata_asset->published)) ? \'Never\' : date(\''.$default_format.'\', $metadata_asset->published);'."\n";
						} else {
							echo '$metadata_replacements['.var_export($keyword, 1).'] = date(\''.$default_format.'\', $metadata_asset->'.substr($keyword,6).');'."\n";
						}
						$replaced = TRUE;
					} else {
						// Does the keyword match a certain date format? If so, replace
						// here; if not, then it will gets replaced further down (using date()
						// function).
						$date_formats = get_date_formats();
						$format_name = substr($keyword, strlen($name.'_'));
						if (array_key_exists($format_name, $date_formats)) {
							$format_expr = str_replace('/', '//', $date_formats[$format_name]);
							if ($keyword == 'asset_published') {
								echo '$metadata_replacements['.var_export($keyword, 1).'] = (is_null($metadata_asset->published)) ? \'Never\' : format_date($metadata_asset->published, \''.$format_name.'\');'."\n";
							} else {
								if ($format_name == 'iso8601') {
									echo '$metadata_replacements['.var_export($keyword, 1).'] = date(\''.$format_expr.'\', $metadata_asset->'.substr($keyword, strlen('asset_'), strlen($keyword) - strlen('_'.$format_name) - strlen('asset_')).').substr(date(\'O\'),0,3).\':\'.substr(date(\'O\'),-2);'."\n";
								} else {
									echo '$metadata_replacements['.var_export($keyword, 1).'] = format_date($metadata_asset->'.substr($keyword, strlen('asset_'), strlen($keyword) - strlen('_'.$format_name) - strlen('asset_')).', \''.$format_name.'\');'."\n";
								}
							}
							$replaced = TRUE;
						}
					}
				}//end if
			}//end foreach which loops through $prefix array

			$base_contextid = NULL;
			$full_keyword = $keyword;
			if (0 === strpos($keyword, 'asset_url_')) {
				// get the context id required from the name specified in the keyword
				// if there is a '_' in the context name, take it as a space. so, 'Default Context' and 'Default_Context' are treated as the same.
				$context_name = substr($keyword, 10);
				$context_name = str_replace('_', ' ', $context_name);
				$context_data = MatrixDAL::executeAll('core', 'getContextByName', Array('name' => Array($context_name)));

				// set the base contextid used later to get a correct URL and re-set the keyword without the context name to retrieve the replacement value later
				if (!empty($context_data)) {
					$base_contextid = $context_data[0]['contextid'];
					$keyword = 'asset_url';
				}
			}

			if (!$replaced) {
				switch ($keyword) {
					case 'asset_assetid' :
						echo '$metadata_replacements['.var_export($keyword, 1).'] = $metadata_asset->id;'."\n";
					break;
					case 'asset_name'		:
					case 'asset_short_name' :
					case 'asset_version'	:
						echo '$metadata_replacements['.var_export($keyword, 1).'] = $metadata_asset->'.substr($keyword,6).';'."\n";
					break;

					// version cases: if the version is undefined (eg. shadow assets), then make it expand
					// something that would result in all version components returning a blank
					case 'asset_version_major'	  :
						echo 'list($metadata_replacements['.var_export($keyword, 1).'],,) = explode(\'.\',empty($metadata_asset->version) ? \'..\' : $metadata_asset->version);'."\n";
					break;

					case 'asset_version_minor'	  :
						echo 'list(,$metadata_replacements['.var_export($keyword, 1).'],) = explode(\'.\',empty($metadata_asset->version) ? \'..\' : $metadata_asset->version);'."\n";
					break;

					case 'asset_version_micro'	  :
						echo 'list(,,$metadata_replacements['.var_export($keyword, 1).']) = explode(\'.\',empty($metadata_asset->version) ? \'..\' : $metadata_asset->version);'."\n";
					break;

					case 'asset_url' :
						echo '$metadata_replacements['.var_export($full_keyword, 1).'] = $metadata_asset->getURL(NULL, FALSE, $base_contextid);'."\n";
					break;

					case 'asset_name_linked' :
						echo '$url = $metadata_asset->getURL();'."\n";
						echo '$keyword = "";'."\n";
						echo 'if (!empty($url)) $keyword = "<a href=\"".$url."\">".$metadata_asset->name."</a>";'."\n";
						echo '$metadata_replacements['.var_export($keyword, 1).'] = $keyword;'."\n";
					break;

					case 'asset_short_name_linked' :
						echo '$url = $metadata_asset->getURL();'."\n";
						echo '$keyword = "";'."\n";
						echo 'if (!empty($url) && isset($metadata_asset->short_name)) $keyword = "<a href=\"".$url."\">".$metadata_asset->short_name."</a>";'."\n";
						echo '$metadata_replacements['.var_export($keyword, 1).'] = $keyword;'."\n";
					break;

					case 'asset_href' :
						echo '$metadata_replacements['.var_export($keyword, 1).'] = $metadata_asset->getHref();'."\n";
					break;

					case 'asset_thumbnail' :
					case 'asset_thumbnail_url' :
					case 'asset_thumbnail_width' :
					case 'asset_thumbnail_height' :
					case 'asset_thumbnail_alt' :
					case 'asset_thumbnail_caption' :
					case 'asset_thumbnail_assetid' :
					case 'asset_thumbnail_title' :
					case 'asset_thumbnail_name' :
						if (!$thumbnail_generated) {
							echo '$thumbnail = null;';
							echo 'if ($metadata_asset->useSystemVersion()) {'."\n";
							echo '    $notice_links = unserialize(file_to_string($metadata_asset->data_path.\'/.sq_notice_links\'));'."\n";
							echo '    foreach ($notice_links as $link) {'."\n";
							echo '		  if ($link[\'value\'] == \'thumbnail\') {'."\n";
							echo '			  $thumbnail = $GLOBALS[\'SQ_SYSTEM\']->am->getAsset($link[\'minorid\']);'."\n";
							echo '			  break;'."\n";
							echo '		  }'."\n";
							echo '	  }'."\n";
							echo '} else {'."\n";
							echo '    if ($metadata_asset->id) {';
							echo '	      $link = $GLOBALS[\'SQ_SYSTEM\']->am->getLink($metadata_asset->id, SQ_LINK_NOTICE, \'image\', FALSE, \'thumbnail\');'."\n";
							echo '	      if (!empty($link)) {'."\n";
							echo '		      $thumbnail = $GLOBALS[\'SQ_SYSTEM\']->am->getAsset($link[\'minorid\'], $link[\'minor_type_code\']);'."\n";
							echo '	      }'."\n";
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
							case 'asset_thumbnail_width' :
							case 'asset_thumbnail_height' :
							case 'asset_thumbnail_alt' :
							case 'asset_thumbnail_caption' :
							case 'asset_thumbnail_title' :
							case 'asset_thumbnail_name' :
								$sub_keyword = substr($keyword, 16);
								echo '$metadata_replacements['.var_export($keyword, 1).'] = (is_null($thumbnail)) ? \'\' : $thumbnail->attr(\''.$sub_keyword.'\');'."\n";
							break;
							case 'asset_thumbnail_assetid' :
								echo '$metadata_replacements['.var_export($keyword, 1).'] = (is_null($thumbnail)) ? \'\' : $thumbnail->id;'."\n";
							break;
						}
					break;

					case 'asset_created_by_assetid' :
					case 'asset_created_by_url' :
					case 'asset_created_by_name' :
					case 'asset_updated_by_name' :
					case 'asset_status_changed_by_name':
						$pattern = '/^asset_(.*?)_by_(.*)$/';
						preg_match($pattern, $keyword, $matches);
						$field_name = $matches[1];
						$user_member = $matches[2];
						switch ($user_member) {
							case 'assetid':
								$user_member = 'id';
								break;
							case 'url':
								$user_member = 'getURL()';
								break;
						}
						echo 'if (!isset($metadata_'.$field_name.'_by_user) && !empty($metadata_asset->'.$field_name.'_userid)) {'."\n";
						echo '	$metadata_'.$field_name.'_by_user = $GLOBALS[\'SQ_SYSTEM\']->am->getAsset($metadata_asset->'.$field_name.'_userid, \'\', TRUE);'."\n";
						echo '}'."\n";
						echo 'if (!empty($metadata_'.$field_name.'_by_user)) {'."\n";
						echo '	$metadata_replacements['.var_export($keyword, 1).'] = $metadata_'.$field_name.'_by_user->'.$user_member.";\n";
						echo '} else {'."\n";
						echo '	$metadata_replacements['.var_export($keyword, 1).'] = \''.translate('unknown').'\';'."\n";
						echo '}'."\n";
					break;

					case 'asset_created_by_first_name' :
					case 'asset_created_by_last_name'  :
					case 'asset_created_by_email'	   :
					case 'asset_updated_by_first_name' :
					case 'asset_updated_by_last_name'  :
					case 'asset_updated_by_email'	   :
						echo 'if (!isset($metadata_'.substr($keyword,6,7).'_by_user) && !empty($metadata_asset->'.substr($keyword,6,7).'_userid)) {
						$metadata_'.substr($keyword,6,7).'_by_user = $GLOBALS[\'SQ_SYSTEM\']->am->getAsset($metadata_asset->'.substr($keyword,6,7).'_userid,\'\', TRUE);'."\n}\n";
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
						$metadata_published_by_user = $GLOBALS[\'SQ_SYSTEM\']->am->getAsset($metadata_asset->published_userid, \'\', TRUE);'."\n}\n";
						echo 'if (!empty($metadata_published_by_user)) { ';
						echo '$metadata_replacements['.var_export($keyword, 1).'] = $metadata_published_by_user->name;'."\n";
						echo '} else {'."\n";
						echo '$metadata_replacements['.var_export($keyword, 1).'] = \''.translate('unknown').'\';'."\n";
						echo '}}'."\n";
					break;

					case 'asset_published_by_first_name' :
					case 'asset_published_by_last_name'  :
					case 'asset_published_by_email'		 :
						echo 'if (empty($metadata_asset->published_userid)) {'."\n";
						echo '$metadata_replacements['.var_export($keyword, 1).'] = \'\';'."\n";
						echo '} else { if (!isset($metadata_published_by_user)) {
						$metadata_published_by_user = $GLOBALS[\'SQ_SYSTEM\']->am->getAsset($metadata_asset->published_userid, \'\', TRUE);'."\n}\n";
						echo 'if (!empty($metadata_published_by_user)) { ';
						echo '$metadata_replacements['.var_export($keyword, 1).'] = $metadata_published_by_user->attr(\''.substr($keyword,19).'\');'."\n";
						echo '} else {'."\n";
						echo '$metadata_replacements['.var_export($keyword, 1).'] = \''.translate('unknown').'\';'."\n";
						echo '}}'."\n";

					break;

					case 'asset_read_permission'  :
					case 'asset_write_permission' :
					case 'asset_admin_permission' :
					case 'asset_read_permission_assetid'	:
					case 'asset_write_permission_assetid' :
					case 'asset_admin_permission_assetid' :
					case 'asset_read_permission_email'	:
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
						echo '	$user = $am->getAsset($userid);'."\n";
						echo '	if (is_null($user) || !$am->isTypeDecendant($user->type(), \'user\')) {'."\n";
						echo '		continue;'."\n";
						echo '	}'."\n";
						if (preg_match('/.*_assetid$/', $keyword)) {
							echo '$metadata_'.$perm_type.'_perm_info[] = $user->id;'."\n";
						}
						else if (preg_match('/.*_email$/', $keyword)) {
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
					case 'system_name'	 :
					case 'system_owner'  :
					case 'default_email' :
					case 'tech_email'	 :
						echo '$metadata_replacements['.var_export($keyword, 1).'] = SQ_CONF_'.strtoupper($keyword).';'."\n";
					break;

					default :
						if (substr($keyword, 0, 11) == 'asset_role_') {
							echo '$metadata_roles_info = Array();'."\n";
							if (preg_match('/asset_role_([0-9]+)(.*)$/', $keyword, $matches)) {
								$fetch_global_roles = (SQ_CONF_ENABLE_GLOBAL_ROLES == '1');
								$role_assetid = $matches[1];
								echo '$metadata_roles = $am->getRole($metadata_asset->id, '.$role_assetid.', NULL, FALSE, '.(($fetch_global_roles)?'TRUE':'FALSE').', TRUE);'."\n";
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
							// avoid replacing metadata_field_xxxxx keywords until later
							// override the default replacement with the original keyword
							$metadata_replacements[$keyword] = '%'.$keyword.'%';

						} else if (substr($keyword, 0, 18) == 'asset_thumbnail_v_') {
							// Print the asset variety of a thumbnail
							$variety_name = substr($keyword, 18);
							if (!$thumbnail_generated) {
								echo '$thumbnail = null;'."\n";
								echo 'if ($metadata_asset->useSystemVersion()) {'."\n";
								echo '    $notice_links = unserialize(file_to_string($metadata_asset->data_path.\'/.sq_notice_links\'));'."\n";
								echo '    foreach ($notice_links as $link) {'."\n";
								echo '		  if ($link[\'value\'] == \'thumbnail\') {'."\n";
								echo '			  $thumbnail = $GLOBALS[\'SQ_SYSTEM\']->am->getAsset($link[\'minorid\']);'."\n";
								echo '			  break;'."\n";
								echo '		  }'."\n";
								echo '	  }'."\n";
								echo '} else {'."\n";
								echo '    if ($metadata_asset->id) {'."\n";
								echo '	      $link = $GLOBALS[\'SQ_SYSTEM\']->am->getLink($metadata_asset->id, SQ_LINK_NOTICE, \'image\', FALSE, \'thumbnail\');'."\n";
								echo '	      if (!empty($link)) {'."\n";
								echo '		      $thumbnail = $GLOBALS[\'SQ_SYSTEM\']->am->getAsset($link[\'minorid\'], $link[\'minor_type_code\']);'."\n";
								echo '	      }'."\n";
								echo '    }'."\n";
								echo '}'."\n";
							}

							echo '$metadata_replacements['.var_export($keyword, 1).'] = (is_null($thumbnail)) ? \'\' : $thumbnail->getKeywordReplacement(\'image_v_'.$variety_name.'\');';

						} else {
							// searching for special date format keywords
							$original = $keyword;
							$keyword = substr($keyword, 6);
							$key	 = substr($keyword, 0, strpos($keyword, '_'));
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
	* If $contextid is NULL, the current context will be used, like usual.
	* However, if $contextid is "all", ALL content files will be generated. This
	* is intended for regenerate metadata HIPOs, or first creation.
	*
	* @param string		$assetid		the ID of the asset to regenerate metadata for
	* @param mixed		$contextid		the context to regenerate
	* @param boolean	$update_asset	update the asset?
	*
	* @return boolean
	* @access public
	*/
	function regenerateMetadata($assetid, $contextid=NULL, $update_asset=TRUE)
	{
		if ($contextid === NULL) {
			$contextid = $GLOBALS['SQ_SYSTEM']->getContextId();
		}

		if ($contextid === 'all') {
			$all_contexts = $GLOBALS['SQ_SYSTEM']->getAllContexts();
			$last_contextid = end(array_keys($all_contexts));
			$ok = TRUE;

			// Since we're doing this for all contexts, and search indexing events should ideally
			// be handled once for all contexts to avoid num_contexts * num_contexts iterations,
			// we need to ensure search reindexing is "fast tracked".
			//
			// We ensure reindex tasks are fast tracked for all but the last context, so the reindex
			// events will be handled correctly once at the end.	
			foreach ($all_contexts as $contextid => $context_data) {

				// Get the asset in each context
				$GLOBALS['SQ_SYSTEM']->changeContext($contextid);
				$temp_asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);

				// If we're not the last context, fast track!
				if ($contextid != $last_contextid) {
					$temp_asset->fastTrack(Array('search_manager_reindex_metadata', 'trigger_event_metadata_updated'));
				}

				$ok = $ok && $this->generateContentFile($assetid, TRUE, $contextid, $update_asset);

				// Reset fast tracked tasks.
				if ($contextid != $last_contextid) {
					$temp_asset->unFastTrack(Array('search_manager_reindex_metadata', 'trigger_event_metadata_updated'));
				}

				$GLOBALS['SQ_SYSTEM']->restoreContext();
			}
		} else {
			$ok = $this->generateContentFile($assetid, TRUE, $contextid, $update_asset);
		}

		return $ok;

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
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		$bind_vars	= Array (
						'assetid'	=> $assetid,
					  );
		try {
			$result = MatrixDAL::executeQuery('core', 'purgeMetadataSchema', $bind_vars);
		} catch (Exception $e) {
			throw new Exception('Unable to purge metadata schema for assetid #'.$assetid.' due to the following database error:'.$e->getMessage());
		}

		try {
			$result = MatrixDAL::executeQuery('core', 'purgeMetadataValue', $bind_vars);
		} catch (Exception $e) {
			throw new Exception('Unable to purge metadata value for assetid #'.$assetid.' due to the following database error:'.$e->getMessage());
		}

		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();

		return TRUE;

	}//end purgeMetadata()


	/**
	* Get the value of an asset's metadata field based on that field's asset ID to populate its interface
	*
	* @param string		$assetid		The ID of the asset that we want the
	*									metadata value of
	* @param string		$field_assetid	The field that we wish to check
	* @param boolean	$mute_errors	Set TRUE to mute errors that occur when
	*									the asset (specified in $assetid) does
	*									not have a schema applied to it
	* @param boolean	$has_errors		This parameter is set to TRUE when an
	* 									error occurs so that errors are acknowledged
	* 									even they are mute ($mute_errors = TRUE)
	* @param int		$contextid		The context to get the value of metadata for
	*
	* @return array
	* @access public
	*/
	function getMetadataInterfaceValueByAssetid($assetid, $field_assetid, $mute_errors=FALSE, &$has_errors=FALSE, $contextid=NULL)
	{
		if (empty($assetid)) return NULL;
		
		// is this actually a metadata field or what?!
		if (!$this->isMetadataFieldAssetid($field_assetid)) {
			$has_errors = TRUE;
			if (!$mute_errors) {
				trigger_localised_error('SYS0308', E_USER_WARNING, $field_assetid);
			}
			return NULL;
		}

		if ($contextid === NULL) {
			$contextid = $GLOBALS['SQ_SYSTEM']->getContextId();
		}

		// get the parent schema of this metadata field
		$schemaid = $GLOBALS['SQ_SYSTEM']->am->getParents($field_assetid, 'metadata_schema');

		// get the schemas that apply to this asset
		$schemas = $this->getSchemas($assetid, TRUE);

		if (array_intersect($schemas, array_keys($schemaid)) == Array()) {
			$has_errors = TRUE;
			if (!$mute_errors) {
				// SYS0307: Asset #<assetid> does not have the schema for field #<field assetid> applied
				trigger_localised_error('SYS0307', E_USER_WARNING, $assetid, $field_assetid);
			}
			return NULL;
		}

		$sql = 'SELECT
					value
				FROM
					'.SQ_TABLE_RUNNING_PREFIX.'ast_mdata_val mdv';
		$where = 'mdv.assetid	= :assetid
					AND	mdv.fieldid	= :field_assetid
					AND mdv.contextid = :contextid';
		$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 'mdv');

		try {
			$query = MatrixDAL::preparePdoQuery($sql.$where);
			MatrixDAL::bindValueToPdo($query, 'assetid', $assetid);
			MatrixDAL::bindValueToPdo($query, 'field_assetid', $field_assetid);
			MatrixDAL::bindValueToPdo($query, 'contextid', $contextid);
			$value = MatrixDAL::executePdoOne($query);
		} catch (Exception $e) {
			// Unable to get metadata interface value due to DB error
			throw new Exception(translate_error('SYS0343', $field_assetid, $assetid, $contextid, $e->getMessage()));
		}//end try catch


		// Matrix DAL returns FALSE when no rows found, for consistancy we should return NULL
		if ($value === FALSE) {
			$value = NULL;
		}

		return $value;

	}//end getMetadataInterfaceValueByAssetid()


	/**
	* Get the value of an asset's metadata field based on that field's asset ID
	*
	* Because this value is for displaying on the front end, use the default value if required
	*
	* @param string		$assetid				The ID of the asset that we want the
	*											metadata value of
	* @param string		$field_assetid			The field that we wish to check
	* @param boolean	$mute_errors			Set TRUE to mute errors that occur when
	*											the asset (specified in $assetid) does
	*											not have a schema applied to it
	* @param boolean	$replace_nested_keyword	Set TRUE to replace nested keywords.
	*											If FALSE the function may return the value
	*											of the metadata field that contains keywords
	* @param int		$contextid				The context to get the value of metadata for
	*
	* @return array
	* @access public
	*/
	function getMetadataValueByAssetid($assetid, $field_assetid, $mute_errors=FALSE, $replace_nested_keyword=FALSE, $contextid=NULL)
	{
		// Could change the context here, but the asset isn't used for anything
		// other than getting a data path, so will refrain for now
		if ($contextid === NULL) {
			$contextid = $GLOBALS['SQ_SYSTEM']->getContextId();
		}
		$contextid = (Int) $contextid;
		
		if ($contextid === 0) {
			$metadata_basename = 'metadata.php';
		} else {
			$metadata_basename = 'metadata.'.$contextid.'.php';
		}

		$value = '';

		// sometimes there are nested keywords in the value of the metadata field
		// We are asked to replace these nested keywords
		if ($replace_nested_keyword) {

			// get the metadata value from the generated metadata.php file
			$am =& $GLOBALS['SQ_SYSTEM']->am;
			$asset = $am->getAsset($assetid);
			$_metadata_filename = $asset->data_path.'/'.$metadata_basename;

			// if the file exists then retrieve the value of the metadata field
			if (is_file($_metadata_filename)) {
				$metadata = $this->loadMetadataFile($assetid, $_metadata_filename);

				if (isset($metadata['values'])) {
					$metadata_values = $metadata['values'];
					$field_names = $GLOBALS['SQ_SYSTEM']->am->getAssetInfo(Array($field_assetid), 'metadata_field', FALSE, 'name');
					if(isset($metadata_values[$field_names[$field_assetid]]))
					    $value = $metadata_values[$field_names[$field_assetid]];
				}	
			}
		}

		if ($value === '') {
			$value = $this->getMetadataInterfaceValueByAssetid($assetid, $field_assetid, $mute_errors, $has_errors, $contextid);

			//if there is no error such as the field ID is not a metadata field ID or the metadata schema containing
			//that field is not applied to the asset
			if (!$has_errors && is_null($value)) {
				// not set - need the default value
				$value = $this->getMetadataFieldDefaultValue($field_assetid, $contextid);
			}
		}

		return $value;

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
			$sql = 'SELECT
						COUNT(*)
					FROM
						sq_ast_typ_inhd ati
							JOIN '.SQ_TABLE_RUNNING_PREFIX.'ast a ON ati.type_code = a.type_code';
			$where = 'a.assetid = :field_assetid
					AND	ati.inhd_type_code = :inhd_type_code';
			$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 'a');

			try {
				$query = MatrixDAL::preparePdoQuery($sql.$where);
				MatrixDAL::bindValueToPdo($query, 'field_assetid', $field_assetid);
				MatrixDAL::bindValueToPdo($query, 'inhd_type_code', 'metadata_field');
				$count = MatrixDAL::executePdoOne($query);
			} catch (Exception $e) {
				throw new Exception('Unable to determine whether the asset is a metadata field due to the following database error:'.$e->getMessage());
			}//end try catch

			$cached_tests[$field_assetid] = ($count > 0);
		}

		return $cached_tests[$field_assetid];

	}//end isMetadataFieldAssetid()


	/**
	* Returns whether the asset ID being passed into this function is some kind
	* of metadata field asset
	*
	* @param string	$field_assetid	the ID of the metadata field asset
	* @param string	$contextid		the context that this applies to
	*
	* @return boolean
	* @access public
	*/
	function getMetadataFieldDefaultValue($field_assetid, $contextid=NULL)
	{
		if ($contextid === NULL) {
			$contextid = $GLOBALS['SQ_SYSTEM']->getContextId();
		}

		// Whoops, the asset ID passed isn't a metadata field
		if (!$this->isMetadataFieldAssetid($field_assetid)) {
			trigger_localised_error('SYS0308', E_USER_WARNING, $field_assetid);
		}

		// If the current context is the default context, this has to get the default value in
		// the default context, so, it doesn't matter if the field is contextable or not.
		// If this is not a contextable metadata field, then grab the default value from context zero.
		if ($contextid != 0) {
			// If the asset is already in asset cache, get the attribute from the cached asset - it's quicker than connecting to the db
			$asset = $GLOBALS['SQ_SYSTEM']->am->_asset_cache->get($contextid.'\\'.$field_assetid);
			if (empty($asset)) {
				// Try Deja vu
				$deja_vu = $GLOBALS['SQ_SYSTEM']->getDejaVu();
				if ($deja_vu) {
					$asset = $deja_vu->recall(SQ_DEJA_VU_ASSET, $field_assetid);
				}
			} 
			
			if (!empty($asset)) {
				$is_contextable = (boolean)$asset->attr('is_contextable');
			} else {
				$is_contextable = $GLOBALS['SQ_SYSTEM']->am->getAttributeValuesByName('is_contextable', 'metadata_field', Array($field_assetid));
				$is_contextable = (boolean)$is_contextable[$field_assetid];
			}

			if ($is_contextable === FALSE) {
				$contextid = 0;
			}
		}

		static $cached_defaults = Array();
		$recache_value = TRUE;
		if (in_array($contextid, array_keys($cached_defaults)) === TRUE) {
			if (in_array($field_assetid, array_keys($cached_defaults[$contextid])) === TRUE) {
				$recache_value = FALSE;
			}
		}

		// We should recache the value
		if ($recache_value === TRUE) {
			$sql = 'SELECT
						default_val
					FROM
						'.SQ_TABLE_RUNNING_PREFIX.'ast_mdata_dflt_val mdv';
			$where = 'mdv.assetid = :field_assetid AND mdv.contextid = :contextid';
			$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 'mdv');

			try {
				$query = MatrixDAL::preparePdoQuery($sql.$where);
				MatrixDAL::bindValueToPdo($query, 'field_assetid', $field_assetid);
				MatrixDAL::bindValueToPdo($query, 'contextid', $contextid);
				$default_val = MatrixDAL::executePdoOne($query);
			} catch (Exception $e) {
				throw new Exception(translate_error('SYS0335', $field_assetid, $contextid, $e->getMessage()));
			}//end try catch

			$cached_defaults[$contextid][$field_assetid] = $default_val;
		}

		return $cached_defaults[$contextid][$field_assetid];

	}//end getMetadataFieldDefaultValue()


	/**
	* Sets the default value of a certain metadata field asset (as identified
	* by assetid)
	*
	* @param string	$field_assetid	the ID of the metadata field asset
	* @param string	$value			the value to be stored
	* @param string	$contextid		the context that this applies to
	*
	* @return boolean
	* @access public
	*/
	function setMetadataFieldDefaultValue($field_assetid, $value, $contextid=NULL)
	{
		if ($contextid === NULL) {
			$contextid = $GLOBALS['SQ_SYSTEM']->getContextId();
		}

		// is this actually a metadata field or what?!
		if (!$this->isMetadataFieldAssetid($field_assetid)) {
			trigger_localised_error('SYS0308', E_USER_WARNING, $field_assetid);
		}

		// If the current context is the default context, this has to get the default value in
		// the default context, so, it doesn't matter if the field is contextable or not.
		// If this is not a contextable metadata field, then grab the default value from context zero.
		if ($contextid != 0) {
			// If the asset is already in asset cache, get the attribute from the cached asset - it's quicker than connecting to the db
			$asset = $GLOBALS['SQ_SYSTEM']->am->_asset_cache->get($contextid.'\\'.$field_assetid);
			if (empty($asset)) {
				// Try Deja vu
				$deja_vu = $GLOBALS['SQ_SYSTEM']->getDejaVu();
				if ($deja_vu) {
					$asset = $deja_vu->recall(SQ_DEJA_VU_ASSET, $field_assetid);
				}
			} 
			
			if (!empty($asset)) {
				$is_contextable = (boolean)$asset->attr('is_contextable');
			} else {
				$is_contextable = $GLOBALS['SQ_SYSTEM']->am->getAttributeValuesByName('is_contextable', 'metadata_field', Array($field_assetid));
				$is_contextable = (boolean)$is_contextable[$field_assetid];
			}

			if ($is_contextable === FALSE) {
				$contextid = 0;
			}
		}

		$sql = 'SELECT
					default_val
				FROM
					'.SQ_TABLE_RUNNING_PREFIX.'ast_mdata_dflt_val mdv';
		$where = 'mdv.assetid = :field_assetid
					AND mdv.contextid = :contextid';
		$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 'mdv');

		try {
			$query = MatrixDAL::preparePdoQuery($sql.$where);
			MatrixDAL::bindValueToPdo($query, 'field_assetid', $field_assetid);
			MatrixDAL::bindValueToPdo($query, 'contextid', $contextid);
			$row = MatrixDAL::executePdoAll($query);
			if (isset($row[0]) && !empty($row[0])) {
				$row = $row[0];
			}
		} catch (Exception $e) {
			// "Cannot get default value of field in context, DB error"
			throw new Exception(translate_error('SYS0336', $field_assetid, $contextid, $e->getMessage()));
		}

		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		try {
			// does the default value exist yet, then?
			if (is_null($row) || empty($row)) {
				// "Cannot set default value of field in context, DB error"
				$message = 'SYS0336';
				$bind_vars	= Array (
								'field_assetid'	=> $field_assetid,
								'contextid'		=> $contextid,
								'value'			=> $value,
							  );
				$value = MatrixDAL::executeQuery('core', 'setMetadataFieldDefaultValue', $bind_vars);
			} else {
				// "Cannot update default value of field in context, DB error"
				$message = 'SYS0337';
				$bind_vars	= Array (
								'field_assetid'	=> $field_assetid,
								'contextid'		=> $contextid,
								'default_val'	=> $value,
							  );
				$value = MatrixDAL::executeQuery('core', 'updateMetadataFieldDefaultValue', $bind_vars);
			}
		} catch (Exception $e) {
			throw new Exception(translate_error($message, $field_assetid, $contextid, $e->getMessage()));
		}//end try catch


		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();

		return TRUE;

	}//end setMetadataFieldDefaultValue()


	/**
	* Escapes the passed value to be used inside quotes in a block of PHP (eg.
	* when the field names and values are cached out to each asset's metadata.php)
	*
	* @param string		$value			the value to escape
	* @param boolean	$escape_single	whether or not the apostrophes are escaped with backslashes; defaults to FALSE
	*
	* @return string
	* @access public
	*/
	function escapeMetadata($value, $escape_single=FALSE)
	{
		if ($escape_single) {
			$value = str_replace("'", "\'", $value);
		}

		// For the thesaurus metadata, using commas to separate values
		// We need to replace the commas in the thesaurus back.
		$value = str_replace('&#44;', ',', $value);

		// escape only html-specific chars for the moment
		$value = htmlspecialchars($value);

		// escape everything else (chars > 126)
		$result = '';

		// if default character set is utf-8 we dont wanna mess around the characters
		// we are for now just checking for it not to be utf-8 but can append other encoding
		// we might want to skip later on
		if (strtolower(SQ_CONF_DEFAULT_CHARACTER_SET) != 'utf-8') {
			// escape everything else (chars > 126)
			for ($i = 0; $i < strlen($value); ++$i) {
				$ord = ord($value[$i]);
				if ($ord > 126) {
					$result .= '&#'.$ord.';';
				} else {
					$result .= $value[$i];
				}
			}// end for
		}

		return $result ? $result : $value;

	}//end escapeMetadata()


	/**
	 * Given the ID of an asset that has metadata applied and the name of a
	 * field that is applied to that asset, get the asset ID of that metadata field
	 *
	 * Returns FALSE if the asset ID does not exist, or if the asset doesn't
	 * have a field matching the field name
	 *
	 * Throws an assertion exception if the asset ID is completely invalid, or
	 * if the field name is not a string.
	 *
	 * @param string	$assetid			The ID of the asset we wish to check
	 * @param string	$target_field_name	The name of the field we wish to find
	 *
	 * @return mixed string|boolean
	 */
	public function getFieldAssetIdFromName($assetid, $target_field_name)
	{
		assert_valid_assetid($assetid);
		assert_true(is_string($target_field_name));

		// Get a list of the applied schemas' field assets - first their IDs,
		// then their names
		$schemaids = $this->getSchemas($assetid, TRUE);
		$fields = $this->getMetadataFields($schemaids);

		$field_assetids = array_keys($fields);
		$field_names = $GLOBALS['SQ_SYSTEM']->am->getAssetInfo($field_assetids, 'metadata_field', FALSE, 'name');

		// Is the field name we want in this list?
		// If found, it will return the key (asset ID) that we want. If not,
		// it will return FALSE. Either way, return this to the function
		$assetid = array_search($target_field_name, $field_names);

		return $assetid;

	}//end getFieldAssetIdFromName()



	/**
	 * For given asset with metadata schema(s) applied, this function returns 
	 * the value for all metadata fields of given type in following format:
	 *
	 * 	Array( 
	 *		'<fieldname>' => '<metadata_value>'
	 * 	)
	 *
	 * @param string	$assetid				The ID of the asset we wish to get metadata value of
	 * @param string	$metadata_field_type	The asset type of the field we wish to get
	 * @param boolean	$contextid
	 *
	 * @return array
	 * @access public
	 */
	public function getMetadataFieldTypeValues($assetid, $metadata_field_type, $contextid=NULL) 
	{
		$schemas = $this->getSchemas($assetid, TRUE);
		if (empty($schemas)) return Array();

		$metadata_field_type = strtolower($metadata_field_type);
		$field_values = Array();
		
		foreach($schemas as $schemaid) {
			$fields = $this->getMetadataFields($schemaid);
			
			foreach($fields as $fieldid => $field_type) {
				if ($field_type[0]['type_code'] != $metadata_field_type) continue;
				$field_info = $GLOBALS['SQ_SYSTEM']->am->getAssetInfo($fieldid);
				if (empty($field_info)) continue;

				$field_values[$field_info[$fieldid]['name']] = $this->getMetadataValueByAssetid($assetid, $fieldid, FALSE, FALSE, $contextid);
			}
		}

		return $field_values;
	}


}//end class
?>
