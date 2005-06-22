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
* $Id: metadata_field_select.inc,v 1.7.2.2 2005/06/22 01:34:32 dmckee Exp $
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
* @version $Revision: 1.7.2.2 $
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
	function Metadata_Field_Select($assetid=0)
	{
		$this->Metadata_Field($assetid);

	}//end constructor


	/**
	* Get a selection attribute that can be used to paint and process the values of this field
	*
	* @return object Asset_Attribute_Selection
	* @access public
	*/
	function getSelectionAttribute()
	{
		require_once SQ_ATTRIBUTES_PATH.'/selection/selection.inc';

		$selection = new Asset_Attribute_Selection();
		$edit_params = $this->attr('edit_params');
		if (!empty($edit_params)) {
			$selection->_edit_params = $edit_params;
		}
		$selection->_params['options'] = $this->attr('select_options');
		$selection->_params['multiple'] = $this->attr('multiple');

		$default = $this->attr('default');
		if ($this->attr('multiple')) {
			$default = explode('; ', $default);
		}
		$selection->setValue($default);

		return $selection;

	}//end getSelectionAttribute()


	/**
	* Generate an array of current values for the passed metadata field names
	*
	* @param object Asset			&$asset		the asset whose field values we are getting
	* @param string					$value_str	the current value for this field in the asset (NOT $asset)
	*											NULL means that there is no current value set
	*
	* @access public
	* @return string	the metadata value for the passed asset
	*/
	function getMetadataValue(&$asset, $value_str)
	{
		return str_replace('\\;', ';', parent::getMetadataValue($asset, $this->getValueFromKey($value_str)));

	}//end getMetadataValue()


	/**
	 * Returns the option value corresponding to the given option key
	 * If no corresponding value is found, the option_key is returned
	 *
	 * @param string $option_key	an option key
	 *
	 * @return string
	 * @access public
	 */
	function getValueFromKey($option_key)
	{
		$ret = $option_key;

		if (isset($option_key) && $this->attr('visible_part') == 'value') {
			// returns the Value instead of the Key
			$options = $this->attr('select_options');
			if (isset($options[$option_key])) {
				$ret = $options[$option_key];
			}
		}

		return $ret;

	}//end getValueFromKey()


}//end class

?>