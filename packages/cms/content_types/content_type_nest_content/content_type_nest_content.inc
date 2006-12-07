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
* $Id: content_type_nest_content.inc,v 1.24 2006/12/07 00:04:16 emcdonald Exp $
*
*/


require_once SQ_CORE_PACKAGE_PATH.'/content_type/content_type.inc';

/**
* Content_Type_Nest_Content
*
* Purpose
*
*
* @author  Greg Sherwoood <greg@squiz.net>
* @version $Revision: 1.24 $
* @package MySource_Matrix_Packages
* @subpackage cms
*/
class Content_Type_Nest_Content extends Content_Type
{


	/**
	* Constructor
	*
	* @param int	$assetid	the asset id to be loaded
	*
	*/
	function Content_Type_Nest_Content($assetid=0)
	{
		$this->Content_Type($assetid);

	}//end constructor


	/**
	* Returns an array of all the permitted link type, the type asset and the cardinality
	*
	* @return void
	* @access private
	* @see Asset::_getAllowLinks()
	*/
	function _getAllowedLinks()
	{
		return Array(
				SQ_LINK_NOTICE	=> Array(
									'asset'	=> Array(
												'card'		=> 1,
												'exclusive'	=> FALSE,
											   ),
								   ),
			   );

	}//end _getAllowedLinks()


	/**
	* Returns true if there is no nested assetid associated with this content type
	*
	* @return boolean
	* @access public
	*/
	function isEmpty()
	{
		$link = $GLOBALS['SQ_SYSTEM']->am->getLink($this->id, SQ_LINK_NOTICE, '', TRUE, 'nested_asset');
		return empty($link);

	}//end isEmpty()


	/**
	* Return a human readable description of the passed link
	*
	* @param int	$linkid	the link ID of the link to describe
	*
	* @return string
	* @access public
	*/
	function describeLink($linkid)
	{
		$link = $GLOBALS['SQ_SYSTEM']->am->getLinkById($linkid);
		if ($link['value'] == 'nested_asset') {
			return translate('cms_ct_nest_content_link_desc');
		}

	}//end describeLink()


	/**
	* Returns the editable content of this asset
	*
	* @return string
	* @access public
	*/
	function getContent()
	{
		return '';

	}//end getContent()


	/**
	* Sets the editable content of this asset
	*
	* @return boolean
	* @access public
	*/
	function setContent()
	{
		return FALSE;

	}//end setContent()


}//end class

?>