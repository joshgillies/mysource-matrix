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
* $Id: design_area_nest_content.inc,v 1.13 2004/08/27 00:17:39 gsherwood Exp $
* $Name: not supported by cvs2svn $
*/


require_once SQ_CORE_PACKAGE_PATH.'/designs/design_area/design_area.inc';

/**
* Design_Area_Nest_Content
*
* Purpose
*
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Version$ - 1.0
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Design_Area_Nest_Content extends Design_Area
{


	/**
	* Set the value for the passed variable to the passed value
	*
	* @param string		$name		the name of the attribute
	* @param string		$value		the new value of the attribute
	*
	* @return boolean
	* @access public
	*/
	function setAttrValue($name, $value)
	{
		// if they are trying to set the type_codes, split the string
		if ($name == 'type_codes') {
			if (!is_array($value)) $value = preg_split('/[ ,]+/', (string) $value);
			foreach($value as $type_code) {
				if (!$GLOBALS['SQ_SYSTEM']->am->installed($type_code)) {
					trigger_error('Asset Type "'.$type_code.'" is not installed in this system, unable to set "type_codes"', E_USER_WARNING);
					return false;
				}
			}
		}

		return parent::setAttrValue($name, $value);

	}//end setAttrValue()


	/**
	* Returns an array of all the permitted links type, the type asset and the cardinality
	*
	* In the form:
	*   Array('[link_type]' => Array('[type_code]' => '[cardinality]));
	* Where:
	*   link_type   = SQ_LINK_UNITE|SQ_LINK_USES|SQ_LINK_NOTICE
	*   cardinality = 1|M
	*
	* @return array
	* @access private
	*/
	function _getAllowedLinks()
	{

		return Array(SQ_LINK_TYPE_1 => Array(),
					 SQ_LINK_TYPE_2 => Array(),
					 SQ_LINK_TYPE_3 => Array(),
					 SQ_LINK_NOTICE => Array('asset' => Array('card' => 1, 'exclusive' => false))
					);

	}//end _getAllowedLinks()


	/**
	* Returns the particular info for the nested assets
	*
	* Returns NULL if no link found
	*
	* @param boolean	$field	field in the link that is required (any valid field from getLink())
	*
	* @return mixed NULL if no asset found or the field value
	* @access private
	* @see Asset::getLink()
	*/
	function _getNestAssetInfo($field)
	{
		if (!isset($this->_tmp['nest_assetid'])) {
			$this->_tmp['nest_assetid'] = $GLOBALS['SQ_SYSTEM']->am->getLink($this->id, SQ_LINK_NOTICE, '', true, 'nest_content_asset');
		}
		return (empty($this->_tmp['nest_assetid'])) ? null : $this->_tmp['nest_assetid'][$field];

	}//end _getNestAssetInfo()


	/**
	* Called whenever any type of link is changed
	*
	* @return boolean
	* @access private
	*/
	function _linksUpdated()
	{
		if (!parent::_linksUpdated()) return false;
		unset($this->_tmp['nest_assetid']);
		return true;

	}//end _linksUpdated()


	/*
	* Outputs either the straight HTML or PHP code that is needed for this design area to be represented
	*
	* @param object Design	$design the design for which we are currently printing
	*
	* @return boolean
	* @access public
	*/
	function printArea(&$design) 
	{
		$assetid = (int) $this->_getNestAssetInfo('minorid');

		// return true because it just means that they haven't set anything yet
		if (empty($assetid)) return true;

		echo '<', '?php
		$nest_content_'.$this->attr('id_name').'_asset = &$GLOBALS[\'SQ_SYSTEM\']->am->getAsset('.$assetid.');
		if (!is_null($nest_content_'.$this->attr('id_name').'_asset)) {
			if ($nest_content_'.$this->attr('id_name').'_asset->readAccess()) {
				$nest_content_'.$this->attr('id_name').'_asset->printBody();
			}
		}
		?', '>';

		return true;

	}//end printArea()


}//end class

?>
