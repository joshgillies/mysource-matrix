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
* $Id: metadata_field_select.inc,v 1.27 2009/10/12 22:57:45 lwright Exp $
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
* @version $Revision: 1.27 $
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
	* Get the default values for this field
	*
	* @return array
	* @access public
	*/
	public function getDefaultValue()
	{
		// return the correct default key/value depending on the 'Visible Part'
		$default_key = parent::getDefaultValue();
		if ($this->attr('visible_part') == 'key') {
			return $default_key;
		} else if ($this->attr('visible_part') == 'value') {
			return $this->getValueFromKey($default_key);
		}

	}//end getDefaultValue()


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
		return str_replace('\\;', ';', parent::getMetadataValue($asset, $this->getValueFromKey($value_str)));

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
		if (empty($option_keys) || $this->attr('visible_part') == 'key') {
			return $option_str;
		}

		$value_str = '';
		$options = $this->getContextedSelectOptions();

		foreach ($option_keys as $key) {
			if (isset($options[$key])) {
				$value_str .= $options[$key].'\\; ';
			}
		}
		if (strlen($value_str) == 0) {
			// no corresponding values found
			return $option_str;
		}

		// trim the last separator
		$value_str = substr($value_str, 0, -3);

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
				$GLOBALS['SQ_SYSTEM']->changeContext(0);
				$default_asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($this->id);
				$default_options = $default_asset->attr('select_options');
				$GLOBALS['SQ_SYSTEM']->am->forgetAsset($default_asset);
				unset($default_asset);
				$GLOBALS['SQ_SYSTEM']->restoreContext();

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

}//end class

?>
