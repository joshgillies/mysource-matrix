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
* $Id: metadata_section.inc,v 1.7.2.4 2004/03/26 16:41:50 brobertson Exp $
* $Name: not supported by cvs2svn $
*/

require_once SQ_CORE_PACKAGE_PATH.'/folder/folder.inc';

/**
* Metadata_Section
*
* Purpose
*
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Version$ - 1.0
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Metadata_Section extends Folder
{

	/**
	* Constructor
	*
	* @param int	$assetid	the asset id to be loaded
	*
	*/
	function Metadata_Section($assetid=0)
	{
		$this->Folder($assetid);

	}//end constructor


	/**
	* Returns an array of all the permitted links type, the type asset and the cardinality
	*
	* @see Asset::_getAllowLinks()
	* @access private
	*/
	function _getAllowedLinks()
	{
		return Array(SQ_LINK_TYPE_1 => Array(),
					 SQ_LINK_TYPE_2 => Array('metadata_field' => Array('card' => 'M', 'exclusive' => false)),
					 SQ_LINK_TYPE_3 => Array(),
					 SQ_LINK_NOTICE => Array()
					);

	}//end _getAllowedLinks()


	/**
	* Prepares for linking by checking that the link values are valid for this link
	*
	* This function will be called if this asset is the major or minor party in the link, so the side_of_link flag
	* should be checked to work out what side of the link this asset is on. The return value should indicate
	* if any of the link fields were changed.
	*
	* @param object Asset	$asset			the major or minor asset that we are linking to
	* @param string			$side_of_link	the side of the link we are on (major or minor)
	* @param string			$link_type		the type of link we are creating
	* @param string			$value			the value that is to be associated with the link
	* @param string			$sort_order		the position in the links list that this link should take,
	* @param string			$dependant		'0' / '1' on whether the this asset is dependant on the
	*										asset that will be linked by the new link
	* @param string			$exclusive		'0' / '1' on whether the this asset is linked exclusivly
	*										to the asset that will be linked by the new link
	*
	* @return boolean
	* @access public
	*/
	function prepareLink(&$asset, $side_of_link, &$link_type, &$value, &$sort_order, &$dependant, &$exclusive)
	{
		// if this is a metadata field then we need to make it a dependant and exclusive link
		if ($side_of_link == 'major' && is_a($asset, 'metadata_field') && ($dependant != '1' || $exclusive != '1')) {
			$dependant = '1';
			$exclusive = '1';
			return true;
		}

		return false;

	}//end prepareLink()


}//end class

?>