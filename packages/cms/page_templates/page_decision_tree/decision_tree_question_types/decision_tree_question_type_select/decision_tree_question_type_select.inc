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
* $Id: decision_tree_question_type_select.inc,v 1.2 2012/08/30 00:58:08 ewang Exp $
*
*/


require_once SQ_PACKAGES_PATH.'/cms/page_templates/page_decision_tree/decision_tree_question/decision_tree_question.inc';


/**
* Decision_Tree_Question_Type_Select
*
* Purpose
*
*
* @author  Mohamed Haidar <mhaidar@squiz.net>
* @version $Revision: 1.2 $
* @package MySource_Matrix_Packages
* @subpackage cms
*/
class Decision_Tree_Question_Type_Select extends Decision_Tree_Question
{


	/**
	* Constructor
	*
	* @param int	$assetid	the asset id to be loaded
	*
	*/
	function __construct($assetid=0)
	{
		parent::__construct($assetid);

	}//end constructor


	/**
	* Returns an array of all the permitted link type, the type asset and the cardinality
	*
	* @return array
	* @access private
	* @see Asset::_getAllowLinks()
	*/
	function _getAllowedLinks()
	{
		$page_links = parent::_getAllowedLinks();
		return $page_links;

	}//end _getAllowedLinks()

	
	/**
	* generates the html for this question to display it in the frontend
	*
	* @return string
	* @access public
	*/
	function getHtmlField(){
		
		if ($this->attr('style') == 'DROP_DOWN') {
			$replacement =	$this->getHtmlSelectField();
		} else {
			$replacement =	$this->getHtmlRadioField();
		}	
		return $replacement;
		
	}
	
	/**
	* generates the html for a select field
	*
	* @return string
	* @access public
	*/
	function getHtmlSelectField()
	{
		$name = 'q'.$this->id;

		// prepare a valid value for the field id
		$extras = 'id="'.str_replace(':', '_', $name).'" '.$this->attr('extra');
		$extras .= ' onChange="get_next_step(this.value,'.$this->id.')"';
		
		$default_value = $this->attr('empty_key');
		// If the default value is NULL on some configuration, make it empty string to prevent infinite loop.
		if (is_null($default_value)) {
			$default_value = '';
		}//end if
		
		ob_start();
		combo_box($name, $this->getOptions(), FALSE, $default_value, 0, $extras);
		$html = ob_get_contents();
		ob_end_clean();
		return $html;
		
	}//end getHtmlSelectField()
	
	
	/**
	* generates the html for radio box field
	*
	* @return string
	* @access public
	*/
	function getHtmlRadioField()
	{
		$name = 'q'.$this->id;
		// prepare a valid value for the field id
		
		$default_value = $this->attr('empty_key');
		// If the default value is NULL on some configuration, make it empty string to prevent infinite loop.
		if (is_null($default_value)) {
			$default_value = '';
		}//end if
		
		$options = $this->getOptions();
		$onclick = 'get_next_step(this.value,'.$this->id.')';
		
		ob_start();
		echo "<ul>";
		$i = 0;
		foreach ($options as $value => $title){
			if ($value === $default_value) continue;
			$for = htmlspecialchars($name.'_'.$i);
			$extras = 'id="'.$name.'_'.$i.'" '.$this->attr('extra');
			if ($i == 0) {
				//special treatment for the first radio box so the javascript can identify this question.
				$extras = 'id="'.$name.'" '.$this->attr('extra');
				$for = $name;
			}
			echo "<li>";
			radio_button($this->attr('name'), $value, FALSE, $onclick, $extras);
			label($title, $for);
			echo "</li>";
			$i++;
		}
		echo "</ul>";	
		$html = ob_get_contents();
		ob_end_clean();
		return $html;
		
	}//end getHtmlRadioField()
	

	/**
	* returns the options currently in the dropdown list
	*
	* @return array
	* @access public
	*/
	function getOptions()
	{
		$options = $this->attr('options');
		
		if (count($options) > 0) {
			$decoded_options = Array();
			$decoded_options[$this->attr('empty_key')] = $this->attr('empty_text');	
			// remove empty options
			foreach (array_keys($options) as $k) {
				if ($options[$k] != '') {
					$decoded_options[html_entity_decode($k, ENT_COMPAT, SQ_CONF_DEFAULT_CHARACTER_SET)] = html_entity_decode($options[$k], ENT_COMPAT, SQ_CONF_DEFAULT_CHARACTER_SET);
				}
			}
			$options = $decoded_options;	
		}
		
		return $options;

	}//end getOptions()


}//end class
?>
