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
* $Id: form_question_type_password.inc,v 1.19 2005/05/18 23:44:39 tbarrett Exp $
*
*/


require_once dirname(__FILE__).'/../../form_question/form_question.inc';

/**
* Form_Question_Type_Password
*
* Purpose
*
*
* @author  Marc Mcintyre <mmcintyre@squiz.net>
* @version $Revision: 1.19 $
* @package MySource_Matrix_Packages
* @subpackage cms
*/
class Form_Question_Type_Password extends Form_Question
{


	/**
	* Constructor
	*
	* @param int	$assetid	the asset id to be loaded
	* @param array	$data		the section's entry for this question (contains all question data)
	*
	*/
	function Form_Question_Type_Password($assetid=0,$data=Array())
	{
		$this->Form_Question($assetid, $data);

	}//end constructor


	/**
	* Returns an array of all the permitted link type, the type asset and the cardinality
	*
	* @see Asset::_getAllowLinks()
	* @access private
	*/
	function _getAllowedLinks()
	{
		return Array();

	}//end _getAllowedLinks()


	/**
	* generates the html for this question to display it in the frontend
	*
	* @access public
	* @return string
	*/
	function getAnswer()
	{
		$width  = $this->attr('width');
		$max    = $this->attr('max');
		$extras = $this->attr('extra');
		$name   = $this->id.'_1';
		$name2   = $this->id.'_2';


		// generate some php for the value, which can be used for both
		// auto-answer and returning to saved forms
		$value = '';

		ob_start();
		password_box($name, $value, $width, $max, $extras);
		if ($this->attr('verify')) {
			echo '<br />';
			password_box($name2, $value, $width, $max, $extras);
		}
		$html = ob_get_contents();
		ob_end_clean();

		// calling text_box will convert the open and close php tags into their html counter-parts
		// so we need to convert them back to proper php tags

		return html_entity_decode($html);

	}//end getAnswer()


	/**
	* Look at user-specified and internal rules to see if this question currently has a valid value
	*
	* @access public
	* @return boolean
	*/
	function hasValidValue()
	{
		$val = $this->getValue();
		if ($this->attr('verify') && ($val != $this->_tmp['verify_value'])) {
			$this->failed_rules[] = translate('cms_form_password_entered_differently', $this->attr('name'));
			return false;
		}
		if (strlen($val) < $this->attr('min')) {
			$this->failed_rules[] = translate('cms_form_password_length_>=', $this->attr('name'), $this->attr('min'));
			return false;
		}
		if (strlen($val) > $this->attr('max')) {
			$this->failed_rules[] = translate('cms_form_password_length_<=', $this->attr('name'), $this->attr('max'));
			return false;
		}
		return parent::hasValidValue();

	}//end hasValidValue()


	/**
	* Populates the question response from the form data
	*
	* @return boolean
	* @access public
	*/
	function populate()
	{
		if (isset($_POST[$this->id.'_1'])) {
			$this->active_submission[$this->id] = $_POST[$this->id.'_1'];
			if ($this->attr('verify')) {
				$this->_tmp['verify_value'] = $_POST[$this->id.'_2'];
			}
		}
		return true;

	}//end populate()


	/**
	* returns an answer summary in XML format (suitable for submission logs)
	*
	* @param mixed	$answer	the answer for the question
	*
	* @access public
	* @return string the summary XML
	*/
	function getXML($answer)
	{
		ob_start();

		// save as a text question for now, we may need a better idea here
		echo '<text_q id="'.addslashes($this->id).'"  name="'.htmlspecialchars($this->attr('name')).'">';
		echo htmlspecialchars($answer);
		echo '</text_q>';

		$contents = ob_get_contents();
		ob_end_clean();

		return $contents;

	}//end getXML()


	/**
	* returns the answer for this question for the summary, generally overidden by children
	*
	* @param $answer the answer of the question
	*
	* @access public
	* @return string
	*/
	function getSummary($answer)
	{
		return '';

	}//end getSummary()


}//end class
?>