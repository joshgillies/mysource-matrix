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
* $Id: form_question_management.inc,v 1.2 2005/10/13 03:51:37 sdanis Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Form_Question_Management
*
* @see Asset_Management
*
* @author  Marc McIntyre <mmcintyre@squiz.net>
* @version $Revision: 1.2 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Form_Question_Management extends Asset_Management
{


	/**
	* Constructor
	*
	*/
	function Form_Question_Management(&$pm)
	{
		$this->Asset_Management($pm);

		$this->vars = Array(
						'name'					=> Array(
													'added'		=> '0.0.1',
													'type'		=> 'text',
													'default'	=> '',
												   ),
						'note'					=> Array(
													'added'		=> '0.0.1',
													'type'		=> 'text',
													'default'	=> '',
												   ),
						'rules'					=> Array(
													'added'		=> '0.0.1',
													'type'		=> 'serialise',
													'default'	=> Array(),
												   ),
						'is_required'			=> Array(
													'added'		=> '0.3',
													'type'		=> 'boolean',
													'default'	=> '0',
												   ),
						'cust_required_error'	=> Array(
													'added'		=> '0.5',
													'type'		=> 'text',
													'default'	=> '',
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
		// first do things as normal
		$res = parent::_upgrade($current_version);

		if (version_compare($current_version, '0.3', '<')) {
			bam('UPGRADING FORM QUESTION FROM v < 0.3');
			// get all question assets and fill in their is_required attr from their required attr if necessary
			$question_assets = $GLOBALS['SQ_SYSTEM']->am->getTypeAssetids('form_question', false, true);
			$irrelevant_types = Array();
			foreach ($question_assets as $id => $type) {
				if (!isset($irrelevant_types[$type])) {
					$q =& $GLOBALS['SQ_SYSTEM']->am->getAsset($id, $type);
					if (!isset($q->vars['required'])) {
						$irrelevant_types[$type] = 1;
						continue;
					}
					$q->setAttrValue('is_required', $q->attr('required'));
					$q->saveAttributes();
				}
			}
		}
		return true;

	}//end _upgrade()


}//end class
?>
