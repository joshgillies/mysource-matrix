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
* $Id: metadata_field_select.inc,v 1.35.6.1 2011/10/25 01:12:55 mhaidar Exp $
*
*/


require_once dirname(__FILE__).'/../../metadata_field/metadata_field.inc';

/**
* Metadata_Field_Select
*
* Purpose
*
*
* @author  Dmitri Iarandine <diarandine@squiz.net>
* @version $Revision: 1.35.6.1 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Metadata_Field_Select extends Metadata_Field
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
	* Queue the setting of the value of an attribute
	*
	* Overrides the default only to clean out the cached select options array
	* when the select options are (re)set, so any future calls to
	* getContextedSelectOptions() are current.
	*
	* @param string	$name	the name of the attribute
	* @param string	$value	the new value of the attribute
	*
	* @return boolean
	* @access public
	* @see saveAttributes()
	*/
	public function setAttrValue($name, $value)
	{
		if ($name === 'select_options') {
			$this->_clearContextedSelectOptionsCache();
		}
		return parent::setAttrValue($name, $value);

	}//end setAttrValue()


	/**
	* Get a selection attribute that can be used to paint and process the values of this field
	*
	* @return object
	* @access public
	*/
	public function getSelectionAttribute()
	{
		require_once SQ_ATTRIBUTES_PATH.'/selection/selection.inc';

		$selection = new Asset_Attribute_Selection();
		$edit_params = $this->attr('edit_params');
		if (!empty($edit_params)) {
			$selection->_edit_params = array_merge($selection->_edit_params, $edit_params);
		}

		$selection->_params['options'] = $this->getContextedSelectOptions();
		$selection->_params['multiple'] = $this->attr('multiple');
		$selection->_params['allow_empty'] = $this->attr('allow_empty');

		$default = $this->attr('default');
		if ($this->attr('multiple')) {
			$default = explode('; ', $default);
		}
		if ($selection->validateValue($default)) {
			$selection->setValue($default);
		}

		return $selection;

	}//end getSelectionAttribute()


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
		$option_keys = explode('\\; ', $option_str);
		if (empty($option_keys)) {
			return $option_str;
		}

		$value_str = '';
		$options = $this->getContextedSelectOptions();

		foreach ($option_keys as $key) {
			$key = str_replace('\\;', ';', $key);
			$key = str_replace('\\=', '=', $key);
			$key = str_replace('\\\\', '\\', $key);
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
	* Get the select options that apply to this context
	*
	* This method gets the current select options saved to the context, and
	* augments them with any keys now in the default context (and its values),
	* and removes the contexts that no longer exist.
	* This is necessary because the default context is the ultimate authority
	* on what kets
	*
	* These are cached in a _tmp var indexed by context ID, and is cleared when
	* the select_options attribute is saved.
	*
	* @return array
	* @see Metadata_Field_Select::_clearContextedSelectOptionsCache()
	*/
	public function getContextedSelectOptions()
	{
		// Create the cache array if it doesn't exist yet
		if (array_key_exists('cached_select_options', $this->_tmp) === FALSE) {
			$this->_tmp['cached_select_options'] = Array();
		}

		$contextid = $GLOBALS['SQ_SYSTEM']->getContextId();

		if ($contextid === 0) {
			// Don't really need to cache the select options in the default
			// context, because it is it's our own law to itself
			return $this->attr('select_options');
		} else {
			// Alternate context
			if (array_key_exists($contextid, $this->_tmp['cached_select_options']) === FALSE) {
				// Get the base
				$select_options = $this->attr('select_options');

				// Get ourselves in default context form, and get the current
				// default keys and values
				$default_options = $GLOBALS['SQ_SYSTEM']->am->getAttributeValuesByName('select_options', 'metadata_field_select', Array($this->id), 0);
				$default_options = unserialize($default_options[$this->id]);

				// Banish any keys that don't exist in the default version
				foreach ($select_options as $select_key => $select_value) {
					if (array_key_exists($select_key, $default_options) === FALSE) {
						unset($select_options[$select_key]);
					}
				}

				// Add any keys that don't exist in the contexted version,
				// with the value added from the default context
				foreach ($default_options as $default_key => &$default_value) {
					if (array_key_exists($default_key, $select_options) === FALSE) {
						$select_options[$default_key] = $default_value;
					}
				}

				$this->_tmp['cached_select_options'][$contextid] = $select_options;
			}

			return $this->_tmp['cached_select_options'][$contextid];
		}

	}//end getContextedSelectOptions()


	/**
	* Cleans up the contexted select options cache
	*
	* This resets the cache set up by the above function. When the context is
	* the default, it resets everything (since all contexts depend on it). In
	* an alternate context, it only resets that context's cache.
	*
	* @see Metadata_Field_Select::getContextedSelectOptions()
	* @return void
	*/
	protected function _clearContextedSelectOptionsCache()
	{
		// Create the cache array if it doesn't exist yet
		if (array_key_exists('cached_select_options', $this->_tmp) === TRUE) {
			$contextid = $GLOBALS['SQ_SYSTEM']->getContextId();

			if ($contextid === 0) {
				$this->_tmp['cached_select_options'] = Array();
			} else {
				if (array_key_exists($contextid, $this->_tmp['cached_select_options']) === TRUE) {
					unset($this->_tmp['cached_select_options'][$contextid]);
				}
			}
		}

	}//end _clearContextedSelectOptionsCache()


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
		$option_keys = explode('\\; ', $option_str);
		if (empty($option_keys)) return $option_keys;

		$value_str = '';
		$options = $this->attr('select_options');

		foreach ($option_keys as $key) {
			foreach($options as $index => $element) {
				if ($index === $key) {
					$value_str .= $element.'\\; ';
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
