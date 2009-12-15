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
* $Id: test_incomplete_attachments.inc,v 1.2 2009/12/15 00:16:59 bpearson Exp $
*
*/

/**
* Test_Incomplete_Attachments
*
*	A test to find incomplete attachments under a form
*
*
* @author  Benjamin Pearson <bpearson@squiz.net>
* @version $Revision: 1.2 $
* @package MySource_Matrix
*/
class Test_Incomplete_Attachments
{


	/**
	 * Return the name of this test
	 *
	 * @return string
	 * @access public
	 */
	public static function getName()
	{
		return 'Invalid Incomplete Attachments Test';

	}//end getName()


	/**
	 * Return a description of this test
	 *
	 * @return string
	 * @access public
	 */
	public static function getDescription()
	{
		return 'A test to find incomplete attachments under a form';

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
			$path  = $asset->data_path;
			$path .= '/incomplete_attachments';
			$files = list_dirs($path);
			foreach ($files as $file) {
				if (strpos($file, 's') === 0) {
					// This is an incomplete submission, so check if the submission is still valid
					$incomplete_submission_assetid = substr($file, 1);
					$incomplete_submission = $GLOBALS['SQ_SYSTEM']->am->getAsset($incomplete_submission_assetid, '', TRUE);
					if (is_null($incomplete_submission)) {
						$status = FALSE;
						$count++;
						$errors[] = 'Form #'.$assetid.' has an incomplete attachment for a non existant submission (#'.$incomplete_submission_assetid.')';
					} else {
						$GLOBALS['SQ_SYSTEM']->am->forgetAsset($incomplete_submission_asset, TRUE);
					}//end if
					unset($incomplete_submission);
				}//end if
			}//end foreach
			$GLOBALS['SQ_SYSTEM']->am->forgetAsset($asset, TRUE);
			unset($asset);
		}//end foreach

		if (!$status && !empty($count)) {
			$messages[] = 'There are '.$count.' incomplete attachments found on the system';
		}//end if

		return $status;

	}//end test()

}//end class
?>
