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
* $Id: metadata_section.inc,v 1.18.2.1 2007/03/05 22:34:56 hnguyen Exp $
*
*/

require_once SQ_CORE_PACKAGE_PATH.'/folder/folder.inc';

/**
* Metadata_Section
*
* Purpose
*
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Revision: 1.18.2.1 $
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
	* @return array
	* @access private
	* @see Asset::_getAllowLinks()
	*/
	function _getAllowedLinks()
	{
		return Array(
				SQ_LINK_TYPE_1	=> Array(),
				SQ_LINK_TYPE_2	=> Array('metadata_field'	=> Array('card'	=> 'M', 'dependant'	=> TRUE)),
				SQ_LINK_TYPE_3	=> Array(),
				SQ_LINK_NOTICE	=> Array(),
			   );

	}//end _getAllowedLinks()


	/**
	* Prepares for linking by checking that the link values are valid for this link
	*
	* This function will be called if this asset is the major or minor party in the link, so the side_of_link flag
	* should be checked to work out what side of the link this asset is on. The return value should indicate
	* if any of the link fields were changed.
	*
	* @param object	&$asset			the major or minor asset that we are linking to
	* @param string	$side_of_link	the side of the link we are on (major or minor)
	* @param string	&$link_type		the type of link we are creating
	* @param string	&$value			the value that is to be associated with the link
	* @param string	&$sort_order	the position in the links list that this link should take,
	* @param string	&$dependant		'0' / '1' on whether the this asset is dependant on the
	*								asset that will be linked by the new link
	* @param string	&$exclusive		'0' / '1' on whether the this asset is linked exclusivly
	*								to the asset that will be linked by the new link
	*
	* @return boolean
	* @access public
	*/
	function prepareLink(&$asset, $side_of_link, &$link_type, &$value, &$sort_order, &$dependant, &$exclusive)
	{
		// if this is a metadata field then we need to make it a dependant and exclusive link
		if ($side_of_link == 'major' && is_a($asset, 'metadata_field') && ($dependant != '1' || $exclusive == '1')) {
			$link_type = SQ_LINK_TYPE_2;
			$dependant = '1';
			$exclusive = '0';
			return TRUE;
		}

		return FALSE;

	}//end prepareLink()


}//end class

?>
