<?php
/**
* Copyright (c) 2003 - Squiz Pty Ltd
*
* $Id: form_submission.inc,v 1.9 2003/09/26 05:26:42 brobertson Exp $
* $Name: not supported by cvs2svn $
*/


require_once SQ_INCLUDE_PATH.'/asset.inc';

/**
* Form_Submission
*
* Purpose
*
*
* @author  Marc Mcintyre <mmcintyre@squiz.net>
* @version $Version$ - 1.0
* @package MySource_Matrix_Packages
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
		return $this->Asset($assetid);
	
	} // end  Form_Submission()


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

	} // end create()


	/**
	* returns the answer of an question
	*
	* @param int $id the id of the question
	*
	* @return mixed answer
	* @access public
	*/
	function getAnswer($id)
	{
		$a = $this->attr('attributes');
		if (isset($a['answers'][$id]['answer'])) {
			return $a['answers'][$id]['answer'];
		}
		return false;

	}// end getAnswer()
	

	/**
	* returns the name of an question
	*
	* @param int $id the id of the question
	*
	* @return string name
	* @access public
	*/
	function getName($id)
	{
		$a = $this->attr('attributes');
		if (isset($a['answers'][$id]['name'])) {
			return $a['answers'][$id]['name'];
		}
		return false;

	} // end getName()


	/**
	* sets the answer of a question in this submission object
	*
	* @param integer	$id		the id of the question
	* @param string		$name	the name of the question (useful for printing form summary)
	* @param mixed		$value	the value of the question
	*
	* @return boolean
	* @access public
	*/
	function setAnswer($id, $value)
	{
		$a = $this->attr('attributes');
		if (empty($a['answers'])) $a['answers'] = Array();
		
		$a['answers'][$id]['answer']    = $value;		
		
		if (!$this->setAttrValue('attributes', $a)) {
			return false;
		}

		return true;
	
	} // end setAnswer()


	/**
	* set a form error into the error store
	*
	* @param string		$error	the error to set
	*
	* @access public
	* @return boolean
	*/
	function setError($error)
	{
		$a = $this->attr('attributes');
		if (!$a['is_error']) $a['is_error'] = true;
		if (!isset($a['errors'])) $a['errors'] = Array();
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
	* @return Array(string)
	*/
	function getErrors()
	{
		$a = $this->attr('attributes');
		return $a['errors'];
	
	} // end getErrors()


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
	
	} // end flushErrors()


	/**
	* returns true if there is a form error
	*
	* @access public
	* @return boolean
	*/
	function isError()
	{
		$a = $this->attr('attributes');
		return (isset($a['is_error']) && ($a['is_error'])) ? true : false;
	
	}//end isError()


	/**
	* returns the answers for question in the form:
	* 
	* @access public
	* @return Array()
	*/
	function getAnswers()
	{
		$a = $this->attr('attributes');
		return $a['answers'];

	} // getAnswers()


	/**
	* sets a summary about a question used to print a complete summary of the answers of this form
	*
	* @param integer	$question_id	the id of the question
	* @param string		$name			the name of the question
	* @param string		$value			the value of the question
	* @param integer	$section_id		the id of the section that this question belongs to
	* @param string		$section name	the name of the section that this question belongs to
	*
	* @access public
	* @return boolean
	*/
	function setSummary($question_id, $name, $value, $section_id, $section_name)
	{
		$a = $this->attr('attributes');
	
		if (empty($a['summary'])) $a['summary'] = Array();

		if (!isset($a['summary'][$section_id])) {
			$a['summary'][$section_id] = Array(
				'section_name'	=> $section_name,
				'questions'		=> Array(),
			);
		}
		
		$a['summary'][$section_id]['questions'][$question_id]['answer']  = $value;		
		$a['summary'][$section_id]['questions'][$question_id]['name']    = $name;
		
		if (!$this->setAttrValue('attributes', $a)) {
			return false;
		}
		$this->saveAttributes();

		return true;

	} // end setSummary()


	/**
	* returns an Array with summary information
	*
	* @access public
	* @return Array()
	*/
	function getSummary()
	{
		$a = $this->attr('attributes');
		if (!$a) return false;
	
		return $a['summary'];
	
	} // end getSummary()
} // end class
?>
