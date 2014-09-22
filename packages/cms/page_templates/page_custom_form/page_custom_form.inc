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
* $Id: page_custom_form.inc,v 1.30 2013/08/06 01:16:13 ewang Exp $
*
*/


require_once SQ_CORE_PACKAGE_PATH.'/page/page.inc';

/**
* Page_Custom_Form
*
* Purpose
*
*
* @author  Marc Mcintyre <mmcintyre@squiz.net>
* @version $Revision: 1.30 $
* @package MySource_Matrix_Packages
* @subpackage cms
*/
class Page_Custom_Form extends Page
{


	/**
	* Constructor
	*
	* @param int	$assetid	the asset id to be loaded
	*
	*/
	function __construct($assetid=0)
	{
		$this->_ser_attrs = TRUE;
		parent::__construct($assetid);

	}//end constructor


	/**
	* Perform any additional processing required during the creation of this asset
	*
	* Custom forms create a form asset when they are created
	*
	* @param array	&$link	information used to create the initial link
	*
	* @return boolean
	* @access private
	*/
	function _createAdditional(&$link)
	{
		if (!parent::_createAdditional($link)) return FALSE;

		// create a new form_email (form with emailing abilities)
		$GLOBALS['SQ_SYSTEM']->am->includeAsset('form_email');
		$form = new Form_Email();
		$copy_link = Array('asset' => &$this, 'link_type' => SQ_LINK_TYPE_2, 'is_dependant' => 1, 'is_exclusive' => 1);

		$form->setAttrValue('name', translate('Form Contents'));

		return $form->create($copy_link);

	}//end _createAdditional()


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
		$page_links[SQ_LINK_TYPE_2]['form_email'] = Array('card' => '1', 'exclusive' => TRUE);

