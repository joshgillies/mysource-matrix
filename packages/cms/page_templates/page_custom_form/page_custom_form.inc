<?php
/**
* +--------------------------------------------------------------------+
* | MySource 3 - MySource Matrix                                       |
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
* $Id: page_custom_form.inc,v 1.8 2003/11/18 15:43:37 brobertson Exp $
* $Name: not supported by cvs2svn $
*/


require_once dirname(__FILE__).'/../../page/page.inc';

/**
* Page_Custom_Form
*
* Purpose
*
*
* @author  Marc Mcintyre <mmcintyre@squiz.net>
* @version $Version$ - 1.0
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

	} // end Page_Custom_Form()


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

		// create a new form_email (form with emailing abilities)
		$GLOBALS['SQ_SYSTEM']->am->includeAsset('form_email');
		$form = new Form_Email();
		$copy_link = Array('asset' => &$this, 'link_type' => SQ_LINK_TYPE_2, 'dependant' => 1, 'exclusive' => 1);

		$form->setAttrValue('name', 'Form Contents');
		if (!$form->create($copy_link)) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			$this->id = 0;
			return false;
		}

		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');

		return $linkid;

	}//end create()


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
