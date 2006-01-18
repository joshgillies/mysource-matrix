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
* $Id: form_submission_management.inc,v 1.15 2006/01/18 04:31:45 sdanis Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Form_Submission_Management
*
* @see Asset_Management
*
* @author  Marc McIntyre <mmcintyre@squiz.net>
* @version $Revision: 1.15 $
* @package MySource_Matrix_Packages
* @subpackage cms
*/
class Form_Submission_Management extends Asset_Management
{


	/**
	* Constructor
	*
	*/
	function Form_Submission_Management(&$pm)
	{
		$this->Asset_Management($pm);

		$this->vars = Array(
						'id'			=> Array(
											'added'		=> '0.0.1',
											'type'		=> 'text',
											'default'	=> '',
										   ),
						'attributes'	=> Array(
											'added'			=> '0.0.1',
											'type'			=> 'serialise',
											'default'		=> Array(),
											'description'	=> 'array with keys "answers" and "summary"',
										   ),
						'complete'		=> Array(
											'added'			=> '0.2',
											'type'			=> 'boolean',
											'default'		=> FALSE,
											'description'	=> 'A historical artifact; always true',
										   ),
						'xml'			=> Array(
											'added'		=> '0.2',
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
	* @access private
	*/
	function _upgrade($current_version)
	{
		if (!parent::_upgrade($current_version)) return FALSE;

		if (version_compare($current_version, '0.3.1', '<')) {

			bam('STARTING FORM SUBMISSION UPGRADE TO VERSION 0.3.1');

			// if they are had already upgraded to version 0.3 (which is where this upgrade should really have happened
			// then we need to just get the assets that are using the default attribute value
			if (version_compare($current_version, '0.3', '>=')) {
				$db =& $GLOBALS['SQ_SYSTEM']->db;
				$sql = 'SELECT a.assetid, a.type_code
						FROM (sq_ast_attr at INNER JOIN sq_ast a ON a.type_code = at.type_code)
								LEFT OUTER JOIN sq_ast_attr_val v ON at.attrid = v.attrid AND a.assetid = v.assetid
						WHERE at.type_code IN (
								SELECT type_code
								FROM sq_ast_typ_inhd
								WHERE inhd_type_code = '.$db->quote($this->getAssetType()).'
							)
						  AND at.name = '.$db->quote('complete').'
						  AND v.custom_val IS NULL
						';
				$assetids = $db->getAssoc($sql);
				assert_valid_db_result($assetids);
				unset($db);
			// otherwise we haven't been upgraged to 0.3, so let's just convert all the submissions
			} else {
				$assetids = $GLOBALS['SQ_SYSTEM']->am->getTypeAssetIds($this->getAssetType(), FALSE, TRUE);
			}

			$errors = FALSE;
			foreach ($assetids as $assetid => $type) {
				$asset =& $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid, $type);
				if (is_null($asset)) continue;

				if (!$GLOBALS['SQ_SYSTEM']->am->acquireLock($asset->id, 'attributes')) {
					trigger_error('Form Submission  Upgrade [to 0.3.1] - Unable to Acquire Lock on "'.$asset->name.'" (#'.$asset->id.')', E_USER_WARNING);
					$errors = TRUE;

				} else {
					$asset->setAttrValue('complete', 'true');
					if (!$asset->saveAttributes()) {
						trigger_error('Form Submission Upgrade [to 0.3.1] - Unable to set complete flag on "'.$asset->name.'" (#'.$asset->id.')', E_USER_WARNING);
						$errors = TRUE;
					}
					$GLOBALS['SQ_SYSTEM']->am->releaseLock($asset->id, 'attributes');

				}// end if

				$GLOBALS['SQ_SYSTEM']->am->forgetAsset($asset);
				unset($asset);

			}// end foreach

			bam('FORM SUBMISSION UPGRADE TO VERSION 0.3.1 COMPLETED '.(($errors) ? 'WITH ERRORS' : 'SUCCESSFULLY'));

		}//end if

		return TRUE;

	}//end _upgrade()


}//end class
?>
