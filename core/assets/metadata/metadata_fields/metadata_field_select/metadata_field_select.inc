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
* $Id: metadata_field_select.inc,v 1.41 2012/08/30 01:09:09 ewang Exp $
*
*/

require_once dirname(__FILE__).'/../../metadata_field/metadata_field.inc';
require_once SQ_FUDGE_PATH.'/csv/csv.inc';


/**
* Metadata_Field_Select
*
* Purpose
*
*
* @author  Dmitri Iarandine <diarandine@squiz.net>
* @version $Revision: 1.41 $
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

			// Validate and fix the option key values
			$validated_values = Array();
			foreach($value as $key => $option) {
				// Option keys cannot contain '; ' string
				if (strpos($key, '; ') !== FALSE) {
					trigger_localised_error('SYS0353', sprintf(translate('Cannot use the Metadata Select Field\'s reserved delimiter string "; " (semi-colon and space) as a part of the option key in "%1$s". Replacing "; " with ";".'), $key), E_USER_NOTICE);
					$key = preg_replace('/;\s+/', ';', $key);
				}
				$validated_values[$key] = $option;
			}
			$value = $validated_values;
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
				$sub_where = 'WHERE assetid = :host_assetid_1 ';
				$sub_where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($sub_where);

				$union_sql = 'SELECT default_val FROM '.SQ_TABLE_RUNNING_PREFIX.'ast_mdata_dflt_val';
				$union_where = '%tuple_keyword%(assetid, contextid) NOT IN ('.$sub_sql.$sub_where.') AND assetid = :fieldid_1 AND contextid = :contextid_1';
				$union_where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($union_where);

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
								'host_assetid'		=> (string)$host_assetid,
								'fieldid'			=> (string)$this->id,
								'contextid'			=> (int)$contextid,
								'host_assetid_1'	=> (string)$host_assetid,
								'fieldid_1'			=> (string)$this->id,
								'contextid_1'		=> (int)$contextid,
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


	/**
	* Replace options with an imported CSV file.
	*
	* If 1 column: value only - key is sequentially assigned (from 0)
	* If 2 columns: key, value
	* If 3 columns: key, value, default [1|0]
	*
	* @param string	 $filename	The (temp) file name to import
	* @param boolean $header    Whether the file has a csv header or not
	*
	* @return void
	*/
	public function importOptionsFromCSV($filename, $header=FALSE)
	{
		$csv = new CSV($filename);

		$start_line = 1;
		if ($header == TRUE) {
			$start_line = 2;
		}
		$csv->import($start_line);

		$values = $csv->values;
		$options = Array();
		$defaults = Array();

		if (!empty($values)) {
			// We will assume that the number of columns in the first row
			// is representative of the entire list
			$num_columns = count($values[0]);
			$index = 0;

			foreach ($values as $row) {
				// if the very first bit for us is not populated
				/// it means its a empty line, discard it !!!
				if (trim($row[0]) === '') continue;
				if ($num_columns === 1) {
					// If we haven't provided a first column, then key and value
					// are the same
					$value = htmlentities($row[0], ENT_NOQUOTES, SQ_CONF_DEFAULT_CHARACTER_SET);
					$options[$index] = $value;
					$index++;
				} else {
					$key = htmlentities($row[0], ENT_NOQUOTES, SQ_CONF_DEFAULT_CHARACTER_SET);
					$value = htmlentities($row[1], ENT_NOQUOTES, SQ_CONF_DEFAULT_CHARACTER_SET);
					$options[$key] = $value;

					if ($num_columns === 3) {
						if ($row[2] == 1) {
							$defaults[] = $key;
						}
					}
				}
			}

			$this->setAttrValue('select_options', $options);
			if ($num_columns === 3) {
				if (!$this->attr('multiple')) {
					if (count($defaults) > 1) {
						trigger_localised_error('CORE0309', translate('More than one default option was set in the CSV file, but the Metadat Field is not set for multiple selection. Only the first default option found will be used.'), E_USER_NOTICE);
					}
					if (count($defaults) > 0) {
						$defaults = Array($defaults[0]);
					} else {
						$defaults = Array();
					}
				}

				$this->setAttrValue('default', implode('; ', $defaults));
			} else {
				// What happens if it gets overwritten?
				$current_default = $this->attr('default');
				if (!is_array($current_default)) {
					$current_default = Array($current_default);
				}
				foreach ($current_default as $default_key) {
					// Remove any default options that are not choices in this
					// particular situation
					if (!isset($options[$default_key])) {
						unset($current_default[$default_key]);
					}
				}
				$this->setAttrValue('default', implode('; ', $current_default));
			}
			$this->saveAttributes();
		}

	}//end importOptionsFromCSV()


}//end class

?>
