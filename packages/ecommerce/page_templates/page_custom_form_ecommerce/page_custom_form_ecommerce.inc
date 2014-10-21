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
* $Id: page_custom_form_ecommerce.inc,v 1.12 2012/08/30 00:58:34 ewang Exp $
*
*/

require_once SQ_PACKAGES_PATH.'/cms/page_templates/page_custom_form/page_custom_form.inc';

/**
* Page_Custom_Form_Ecommerce
*
* Purpose
*
*
* @author  Andrei Railean <arailean@squiz.net>
* @version $Revision: 1.12 $
* @package MySource_Matrix_Packages
* @subpackage ecommerce
*/
class Page_Custom_Form_Ecommerce extends Page_Custom_Form
{


	/**
	* Perform any additional processing required during the creation of this asset
	*
	* Ecommerce form creates a form when it is created
	*
	* @param array	&$link	information used to create the initial link
	*
	* @access private
	* @return boolean
	*/
	function _createAdditional(&$link)
	{
		if (!page::_createAdditional($link)) return false;

		// create a new form_ecommerce (form with emailing abilities)
		$GLOBALS['SQ_SYSTEM']->am->includeAsset('form_ecommerce');
		$form = new Form_Ecommerce();
		$copy_link = Array('asset' => &$this, 'link_type' => SQ_LINK_TYPE_2, 'is_dependant' => 1, 'is_exclusive' => 1);

		$form->setAttrValue('name', 'Ecommerce Form Contents');

		// We provide the parent id to the ecom form
		// this id can be used while creating the asset and when getUrl is being used
		// if the following line is removed, the creation of this asset under a folder
		// with metadata field and the keyword %asset_url% will fail
		$form->_tmp['parent_form_page_assetid'] =& $this->id;

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
		//unset($page_links[SQ_LINK_TYPE_2]['form_email']);
		$page_links[SQ_LINK_TYPE_2]['form_ecommerce'] = Array('card' => '1', 'exclusive' => true);

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
		$form_link = $GLOBALS['SQ_SYSTEM']->am->getLink($this->id, SQ_LINK_TYPE_2, 'form_ecommerce', true, '', 'major');
		$form = $GLOBALS['SQ_SYSTEM']->am->getAsset($form_link['minorid'], $form_link['minor_type_code']);

		if (is_null($form)) {
			trigger_localised_error('ECOM0002', sprintf(translate('Unable to locate the Ecommerce Rules for ecommerce custom form page "%1$s" (Id: #%2$s)'), $this->name, $this->id), E_USER_ERROR);
			return false;
		}
		$form->printBody();

		return true;

	}//end printBody()


}//end class
?>
