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
* $Id: assetid.inc,v 1.21 2012/08/30 01:09:17 ewang Exp $
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
*	Type codes are a list of asset types that may be selected with a flag to extend the selectable types
*	based on lineage. (A|D|I) -> Ancestors, Descendants or Only Itself
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Revision: 1.21 $
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
	function Asset_Attribute_Assetid($attributeid=0, $value=NULL)
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
	* @param object	&$node	The XML_Tree_Node containing the params
	*
	* @return boolean
	* @access public
	*/
	function setEditParams(&$node)
	{
		if (!parent::setEditParams($node)) return FALSE;

		$this->_edit_params['type_codes'] = Array();
		if (isset($node->attributes()->type_codes)) {
			$codes = explode(',', (string)$node->attributes()->type_codes);
			foreach ($codes as $code_name) {
				list($code, $flag) = explode(':', $code_name);
				$this->_edit_params['type_codes'][$code] = $flag;
			}
		}

		return TRUE;

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
	function paint($prefix, $read_only=FALSE)
	{
		if ($read_only) {
			if (!$this->value) {
				echo translate('No asset selected');

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
	* @param string	$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	function process($prefix)
	{
		$type_code_flag = TRUE;
		if (!isset($_REQUEST[$prefix]['assetid'])) {
			return FALSE;
		}
		$value = $_REQUEST[$prefix]['assetid'];
		if ($value != 0) {
			$type_code_flag = empty($this->_edit_params['type_codes']) ? TRUE : FALSE;
			$value_asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($value);
			foreach ($this->_edit_params['type_codes'] as $type_accepted => $inheritance) {
				if ($value_asset instanceof $type_accepted) {
					$type_code_flag = TRUE;
					break;
				}
			}
		}

		if (!$type_code_flag) {
			$this->processed = FALSE;
			return $type_code_flag;
		}
		if ($this->value != $value && $this->setValue($value)) {
			$this->processed = TRUE;
		} else {
			$this->processed = FALSE;
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
			$error_msg = sprintf(translate('Attempted to set value of attribute "%1$s" [#%2$s] to an invalid asset ID'), $this->name, $this->id);

			if (!assert_valid_assetid($value, $error_msg, TRUE, FALSE)) {
				return FALSE;
			}
			// In we are importing the assets, don't validate the asset id as the asset might not in the db yet
			if (!defined('SQ_IN_IMPORT') || !SQ_IN_IMPORT) {
				$asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($value);
				if (is_null($asset)) return FALSE;
			}
		}
		return TRUE;

	}//end validateValue()


}//end class

?>
