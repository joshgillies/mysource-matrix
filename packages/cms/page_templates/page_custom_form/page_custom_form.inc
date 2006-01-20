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
* $Id: page_custom_form.inc,v 1.20 2006/01/20 02:48:04 sdanis Exp $
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
* @version $Revision: 1.20 $
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
	* @return boolean
	* @access private
	*/
	function _createAdditional(&$link)
	{
		if (!parent::_createAdditional($link)) return FALSE;

		// create a new form_email (form with emailing abilities)
		$GLOBALS['SQ_SYSTEM']->am->includeAsset('form_email');
		$form =& new Form_Email();
		$copy_link = Array('asset' => &$this, 'link_type' => SQ_LINK_TYPE_2, 'is_dependant' => 1, 'is_exclusive' => 1);

		$form->setAttrValue('name', translate('cms_form_contents'));
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
		$form_link = $GLOBALS['SQ_SYSTEM']->am->getLink($this->id, SQ_LINK_TYPE_2, 'form', FALSE, '', 'major');
		$form =& $GLOBALS['SQ_SYSTEM']->am->getAsset($form_link['minorid'], $form_link['minor_type_code']);

		if (is_null($form)) {
			trigger_localised_error('CMS0023', E_USER_ERROR, $this->name, $this->id);
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
		$form_link = $GLOBALS['SQ_SYSTEM']->am->getLink($this->id, SQ_LINK_TYPE_2, 'form_email', TRUE, '', 'major');
		$form =& $GLOBALS['SQ_SYSTEM']->am->getAsset($form_link['minorid'], $form_link['minor_type_code']);

		if (is_null($form)) {
			trigger_localised_error('CMS0023', E_USER_ERROR, $this->name, $this->id);
			return FALSE;
		}
		$form->printBody();

		return TRUE;

	}//end printBody()


	/**
	* Called by the design to print the body of this asset when in limbo
	*
	* Custom Form should allow the logs to be viewed
	*
	* @return void
	* @access public
	*/
	function printLimbo()
	{
		$lm =& $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('layout_manager');

		if ($lm->hasLayout($this->type(), 'log')) {
			parent::printLimbo();
		} else {
			require_once SQ_INCLUDE_PATH.'/backend_outputter.inc';
			$o =& $GLOBALS['SQ_SYSTEM']->backend->out;
			$edit_fns = $this->getEditFns();
			$prefix = $this->getPrefix();
			$edit_fns->paintExport($this, $o, $prefix);
			$edit_fns->paintSubmissions($this, $o, $prefix);
		}

	}//end printLimbo()


}//end class
?>
