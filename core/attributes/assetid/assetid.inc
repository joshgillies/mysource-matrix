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
* $Id: assetid.inc,v 1.5 2004/09/08 04:17:47 gsherwood Exp $
* $Name: not supported by cvs2svn $
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
* @version $Version$ - 1.0
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
		$this->Asset_Attribute($attributeid, (int) $value);

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
	* @access public
	*/
	function paint($prefix, $read_only=false)
	{
		if ($read_only) {
			if (!$this->value) {
				echo 'No asset selected';
			} else {
				$asset = &$GLOBALS['SQ_SYSTEM']->am->getAsset($this->value);
				if (is_null($asset)) echo 'Invalid assetid';
				else echo $asset->name.' (#'.$asset->id.')';
			}
			return;
		}

		require_once SQ_LIB_PATH.'/html_form/html_form.inc';
		asset_finder($prefix, $this->value, $this->_edit_params['type_codes']);

	}// end paint()


	/**
	* Prints the interface for filling in a value
	*
	* @param string	prefix	prefix for the form element
	*
	* @access public
	*/
	function process($prefix)
	{
		if (!isset($_POST[$prefix]['assetid'])) return false;
		$value = (int) $_POST[$prefix]['assetid'];
		if ($this->value != $value && $this->setValue($value)) {
			$this->processed = true;
		} else {
			$this->processed = false;
		}

	}// end process()


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
			$error_msg = 'Atempted to set value of attribute "'.$this->name.'" [#'.$this->id.'] to an invalid asset ID';
			if (!assert_valid_assetid($value, $error_msg, true, false)) return false;
			$asset = &$GLOBALS['SQ_SYSTEM']->am->getAsset($value);
			if (is_null($asset)) return false;
		}
		return true;

	}//end validateValue()


	/**
	* Retrieve a list of indexable values from this attribute
	*
	* The return value not only contains the indexable values, but also the number of
	* times they occur in this attributes value
	*
	* @return array(string => int)
	* @access public
	*/
	function getKeyData()
	{
		if (empty($this->value)) return Array();
		return Array((string) $this->value => 1);

	}//end getKeyData()


}//end class

?>
