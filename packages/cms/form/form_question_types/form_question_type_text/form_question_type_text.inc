<?php

require_once dirname(__FILE__).'/../../form_question/form_question.inc';

/**
* Form_Question_Type_Text
*
* Purpose
*
*
* @author  Marc Mcintyre <mmcintyre@squiz.net>
* @version $Version$ - 1.0
* @package Resolve_Packages
* @subpackage cms
*/
class Form_Question_Type_Text extends Form_Question
{

	/**
	* Constructor
	*
	* @param int	$assetid	the asset id to be loaded
	*
	*/
	function Form_Question_Type_Text($assetid=0) 
	{
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
		$links[SQ_LINK_TYPE_3]['form_question_rule_type_comparison'] = Array('card' => '1', 'exclusive' => true);
		$links[SQ_LINK_TYPE_3]['form_question_rule_type_email'] = Array('card' => '1', 'exclusive' => true);
		$links[SQ_LINK_TYPE_3]['form_question_rule_type_numeric'] = Array('card' => '1', 'exclusive' => true);
		$links[SQ_LINK_TYPE_3]['form_question_rule_type_found_in'] = Array('card' => '1', 'exclusive' => true);
		$links[SQ_LINK_TYPE_3]['form_question_rule_type_regexp'] = Array('card' => '1', 'exclusive' => true);
		$links[SQ_LINK_TYPE_3]['form_question_rule_type_contain'] = Array('card' => '1', 'exclusive' => true);
		return $links;

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
		return parent::printBody();

	} // end printBody()


	/**
	* generates the html for this question to display it in the frontend
	*
	* @access public
	* @return string
	*/
	function getAnswer($content_file=true, $value='')
	{
		$height = ($this->attr('height')) ? $this->attr('height') : 1;
		$width  = $this->attr('width');
		$max    = $this->attr('max');
		$extras = $this->attr('extra');
		$name   = $this->id;


		// generate some php for the value, which can be used for both
		// auto-answer and returning to saved forms
		if ($content_file) {
			$html  = '<?php $q = &$am->getAsset('.$this->id.', \'form_question_type_text\'); ?>';
			$value = '<?php echo $q->getValue($submission); ?>';
		} else {
			$html = '';
		}
		
		ob_start();
		if ($height == 1) {
			text_box($name, $value, $width, $max, $extras);
		} else {
			text_area($name, $value, $width, $height, $max, $extras);
		}
		$html .= ob_get_contents();
		ob_end_clean();
		
		// calling text_box will convert the open and close php tags into their html counter-parts
		// so we need to convert them back to proper php tags

		return html_entity_decode($html);

	} // end getAnswer()

	
	/**
	*
	*
	*/
	function getValue(&$submission)
	{
		if ($submission) {
			$answer = $submission->getAnswer($this->id);
			if ($answer) {
				echo $answer;
			}
		}
	}


	/**
	*
	*
	*
	*
	*/
	function processForm(&$submission)
	{
		if (!$submission) return false;
		
		if (isset($_POST[$this->id])) {
			$val = $_POST[$this->id];
			$submission->setAnswer($this->id, $val);
			$submission->setSummary($this->id, $this->attr('name'), $val, $this->section_id, $this->section_name);
			parent::processForm($submission);
		}
		
		return true;
	}


}//end class
?>
