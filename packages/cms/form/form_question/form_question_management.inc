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
* $Id: form_question_management.inc,v 1.17 2005/05/20 05:27:31 tbarrett Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Form_Question_Management
*
* @see Asset_Management
*
* @author  Marc McIntyre <mmcintyre@squiz.net>
* @version $Revision: 1.17 $
* @package MySource_Matrix_Packages
* @subpackage cms
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
						'name'			=> Array(
											'added'		=> '0.0.1',
											'type'		=> 'text',
											'default'	=> '',
										   ),
						'note'			=> Array(
											'added'		=> '0.0.1',
											'type'		=> 'text',
											'default'	=> '',
										   ),
						'rules'			=> Array(
											'added'		=> '0.0.1',
											'type'		=> 'serialise',
											'default'	=> Array(),
										   ),
						'is_required'	=> Array(
											'added'		=> '0.3',
											'type'		=> 'boolean',
											'default'	=> '0',
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
