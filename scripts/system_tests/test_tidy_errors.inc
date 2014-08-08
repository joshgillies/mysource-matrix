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
* $Id: test_tidy_errors.inc,v 1.3 2012/08/30 01:04:53 ewang Exp $
*
*/

/**
* Test_Tidy_Errors
*
*	A test to run to find HTML Tidy fails in wysiwyg content areas
*
*
* @author  Benjamin Pearson <bpearson@squiz.net>
* @version $Revision: 1.3 $
* @package MySource_Matrix
*/
class Test_Tidy_Errors
{


	/**
	 * Return the name of this test
	 *
	 * @return string
	 * @access public
	 */
	public static function getName()
	{
		return 'HTML Tidy Errors Test';

	}//end getName()


	/**
	 * Return a description of this test
	 *
	 * @return string
	 * @access public
	 */
	public static function getDescription()
	{
		return 'A test to find HTML Tidy fails in WYSIWYG content areas';

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
		$am = $GLOBALS['SQ_SYSTEM']->am;
		$wysiwygs = $am->getTypeAssetids('content_type_wysiwyg');

		foreach ($wysiwygs as $wysiwyg) {
			$asset = $am->getAsset($wysiwyg, '', TRUE);
			if (is_null($asset)) {
				$status = FALSE;
				$count++;
				$errors[] = 'Asset #'.$wysiwyg.' could not be loaded';
			} else {
				$tidyStatus = $asset->attr('htmltidy_status');
				if ($tidyStatus != 'pass') {
					$status = FALSE;
					$count++;
					$errors[] = 'HTML Tidy has not validated Asset #'.$wysiwyg;
				}//end if
				$am->forgetAsset($asset, TRUE);
			}//end if
		}//end foreach

		if (!$status && !empty($count)) {
			$messages[] = 'There are '.$count.' content types with HTML Tidy issues';
		}//end if

		return $status;

	}//end test()

}//end class
?>
