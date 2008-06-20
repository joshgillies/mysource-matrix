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
* $Id: metadata_field_select.inc,v 1.23 2008/06/20 06:21:12 bpearson Exp $
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
* @version $Revision: 1.23 $
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
		$selection->_params['options'] = $this->attr('select_options');
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
		$options = $this->attr('select_options');

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
