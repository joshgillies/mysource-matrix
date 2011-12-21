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
* $Id: decision_tree_question.inc,v 1.1 2011/12/21 02:28:40 mhaidar Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset.inc';

/**
* Decision_Tree_Question
*
*
* @author  Mohamed Haidar <mhaidar@squiz.net>
* @version $Revision: 1.1 $
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
	
	
}//end class

?>
