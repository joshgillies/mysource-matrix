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
* $Id: metadata_field_hierarchy.inc,v 1.6 2009/10/12 05:13:38 lwright Exp $
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
* @version $Revision: 1.6 $
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

		$contextid = $GLOBALS['SQ_SYSTEM']->getContextId();

		if ($contextid === 0) {
			// we're in the current context, so we can just get the select
			// options straight
			$selection->_params['options'] = $this->attr('hierarchy_options');
		} else {
			$hierarchy_options = $this->attr('hierarchy_options');

			// Get the current default keys, and fill up the empty keys in the
			// current context
			$GLOBALS['SQ_SYSTEM']->changeContext(0);
			$default_asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($this->id);
			$default_options = $default_asset->attr('hierarchy_options');
			$GLOBALS['SQ_SYSTEM']->am->forgetAsset($default_asset);
			unset($default_asset);
			$GLOBALS['SQ_SYSTEM']->restoreContext();

			foreach ($default_options as $default_key => $default_value) {
				if (array_key_exists($default_key, $select_options) === FALSE) {
					$hierarchy_options[$default_key] = $default_value;
				}
			}

			$selection->_params['options'] = $hierarchy_options;
		}

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
	* Get the default values for this field
	*
	* @return array
	* @access public
	*/
	public function getDefaultValue()
	{
		// return the correct default key/value depending on the 'Visible Part'
		$default_key = parent::getDefaultValue();

		// using semi-colons to delimit so remove any escaping done by DAL
		$default_key = str_replace('\\', '', $default_key);

		$options = $this->attr('hierarchy_options');

		if (is_array($options)) {
			$edit_fns = $this->getEditFns();
			$default_key = $edit_fns->getHierarchyTree($default_key, $options);
		}

		return $default_key;

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
		if (empty($option_keys)) return $option_str;

		$value_str = '';
		$options = $this->attr('hierarchy_options');

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


}//end class

?>
