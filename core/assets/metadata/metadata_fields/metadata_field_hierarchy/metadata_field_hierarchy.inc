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
* $Id: metadata_field_hierarchy.inc,v 1.10.6.1 2011/05/04 04:49:15 mhaidar Exp $
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
* @version $Revision: 1.10.6.1 $
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
		if (empty($option_keys)) return $option_str;

		$value_str = '';
		$options = $this->getContextedHierarchyOptions();

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
		$option_keys = explode('\\; ', $option_str);
		if (empty($option_keys)) return $option_keys;

		$value_str = '';
		$options = $this->attr('hierarchy_options');

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

	}//end getValueFromKey()


}//end class

?>
