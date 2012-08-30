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
* $Id: poll_question.inc,v 1.8 2012/08/30 00:58:09 ewang Exp $
*
*/
require_once SQ_INCLUDE_PATH.'/asset.inc';

/**
* Poll_Question
*
* Dependant of Page_Poll
*
*
* @author  Nathan de Vries <ndvries@squiz.net>
* @author  Robert Howard <rhoward@squiz.net>
* @author  David Heppell <dheppell@squiz.net>
* @version $Revision: 1.8 $
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
		$this->_ser_attrs = TRUE;
		$this->Asset($assetid);

	}//end constructor


	/**
	* Returns name of the asset
	*
	* @param boolean	$short_name	whether or not we are after the shortname or the full name
	* @param int		$contextid	what context to return the name from
	*
	* @return string
	* @access private
	* @see Asset::_getName()
	*/
	protected function _getName($short_name=FALSE, $contextid=NULL)
	{
		// No context specified, using the current context
		if ($contextid === NULL) {
			$contextid = $GLOBALS['SQ_SYSTEM']->getContextId();
		}//end if

		// Obtain the attribute value for Name from the specified Context
		$values = $GLOBALS['SQ_SYSTEM']->am->getAttributeValuesByName('name', $this->type(), Array($this->id), $contextid);
		if (empty($values) === TRUE) {
			return parent::_getName($short_name, $contextid);
		} else {
			return $values[$this->id];
		}

	}//end _getName()


	/**
	* Delete an option from the array
	*
	* @param array	&$poll_options	The array from which the option will be deleted
	*
	* @return boolean
	* @access public
	*/
	function updateOptions(&$poll_options)
	{
		$this->setAttrValue('options', $poll_options);
		return TRUE;

	}//end updateOptions()


}//end class


?>
