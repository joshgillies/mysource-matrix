<?php

require_once dirname(__FILE__).'/../../form_question/form_question.inc';

/**
* Form_Question_Type_Tickbox_List
*
* Purpose
*
*
* @author  Marc Mcintyre <mmcintyre@squiz.net>
* @version $Version$ - 1.0
* @package MySource_Matrix_Packages
* @subpackage cms
*/
class Form_Question_Type_Tickbox_List extends Form_Question
{


	/**
	* Constructor
	*
	* @param int	$assetid	the asset id to be loaded
	*
	*/
	function Form_Question_Type_Tickbox_List($assetid=0) 
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
		$links[SQ_LINK_TYPE_3]['question_rule_type_compare'] = Array('card' => '1', 'exclusive' => true);
	//	$links[SQ_LINK_TYPE_3]['question_rule_type_email'] = Array('card' => '1', 'exclusive' => true);
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
	
	} // end printBody()


	/**
	* generates the html for this question to display it in the frontend
	*
	* @access public
	* @return string
	*/
	function getAnswer()
	{

		// generate some php for the value, which can be used for both
		// auto-answer and returning to saved forms
		
		$html  = '<'.'?php $q = &$am->getAsset('.$this->id.', \'form_question_type_select\'); ?'.'>';
		$html .= $this->list_box($this->attr('options'), $this->attr('multiple'), $this->attr('horizontal'), $this->attr('extra'));	
		return $html;

	} // end getAnswer()

	function list_box($options, $multiple = false, $hor = false, $extras ='')
	{
		$type  = ($multiple) ? 'checkbox' : 'radio';
		if (!stristr($extras, 'class=')) $extras .= ' class="sq-form-field"';
		$name  = $this->id .(($multiple) ? '[]' : '');

		$html  = '';
		$html .= "<table cellpadding=\"0\" cellspacing=\"0\">\n";
		$html .= ($hor) ? "<tr>\n" : '';
		
		foreach ($options as $key => $value) {
			$html .= ($hor) ? "<td>\n" : "<tr><td>\n";
			$html .= '<input type="'.$type.'" ';
			$html .= 'name="'.htmlspecialchars($key).'_'.$name.'" ';
			$html .= 'value="'.htmlspecialchars($key).'" ';
			$html .= '<'.'?php echo $q->isSelected(\''.$key.'_'.$name.'\'); ?'.'>';
			$html .= $extras.' />';
			$html .= '&nbsp;'.$value;
			$html .= ($hor) ? "</td>\n" : "</td></tr>";
		}

		$html .= ($hor) ? "</tr>\n" : '';
		$html .= '</table>';
		return $html;
	}
	
	/**
	*
	*
	*/
	function getValue(&$submission)
	{
		if ($submission) {
			$answer = $submission->getAnswer($this->id);
			if ($answer) {
				return $answer;
			}
		} 
		return $this->attr('default');
	}

	function getOptions()
	{
		return split("\r\n", $this->attr('options'));
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

		$val = $_POST[$this->id];
		$submission->setAnswer($this->id, $val, $this->attr('name'));
		
		parent::processForm($submission);
		
		return true;
	}

	function isSelected($id)
	{

	}


}//end class
?>
