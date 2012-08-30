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
* $Id: form_submission_management.inc,v 1.26 2012/08/30 00:58:08 ewang Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Form_Submission_Management
*
* @see Asset_Management
*
* @author  Marc McIntyre <mmcintyre@squiz.net>
* @version $Revision: 1.26 $
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
						'id'				=> Array(
												'added'		=> '0.0.1',
												'type'		=> 'text',
												'default'	=> '',
											   ),
						'attributes'		=> Array(
												'added'			=> '0.0.1',
												'type'			=> 'serialise',
												'default'		=> Array(),
												'description'	=> 'array with keys "answers", "summary", "extra_data"',
											   ),
						'complete'			=> Array(
												'added'			=> '0.2',
												'type'			=> 'boolean',
												'default'		=> FALSE,
												'description'	=> 'TRUE if submission complete, FALSE if incomplete and not yet submitted.',
											   ),
						'submitted'			=> Array(
												'added'			=> '0.8',
												'type'			=> 'datetime',
												'default'		=> '---------- --:--:--',
												'parameters'	=> Array(
																	'allow_null'	=> TRUE,
																	'show'			=> Array('d','m','y','h','i'),
																   ),
												'description'	=> 'The date the form submission was submitted to Matrix',
											   ),
						'captcha_status'	=> Array(
												'added'			=> '0.2',
												'type'			=> 'boolean',
												'default'		=> FALSE,
												'description'	=> 'TRUE if a CAPTCHA has been correctly submitted for this form so far.',
											   ),
						'xml'				=> Array(
												'added'		=> '0.2',
												'type'		=> 'text',
												'default'	=> '',
											   ),
						'current_page'		=> Array(
												'added'		=> '0.3',
												'type'		=> 'int',
												'default'	=> 0,
												'description'	=> 'The page the user last saw on this submission, updated with the current page when a user jumps back and forth through the form',
												'is_admin'	=> FALSE,
											   ),
						'latest_page'		=> Array(
												'added'		=> '0.3',
												'type'		=> 'int',
												'default'	=> 0,
												'description'	=> 'The last page the user saw - ie. the page furthest along the line that has been seen, irrespective of whether a user backtracked to a previous page',
												'is_admin'	=> FALSE,
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

		if (version_compare($current_version, '0.8', '<')) {
			echo('UPGRADING FORM SUBMISSION FROM '.$current_version."\n");
			// Upgrade the form submission
			// Form submission will now have a new attribute called submitted
			// This field will be the datetime the submission was completed
			// This corrects Bug #4000 -
			// The submission uses the created attribute on the asset,
			// which is inaccurate on multipage forms where the user can
			// save and return at another time. One shot forms are unaffected

			$count = 0;

			$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
			$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

			pre_echo('Updating Form Submissions...');

			// get all submission assets
			$submission_ids = $GLOBALS['SQ_SYSTEM']->am->getTypeAssetids('form_submission', FALSE, TRUE);

			// update them
			foreach ($submission_ids as $id => $type) {
				$submission = $GLOBALS['SQ_SYSTEM']->am->getAsset($id);
				$complete = $submission->attr('complete');
				$submitted = $submission->attr('submitted');

				// Set the default
				$date = $submission->created;

				// Only worry about null submitted dates and completed submissions
				if ($complete && $submitted == '---------- --:--:--') {
					// Try and find the form
					$answers = $submission->attr('attributes');
					if (!isset($answers['answers'])) {
						$form_id = 0;
					} else {
						// Default to nothing unless something is found
						$form_id = 0;

						$section_assetids = Array();
						foreach ($answers['answers'] as $assetid => &$data) {
							$assetid_bits = explode(':', $assetid, 2);
							$real_assetid = $assetid_bits[0];

							// Only use this asset ID if we haven't seen it before
							if (array_search($real_assetid, $section_assetids) === FALSE) {
								$section_assetids[] = $real_assetid;

								// check that the real asset ID actually exists before sending to getParents
								$real_asset_info = $GLOBALS['SQ_SYSTEM']->am->getAssetInfo(Array($real_assetid), 'asset', FALSE, 'name');
								if (empty($real_asset_info) === FALSE) {
									$form_id = key($GLOBALS['SQ_SYSTEM']->am->getParents($real_assetid, 'form', FALSE));
									break;
								}
							}
						}

					}//end if

					// No form found cannot proceed
					if (!empty($form_id)) {
						$form = $GLOBALS['SQ_SYSTEM']->am->getAsset($form_id, '');
						$sub_folder = $form->getSubmissionsFolder('submissions_folder');

						// On upgrading, the best we can do is look for update time for the link.
						// Failing that, we will just have to use the created date again.
						$sql = 'SELECT updated FROM sq_ast_lnk WHERE link_type=:type AND minorid=:minorid AND majorid=:majorid';
						try {
							$query = MatrixDAL::preparePdoQuery($sql);
							MatrixDAL::bindValueToPdo($query, 'type', SQ_LINK_TYPE_3);
							MatrixDAL::bindValueToPdo($query, 'minorid', $submission->id);
							MatrixDAL::bindValueToPdo($query, 'majorid', $sub_folder->id);
							$result = MatrixDAL::executePdoOne($query);
						} catch (Exception $e) {
							$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
							$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();

							pre_echo('UPGRADE FAILED - Database Error: '.$e->getMessage());
							return FALSE;
						}

						if (!empty($result)) {
							$date = strtotime($result);
						}//end if
						// Point the POST var to the current submission id
						$_POST['SQ_FORM_'.$form->id.'_SUBMISSION'] = $submission->id;
						$submission->setAttrValue('xml', $form->getXML($date));
					}//end if

					// Save the date
					$GLOBALS['SQ_SYSTEM']->setRunLevel(SQ_RUN_LEVEL_FORCED);
					$submission->setAttrValue('submitted', ts_iso8601($date));
					$count++;
					if (!$submission->saveAttributes()) {
						$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
						$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();

						pre_echo('UPGRADE FAILED - Could not save attributes');
						return FALSE;
					}//end if
					$GLOBALS['SQ_SYSTEM']->restoreRunLevel();
				}//end if

				$GLOBALS['SQ_SYSTEM']->am->forgetAsset($submission, TRUE);
				unset($submission);
			}//end foreach

			pre_echo('Updated - '.$count.' form submission(s)...');

			$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
			$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();

			pre_echo('FORM SUBMISSIONS SUCCESSFULLY UPGRADED TO VERSION 0.8');
		}//end if
		return TRUE;

	}//end _upgrade()


}//end class
?>
