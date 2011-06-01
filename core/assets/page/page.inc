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
* $Id: page.inc,v 1.19 2009/03/27 00:40:59 ewang Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset.inc';

/**
* Page
*
* Purpose
*
*
* @author  Greg Sherwoood <greg@squiz.net>
* @version $Revision: 1.19 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Page extends Asset
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
	* Perform any pre-validation required during the creation of this asset
	*
	* Pages ensure a name and short_name has been set before they are created
	*
	* @param array	&$link	information used to create the initial link
	*
	* @return boolean
	* @access private
	*/
	protected function _preCreateCheck(Array &$link)
	{
		if (!parent::_preCreateCheck($link)) return FALSE;

		$name = trim($this->attr('name'));
		if ($name == '') {
			trigger_localised_error('CORE0083', E_USER_WARNING, $GLOBALS['SQ_SYSTEM']->am->getTypeInfo($this->type(), 'name'));
			return FALSE;
		}

		// if the short name hasn't been set then short and long names are the same to begin with
		if (trim($this->attr('short_name')) == '') {
			$this->setAttrValue('short_name', $name);
		}

		return TRUE;

	}//end _preCreateCheck()


	/**
	* Perform any additional processing required during the creation of this asset
	*
	* Pages add a default web path when they are created
	*
	* @param array	&$link	information used to create the initial link
	*
	* @return boolean
	* @access private
	*/
	protected function _createAdditional(Array &$link)
	{
		if (!parent::_createAdditional($link)) return FALSE;

		return $this->makeAndSaveInitialWebPath(strtolower($this->attr('name')), $link);

	}//end _createAdditional()


	/**
	* Returns an array of all the permitted link type, the type asset and the cardinality
	*
	* @return array
	* @access private
	* @see Asset::_getAllowLinks()
	*/
	public function _getAllowedLinks()
	{
		$allowed = parent::_getAllowedLinks();
		$allowed[SQ_LINK_TYPE_1]['page']   = Array('card' => 'M', 'exclusive' => FALSE);
		$allowed[SQ_LINK_TYPE_1]['file']   = Array('card' => 'M', 'exclusive' => FALSE);
		$allowed[SQ_LINK_TYPE_1]['folder'] = Array('card' => 'M', 'exclusive' => FALSE);
		$allowed[SQ_LINK_TYPE_1]['data_source'] = Array('card' => 'M', 'exclusive' => FALSE);

		$allowed[SQ_LINK_TYPE_2]['page']   = Array('card' => 'M', 'exclusive' => FALSE);
		$allowed[SQ_LINK_TYPE_2]['file']   = Array('card' => 'M', 'exclusive' => FALSE);
		$allowed[SQ_LINK_TYPE_2]['folder'] = Array('card' => 'M', 'exclusive' => FALSE);
		$allowed[SQ_LINK_TYPE_2]['data_source'] = Array('card' => 'M', 'exclusive' => FALSE);

		$allowed[SQ_LINK_TYPE_3]['page'] = Array('card' => 'M', 'exclusive' => FALSE);

		$allowed[SQ_LINK_NOTICE]['page'] = Array('card' => 'M', 'exclusive' => FALSE);

		return $allowed;

	}//end _getAllowedLinks()


	/**
	* Called by the design to print the body of this asset
	*
	* It's up to the kids to override
	*
	* @return void
	* @access public
	*/
	public function printBody()
	{

	}//end printBody()


	/**
	* Returns name of the asset
	*
	* @param boolean	$short_name	whether or not we are after the short name or the full name
	*
	* @return string
	* @access private
	* @see Asset::_getName()
	*/
	protected function _getName($short_name=FALSE)
	{
		return (($short_name) ? $this->attr('short_name') : $this->attr('name'));

	}//end _getName()


}//end class

?>
