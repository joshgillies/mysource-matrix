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
* $Id: page_custom_form.inc,v 1.13.2.3 2005/05/02 02:44:58 gsherwood Exp $
* $Name: not supported by cvs2svn $
*/


require_once SQ_CORE_PACKAGE_PATH.'/page/page.inc';

/**
* Page_Custom_Form
*
* Purpose
*
*
* @author  Marc Mcintyre <mmcintyre@squiz.net>
* @version $Revision: 1.13.2.3 $
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

	}// end Page_Custom_Form()


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
	* @return boolean	indicating success (currently always true)
	* @access public
	*/
	function printFrontend()
	{
		$form_link = $GLOBALS['SQ_SYSTEM']->am->getLink($this->id, SQ_LINK_TYPE_2, 'form', false, '', 'major');
		$form = &$GLOBALS['SQ_SYSTEM']->am->getAsset($form_link['minorid'], $form_link['minor_type_code']);

		if (is_null($form)) {
			trigger_error('Could not locate the form asset for Custom Form "'.$this->name.'"', E_USER_ERROR);
			return false;
		}

		return parent::printFrontend();

	}


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
			trigger_error('Could not locate the form asset for Custom Form "'.$this->name.'"', E_USER_ERROR);
			return false;
		}
		$form->printBody();

		return true;

	} // end printBody()

} // end class
?>
