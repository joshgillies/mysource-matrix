<?php

require_once SQ_INCLUDE_PATH.'/asset.inc';

/**
* Folder
*
* Purpose
*
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Version$ - 1.0
* @package Resolve::packages::__core__
*/
class Folder extends Asset
{

	/**
	* Constructor
	*
	* @param int    $assetid      the asset id to be loaded
	*
	*/
	function Folder($assetid=0) 
	{
		$this->Asset($assetid);
	}


	/**
	* Create this asset
	*
	* @param array(string)	&$link	information used to create the initial link
	* @param string			$name	the name of this folder
	*
	* @return boolean
	* @access public
	*/
	function create(&$link, $name)
	{
		$name = trim($name);

		if ($name == '') {
			trigger_error('Unable to create a Folder with a blank name', E_USER_WARNING);
			return false;
		}

		if (parent::create($link)) {

			return $this->setAttrValue('name',   $name);

		}//end if

		return false;

	}//end create()


	/**
	* Returns name of the asset
	*
	* @param bool	$short_name	whether or not we are after the shortname or the full name
	*
	* @see Asset::_getName()
	* @access public
	*/
	function _getName($short_name=false)
	{
		return $this->attr('name');
	}//end _getName();


	/**
	* Returns an array of all the permitted links type, the type asset and the cardinality
	*
	* @see Asset::_getAllowLinks()
	* @access private
	*/
	function _getAllowedLinks()
	{

		return Array(SQ_LINK_EXCLUSIVE => Array('asset' => 'M'),
					 SQ_LINK_UNITE     => Array('asset' => 'M'),
					 SQ_LINK_USES      => Array('asset' => 'M'),
					 SQ_LINK_NOTICE    => Array('asset' => 'M')
					);

	}//end _getAllowedLinks()


	/**
	* Processes a backend submission from this asset, returns true if all OK
	*
	* @param object Backend_Outputter	$o
	* @param array(string)				&$link	information used to create the initial link
	*
	* @return boolean
	* @access public
	*/
	function processBackend(&$o, &$link)
	{

		switch($_POST['asset_action']) {
			case 'create' :
				$ei = &$this->getEI();
				$ei->process($this, $o, true);

				$name = $this->getAttribute('name');
				if ($name->processed) {
					return $this->create($link, $name->value);
				} else {
					return false;
				}
				break;

			default :
				return parent::processBackend($o, $link);

		}// end switch

	}// end processBackend()

}//end class
?>
