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
* $Id: test_form_integrity.inc,v 1.4 2012/08/30 01:04:53 ewang Exp $
*
*/

/**
* Test_Form_Integrity
*
*	A test to check the integrity of a form (valid questions/sections, submissions completed, valid form actions)
*
*
* @author  Benjamin Pearson <bpearson@squiz.net>
* @version $Revision: 1.4 $
* @package MySource_Matrix
*/
class Test_Form_Integrity
{


	/**
	 * Return the name of this test
	 *
	 * @return string
	 * @access public
	 */
	public static function getName()
	{
		return 'Form Integrity Test';

	}//end getName()


	/**
	 * Return a description of this test
	 *
	 * @return string
	 * @access public
	 */
	public static function getDescription()
	{
		return 'A test to check the integrity of forms';

	}//end getDescription()


	/**
	 * The test to run
	 *
	 * @param array	$messages	The variable to 'chuck'	all the messages
	 * @param array	$errors		The variable to 'chuck'	all the verbose messages
	 *
	 * @return boolean|int
	 * @access public
	 */
	public static function test(&$messages, &$errors)
	{
		$status = TRUE;
		$count = 0;
		$form_assetids = $GLOBALS['SQ_SYSTEM']->am->getTypeAssetIds('form', FALSE);
		foreach ($form_assetids as $assetid) {
			$asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
			$invalid = FALSE;

			// Check sections/questions are valid
			$questions = $asset->getAllQuestions();
			foreach ($questions as $question_id => $question_details) {
				$question = $GLOBALS['SQ_SYSTEM']->am->getAsset($question_id, '', TRUE);
				if (is_null($question)) {
					$status = FALSE;
					$invalid = TRUE;
					$errors[] = 'Question #'.$question_id.' on Form #'.$assetid.' is invalid';
				} else {
					$GLOBALS['SQ_SYSTEM']->am->forgetAsset($question);
				}//end if
				unset($question);
			}//end foreach

			// Check submissions are completed (ECommerce Form does have a submission folder so ignore)
			if (strtolower($asset->type()) !== 'form_ecommerce') {
				$submission_folder = $asset->getSubmissionsFolder();
				if (is_null($submission_folder)) {
					$status = FALSE;
					$invalid = TRUE;
					$errors[] = 'Submission Folder does not exist for Form #'.$assetid;
				} else {
					$submissions_found = $GLOBALS['SQ_SYSTEM']->am->getChildren($submission_folder->id, 'form_submission', FALSE);
					foreach ($submissions_found as $sub_id => $sub_type_code) {
						$sub = $GLOBALS['SQ_SYSTEM']->am->getAsset($sub_id, '', TRUE);
						if (is_null($sub)) {
							$status = FALSE;
							$invalid = TRUE;
							$errors[] = 'Submission #'.$sub_id.' under Form #'.$assetid.' and could not be loaded';
						} else {
							$sub_complete = $sub->attr('complete');
							if (!$sub_complete) {
								$status = FALSE;
								$invalid = TRUE;
								$errors[] = 'Submission #'.$sub_id.' under Form #'.$assetid.' and not completed';
							}//end if
							$GLOBALS['SQ_SYSTEM']->am->forgetAsset($sub, TRUE);
						}//end if
					}//end foreach
					$GLOBALS['SQ_SYSTEM']->am->forgetAsset($submission_folder, TRUE);
				}//end if
			}//end if

			// Valid form actions
			$actions = $asset->attr('actions');
			foreach ($actions as $action) {
				$action_type = $action['type_code'];
				$GLOBALS['SQ_SYSTEM']->am->includeAsset($action_type);

				if (!$action['active'] && !call_user_func(Array($action_type, 'isValid'), $asset, $action['settings'])) {
					$status = FALSE;
					$invalid = TRUE;
					$errors[] = 'Form #'.$assetid.' has an invalid form action ('.$action_type.')';
				}//end if
			}//end foreach

			// Ok, this form has issues
			if ($invalid) $count++;

			$GLOBALS['SQ_SYSTEM']->am->forgetAsset($asset, TRUE);
		}//end foreach

		if (!$status) {
			$messages[] = 'There are '.$count.' forms found on the system with problems';
		}//end if

		return $status;

	}//end test()

}//end class
?>
