<?php

require_once SQ_INCLUDE_PATH.'/asset.inc';

/**
* Form_Question
*
* Purpose
*
*
* @author  Marc Mcintyre <mmcintyre@squiz.net>
* @version $Version$ - 1.0
* @package MySource_Matrix_Packages
* @subpackage cms
*/
class Form_Question extends Asset
{


	/**
	* @var $section_name the name of the section that this question belongs to
	*/
	var $section_name = '';
	var $section_id   = 0;


	/**
	* Constructor
	*
	* @param int	$assetid	the asset id to be loaded
	*
	*/
	function Form_Question($assetid=0) 
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
		return parent::paintBackend($o);

	}// end paintBackend()


	/**
	* Called by the design to print the body of this asset
 	*
	* @access public
	*/
	function printBody()
	{
		return parent::printBody();

	}// end printBody()


	/**
	* gets called when anything in the question gets changed, so we can regenerate the section and form(s)
	*
	* @see Asset::_updated()
	*
	* @return boolean
	* @access private
	*/
	function _updated()
	{
		if (!parent::_updated()) return false;

		$link = $GLOBALS['SQ_SYSTEM']->am->getLinks($this->id, SQ_LINK_TYPE_2, 'form_section', true, 'minor');
		
		if (!empty($link[0])) {
			$section = &$GLOBALS['SQ_SYSTEM']->am->getAsset($link[0]['majorid'], $link[0]['major_type_code']);
			if (is_null($section)) return false;
			return $section->fileRegeneration();
		}
		return true;
	}



	/**
	* returns a string based on the passed in value
	* used for generating forms eg. if value is 'answer', this
	* function returns the html for this question type
	*
	* @param	string	 $val
	* 
	* @access public
	* @return string
	*/
	function getVal($val = '')
	{
		switch (strtolower($val)) {
			case 'title' :
				return $this->attr('name');
			case 'answer' :
				return $this->getAnswer();
			break;
			case 'note' :
				return $this->attr('note');
			case 'value' :
				return $this->getValue();
		}
	}

	/**
	* 
	*/
	function processForm(&$submission)
	{
		$rules = &$this->getRules();
		
		for (reset($rules); null !== ($k = key($rules)); next($rules)) {
			$rule =& $rules[$k];
			$rule->processForm($submission, $this->id, $this->attr('name')); 
		}
	}


	/**
	* @abstact
	*/
	function getValue()
	{
		return true;
	}


	/**
	* @abstact
	*/
	function getAnswer($content_file=true, $value='')
	{
		return true;
	}


	/**
	* @abstract
	*/
	function generateError()
	{	
		return true;
	
	} // end generateError() 


	/**
	* returns all the links to rules that this question has
	*
	* @access public
	* @return Array(link)
	*/
	function getRuleLinks()
	{
		if (empty($this->tmp['rule_links'])) {
			$this->tmp['rule_links'] = Array();
			$this->tmp['rule_links'] = $GLOBALS['SQ_SYSTEM']->am->getLinks($this->id, SQ_LINK_TYPE_3, 'form_question_rule', false, 'major');	
		}
		return $this->tmp['rule_links'];
	
	} // end getRuleLinks()


	/**
	* returns all the rules that this question has
	*
	* @access public
	* @return Array(link)
	*/
	function &getRules()
	{
		$am = &$GLOBALS['SQ_SYSTEM']->am;
		$links = $this->getRuleLinks();
		$rules = Array();
		
		foreach ($links as $link) {
			$rules[] = &$am->getAsset($link['minorid'], $link['minor_type_code']);
		}
		return $rules;
	
	} // end getRules()


	/**
	* returns all the form asset ids this question is attached to
	*
	* @access public
	* @return Array(integer ids)
	*/
	function getFormIds()
	{
		if (empty($this->tmp['form_ids'])) {
			$this->tmp['form_ids'] = Array();
			$this->tmp['form_ids'] = $GLOBALS['SQ_SYSTEM']->am->getParents($this->id, 'form', false);
		}
		return (!empty($this->tmp['form_ids'])) ? $this->tmp['form_ids'] : Array();

	} // end getFormIds()


	/**
	* returns all the forms this question is attached to 
	* 
	* @access public
	* @return Array(&object form)
	*/
	function &getForms()
	{
		$am = &$GLOBALS['SQ_SYSTEM']->am;
		$ids = $this->getFormids();
		$forms = Array();
		foreach ($ids as $id) {
			$forms[] = &$am->getAsset($id);
		}
		return (!empty($forms)) ? $forms : Array();

	} // end getForms()


	/**
	* returns the number of rules this question has
	*
	* @access public
	* @return Array(link)
	*/
	function getRuleCount()
	{
		return count($this->getRuleLinks());
	
	}// end getRuleCount()


	/**
	* catches the special information change and sets it in this question's forms
	* 
	* @see Asset::setAttrValue()
	*
	* @param string		$name				the name of the attribute
	* @param mixed		$value				the value to set 
	* @param boolean	$dont_run_updated	run an update or not
	*
	* @access public
	* @return boolean
	*/
	function setAttrValue($name, $value, $dont_run_updated=false) {
		
		if ($name == 'special_info') {
			$forms = &$this->getForms();
			for (reset($forms); null !== ($k = key($forms)); next($forms)) {
				$form =& $forms[$k];
				$form->setSpecialInformation($this->id, $value);
			}
		}
		return parent::setAttrValue($name, $value, $dont_run_updated);
		
	} // end setAttrValue()


	/**
	* sets the name of the section that this question belongs to	
	*
	* @param string $name the name of the section
	*
	* @access public
	* @return boolean 
	*/
	function setSection($name = '', $id)
	{
		$this->section_name = $name;
		$this->section_id   = $id;
		return true;
	}

	function _getName($short_name = false)
	{
		return $this->attr('name');
	}

	function getSummary($answer)
	{
		return $answer;
	}

}//end class
?>
