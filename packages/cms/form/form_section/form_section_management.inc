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
* $Id: form_section_management.inc,v 1.25 2012/08/30 00:58:08 ewang Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Form_Section_Management
*
* @see Asset_Management
*
* @author  Marc McIntyre <mmcintyre@squiz.net>
* @version $Revision: 1.25 $
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
													'added'			=> '0.0.1',
													'type'			=> 'text',
													'default'		=> '',
													'is_admin'		=> FALSE,
													'is_contextable'=> TRUE,
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
						'actions'				=> Array(
														'added'		=> '0.3',
														'type'		=> 'serialise',
														'default'	=> Array(),
													),												   
					  );

		}//end constructor


	/**
	* returns an array with information about the events that the form is listening for
	*
	* @return boolean
	* @access public
	*/
	function getEventList()
	{
		return Array(
				Array(
					'event_name'					=> 'requestKeywords',
					'broadcast_type_code'			=> 'content_type',
					'broadcast_strict_type_code'	=> FALSE,
					'options'						=> Array(
														'side_of_link'	=> 'major',
														'indirect'		=> TRUE,
														'is_exclusive'	=> NULL,
														'is_dependant'	=> 1,
														'value'			=> 'format',
														'link_type'		=> SQ_LINK_TYPE_2 | SQ_LINK_TYPE_1,
													   ),
				),
			   );

	}//end getEventList()


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
				$form_section = $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);

				// reasonably critical upgrade - report any assets that didn't make it
				if (is_null($form_section)) {
					pre_echo('Could get asset #'.$assetid);
					continue;
				}

				// regenerate the bodycopy_container's content file
				$form_section_edit_fns = $form_section->getEditFns();
				if (!$form_section_edit_fns->generateContentFile($form_section)) {
					pre_echo('Could not generate content file for asset #'.$assetid);
				}

				unset($form_section_edit_fns);
				$GLOBALS['SQ_SYSTEM']->am->forgetAsset($form_section);
			}
			pre_echo($asset_name.' UPGRADE COMPLETE - FROM VERSION '.$current_version.' TO 0.6');

		}//end version 0.5 upgrade

		// attribute clean up following bugfix #2461 and #2462
		if (version_compare($current_version, '0.7', '<')) {
			$form_section_ids = $GLOBALS['SQ_SYSTEM']->am->getTypeAssetids($this->getAssetType(), FALSE);
			foreach ($form_section_ids as $id) {
				$form_section_ = $GLOBALS['SQ_SYSTEM']->am->getAsset($id);
				// remove questions that are not present in the sort order array
				$sort_order = $form_section_->attr('sort_order');
				$questions = $form_section_->attr('questions');
				$new_questions = Array();
				foreach ($sort_order as $shadowid) {
					$id_parts = explode(':q', $shadowid);
					$new_questions[$id_parts[1]] = $questions[$id_parts[1]];
				}
				$form_section_->setAttrValue('questions', $new_questions);
				$form_section_->saveAttributes();
			}
			pre_echo(count($form_section_ids).' FORM_SECTIONS UPDATED');
		}

		return TRUE;

	}//end _upgrade()



}//end class
?>
