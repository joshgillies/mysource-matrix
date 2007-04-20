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
* $Id: form_section_management.inc,v 1.14.4.1 2007/04/20 01:45:55 hnguyen Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Form_Section_Management
*
* @see Asset_Management
*
* @author  Marc McIntyre <mmcintyre@squiz.net>
* @version $Revision: 1.14.4.1 $
* @package MySource_Matrix_Packages
* @subpackage cms
*/
class Form_Section_Management extends Asset_Management
{


	/**
	* Constructor
	*
	*/
	function Form_Section_Management(&$pm)
	{
		$this->Asset_Management($pm);

		$this->vars = Array(
						'name'					=> Array(
													'added'		=> '0.0.1',
													'type'		=> 'text',
													'default'	=> '',
												   ),
						'sort_order'			=> Array(
													'added'		=> '0.0.1',
													'type'		=> 'serialise',
													'default'	=> Array(),
												   ),
						'questions'				=> Array(
													'added'		=> '0.0.1',
													'type'		=> 'serialise',
													'default'	=> Array(),
												   ),
						'question_col_width'	=> Array(
													'added'		=> '0.0.1',
													'type'		=> 'html_width',
													'default'	=> '40%',
												   ),
						'answer_col_width'		=> Array(
													'added'		=> '0.0.1',
													'type'		=> 'html_width',
													'default'	=> '60%',
												   ),
						'section_width'			=> Array(
													'added'		=> '0.0.1',
													'type'		=> 'html_width',
													'default'	=> '100%',
												   ),
						'next_questionid'		=> Array(
													'added'		=> '0.3',
													'type'		=> 'int',
													'default'	=> 0,
												   ),
					  );

		}//end constructor



	/**
	* Upgrade
	*
	* @param string	$current_version	the version of the asset that is currenly installed
	*
	* @return boolean
	* @access public
	*/
	function _upgrade($current_version)
	{
		if (!parent::_upgrade($current_version)) return FALSE;

		$asset_name = strtoupper($this->_pm->getTypeInfo($this->getAssetType(), 'name'));

		if (version_compare($current_version, '0.6', '<')) {
			// version 0.3 -> 0.5 (0.4 was a botched version)
			// escaping of "<?php", "<?", etc. now happens in content_type::paint(), but we need to
			// regenerate each bodycopy_container's content file so that we catch any nasties that might
			// already be present that we need to get rid of.
			// ignore cases where <?php has already been escaped to &lt;?php
			pre_echo('STARTING '.$asset_name.' UPGRADE - TO VERSION 0.6');
			// get all 'bodycopy_container' assets (or their descendants)

			$form_sections = $GLOBALS['SQ_SYSTEM']->am->getTypeAssetIds($this->getAssetType(), FALSE);

			pre_echo('Upgrading '.count($form_sections).' assets...');
			foreach ($form_sections as $assetid) {
				$form_section =& $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);

				// reasonably critical upgrade - report any assets that didn't make it
				if (is_null($form_section)) {
					pre_echo('Could get asset #'.$assetid);
					continue;
				}

				// regenerate the bodycopy_container's content file
				$form_section_edit_fns =& $form_section->getEditFns();
				if (!$form_section_edit_fns->generateContentFile($form_section)) {
					pre_echo('Could not generate content file for asset #'.$assetid);
				}

				unset($form_section_edit_fns);
				$GLOBALS['SQ_SYSTEM']->am->forgetAsset($form_section);
			}
			pre_echo($asset_name.' UPGRADE COMPLETE - FROM VERSION '.$current_version.' TO 0.6');

		}//end version 0.5 upgrade

		return TRUE;

	}//end _upgrade()


}//end class
?>
