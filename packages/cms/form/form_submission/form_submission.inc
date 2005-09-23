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
* $Id: form_submission.inc,v 1.24.2.1 2005/09/23 06:54:37 arailean Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset.inc';

/**
* Form_Submission
*
* Purpose
*
*
* @author  Marc Mcintyre <mmcintyre@squiz.net>
* @version $Revision: 1.24.2.1 $
* @package MySource_Matrix_Packages
* @subpackage cms
*/
class Form_Submission extends Asset
{


	/**
	* Constructor
	*
	* @param int	$assetid	the asset id to be loaded
	*/
	function Form_Submission($assetid=0)
	{
		$this->_ser_attrs = true;
		return $this->Asset($assetid);

	}//end constructor


	/**
	* Returns the answer of an question
	*
	* @param int	$id	the id of the question
	*
	* @return mixed string|array|int|boolean
	* @access public
	*/
	function getAnswer($id)
	{
		$a = $this->attr('attributes');
		if (isset($a['answers'][$id]['answer'])) {
			return $a['answers'][$id]['answer'];
		}
		return false;

	}//end getAnswer()


	/**
	* returns the name of an question
	*
	* @param int	$id	the id of the question
	*
	* @return string
	* @access public
	*/
	function getName($id)
	{
		$a = $this->attr('attributes');
		if (isset($a['answers'][$id]['name'])) {
			return $a['answers'][$id]['name'];
		}
		return false;

	}//end getName()


	/**
	* Sets the answer of a question in this submission object
	*
	* @param int	$id		the id of the question
	* @param mixed	$value	the value of the question
	*
	* @return boolean
	* @access public
	*/
	function setAnswer($id, $value)
	{
		$a = $this->attr('attributes');
		if (empty($a['answers'])) $a['answers'] = Array();

		$a['answers'][$id]['answer'] = $value;

		if (!$this->setAttrValue('attributes', $a)) {
			return false;
		}

		return true;

	}//end setAnswer()


	/**
	* Set a form error into the error store
	*
	* @param string	$error	the error to set
	*
	* @return boolean
	* @access public
	*/
	function setError($error)
	{
		$a = $this->attr('attributes');
		if (!isset($a['is_error']) || !$a['is_error']) {
			$a['is_error'] = true;
		}
		if (!isset($a['errors'])) $a['errors'] = Array();
		$a['errors'][] = $error;

		if (!$this->setAttrValue('attributes', $a)) {
			return false;
		}
		return true;

	}//end setError()


	/**
	* Get the form errors from the store
	*
	* @return array
	* @access public
	*/
	function getErrors()
	{
		$a = $this->attr('attributes');
		return $a['errors'];

	}//end getErrors()


	/**
	* Flush the errors in the store
	*
	* @return boolean
	* @access public
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

	}//end flushErrors()


	/**
	* Returns true if there is a form error
	*
	* @return boolean
	* @access public
	*/
	function isError()
	{
		$a = $this->attr('attributes');
		return (isset($a['is_error']) && ($a['is_error'])) ? true : false;

	}//end isError()


	/**
	* Returns the answers for question in the form:
	*
	* @return array
	* @access public
	*/
	function getAnswers()
	{
		$a = $this->attr('attributes');
		return $a['answers'];

	}//end getAnswers()


	/**
	* Sets a summary about a question used to print a complete summary of the answers of this form
	*
	* @param int	$question_id	the id of the question
	* @param string	$name			the name of the question
	* @param string	$value			the value of the question
	*
	* @return boolean
	* @access public
	*/
	function setSummary($question_id, $name, $value)
	{
		$a = $this->attr('attributes');

		if (empty($a['summary'])) $a['summary'] = Array();

		$a['summary'][$question_id]['answer']  = $value;
		$a['summary'][$question_id]['name']    = $name;

		if (!$this->setAttrValue('attributes', $a)) {
			return false;
		}

		return true;

	}//end setSummary()


	/**
	* Returns an Array with summary information
	*
	* @param int	$q_id	the id of the question
	* @param string	$value	the value of the question
	*
	* @return array
	* @access public
	*/
	function getSummary($q_id=null, $value='answer')
	{
		$a = $this->attr('attributes');
		if (!$a) return false;

		if (is_null($q_id)) {
			return $a['summary'];
		} else {
			return $a['summary'][$q_id][$value];
		}

	}//end getSummary()


	/**
	* Save attributes set with setAttrValue to the database
	*
	* @param boolean	$dont_run_updated	if true this->_updated() won't be run
	*										(THIS IS ONLY EVER CALLED FROM Asset::create())
	*
	* @return boolean
	* @access public
	* @see asset::setAttrValue()
	*/
	function saveAttributes($dont_run_updated=false)
	{
		$GLOBALS['SQ_SYSTEM']->setRunLevel(SQ_RUN_LEVEL_FORCED);
		$val = parent::saveAttributes($dont_run_updated);
		$GLOBALS['SQ_SYSTEM']->restoreRunLevel();

		return $val;

	}//end saveAttributes()


	/**
	* Increments the version number of the asset based on the part of the version passed in
	*
	* This will override incrementVersion() so the parents of form submissions
	* never get version increases linked to stuff that form submissions do. This
	* is for speed purposes, and generally cuts out half the number of queries
	* required to create a new submission
	*
	* @param string		$number			the part of the version to update (major|minor|micro)
	* @param boolean	$update_parents	should we go through an update all our dependant parents as well ?
	*									NOTE: this is passed to incrementVersion to let it know that the parents
	*									should have their versions updated as well.
	*
	* @return boolean
	* @access public
	*/
	function incrementVersion($number='micro', $update_parents=true)
	{
		return parent::incrementVersion($number, false);

	}//end incrementVersion()


	/**
	* Get XML contents of this submission
	*
	* @return void
	* @access public
	*/
	function getXML()
	{
		return $this->attr('xml');

	}//end getXML()


}//end class
?>
