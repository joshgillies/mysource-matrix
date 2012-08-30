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
* $Id: test_submission_integrity.inc,v 1.3 2012/08/30 01:04:53 ewang Exp $
*
*/

/**
* Test_Submission_Integrity
*
*	A test to check the integrity of submissions (answers match questions on the system)
*
*
* @author  Benjamin Pearson <bpearson@squiz.net>
* @version $Revision: 1.3 $
* @package MySource_Matrix
*/
class Test_Submission_Integrity
{


	/**
	 * Return the name of this test
	 *
	 * @return string
	 * @access public
	 */
	public static function getName()
	{
		return 'Submission Integrity Test';

	}//end getName()


	/**
	 * Return a description of this test
	 *
	 * @return string
	 * @access public
	 */
	public static function getDescription()
	{
		return 'A test to check the integrity of submissions';

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
		$submission_assetids = $GLOBALS['SQ_SYSTEM']->am->getTypeAssetIds('form_submission', TRUE);
		foreach ($submission_assetids as $assetid) {
			$asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);

			// Check questions are valid
			$attributes = $asset->attr('attributes');
			$answers = array_get_index($attributes, 'answers', Array());
			foreach ($answers as $id => $details) {
				$valid_asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($id, '', TRUE);
				if (is_null($valid_asset)) {
					$status = FALSE;
					$count++;
					$errors[] = 'Submission #'.$assetid.' has an answer to a question (#'.$id.') that does not exist';
				} else {
					$GLOBALS['SQ_SYSTEM']->am->forgetAsset($valid_asset, TRUE);
				}//end if
				unset($valid_asset);
			}//end foreach

			$GLOBALS['SQ_SYSTEM']->am->forgetAsset($asset, TRUE);
		}//end foreach

		if (!$status && !empty($count)) {
			$messages[] = 'There are '.$count.' submissions found with problems';
		}//end if

		return $status;

	}//end test()

}//end class
?>
