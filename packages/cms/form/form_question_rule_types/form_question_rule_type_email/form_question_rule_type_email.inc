<?php

require_once dirname(__FILE__).'/../../form_question_rule/form_question_rule.inc';
require_once SQ_FUDGE_PATH .'/general/www.inc';

/**
* Form_Question_Rule_Type_Email
*
* Purpose
*
*
* @author  Marc Mcintyre <mmcintyre@squiz.net>
* @version $Version$ - 1.0
* @package MySource_Matrix_Packages
* @subpackage cms
*/
class Form_Question_Rule_Type_Email extends Form_Question_Rule
{

	/**
	* Constructor
	*
	* @param int	$assetid	the asset id to be loaded
	*
	*/
	function Form_Question_Rule_Type_Email($assetid=0) 
	{
		$this->Form_Question_Rule($assetid);
	
	}// end Form_Question_Rule_Type_Email()


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

	}// end create()


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
	*
	* @access public
	*/
	function printBody()
	{
		return true;
		
	} // end printBody()


	/**
	* returns true if the question is valid, otherwise 
	* adds an error to the submission object, and return false
	* 
	* @see Form_Question_Rule::processForm()
	*
	* @param &object form_submission	$submission		the submission object for this form
	* @param	int						$questionid		the id of the question this rule is linked to 
	* @param string						$question_name	the name of the question (used for producing errors)
	*
	* @access public
	* @return boolean
	*/
	function processForm(&$submission, $questionid, $question_name)
	{
		$answer   = $submission->getAnswer($questionid);
		$operator = ($this->attr('operator')) ? 1 : 0;

		$valid = valid_email($answer);

		if ($valid == $operator) {
			return true;
		} else {
			$error = $question_name.' must '. (($operator) ? '' : 'not') .' be a valid Email Address';
			$submission->setError($error);
			return false;
		}

	}// end processForm()

}// end class
?>
