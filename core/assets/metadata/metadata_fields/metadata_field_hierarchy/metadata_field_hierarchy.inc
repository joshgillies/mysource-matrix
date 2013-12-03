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
* $Id: metadata_field_hierarchy.inc,v 1.17 2012/08/30 01:09:09 ewang Exp $
*
*/


require_once dirname(__FILE__).'/../../metadata_field/metadata_field.inc';

/**
* Metadata_Field_Hierarchy
*
* Purpose
*
*
* @author  Benjamin Pearson <bpearson@squiz.net>
* @version $Revision: 1.17 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Metadata_Field_Hierarchy extends Metadata_Field
{


	/**
	* Constructor
	*
	* @param int	$assetid	the asset id to be loaded
	*
	*/
	function __construct($assetid=0)
	{
		parent::__construct($assetid);

	}//end constructor


	/**
	* Get a hierarchy attribute that can be used to paint and process the values of this field
	*
	* @return object
	* @access public
	*/
	public function getHierarchyAttribute()
	{
		require_once SQ_ATTRIBUTES_PATH.'/selection/selection.inc';

		$selection = new Asset_Attribute_Selection();
		$edit_params = $this->attr('edit_params');
		if (!empty($edit_params)) {
			$selection->_edit_params = array_merge($selection->_edit_params, $edit_params);
		}

		$selection->_params['options'] = $this->getContextedHierarchyOptions();
		$selection->_params['multiple'] = $this->attr('multiple');
		$selection->_params['allow_empty'] = $this->attr('allow_empty');
		$selection->_params['sort'] = $this->attr('hier_sort');
		$selection->_params['structure'] = $this->attr('hier_structure');

		$default = explode('; ', $this->attr('default'));
		if ($selection->validateValue($default)) {
			$selection->setValue($default);
			$selection->_default_value = $default;
		}

		return $selection;

	}//end getHierarchyAttribute()


	/**
	* Generate an array of current values for the passed metadata field names
	*
	* Returns the metadata value for the passed asset
	*
	* @param Asset	$asset		the asset whose field values we are getting
	* @param string	$value_str	the current value for this field in the asset (NOT $asset)
	*							NULL means that there is no current value set
	*
	* @return string
	* @access public
	*/
	public function getMetadataValue(Asset $asset, $value_str)
	{
		
		// If we are showing the value, then pass it up
		if ($this->attr('visible_part') === 'value') {
			$value_str = $this->getValueFromKey($value_str);
		}
		return str_replace('\\;', ';', parent::getMetadataValue($asset, $value_str));

	}//end getMetadataValue()


	/**
	* Returns the option value(s) corresponding to the given option key(s)
	* If no corresponding value is found, the option_str is returned
	*
	* @param string	$option_str	an string of one or more option keys
	*
	* @return string
	* @access public
	*/
	public function getValueFromKey($option_str)
	{
		$option_keys = explode('; ', $option_str);
		if (empty($option_keys)) return $option_str;

		$value_str = '';
		$options = $this->getContextedHierarchyOptions();

		foreach ($option_keys as $key) {
			$key = str_replace('\\', '', $key);
			if (isset($options[$key])) {
				$value_str .= $options[$key].'; ';
			}
		}
		if (strlen($value_str) == 0) {
			// no corresponding values found
			return $option_str;
		}

		// trim the last separator
		$value_str = substr($value_str, 0, -2);
		$value_str = Metadata_Field::encodeValueString($value_str, Array());

		return $value_str;

	}//end getValueFromKey()


	/**
	* Get the hierarchy options that apply to this context
	*
	* This method gets the current hierarchy options saved to the context, and
	* augments them with any keys now in the default context (and its values),
	* and removes the contexts that no longer exist.
	* This is necessary because the default context is the ultimate authority
	* on what keys exist.
	*
	* These are cached in a _tmp var indexed by context ID, and is cleared when
	* the hierarchy_options attribute is saved.
	*
	* @return array
	* @see Metadata_Field_Hierarchy::_clearContextedHierarchyOptionsCache()
	*/
	public function getContextedHierarchyOptions()
	{
		// Create the cache array if it doesn't exist yet
		if (array_key_exists('cached_hierarchy_options', $this->_tmp) === FALSE) {
			$this->_tmp['cached_hierarchy_options'] = Array();
		}

		$contextid = $GLOBALS['SQ_SYSTEM']->getContextId();

		if ($contextid === 0) {
			// Don't really need to cache the hierarchy options in the default
			// context, because it is it's our own law to itself
			return $this->attr('hierarchy_options');
		} else {
			// Alternate context
			if (array_key_exists($contextid, $this->_tmp['cached_hierarchy_options']) === FALSE) {
				// Get the base
				$hierarchy_options = $this->attr('hierarchy_options');

				// Get ourselves in default context form, and get the current
				// default keys and values
				$default_options = $GLOBALS['SQ_SYSTEM']->am->getAttributeValuesByName('hierarchy_options', 'metadata_field_hierarchy', Array($this->id), 0);
				$default_options = unserialize($default_options[$this->id]);

				// Banish any keys that don't exist in the default version
				foreach ($hierarchy_options as $hierarchy_key => $hierarchy_value) {
					if (array_key_exists($hierarchy_key, $default_options) === FALSE) {
						unset($hierarchy_options[$hierarchy_key]);
					}
				}

				// Add any keys that don't exist in the contexted version,
				// with the value added from the default context
				foreach ($default_options as $default_key => &$default_value) {
					if (array_key_exists($default_key, $hierarchy_options) === FALSE) {
						$hierarchy_options[$default_key] = $default_value;
					}
				}

				$this->_tmp['cached_hierarchy_options'][$contextid] = $hierarchy_options;
			}

			return $this->_tmp['cached_hierarchy_options'][$contextid];
		}

	}//end getContextedHierarchyOptions()


	/**
	* Cleans up the contexted hierarchy options cache
	*
	* This resets the cache set up by the above function. When the context is
	* the default, it resets everything (since all contexts depend on it). In
	* an alternate context, it only resets that context's cache.
	*
	* @see Metadata_Field_Hierarchy::getContextedHierarchyOptions()
	* @return void
	*/
	protected function _clearContextedHierarchyOptionsCache()
	{
		// Create the cache array if it doesn't exist yet
		if (array_key_exists('cached_hierarchy_options', $this->_tmp) === TRUE) {
			$contextid = $GLOBALS['SQ_SYSTEM']->getContextId();

			if ($contextid === 0) {
				$this->_tmp['cached_hierarchy_options'] = Array();
			} else {
				if (array_key_exists($contextid, $this->_tmp['cached_hierarchy_options']) === TRUE) {
					unset($this->_tmp['cached_hierarchy_options'][$contextid]);
				}
			}
		}

	}//end _clearContextedHierarchyOptionsCache()


	/**
	* Returns the option key(s) corresponding to the given option value(s)
	* If no corresponding key is found, the option_str is returned
	*
	* @param string	$option_str	an string of one or more option keys
	*
	* @return string
	* @access public
	*/
	public function getKeyFromValue($option_str)
	{
		$option_keys = explode('; ', $option_str);
		if (empty($option_keys)) return $option_keys;

		$value_str = '';
		$options = $this->attr('hierarchy_options');

		foreach ($option_keys as $key) {
			$key = str_replace('\\', '', $key);
			foreach($options as $index => $element) {
				if ($element === $key) {
					$value_str .= $index.'\\; ';
				}
			}
		}
		if (strlen($value_str) == 0) {
			// no corresponding values found
			return $option_str;
		}

		// trim the last separator
		$value_str = substr($value_str, 0, -3);

		return $value_str;

	}//end getKeyFromValue()
	
	
	/**
	* Return any additional keyword that may be provided by the field
	*
	* This is mainly used in situations such as returning the current key and
	* value for Select fields. Where a keyword is not supported, the function
	* should return NULL.
	*
	* Note that if asset_metadata keywords conflict due to additional keywords,
	* the one with the longest field name will prevail. For instance, a keyword
	* 'asset_metadata_field_value' will select a field named 'field_value' over
	* a field named 'field' with an additional keyword 'value'.
	*
	* @param string	$host_assetid		The ID of the asset that metadata details
	*                                   were requested from
	* @param string	$additional_keyword	The additional keyword requested
	* @param int	$contextid			Use the stored metadata value for this
	*                                   context. If omitted, use the current
	*                                   context.
	*
	* @return mixed string|NULL
	*/
	public function getAdditionalKeywordReplacement($host_assetid, $additional_keyword, $contextid=NULL)
	{
		$replacement = NULL;

		if ($contextid === NULL) {
			$contextid = $GLOBALS['SQ_SYSTEM']->getContextId();
		}

		switch ($additional_keyword) {
			case 'key':
			case 'value':
				// Go directly to the database, and get what it has (which
				// will be the key)
				// We need to narrow the host asset ID where it is because
				// if a default value needs to be used, the custom values table
				// will be all nulls - so we can't narrow by host asset ID by then

				// We need to get custom values from the metadata value table,
				// then default values from the default value table where they
				// don't exist in the custom value table.
				$sql = 'SELECT value FROM '.SQ_TABLE_RUNNING_PREFIX.'ast_mdata_val ';
				$where = 'assetid = :host_assetid AND fieldid = :fieldid AND contextid = :contextid';
				$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where);
				$sql .= $where;

				// Now build the default values query (and sub-query).
				$sub_sql = 'SELECT fieldid, contextid FROM '.SQ_TABLE_RUNNING_PREFIX.'ast_mdata_val ';
				$sub_where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause('', 'ast_mdata_val');

				$union_sql = 'SELECT default_val FROM '.SQ_TABLE_RUNNING_PREFIX.'ast_mdata_dflt_val';
				$union_where = '%tuple_keyword%(assetid, contextid) NOT IN ('.$sub_sql.$sub_where.') AND assetid = :fieldid_1 AND contextid = :contextid_1';
				$union_where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($union_where, 'ast_mdata_dflt_val');

				// Oracle uses '(field, field)' for tuples, PgSQL uses 'ROW(field, field)'.
				if (MatrixDAL::getDbType() === 'oci') {
					$union_where = str_replace('%tuple_keyword%', '', $union_where);
				} else {
					$union_where = str_replace('%tuple_keyword%', 'ROW', $union_where);
				}
				$union_sql .= $union_where;
				$sql = '('.$sql.') UNION ALL ('.$union_sql.')';

				// Go go go!
				$bind_vars = Array(
								'host_assetid'	=> (string)$host_assetid,
								'fieldid'		=> (string)$this->id,
								'contextid'		=> (int)$contextid,
								'fieldid_1'		=> (string)$this->id,
								'contextid_1'	=> (int)$contextid,
							 );

				$query = MatrixDAL::preparePdoQuery($sql);
				foreach ($bind_vars as $bind_var => $bind_value) {
					MatrixDAL::bindValueToPdo($query, $bind_var, $bind_value);
				}

				$key = MatrixDAL::executePdoOne($query);

				// Does the metadata field value (or a default value) exist?
				if ($key !== FALSE) {
					if ($additional_keyword === 'key') {
						$replacement = $key;
					} else if ($additional_keyword === 'value') {
						// Turn it into a value
						$replacement = $this->getValueFromKey($key);
					}
				}

			break;

		}//end switch
		
		if (is_null($replacement)){
			return $replacement;
		} else {
			$value='';
			$value_components = $this->attr('value_components');
			$this->decodeValueString($replacement, $value, $value_components);
			return $value;
		}

	}//end getAdditionalKeywordReplacement()


}//end class

?>
