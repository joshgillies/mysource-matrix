<?php

require_once SQ_INCLUDE_PATH.'/asset.inc';

/**
* Form_Submission
*
* Purpose
*
*
* @author  Marc Mcintyre <mmcintyre@squiz.net>
* @version $Version$ - 1.0
* @package Resolve_Packages
* @subpackage cms
*/
class Form_Submission extends Asset
{

	/**
	* Constructor
	*
	* @param int	$assetid	the asset id to be loaded
	*
	*/
	function Form_Submission($assetid=0) 
	{
		$this->_ser_attrs = true;
		$this->Asset($assetid);
	}


	/**
	* Create this asset
	*
	* @param array()	&$link	information used to create the initial link
	*
	* @see Asset::create()
	* @return mixed int or false
	* @access public
	*/
	function create(&$link)
	{
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');
		$linkid = parent::create($link);

		if (!$linkid) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			$this->id = 0;
			return false;
		}

		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		return $linkid;

	}//end create()


	/**
	* Creates a link between this asset and the passed asset, of a certain type
	* and returns the linkid of the created link
	*
	* @return int
	* @access public
	* @see Asset::creatLink();
	*/
	function createLink(&$minor, $link_type, $value='', $sort_order=-1, $dependant='0', $exclusive='0')
	{
		return parent::createLink($minor, $link_type, $value, $sort_order, $dependant, $exclusive);

	}// end createLink()


	/**
	* Returns TRUE if we can delete the passed link, or a string with the error msg
	*
	* @param int	$linkid	the link id of the link to remove
	*
	* @return mixed TRUE or error msg string
	* @access public
	*/
	function canDeleteLink($linkid)
	{
		return parent::canDeleteLink($linkid);
	
	}//end canDeleteLink()


	/**
	* Returns an array of all the permitted link type, the type asset and the cardinality
	*
	* @see Asset::_getAllowLinks()
	* @access private
	*/
	function _getAllowedLinks()
	{
		$page_links = parent::_getAllowedLinks();
		return $page_links;

	}//end _getAllowedLinks()


	/**
	* Prints out the backend interface for the asset
	*
	* @param object Backend_Outputter $o
	*
	* @access public
	*/
	function paintBackend(&$o)
	{
		parent::paintBackend($o);

	}// end paintBackend()


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
	* returns the value of an answer
	*
	* @param int $id the id of the question
	*
	* @return mixed answer
	* @access public
	*/
	function getAnswer($id)
	{
		$a = $this->attr('attributes');
		if (isset($a['answers'][$id])) {
			return $a['answers'][$id];
		}
		return false;

	}// end getAnswer()
	

	/**
	* sets the answer of a question in this submission object
	*
	* @param integer $id the id of the question
	* @param mixed $value the value of the question
	*
	* @return boolean
	* @access public
	*/
	function setAnswer($id, $value)
	{
		$a = $this->attr('attributes');
		$a['answers'][$id] = $value;
		if (!$this->setAttrValue('attributes', $a)) {
			return false;
		}

		return true;
	
	}// end setAnswer()


	/**
	* set a form error into the error store
	*
	* @param string $error the error to set
	*
	* @access public
	* @return boolean
	*/
	function setError($error)
	{
		$a = $this->attr('attributes');
		if (!$a['is_error']) $a['is_error'] = true;
		if (!is_array($a['errors'])) $a['errors'] = Array();
		$a['errors'][] = $error;
		
		if (!$this->setAttrValue('attributes', $a)) {
			return false;
		}
		return true;
	
	}// end setError()

	
	/**
	* get the form errors from the store
	*
	* @access public
	* @return Array(error)
	*/
	function getErrors()
	{
		$a = $this->attr('attributes');
		return $a['errors'];
	
	}// end getErrors()


	/**
	* flush the errors in the store
	*
	* @access public
	* @return boolean
	*/
	function flushErrors()
	{
		$a = $this->attr('attributes');
		$a['is_error'] = false;
		$a['errors'] = Array();

		if (!$this->setAttrValue('attributes', $a)) {
			return false;
		}
		return true;
	
	}// end flushErrors()


	/**
	* returns true if there is a form error
	*
	* @access public
	* @return boolean
	*/
	function isError()
	{
		$a = $this->attr('attributes');
		return ($a['is_error']) ? true : false;
	
	}//end isError()

}//end class
?>
