<?php
/**
* +--------------------------------------------------------------------+
* | Squiz.net Open Source Licence                                      |
* +--------------------------------------------------------------------+
* | Copyright (c), 2003 Squiz Pty Ltd (ABN 77 084 670 600).            |
* +--------------------------------------------------------------------+
* | This source file may be used subject to, and only in accordance    |
* | with, the Squiz Open Source Licence Agreement found at             |
* | http://www.squiz.net/licence.                                      |
* | Make sure you have read and accept the terms of that licence,      |
* | including its limitations of liability and disclaimers, before     |
* | using this software in any way. Your use of this software is       |
* | deemed to constitute agreement to be bound by that licence. If you |
* | modify, adapt or enhance this software, you agree to assign your   |
* | intellectual property rights in the modification, adaptation and   |
* | enhancement to Squiz Pty Ltd for use and distribution under that   |
* | licence.                                                           |
* +--------------------------------------------------------------------+
*
* $Id: page_custom_form.inc,v 1.16 2005/04/15 00:37:46 lwright Exp $
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
* @version $Revision: 1.16 $
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
	function Page_Custom_Form($assetid=0)
	{
		$this->Page($assetid);

	}//end constructor


	/**
	* Perform any additional processing required during the creation of this asset
	*
	* Custom forms create a form asset when they are created
	*
	* @param array	&$link	information used to create the initial link
	*
	* @access private
	* @return boolean
	*/
	function _createAdditional(&$link)
	{
		if (!parent::_createAdditional($link)) return false;

		// create a new form_email (form with emailing abilities)
		$GLOBALS['SQ_SYSTEM']->am->includeAsset('form_email');
		$form = new Form_Email();
		$copy_link = Array('asset' => &$this, 'link_type' => SQ_LINK_TYPE_2, 'is_dependant' => 1, 'is_exclusive' => 1);

		$form->setAttrValue('name', 'Form Contents');
		return $form->create($copy_link);

	}//end _createAdditional()


	/**
	* Returns an array of all the permitted link type, the type asset and the cardinality
	*
	* @see Asset::_getAllowLinks()
	* @access private
	*/
	function _getAllowedLinks()
	{
		$page_links = parent::_getAllowedLinks();
		$page_links[SQ_LINK_TYPE_2]['form_email'] = Array('card' => '1', 'exclusive' => true);

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
	* @return boolean	indicating success (currently always true)
	* @access public
	*/
	function printFrontend()
	{
		$form_link = $GLOBALS['SQ_SYSTEM']->am->getLink($this->id, SQ_LINK_TYPE_2, 'form', false, '', 'major');
		$form = &$GLOBALS['SQ_SYSTEM']->am->getAsset($form_link['minorid'], $form_link['minor_type_code']);

		if (is_null($form)) {
			trigger_localised_error('CMS0023', E_USER_ERROR, $this->name, $this->id);
			return false;
		}

		return parent::printFrontend();

	}//end printFrontend()


	/**
	* Called by the design to print the body of this asset
	* it's up to the kids to override
	*
	* @access public
	*/
	function printBody()
	{
		$form_link = $GLOBALS['SQ_SYSTEM']->am->getLink($this->id, SQ_LINK_TYPE_2, 'form_email', true, '', 'major');
		$form = &$GLOBALS['SQ_SYSTEM']->am->getAsset($form_link['minorid'], $form_link['minor_type_code']);

		if (is_null($form)) {
			trigger_localised_error('CMS0023', E_USER_ERROR, $this->name, $this->id);
			return false;
		}
		$form->printBody();

		return true;

	}//end printBody()

}//end class
?>
