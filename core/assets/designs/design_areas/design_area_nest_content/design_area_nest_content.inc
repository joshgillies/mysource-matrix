<?php

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
	* @param string		$name				the name of the attribute
	* @param string		$value				the new value of the attribute
	* @param boolean	$dont_run_updated	if true this->_updated() won't be run (THIS IS ONLY EVER CALLED FROM Asset::create())
	*
	* @return boolean
	* @access public
	*/
	function setAttrValue($name, $value, $dont_run_updated=false)
	{
		// if they are trying to set the type_codes, split the string
		if ($name == 'type_codes' && !is_array($value)) {
			$value = preg_split('/[ ,]+/', (string) $value);

		}// end if

		return parent::setAttrValue($name, $value, $dont_run_updated);

	}//end setAttrValue()


	/**
	* Returns an array of all the permitted links type, the type asset and the cardinality
	* In the form
	*
	*    Array('[link_type]' => Array('[type_code]' => '[cardinality]));
	* Where:
	*   link_type   = SQ_LINK_UNITE|SQ_LINK_USES|SQ_LINK_NOTICE
	*   cardinality = 1|M
	*
	* @return Array()
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
	* Returns NULL if no link found
	*
	* @param boolean	$field	field in the link that is required (any valid field from getLink())
	*
	* @return mixed NULL if no asset found or the field value
	* @see Asset::getLink()
	* @access private
	*/
	function _getNestAssetInfo($field)
	{
		if (!isset($this->_tmp['nest_assetid'])) {
			$this->_tmp['nest_assetid'] = $GLOBALS['SQ_SYSTEM']->am->getLink($this->id, SQ_LINK_NOTICE, '', true, 'nest_content_asset');
		}
		return (empty($this->_tmp['nest_assetid'])) ? null : $this->_tmp['nest_assetid'][$field];
	}//end _getNestAssetInfo()


	/**
	* Called whenever any type of link is changed, allows caching of objects, links and other stuff
	* to occur but not cause integrity issues during script execution
	* returns true it it's successfull
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
			$nest_content_'.$this->attr('id_name').'_asset->printBody();
		}// end if
		?', '>';

		return true;

	}//end printArea()

}//end class
?>
