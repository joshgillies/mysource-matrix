<?php
/**
* +--------------------------------------------------------------------+
* | Squiz.net Commercial Module Licence                                |
* +--------------------------------------------------------------------+
* | Copyright (c) Squiz Pty Ltd (ACN 084 670 600).                     |
* +--------------------------------------------------------------------+
* | This source file is not open source or freely usable and may be    |
* | used subject to, and only in accordance with, the Squiz Commercial |
* | Module Licence.                                                    |
* | Please refer to http://www.squiz.net/licence for more information. |
* +--------------------------------------------------------------------+
*
* $Id: poll_question.inc,v 1.2 2005/04/29 05:40:31 gsherwood Exp $
*
*/
require_once SQ_INCLUDE_PATH.'/asset.inc';

/** FIXME: See printBody(). WTF?
* Poll_Question
*
* Dependant of Page_Poll
*
*
* @author  Nathan de Vries <ndvries@squiz.net>
* @author  Robert Howard <rhoward@squiz.net>
* @author  David Heppell <dheppell@squiz.net>
* @version $Revision: 1.2 $
* @package MySource_Matrix_Packages
* @subpackage cms
*/
class Poll_Question extends Asset
{


	/**
	* Constructor
	*
	* @param int	$assetid	the asset id to be loaded
	*
	*/
	function Poll_Question($assetid=0)
	{
		$this->_ser_attrs = true;
		$this->Asset($assetid);

	}//end constructor


	/**
	* Get the name or short name of the asset
	*
	* @param bool	$short_name	whether or not we are after the shortname or the full name
	*
	* @return string
	* @access private
	* @see Asset::_getName()
	*/
	function _getName($short_name=false)
	{
		return $this->attr('name');

	}//end _getName()


	/**
	* Delete an option from the array
	*
	* @param array 	&$option 	The array from which the option will be deleted
	* @param int	$option_id	The ID of the option to delete
	*
	* @access public
	* @return boolean 			False if the option is not found, True if successful
	*/
	function updateOptions(&$poll_options)
	{
		$this->setAttrValue('options', $poll_options);
		return true;

	}//end updateOptions()


}//end class


?>
