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
* $Id: online_quiz_question_group.inc,v 1.1 2007/05/15 03:39:42 mbrydon Exp $
*
*/


require_once SQ_CORE_PACKAGE_PATH.'/folder/folder.inc';


/**
* Online Quiz Question Group
*
* Purpose
*	A group to house Online Quiz Questions
*
* @author  Mark Brydon <mbrydon@squiz.net>
* @version $Revision: 1.1 $
* @package MySource_Matrix_Packages
* @subpackage cms
*/
class Online_Quiz_Question_Group extends Folder
{


	/**
	* Constructor
	*
	* @param string	$assetid	the assetid of the asset to load into this object
	*
	* @return void
	* @access public
	*/
	function Online_Quiz_Question_Group($assetid=0)
	{
		$this->Folder($assetid);

	}//end constructor


	/**
	* Returns name of the asset
	*
	* @param boolean	$short_name	whether or not we are after the short name or the full name
	*
	* @return string
	* @access private
	* @see Asset::_getName()
	*/
	function _getName($short_name=FALSE)
	{
		return $this->attr('name');

	}//end _getName()


	/**
	* Returns an array of all the permitted link type, the type asset and the cardinality
	*
	* @return array
	* @access private
	* @see Asset::_getAllowedLinks()
	*/
	function _getAllowedLinks()
	{
		$page_links = parent::_getAllowedLinks();
		$page_links[SQ_LINK_TYPE_2]['online_quiz_question'] = Array('card' => 'M', 'exclusive' => FALSE);
		$page_links[SQ_LINK_TYPE_3]['online_quiz_question'] = Array('card' => 'M', 'exclusive' => FALSE);
		$page_links[SQ_LINK_TYPE_2]['online_quiz_question_group'] = Array('card' => 'M', 'exclusive' => FALSE);
		$page_links[SQ_LINK_TYPE_3]['online_quiz_question_group'] = Array('card' => 'M', 'exclusive' => FALSE);
		return $page_links;

	}//end _getAllowedLinks()


}//end class
?>
