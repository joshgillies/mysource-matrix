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
* $Id: decision_tree_question.inc,v 1.4 2012/08/30 00:58:08 ewang Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset.inc';

/**
* Decision_Tree_Question
*
*
* @author  Mohamed Haidar <mhaidar@squiz.net>
* @version $Revision: 1.4 $
* @package MySource_Matrix_Packages
* @subpackage cms
*/
class Decision_Tree_Question extends Asset
{

	
	/**
	* Constructor
	*
	* @param int	$assetid	the asset id to be loaded
	**/
	function Decision_Tree_Question($assetid=0)
	{
		$this->_ser_attrs = TRUE;
		$this->Asset($assetid);

	}//end constructor

	
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
		$allowed[SQ_LINK_TYPE_2]['decision_tree_question'] = Array('card' => 'M', 'exclusive' => FALSE);
		
		return $allowed;

	}//end _getAllowedLinks()
	

	/**
	* Returns name of the user
	*
	* @param boolean	$short_name	whether or not we are after the shortname or the full name
	*
	* @return string
	* @access private
	* @see Asset::_getName()
	*/
	public function _getName($short_name=FALSE)
	{
		return ($this->attr('name'));

	}//end _getName()
	
	
	/**
	* Prepares for linking by checking that the link values are valid for this link
	*
	* This function will be called if this asset is the major or minor party in the link, so the side_of_link flag
	* should be checked to work out what side of the link this asset is on. The return value should indicate
	* if any of the link fields were changed.
	*
	* @param Bodycopy_Container	$asset			the major or minor asset that we are linking to
	* @param string				$side_of_link	the side of the link we are on (major or minor)
	* @param string				&$link_type		the type of link we are creating
	* @param string				&$value			the value that is to be associated with the link
	* @param string				&$sort_order	the position in the links list that this link should take,
	* @param string				&$dependant		'0' / '1' on whether the this asset is dependant on the
	*											asset that will be linked by the new link
	* @param string				&$exclusive		'0' / '1' on whether the this asset is linked exclusively
	*											to the asset that will be linked by the new link
	*
	* @return boolean
	* @access public
	*/
	public function prepareLink(Asset $asset, $side_of_link, &$link_type, &$value, &$sort_order, &$dependant, &$exclusive)
	{
		if ($side_of_link == 'minor' && $dependant != '1' && ($link_type & SQ_SC_LINK_SIGNIFICANT) && ($asset instanceof Page_Decision_Tree || $asset instanceof Decision_Tree_Question)){
			$dependant = '1';
			return TRUE;
		}
		
		return FALSE;

	}//end prepareLink()
	
	
}//end class

?>