		return $page_links;

	}//end _getAllowedLinks()


	/**
	* print out headers or HTML that the asset requires to be printed before the design
	*
	* We have to do a little bit of trickery here to make sure that the design
	* printed, and the global asset variables printed out, are for the form
	* PAGE, rather than the form CONTENTS. Hence, we stop the contents asset
	* from sending its printFrontend() up to the design, so the page asset can.
	*
	* @return boolean
	* @access public
	*/
	function printFrontend()
	{
		$form = $this->getForm();

		if (is_null($form) && !array_get_index($this->_tmp, '__creating__', FALSE)) {
			trigger_localised_error('CMS0023', sprintf(translate('Could not locate the form asset for Custom Form "%s"'), $this->name, $this->id), E_USER_ERROR)
			return FALSE;
		}

		return parent::printFrontend();

	}//end printFrontend()


	/**
	* Called by the design to print the body of this asset
	* it's up to the kids to override
	*
	* @return boolean
	* @access public
	*/
	function printBody()
	{

		// evaluate conditions to show conditional divs and replace conditional keywords
		$conditions = $this->getConditions();
		if(!empty($conditions)) {
		    $GLOBALS['SQ_SYSTEM']->pm->startTimer($this, 'evaluate conditions');
		    $this->evaluateConditions();
		    $GLOBALS['SQ_SYSTEM']->pm->stopTimer($this, 'evaluate conditions');
		}


		// start performance mode timer
		$GLOBALS['SQ_SYSTEM']->pm->startTimer($this);

		$form = $this->getForm();

		// If we are still being created, we will not have the "form" asset yet (bug #5979)
		if (!array_get_index($this->_tmp, '__creating__', FALSE)) {
			if (is_null($form)) {
				trigger_localised_error('CMS0023', sprintf(translate('Could not locate the form asset for Custom Form "%s"'), $this->name, $this->id), E_USER_ERROR)
				return FALSE;
			}
			$form->printBody();
		}

		// stop performance mode timer
		$GLOBALS['SQ_SYSTEM']->pm->stopTimer($this);
		return TRUE;

	}//end printBody()


	/**
	* Returns the editable content of this asset
	*
	* @param boolean $empty_content If TRUE, empty content will be returned
	*
	* @return string
	* @access public
	*/
	function getContent($empty_content=TRUE)
	{
		// Editable content of all the Custom Form's bodycopies
		$content = '';

		$form = $this->getForm();
		if (!is_null($form)) {
			$bodycopy = $form->getBodycopy('Page Contents');
			$folder_link = $GLOBALS['SQ_SYSTEM']->am->getLink($form->id, SQ_LINK_TYPE_2, 'folder', TRUE, 'bodycopies_folder');
			if (!empty($folder_link['minorid'])) {
				$bodycopy_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($folder_link['minorid'], SQ_LINK_TYPE_2, 'bodycopy');
			}
			if (!empty($bodycopy_links)) {
				foreach($bodycopy_links as $link) {
					if (!empty($link['minorid'])) {
						$bodycopy = $GLOBALS['SQ_SYSTEM']->am->getAsset($link['minorid']);
						if (!is_null($bodycopy)) {
							$content .= ' '.$bodycopy->getContent();
						}
					}//end if
				}//end foreach
			}//end if
		}//end if

		$form_sections = $form->getAllSections();
		if (!empty($form_sections)) {
			foreach($form_sections as $section) {
				if (!is_null($section)) {
					$section_bodycopy = $section->getFormatBodycopy();
					if (!is_null($section_bodycopy)) {
						$content .= ' '.$section_bodycopy->getContent();
					}//end if
				}//end if
			}//end foreach
		}//end if

		return $content;

	}//end getContent()


	/**
	* Return the form linked to this page
	*
	* @return object
	* @access public
	*/
	function &getForm()
	{
		$form = NULL;

		$form_link = $GLOBALS['SQ_SYSTEM']->am->getLink($this->id, SQ_LINK_TYPE_2, 'form', FALSE, '', 'major');
		if (!empty($form_link)) {
			$form = $GLOBALS['SQ_SYSTEM']->am->getAsset($form_link['minorid'], $form_link['minor_type_code']);
			$form->setParentAsset($this);
		}

		return $form;

	}//end getForm()


	/**
	 * Allow form page to replace response keywords from its form
	 *
	 * @param string $keyword    Keyword to find a replacement for
	 *
	 * @return string
	 * @access public
	 */
	public function getKeywordReplacement($keyword)
	{
	    	$is_question_keyword = preg_match('/^question\_(.*)\_(\d+[\_\:]q\d+)$/', $keyword, $question_matches);
		$is_response_keyword = preg_match('/^response\_(\d+[\_\:]q\d+)(\_raw)?$/', $keyword, $response_matches);
		$is_section_keyword  = preg_match('/^section\_(.*)\_(\d+)$/', $keyword, $section_matches);
		$is_form_keyword	 = preg_match('/^form\_(.*)$/', $keyword, $form_matches);

		if ($is_question_keyword || $is_response_keyword || $is_section_keyword || $is_form_keyword) {
		    $form = $this->getForm();
		    return $form->getDisplayKeywordReplacement($keyword);
		}
		return parent::getKeywordReplacement($keyword);

	}//end getKewordReplacement()


	/**
	*
	* Call this function when the asset has been changed in some way (or its containers), so we should update the condition evaluation array of it
	*
	* @param boolean	$update_parents	should we go through an update all our dependant parents as well ?
	*
	* @return boolean
	* @access private
	*/
	function _updated($update_parents=TRUE)
	{
		if (!parent::_updated($update_parents)) return FALSE;

		$form = $this->getForm();
		if(empty($form)) return TRUE;

		$am = $GLOBALS['SQ_SYSTEM']->am;
		$bodycopy_array = Array();

		// get bodycopy from current form
		$folder_link = $GLOBALS['SQ_SYSTEM']->am->getLink($form->id, SQ_LINK_TYPE_2, 'folder', TRUE, 'bodycopies_folder');
		if (!empty($folder_link)) {
			$folder = $am->getAsset($folder_link['minorid'], $folder_link['minor_type_code']);
			$bodycopy_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($folder->id, SQ_LINK_TYPE_2, 'bodycopy');
			foreach($bodycopy_links as $data) {
				$bodycopy_array[] = $data['minorid'];
			}
		}


		// get all format bodycopies from sections and nested sections
		$sections = $form->getSections();
		foreach($sections as $section) {
			$bodycopy_array = array_merge($bodycopy_array, $section->getNestedFormatBodycopies());
		}


		if(empty($bodycopy_array)) return TRUE;

		return ($this->extractConditionsFromBodycopy($bodycopy_array));
	}//end _updated()


	/**
	* Return TRUE if the link is deletable, otherwise return error message
	*
	* @param int    $linkid the link id of the link to remove
	*
	* @return boolean|string
	* @access public
	*/
	public function isDeletableLink($linkid)
	{
		if ($GLOBALS['SQ_PURGING_TRASH']) {
			return TRUE;
		}

		// Any exclusive type link to this asset cannot be deleted
		$link = $GLOBALS['SQ_SYSTEM']->am->getLinkById($linkid);

		if (array_get_index($link, 'is_exclusive', FALSE)) {
			$major_type_code = isset($link['major_type_code']) ? ucwords(str_replace('_', ' ', $link['major_type_code'])) : translate('Unknown Asset Type');

			$minor_type_code = isset($link['minor_type_code']) ? ucwords(str_replace('_', ' ', $link['minor_type_code'])) : translate('Unknown Asset Type');

			return sprintf(translate('Cannot delete exclusive link between %1$s and %2$s'), $major_type_code, $minor_type_code);

		}

		return TRUE;

	}//end isDeletableLink()

}//end class
?>
