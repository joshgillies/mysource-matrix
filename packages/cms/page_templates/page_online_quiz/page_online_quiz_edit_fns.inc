<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd       |
* | ACN 084 670 600                                                    |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.net) so we may provide   |
* | you a copy.                                                        |
* +--------------------------------------------------------------------+
*
* $Id: page_online_quiz_edit_fns.inc,v 1.2 2006/12/20 23:57:03 rhoward Exp $
*
*/


require_once SQ_PACKAGES_PATH.'/cms/page_templates/page_asset_listing/page_asset_listing_edit_fns.inc';


/**
* Page_Online_Quiz_Edit_Fns
*
* Edit Functions for the shopping cart
*
*
* @author  Luke Wright <lwright@squiz.net>
* @version $Revision: 1.2 $
* @package MySource_Matrix_Packages
* @subpackage ecommerce
*/
class Page_Online_Quiz_Edit_Fns extends Page_Asset_Listing_Edit_Fns
{


	/**
	* Constructor
	*
	* @return void
	* @access public
	*/
	function Page_Online_Quiz_Edit_Fns()
	{
		$this->Page_Asset_Listing_Edit_Fns();

	}//end constructor


	/**
	* Paint the interface for handling the 'user_details_source' attribute
	* and toggling the 'user_details' bodycopy and form
	*
	* @param object	&$asset	the asset to which we belong
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintUserDetailsSource(&$asset, &$o, $prefix)
	{
		$write_access = $asset->writeAccess('attributes');

		$attr =& $asset->getAttribute('user_details_source');
		$attr->paint($prefix.'_'.$attr->id, !$write_access);

		$form =& $asset->getForm();
		if (!is_null($form)) {
			echo '<p>'.get_asset_tag_line($form->id, 'details').'</p>';
		}
		$GLOBALS['SQ_SYSTEM']->am->forgetAsset($form);

		$bodycopy =& $asset->getBodycopy('user_details');
		if (!is_null($bodycopy)) {
			echo '<p>'.get_asset_tag_line($bodycopy->id, 'contents').'</p>';
		}
		$GLOBALS['SQ_SYSTEM']->am->forgetAsset($bodycopy);

		return $write_access;

	}//end paintUserDetailsSource()


	/**
	* Process the interface for handling the 'user_details_source' attribute
	* and toggling the 'user_details' bodycopy and form
	*
	* @param object	&$asset	the asset to which we belong
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function processUserDetailsSource(&$asset, &$o, $prefix)
	{
		$write_access = $asset->writeAccess('attributes');

		if ($write_access) {
			$attr =& $asset->getAttribute('user_details_source');
			$attr->process($prefix.'_'.$attr->id);
			if (!$attr->processed || !$asset->setAttrValue($attr->name, $attr->value)) {
				// bail now if we can't process the attribute properly
				return FALSE;
			}
		}

		return $write_access;

	}//end processUserDetailsSource()


	/**
	* Paint the interface for handling the 'after_submit_action' attribute
	* and toggling the 'thank_you' bodycopy
	*
	* @param object	&$asset	the asset to which we belong
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintAfterSubmitAction(&$asset, &$o, $prefix)
	{
		$write_access = $asset->writeAccess('attributes') && $asset->writeAccess('links');

		$attr =& $asset->getAttribute('after_submit_action');
		$attr->paint($prefix.'_'.$attr->id, !$write_access);

		if ($attr->value == 'thank_you') {
			$bodycopy =& $asset->getBodycopy('thank_you');
			if (!is_null($bodycopy)) {
				echo '<p>'.get_asset_tag_line($bodycopy->id, 'contents').'</p>';
			}
			$GLOBALS['SQ_SYSTEM']->am->forgetAsset($bodycopy);
		}

		return $write_access;

	}//end paintAfterSubmitAction()


	/**
	* Process the interface for handling the 'after_submit_action' attribute
	* and toggling the 'thank_you' bodycopy
	*
	* @param object	&$asset	the asset to which we belong
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function processAfterSubmitAction(&$asset, &$o, $prefix)
	{
		$write_access = $asset->writeAccess('attributes') && $asset->writeAccess('links');

		if ($write_access) {
			$attr =& $asset->getAttribute('after_submit_action');
			$attr->process($prefix.'_'.$attr->id);
			if (!$attr->processed || !$asset->setAttrValue($attr->name, $attr->value)) {
				// bail now if we can't process the attribute properly
				return FALSE;
			}

			// if form is selected, set up the thank_you bodycopy
			$enable_bodycopy = ($attr->value == 'thank_you');
			$asset->setBodycopy('thank_you', $enable_bodycopy);
		}

		return $write_access;

	}//end processAfterSubmitAction()


	/**
	* Paint the interface for handling the 'can_retake' attribute
	* and toggling the'already_taken' bodycopy
	*
	* @param object	&$asset	the asset to which we belong
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintCanRetake(&$asset, &$o, $prefix)
	{
		$write_access = $asset->writeAccess('attributes') && $asset->writeAccess('links');

		$attr =& $asset->getAttribute('can_retake');
		$attr->setEditParam('true_text', translate('yes'));
		$attr->setEditParam('false_text', translate('no'));
		$attr->paint($prefix.'_'.$attr->id, !$write_access);

		if ($attr->value == FALSE) {
			$bodycopy =& $asset->getBodycopy('already_taken');
			if (!is_null($bodycopy)) {
				echo '<p>'.get_asset_tag_line($bodycopy->id, 'contents').'</p>';
			}
			$GLOBALS['SQ_SYSTEM']->am->forgetAsset($bodycopy);
		}

		return $write_access;

	}//end paintCanRetake()


	/**
	* Process the interface for handling the 'after_submit_action' attribute
	* and toggling the 'thank_you' bodycopy
	*
	* @param object	&$asset	the asset to which we belong
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function processCanRetake(&$asset, &$o, $prefix)
	{
		$write_access = $asset->writeAccess('attributes') && $asset->writeAccess('links');

		if ($write_access) {
			$attr =& $asset->getAttribute('can_retake');
			$attr->process($prefix.'_'.$attr->id);
			if (!$attr->processed || !$asset->setAttrValue($attr->name, $attr->value)) {
				// bail now if we can't process the attribute properly
				return FALSE;
			}

			// if form is selected, set up the thank_you bodycopy
			// can_retake == TRUE means we don't need the "Already Taken" bodycopy
			$enable_bodycopy = !($attr->value);
			$asset->setBodycopy('already_taken', $enable_bodycopy);
		}

		return $write_access;

	}//end processCanRetake()


	/**
	* Paint links to the default formats
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintDefaultFormatLinks(&$asset, &$o, $prefix)
	{
		?>
		<table class="sq-backend-table">
			<tr>
				<th><?php echo translate('edit_format') ?></th>
				<th><?php echo translate('note') ?></th>
			</tr>
			<?php
			$bodycopies = $asset->_bodycopies;

			// manually print page_contents and no_results
			unset($bodycopies['page_contents']);
			unset($bodycopies['no_results']);
			$this->_paintDefaultFormatLink($asset->id, 'page_contents', translate('cms_listing_page_contents_desc'));
			$this->_paintDefaultFormatLink($asset->id, 'no_results', translate('cms_listing_no_results_bodycopy_desc'));

			foreach ($bodycopies as $bodycopy_code => $bodycopy_name) {
				$this->_paintDefaultFormatLink($asset->id, $bodycopy_code, translate('online_quiz_bodycopy_'.$bodycopy_code.'_desc'));
			}
			$this->_paintDefaultFormatLink($asset->id, 'user_details', translate('online_quiz_form_user_details_desc'), 'simple_form', 'details');

			$folder =& $asset->getFolder('type_formats');
			$this->_paintDefaultFormatLink($folder->id, 'default_format', translate('cms_listing_default_asset_desc'));
			?>
		</table>
		<?php

		return FALSE;

	}//end paintDefaultFormatLinks()


}//end class

?>
