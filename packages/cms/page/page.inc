<?php

require_once SQ_INCLUDE_PATH.'/asset.inc';

/**
* Page
*
* Purpose
*
*
* @author  Greg Sherwoood <greg@squiz.net>
* @version $Version$ - 1.0
* @package Resolve::packages::cms
*/
class Page extends Asset
{

	/**
	* Constructor
	*
	* @param int	$assetid	the asset id to be loaded
	*
	*/
	function Page($assetid=0) 
	{
		$this->Asset($assetid);
	}


	/**
	* Create this asset
	*
	* @param string	$name	the name of this page
	*
	* @return boolean
	* @access public
	*/
	function create($name)
	{
		$name = trim($name);

		if ($name == '') {
			trigger_error('Unable to create a Page with a blank name', E_USER_WARNING);
			return false;
		}

		if (parent::create()) {
			
			// short and long names are the same to begin with
			$this->setAttrValue('short_name', $name);
			$this->setAttrValue('name', $name);
			
			return true;

		}//end if

		return false;

	}//end create()


	/**
	* Returns an array of all the permitted link type, the type asset and the cardinality
	*
	* @see Asset::_getAllowLinks()
	* @access private
	*/
	function _getAllowedLinks()
	{
		return Array(SQ_LINK_EXCLUSIVE => Array(),
					 SQ_LINK_UNITE     => Array('page' => 'M', 'file' => 'M'),
					 SQ_LINK_USES      => Array(),
					 SQ_LINK_NOTICE    => Array('page' => 'M')
					);

	}//end _getAllowedLinks()


	/**
	* Processes a backend submission from this asset, returns true if all OK
	*
	* @return boolean
	* @access public
	*/
	function processBackend()
	{
		switch($_POST['asset_action']) {
			case 'create' :
				$ei = &$this->getEI();
				$ei->process($this, true);

				$name = $this->getAttribute('name');
				if ($name->processed) {
					return $this->create($name->value);
				} else {
					return false;
				}
				break;

			default :
				return parent::processBackend();

		}// end switch

	}// end processBackend()


	/**
	* Called by the design to print the body of this asset
	* it's up to the kids to override
	*
	* @access public
	*/
	function printBody()
	{
	}// end printBody()


	/**
	* Returns name of the asset
	*
	* @param boolean	$short_name	whether or not we are after the short name or the full name
	*
	* @see Asset::_getName()
	* @access public
	*/
	function _getName($short_name=false)
	{
		return (($short_name) ? $this->attr('short_name') : $this->attr('name'));

	}//end _getName();


}#end class
?>
