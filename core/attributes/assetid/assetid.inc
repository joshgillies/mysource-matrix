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
* $Id: assetid.inc,v 1.13 2006/01/10 01:26:15 tbarrett Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_attribute.inc';

/**
* Assetid Asset Attribute
*
*
*
*
* <!-- Sample Edit Interface XML Node -->
*    <assetid type_codes="user_group:I,backend_user:A,page:D" />
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Revision: 1.13 $
* @package MySource_Matrix
* @subpackage attributes
*/
class Asset_Attribute_Assetid extends Asset_Attribute
{


	/**
	* Constructor
	*
	* @param int	$attributeid	the attribute id to be loaded
	* @param mixed	$value			the current value for the element
	*
	*/
	function Asset_Attribute_Assetid($attributeid=0, $value=null)
	{
		$this->Asset_Attribute($attributeid, $value);
		$this->_edit_params['type_codes'] = Array();

		// default values
		$this->_edit_params['type_codes'] = Array();

	}//end constructor


	/**
	* Takes an XML_Tree_Node from an edit interface XML file and creates
	* the params array from it
	*
	* @param object XML_Tree_Node	$node
	*
	* @return boolean
	* @access public
	*/
	function setEditParams(&$node)
	{
		if (!parent::setEditParams($node)) return false;

		$this->_edit_params['type_codes'] = Array();
		if (isset($node->attributes['type_codes'])) {
			$codes = explode(',', $node->attributes['type_codes']);
			foreach ($codes as $code_name) {
				list($code, $flag) = explode(':', $code_name);
				$this->_edit_params['type_codes'][$code] = $flag;
			}
		}

		return true;

	}//end setEditParams()


	/**
	* Prints the interface for filling in a value
	*
	* @param string		$prefix		prefix for the form element
	* @param boolean	$read_only	are we just printing the value
	*
	* @return void
	* @access public
	*/
	function paint($prefix, $read_only=false)
	{
		if ($read_only) {
			if (!$this->value) {
				echo translate('no_asset_selected');
			} else {
				echo get_asset_tag_line($this->value);
			}
			return;
		}

		require_once SQ_LIB_PATH.'/html_form/html_form.inc';
		asset_finder($prefix, $this->value, $this->_edit_params['type_codes']);

	}//end paint()


	/**
	* Prints the interface for filling in a value
	*
	* @param string	prefix	prefix for the form element
	*
	* @access public
	*/
	function process($prefix)
	{
		if (!isset($_REQUEST[$prefix]['assetid'])) {
			return false;
		}
		$value = $_REQUEST[$prefix]['assetid'];
		if ($this->value != $value && $this->setValue($value)) {
			$this->processed = true;
		} else {
			$this->processed = false;
		}

	}//end process()


	/**
	* Ensure the attribute being set is an integer
	*
	* @param mixed	&$value	value to be validated
	*
	* @return boolean
	* @access public
	*/
	function validateValue(&$value)
	{
		if ($value) {
			$error_msg = translate('fail_setting_assetid', $this->name, $this->id);
			if (!assert_valid_assetid($value, $error_msg, true, false)) {
				return false;
			}
			$asset = &$GLOBALS['SQ_SYSTEM']->am->getAsset($value);
			if (is_null($asset)) return false;
		}
		return true;

	}//end validateValue()


}//end class

?>
